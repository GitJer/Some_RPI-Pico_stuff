"""
TODO:
- check the 'status' with 'set_N' and 'status_sel' with the actual hardware!
    Wait! not just the status, but many, many things should be tested with the pico HW
- "FIFO IRQs" Figure 38. in rp2040-datasheet.pdf
- start without c-program or pin-program
- try except around emulation
- why does the stepper example (with unnecessary sideset) cause the sideset output to toggle?
"""

from sys import argv, exc_info
from os import getcwd, path, chdir
from glob import glob

from interface import Emulator_Interface
from emulation import emulation
from config import EMULATION_STEPS
from state_machine import state_machine

"""
This code emulates a state machine of a RP4020
It provides a GUI to step through the emulation results.
"""

def process_file_pio_h(filename, c_program):
    """ read and parse a pioasm generated header file """
    pio_program = list()
    pio_program_length = None
    pio_program_origin = -1 # note: not actually used
    pio_program_wrap_target = 0
    pio_program_wrap = None

    try:
        with open(filename, 'r') as pio_file:
            line = pio_file.readline()
            while line:
                if line[0] == '/' and line[1] == '/':
                    pass
                elif ".length" in line:
                    d = line.strip().split('=')
                    pio_program_length = int(d[1].replace(',', ''))
                elif ".origin" in line: # note: origin is not actually used
                    d = line.strip().split('=')
                    pio_program_origin = int(d[1].replace(',', ''))
                elif "#define" in line:
                    if "wrap_target" in line:
                        d = line.strip().split(' ')
                        pio_program_wrap_target = int(d[2])
                    elif "wrap" in line:
                        d = line.strip().split(' ')
                        pio_program_wrap = int(d[2])
                elif "static const uint16_t" in line:
                    line = pio_file.readline()
                    while '};' not in line:
                        d = line.strip().split(', //')
                        if len(d) == 2:
                            pio_program.append(d)
                        line = pio_file.readline()
                elif "static inline pio_sm_config" in line:
                    # add to the c-program at t=0
                    line2 = pio_file.readline()
                    while '}' not in line2:
                        if 'sm_config_set_sideset' in line2:
                            parts = line2.split(',')
                            a1 = int(parts[1].strip())
                            c_program.append([0, 'sideset_count', a1])
                            a2 = parts[2].strip().lower() == 'true'
                            c_program.append([0, 'sideset_opt', a2])
                            a3 = parts[3].split(')')[0].strip().lower() == 'true'
                            c_program.append([0, 'sideset_pindirs', a3])
                        line2 = pio_file.readline()
                line = pio_file.readline()
        if len(pio_program) != pio_program_length:
            print("Warning: length specification in pio file doesn't match actual length, continuing anyway")
        if len(pio_program) > 32:
            print("Warning: program too long, continuing anyway")
    except IOError as e:
        print("I/O Error reading pio program file:", e.errno, e.strerror)
    except:
        print("Error reading pio program file:", exc_info()[0])
    return pio_program, pio_program_length, pio_program_origin, pio_program_wrap_target, pio_program_wrap


def process_file_pin_program(filename, pin_program):
    """ read the pin program file and parse it """
    try:
        allowed_parts1 = ['GPIO'+str(i) for i in range(32)]
        allowed_parts1.append('all')
        with open(filename, 'r') as pin_program_file:
            for line in pin_program_file:
                # remove all # characters and all text after it (i.e. comments)
                line = line.split("#", 1)[0]
                # check if the line still has some content
                if line.strip():
                    parts = line.split(',')
                    parts[1] = parts[1].strip()
                    if parts[1] in allowed_parts1:
                        parts[0] = int(parts[0])   # time at which the command should be run
                        parts[2] = int(parts[2])   # argument of the command
                        pin_program.append(parts)
                    else:
                        print("Warning: Unknown command in pin_program: ", parts, 'continuing anyway')
    except IOError as e:
        print("I/O Error reading pin program file:", e.errno, e.strerror)
    except:
        print("Error reading pin program file:", exc_info()[0])


