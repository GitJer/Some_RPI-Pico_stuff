from tkinter import *
from copy import deepcopy


class emulation:
    """ This class controls the emulation of the sm of a RP2040 """

    def __init__(self, rp2040, pin_program, c_program):
        """ init makes the list with the emulation results (output) and
            the variables needed for highlights in the GUI
        """
        # the emulator for the RP2040:
        self.rp2040 = rp2040
        # the (user) changes made to the pins:
        self.pin_program = pin_program
        # the user c-program that influences the PIO and sm
        self.c_program = c_program
        # the output the emulator produces based on the c_program
        self.output = []
        # lines to be highlighted in the GUI parts for e.g. the c-statements and pin-settings
        self.emulation_highligh_pin_program = []
        self.emulation_highligh_c_program = []
        self.emulation_output_c_program = []
        self.emulation_highlight_output_c_program = []

    def emulate(self, number_of_steps):
        """ emulate a number of steps """
        for step in range(number_of_steps):
            # first execute the pin and c_program statements
            self.execute_pin_and_c_program()
            # then run the PIO code in the emulator
            self.rp2040.time_step()
            # copy the current state and append it to the list with all data
            self.output.append((deepcopy(self.rp2040.GPIO),
                                deepcopy(self.rp2040.GPIO_pindirs),
                                deepcopy(self.rp2040.PIO[0].sm[0].vars),
                                deepcopy(self.rp2040.PIO[0].sm[0].settings),
                                deepcopy(self.rp2040.PIO[0].sm_irq),
                                deepcopy(self.emulation_highligh_pin_program),
                                deepcopy(self.emulation_highligh_c_program),
                                deepcopy(
                                    self.emulation_highlight_output_c_program),
                                ))
            # start with a clean list of to-be-highlighted parts
            self.emulation_highligh_pin_program = []
            self.emulation_highligh_c_program = []
            self.emulation_highlight_output_c_program = []

    def bit_string(self, value):
        """ function to produce a string with the binary representation of a value """
        bit_string = ""
        for i in reversed(range(32)):
            if (value & (1 << i)) == 0:
                bit_string += "0"
            else:
                bit_string += "1"
        return bit_string

    def execute_pin_and_c_program(self):
        """ executes the c-program and pin settings as specified by the c- and pin-program """
        # get the current time from the rp2040
        time = self.rp2040.clock

        # set all the GPIO's according to the pin states
        for index, g in enumerate(self.pin_program):
            if g[0] == time:  # TODO: wastfull to go through the whole list. Keep an index or something
                # all pin_statements for this time step are to be highlighted
                self.emulation_highligh_pin_program.append(index)
                # handle the GPIO and 'all' statements
                if 'GPIO' in g[1]:
                    gpio = int(g[1].replace('GPIO', ''))
                    self.rp2040.GPIO[gpio] = int(g[2])
                elif 'all' in g[1]:
                    for gpio in range(32):
                        self.rp2040.GPIO[gpio] = int(g[2])

        # set c_program settings
        # go through the c-program, and at the right time, execute the statements
        for index, c in enumerate(self.c_program):
            if c[0] == time:  # TODO: wastfull to go through the whole list. Keep an index or something
                # all c_statements for this time step are to be highlighted in the GUI
                self.emulation_highligh_c_program.append(index)

                # just a shortcut TODO: now only pio0 and sm0, but there are 2*PIO + 4*sm per PIO
                sm = self.rp2040.PIO[0].sm[0]
                # handle all possible c_statements
                if c[1] == 'put':
                    # place a value in the Tx FIFO
                    if sm.vars["TxFIFO_count"] < 3:
                        sm.vars["TxFIFO"][sm.vars["TxFIFO_count"]] = c[2]
                        sm.vars["TxFIFO_count"] += 1
                elif c[1] == 'get':
                    # get a value from the Rx FIFO, and put it in output
                    if sm.vars["RxFIFO_count"] > 0:
                        self.emulation_highlight_output_c_program.append(
                            len(self.emulation_output_c_program))
                        self.emulation_output_c_program.append(
                            str(time) + " : " + str(sm.vars["RxFIFO"][0]) + " = " + self.bit_string(sm.vars["RxFIFO"][0]))
                        # shift FIFO entries 1 to 4 back one place
                        for i in range(0, 3):
                            sm.vars["RxFIFO"][i] = sm.vars["RxFIFO"][i+1]
                        # set the last entry to 0 (this may not happen in reality!)
                        sm.vars["RxFIFO"][3] = 0
                        # there is now one less item in the Rx FIFO
                        sm.vars["RxFIFO_count"] -= 1
                elif c[1] == 'set_base':
                    sm.settings["set_base"] = c[2]
                elif c[1] == 'set_count':
                    sm.settings["set_count"] = c[2]
                elif c[1] == 'in_base':
                    sm.settings["in_base"] = c[2]
                elif c[1] == 'jmp_pin':
                    sm.settings["jmp_pin"] = c[2]
                elif c[1] == 'sideset_base':
                    sm.settings["sideset_base"] = c[2]
                elif c[1] == 'sideset_count':
                    sm.settings["sideset_count"] = c[2]
                elif c[1] == 'sideset_opt':
                    sm.settings["sideset_opt"] = c[2]
                elif c[1] == 'sideset_pindirs':
                    sm.settings["sideset_pindirs"] = c[2]
                elif c[1] == 'out_base':
                    sm.settings["out_base"] = c[2]
                elif c[1] == 'out_count':
                    sm.settings["out_count"] = c[2]
                elif c[1] == 'out_shift_right':
                    sm.settings["out_shift_right"] = c[2]
                elif c[1] == 'out_shift_autopull':
                    sm.settings["out_shift_autopull"] = c[2]
                elif c[1] == 'pull_threshold':
                    sm.settings["pull_threshold"] = c[2]
                elif c[1] == 'in_shift_right':
                    sm.settings["in_shift_right"] = c[2]
                elif c[1] == 'in_shift_autopush':
                    sm.settings["in_shift_autopush"] = c[2]
                elif c[1] == 'push_threshold':
                    sm.settings["push_threshold"] = c[2]
                elif c[1] == 'get_pc':
                    self.emulation_highlight_output_c_program.append(
                        len(self.emulation_output_c_program))
                    self.emulation_output_c_program.append(str(
                        time) + " : " + "pc=" + str(self.rp2040.PIO[0].sm[0].vars["pc"]))  # TODO: only support pio0 and sm0
                else:
                    print("Error: unknown c-program statement, continuing")
