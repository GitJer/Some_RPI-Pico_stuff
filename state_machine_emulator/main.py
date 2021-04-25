import sys
from interface import RPI_PICO_PIO_interface
import rp2040
import emulation

"""
This code emulates a state machine in a PIO of a RP4020
It provides a GUI to step through the emulation results.
"""


def process_file_pio_h(filename):
    """ read and parse a pioasm generated header file """
    # TODO: public, .word
    pio_program = list()
    with open(filename, 'r') as pio_file:
        line = pio_file.readline()
        while line:
            if line[0] == '/' and line[1] == '/':
                pass
            elif ".length" in line:
                d = line.strip().split('=')
                pio_program_length = int(d[1].replace(',', ''))
            elif ".origin" in line:
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
                # self.program_definitions['pio_program'] = pio_program
            elif "static inline pio_sm_config" in line:
                # add to the c-program at t=0
                line2 = pio_file.readline()
                # print("line2=", line2)
                while '}' not in line2:
                    if 'sm_config_set_sideset' in line2:
                        parts = line2.split(',')
                        # print("parts=", parts)
                        a1 = int(parts[1].strip())
                        c_program.append([0, 'sideset_count', a1])
                        a2 = parts[2].strip().lower() == 'true'
                        c_program.append([0, 'sideset_opt', a2])
                        a3 = parts[3].split(')')[0].strip().lower() == 'true'
                        c_program.append([0, 'sideset_pindirs', a3])
                    line2 = pio_file.readline()
            line = pio_file.readline()
    if len(pio_program) != pio_program_length:
        print("error: program lengths not matching, continuing anyway")
    if len(pio_program) > 32:
        print("error: program too long, continuing anyway")
    return pio_program, pio_program_length, pio_program_origin, pio_program_wrap_target, pio_program_wrap


def process_file_pin_program(filename):
    """ read the pin program file and parse it """
    pin_program = list()
    with open(filename, 'r') as pin_program_file:
        line = pin_program_file.readline()
        while line:
            # only process lines without comments
            if '#' not in line:
                parts = line.split(',')
                parts[0] = int(parts[0])
                parts[2] = int(parts[2])
                pin_program.append(parts)
            line = pin_program_file.readline()
    return pin_program


def process_file_c_program(filename, c_program):
    """ read the c program file and parse it """
    with open(filename, 'r') as c_program_file:
        line = c_program_file.readline()
        while line:
            # only process lines without comments
            if '#' not in line:
                parts = line.strip().split(',')
                for index, p in enumerate(parts):
                    if index != 1:
                        parts[index] = int(parts[index])
                    else:
                        parts[index] = parts[index].strip()
                c_program.append(parts)
            line = c_program_file.readline()
    # return c_program


if __name__ == "__main__":

    # read the arguments. Should be three files: .pio.h pin-program c-program
    if len(sys.argv) == 4:
        pio_h_filename = sys.argv[1]
        pin_program_filename = sys.argv[2]
        c_program_filename = sys.argv[3]
    else:
        print("Usage: python", sys.argv[0], "file.pio.h pin_program c_program")
        exit()

    # flag to indicate that files need to be (re)loaded. This is used when the user pushes the reload button in the GUI
    load_files = True
    while load_files:
        load_files = False
        # TODO: more than one program can be defined!
        # first define the c_program list because in the pio_program there may already be c_statements
        c_program = list()
        pio_program, pio_program_length, pio_program_origin, pio_program_wrap_target, pio_program_wrap = process_file_pio_h(
            pio_h_filename)
        process_file_c_program(c_program_filename, c_program)
        pin_program = process_file_pin_program(pin_program_filename)

        program_definitions = dict()
        program_definitions['pio_program'] = pio_program
        program_definitions['pio_program_length'] = pio_program_length
        program_definitions['pio_program_origin'] = pio_program_origin
        program_definitions['pio_program_wrap_target'] = pio_program_wrap_target
        program_definitions['pio_program_wrap'] = pio_program_wrap

        # make the RP2040 emulation (and it will make the PIO and sm)
        my_rp2040 = rp2040.rp2040(program_definitions)
        my_emulation = emulation.emulation(my_rp2040, pin_program, c_program)

        print("Emulating ... ", end="")
        my_emulation.emulate(500)
        print("finished")
        my_RPI_PICO_PIO_interface = RPI_PICO_PIO_interface(
            pio_program, pin_program, c_program, my_emulation.output, my_emulation.emulation_output_c_program)
        load_files = my_RPI_PICO_PIO_interface.get_reload_flag()