def process_file_c_program(filename, c_program):
    """ read the c program file and parse it """
    try:
        with open(filename, 'r') as c_program_file:
            for line in c_program_file:
                # remove all # characters and all text after it (i.e. comments)
                line = line.split("#", 1)[0]
                # check if the line still has some content
                if line.strip():
                    # a "c-instruction" has two or three parts:
                    # timestamp, command, and possibly an argument of the command
                    # the timestamp and argument must be integers/bool, the command is a (stripped) string
                    parts = line.strip().split(',')
                    parts[1] = parts[1].strip()
                    # check if the command is valid 
                    if parts[1].strip() in ['put', 'get', 'set_base', 'set_count', 'in_base', 'jmp_pin', 'sideset_base', 'sideset_count', 'sideset_opt', 'sideset_pindirs', 'out_base', 'out_count', 'out_shift_right', 'out_shift_autopull', 'pull_threshold', 'in_shift_right', 'in_shift_autopush', 'push_threshold', 'get_pc', 'set_pc', 'irq', 'set_N', 'status_sel', 'dir_out', 'dir_in', 'dir_non']:
                        parts[0] = int(parts[0])
                        parts[1] = parts[1]
                        if len(parts) == 3:
                            parts[2] = parts[2].strip()
                            # convert strings "True" and "False" to boolean, otherwise it is an int
                            if parts[2] == "True":
                                parts[2] = True
                            elif parts[2] == "False":
                                parts[2] = False
                            else:
                                parts[2] = int(parts[2])
                        c_program.append(parts)
                    else:
                        print("Warning: Unknown command in c_program: ", parts, "continuing anyway")
    except IOError as e:
        print("I/O Error reading c program file:", e.errno, e.strerror)
    except:
        print("Error reading c program file:", exc_info()[0])

def print_usage():
    """ print the usage of this program """
    print("Usage:")
    print("python3", argv[0], "file.pio.h pin_program c_program")
    print("or:")
    print("python3", argv[0], "directory_of_files")
    exit()


if __name__ == "__main__":

    # process arguments
    if len(argv) == 4:
        # read the three arguments: .pio.h pin-program c-program
        pio_h_filename = argv[1]
        pin_program_filename = argv[2]
        c_program_filename = argv[3]
    elif len(argv) == 2:
        # read one argument: the directory with the files .pio.h pin-program c-program
        dir_of_files = argv[1]
        # if the first character isn't a '/', add the full path
        if dir_of_files[0] != '/':
            dirname=getcwd()
            dir_of_files = path.join(dirname, dir_of_files)
        # if the last character isn't a '/', add it
        if dir_of_files[-1] != '/':
            dir_of_files += '/'
        chdir(dir_of_files)
        pio_file = glob("*.pio.h")
        pin_file = glob("pin_program")
        c_file = glob("c_program")
        if (len(pio_file)==1 and len(pin_file)==1 and len(c_file)==1):
            pio_h_filename = dir_of_files + pio_file[0]
            pin_program_filename = dir_of_files + pin_file[0]
            c_program_filename = dir_of_files + c_file[0]
        else:
            print_usage()
    else:
        print_usage()
        
    # flag to indicate that files need to be (re)loaded. This is used when the user pushes the reload button in the GUI
    load_files = True
    while load_files:
        load_files = False

        # the pio program (with associated data) is a dict
        program_definitions = dict()
        # the c_program and pin_program are lists
        c_program = list()
        pin_program = list()

        # process the pio.h file (which may already contribute to the c_program)
        pio_program, pio_program_length, pio_program_origin, pio_program_wrap_target, pio_program_wrap = process_file_pio_h(pio_h_filename, c_program)
        program_definitions['pio_program'] = pio_program
        program_definitions['pio_program_length'] = pio_program_length
        program_definitions['pio_program_origin'] = pio_program_origin  # note: not used
        program_definitions['pio_program_wrap_target'] = pio_program_wrap_target
        program_definitions['pio_program_wrap'] = pio_program_wrap
        # process the c_program
        process_file_c_program(c_program_filename, c_program)
        # process the pin_program
        process_file_pin_program(pin_program_filename, pin_program)

        # make the RP2040 emulation (and it will make the PIO and sm)
        my_state_machine = state_machine(program_definitions)
        my_emulation = emulation(my_state_machine, pin_program, c_program)

        # do the emulation
        my_emulation.emulate(EMULATION_STEPS)
        
        # show the interface
        my_Emulator_Interface = Emulator_Interface(program_definitions, pin_program, c_program, my_emulation.output, my_emulation.emulation_output_c_program)

        # check if a reload was requested
        load_files = my_Emulator_Interface.get_reload_flag()
