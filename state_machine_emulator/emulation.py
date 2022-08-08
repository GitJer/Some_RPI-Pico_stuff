from copy import deepcopy


class emulation:
    """ This class controls the emulation of the sm of a RP2040 """

    def __init__(self, state_machine, pin_program, c_program):
        """ init makes the list with the emulation results (output) and
            the variables needed for highlights in the GUI
        """
        # the emulator for the RP2040:
        self.state_machine = state_machine
        # the (user) changes made to the pins:
        self.pin_program = pin_program
        # the user c-program that influences the PIO and sm
        self.c_program = c_program
        # the output the emulator produces based on the c_program; this is used for the GUI
        self.output = []
        # lines to be highlighted in the GUI parts for e.g. the c-statements and pin-settings
        self.emulation_highlight_pin_program = []
        self.emulation_highlight_c_program = []
        self.emulation_output_c_program = []
        self.emulation_highlight_output_c_program = []

    def emulate(self, number_of_steps):
        """ emulate a number of steps """
        for step in range(number_of_steps):
            # prepare for warning messages
            self.warning_messages = []
            # first execute the pin and c_program statements
            warnings = self.execute_pin_and_c_program()
            if warnings:
                self.warning_messages.extend(warnings)
            # then run the PIO code in the emulator
            warnings = self.state_machine.time_step()
            if warnings:
                self.warning_messages.extend(warnings)
            # copy the current state and append it to the list with all data
            self.output.append((deepcopy(self.state_machine.GPIO_data),
                                deepcopy(self.state_machine.vars),
                                deepcopy(self.state_machine.settings),
                                deepcopy(self.state_machine.sm_irq),
                                deepcopy(self.emulation_highlight_pin_program),
                                deepcopy(self.emulation_highlight_c_program),
                                deepcopy(self.emulation_highlight_output_c_program),
                                deepcopy(self.warning_messages)
                                ))
            # start with a clean list of to-be-highlighted parts
            self.emulation_highlight_pin_program = []
            self.emulation_highlight_c_program = []
            self.emulation_highlight_output_c_program = []


    def bit_string(self, value):
        """ function to produce a string with the binary representation of a value """
        return str().join(["0" if (value & (1 << i)) == 0 else "1" for i in reversed(range(32))])


    def execute_pin_and_c_program(self):
        """ executes the c-program and pin settings as specified by the c- and pin-program """
        # get the current time from the state_machine (= system level clock)
        time = self.state_machine.clock
        # warnings
        warning_messages = []

        # set all the GPIO's according to the pin states
        for index, g in enumerate(self.pin_program):
            if g[0] == time:  # TODO? wasteful to go through the whole list each time. Keep an index or something (but then again, 500 steps takes almost no time ... so why bother)
                # all pin_statements for this time step are to be highlighted
                self.emulation_highlight_pin_program.append(index)
                # handle the GPIO and 'all' statements
                if 'GPIO' in g[1]:
                    gpio = int(g[1].replace('GPIO', ''))
                    self.state_machine.GPIO_data["GPIO_external"][gpio] = int(g[2])
                    # also set the real GPIO since external always wins!
                    self.state_machine.GPIO_data["GPIO"][gpio] = int(g[2])
                elif 'all' in g[1]:
                    for gpio in range(32):
                        self.state_machine.GPIO_data["GPIO_external"][gpio] = int(g[2])
                        # also set the real GPIO since external always wins!
                        self.state_machine.GPIO_data["GPIO"][gpio] = int(g[2])
                else:
                    # should already have been filtered out when parsing the file, but anyway:
                    warning_messages.append("Warning: unknown pin_program statement"+str(g[0])+str(g[1])+str(g[2])+"\n")

        # set c_program settings
        # go through the c-program, and at the right time, execute the statements
        for index, c in enumerate(self.c_program):
            if c[0] == time:  # TODO? wasteful to go through the whole list each time. Keep an index or something (but then again, 500 steps takes almost no time ... so why bother)
                # all c_statements for this time step are to be highlighted in the GUI
                self.emulation_highlight_c_program.append(index)
                # handle all possible c_statements
                if c[1] == 'put':
                    # place a value in the Tx FIFO
                    if self.state_machine.vars["TxFIFO_count"] < 4:
                        # there is still room in the TxFIFO: add the value c[2] and increase the number of items in the TxFIFO
                        self.state_machine.vars["TxFIFO"][self.state_machine.vars["TxFIFO_count"]] = c[2]
                        self.state_machine.vars["TxFIFO_count"] += 1
                        # make sure the 'status' is correct
                        if self.state_machine.settings['status_sel'] == 0:
                            if self.state_machine.vars["TxFIFO_count"] < self.state_machine.settings['FIFO_level_N']:
                                self.state_machine.vars["status"] = 0xFFFFFFFF; # binary all ones 
                            else:
                                self.state_machine.vars["status"] = 0;  # binary all zeroes

                elif c[1] == 'get':
                    # get a value from the Rx FIFO, and put it in output
                    if self.state_machine.vars["RxFIFO_count"] > 0:
                        # there are items in RxFIFO
                        self.emulation_highlight_output_c_program.append(
                            len(self.emulation_output_c_program))
                        self.emulation_output_c_program.append(
                            str(time) + " : " + str(self.state_machine.vars["RxFIFO"][0]) + " = " + self.bit_string(self.state_machine.vars["RxFIFO"][0]))
                        # shift FIFO entries 1 to 4 back one place
                        for i in range(0, 3):
                            self.state_machine.vars["RxFIFO"][i] = self.state_machine.vars["RxFIFO"][i+1]
                        # set the last entry to 0 (this may not happen in reality!)
                        self.state_machine.vars["RxFIFO"][3] = 0
                        # there is now one less item in the Rx FIFO
                        self.state_machine.vars["RxFIFO_count"] -= 1
                        # make sure the 'status' is correct
                        if self.state_machine.settings['status_sel'] == 1:
                            if self.state_machine.vars["RxFIFO_count"] < self.state_machine.settings['FIFO_level_N']:
                                self.state_machine.vars["status"] = 0xFFFFFFFF; # binary all ones
                            else:
                                self.state_machine.vars["status"] = 0; # binary all zeroes
                elif c[1] in ['set_base', 'set_count', 'in_base', 'jmp_pin', 'sideset_base', 'sideset_count', 'sideset_opt', 'sideset_pindirs', 'out_base', 'out_count', 'out_shift_right', 'out_shift_autopull', 'pull_threshold', 'in_shift_right', 'in_shift_autopush', 'push_threshold']:
                    self.state_machine.settings[c[1]] = c[2]
                elif c[1] == 'get_pc':
                    # note: the c_program is executed before an emulation step. Thus get_pc shows the previous pc in the gui
                    self.emulation_highlight_output_c_program.append(len(self.emulation_output_c_program))
                    self.emulation_output_c_program.append(str(time) + " : " + "pc=" + str(self.state_machine.vars["pc"]))
                elif c[1] == 'set_pc':
                    # note: this should (maybe) only be used at t=0, the c_program is executed before an emulation step. Thus set_pc can set the starting point
                    # note: If not set explicitly, pc = -1 at the start, so the pc first adds 1 to start at 0. Here the 1 must first be subtracted
                    self.state_machine.vars["pc"] = c[2]-1 
                elif c[1] == 'irq':
                    # clear the bit in c[2]. Note that in c++, when clearing an irq, you have to set the corresponding bit with:
                    # pio0_hw->irq = 1<<irq
                    # Also note that here all irq are visible to the c-program, normally only irq 0-3 are visible!
                    if self.state_machine.sm_irq[c[2]] == 0:
                        # if the irq bit was not set, set it
                        self.state_machine.sm_irq[c[2]] = 1
                    else:
                        # if the irq bit was set, clear it
                        self.state_machine.sm_irq[c[2]] = 0
                elif c[1] == 'set_N':
                    # set the # items in FIFOs that sets status (used in 'mov' instruction)  
                    self.state_machine.settings['FIFO_level_N'] = c[2]
                elif c[1] == 'status_sel':
                    # set the # items in FIFOs that sets status (used in 'mov' instruction)  
                    self.state_machine.settings['status_sel'] = c[2]
                elif c[1] == 'dir_out':
                    # set the pin to output 
                    self.state_machine.GPIO_data["GPIO_pindirs"][c[2]] = 0
                elif c[1] == 'dir_in':
                    # set the pin direction to in 
                    self.state_machine.GPIO_data["GPIO_pindirs"][c[2]] = 1
                elif c[1] == 'dir_non':
                    # unset the pin direction  
                    self.state_machine.GPIO_data["GPIO_pindirs"][c[2]] = -1
                else:
                    # should already have been filtered out when parsing the file, but anyway:
                    warning_messages.append("Warning: unknown c-program statement: " + str(c[0]) + str(c[1]) + str(c[2])+"\n")

        # if out_base and out_count are set, then make the associated pin an output in GPIO_pindirs
        if self.state_machine.settings["out_base"] != -1 and self.state_machine.settings["out_count"] != 0:
            for i in range(self.state_machine.settings["out_base"], self.state_machine.settings["out_base"] + self.state_machine.settings["out_count"]):
                self.state_machine.GPIO_data["GPIO_pindirs"][i] = 0 # indicate this pin is an output
        # if set_base and set_count are set, then make the associated pin an output in GPIO_pindirs
        if self.state_machine.settings["set_base"] != -1 and self.state_machine.settings["set_count"] != 0:
            for i in range(self.state_machine.settings["set_base"], self.state_machine.settings["set_base"] + self.state_machine.settings["set_count"]):
                self.state_machine.GPIO_data["GPIO_pindirs"][i] = 0 # indicate this pin is an output
        # if sideset_base and sideset_count are set, then make the associated pin(s) an output in GPIO_pindirs
        if self.state_machine.settings["sideset_base"] != -1 and self.state_machine.settings["sideset_count"] != 0:
            num_of_bits = self.state_machine.settings["sideset_count"] - 1 if self.state_machine.settings["sideset_opt"] else 0
            for i in range(self.state_machine.settings["sideset_base"], self.state_machine.settings["sideset_base"] + num_of_bits):
                self.state_machine.GPIO_data["GPIO_pindirs"][i] = 0 # indicate this pin is an output

        return warning_messages