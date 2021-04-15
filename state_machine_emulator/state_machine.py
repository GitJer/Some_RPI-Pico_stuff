
class state_machine:
    """ this class emulates a state machine (sm) """

    def __init__(self, sm_number, program_definitions, rp2040):
        """ init makes the variables (such as x and y registers) and settings
            and it makes some class variables needed for correct execution
        """
        self.sm_number = sm_number
        self.name = "sm"+str(sm_number)
        self.rp2040 = rp2040
        self.program = program_definitions['pio_program']
        # self.length = program_definitions['pio_program_length']
        self.origin = program_definitions['pio_program_origin']
        self.wrap_target = program_definitions['pio_program_wrap_target']
        self.wrap = program_definitions['pio_program_wrap']

        self.vars = {}
        self.vars["RxFIFO"] = [0 for _ in range(4)]
        self.vars["RxFIFO_count"] = 0
        self.vars["TxFIFO"] = [0 for _ in range(4)]
        self.vars["TxFIFO_count"] = 0
        self.vars["x"] = 0
        self.vars["y"] = 0
        self.vars["ISR"] = 0
        self.vars["ISR_shift_counter"] = 0
        self.vars["OSR"] = 0
        self.vars["OSR_shift_counter"] = 0
        self.vars["pc"] = self.origin
        self.vars["delay"] = 0

        self.settings = {}
        self.settings["in_shift_direction"] = 0  # right
        self.settings["in_shift_autopush"] = 0  # false
        self.settings["SHIFTCTRL_PUSH_THRESH"] = 32  # false
        self.settings["out_shift_direction"] = 0  # right #TODO:
        self.settings["out_shift_autopull"] = 0  # false
        self.settings["SHIFTCTRL_PULL_THRESH"] = 32  # false
        self.settings["out_special_sticky"] = 0  # TODO:false
        self.settings["out_special_has_enable_pin"] = 0  # TODO:false
        self.settings["out_special_enable_pin_index"] = 0  # TODO:
        self.settings["mov_status_sel"] = 0  # TODO:STATUS_TX_LESSTHAN
        self.settings["mov_status_n"] = 0     # TODO:
        self.settings["status"] = -1
        self.settings["jmp_pin"] = 0
        self.settings["set_base"] = 0
        self.settings["set_count"] = 0
        self.settings["out_base"] = 0
        self.settings["out_count"] = 32
        self.settings["side_set_base"] = 0
        self.settings["side_set_count"] = -1
        self.settings["side_set"] = 0  # disabled
        self.settings["in_base"] = 0
        self.settings["PINCTRL_IN_BASE"] = 0
        self.settings["FDEBUG_RXSTALL"] = 0

        # for e.g. jmp the pc should not be updated after an execution step since it is set by the jmp
        # also for e.g. wait statements the pc should remain unchanged
        self.skip_increase_pc = False
        # for some statements the delay should be postponed to after the instruction (e.g. wait) has finished
        self.delay_delay = False
        # the jmp statement changes the vars["pc"] immediately when the criterion is true, but I want to keep the 'old' pc until it is time to change the pc in 'time_step()'
        self.jmp_to = -1

    def time_step(self):
        """ emulate one time step """

        # check if a delay is being executed: if so, do nothing
        if not self.delay_delay:
            if self.vars["delay"] > 0:
                self.vars["delay"] -= 1
                # print("doing delay")
                return
        self.delay_delay = False

        # check if (e.g. due to a jmp) the pc does not need to be changed
        if self.skip_increase_pc:
            self.skip_increase_pc = False
            if self.jmp_to >= 0:
                self.vars["pc"] = self.jmp_to
                self.jmp_to = -1
        else:
            self.vars["pc"] += 1
            # check if the pc should wrap
            if self.vars["pc"] == self.wrap+1:
                self.vars["pc"] = self.wrap_target

        # get the new instruction and execute it
        # print("pc=", self.vars["pc"])
        instruction = int(self.program[self.vars["pc"]][0], 16)
        # print("instruction=", instruction, "->",
        #       self.program[self.vars["pc"]][1])
        self.execute_instruction(instruction)

        # do auto push
        if self.settings["in_shift_autopush"] and (self.vars["ISR_shift_counter"] >= self.settings["SHIFTCTRL_PUSH_THRESH"]):
            if self.vars["RxFIFO_count"] < 4:
                self.vars["RxFIFO"][self.vars["RxFIFO_count"]
                                    ] = self.vars["ISR"]
                self.vars["RxFIFO_count"] += 1
                self.vars["ISR_shift_counter"] = 0
                self.vars["ISR"] = 0
            else:
                # block: do not go to next instruction
                self.skip_increase_pc = True
                self.FDEBUG_RXSTALL = 1

        # do auto pull
        # TODO: see section 3.5.4.2
        if self.settings["out_shift_autopull"] and (self.vars["OSR_shift_counter"] >= self.in_shift_pull_threshold):
            if self.vars["TxFIFO_count"] != 0:
                # there is data in the TxFIFO, place the first item in the OSR
                self.vars["OSR"] = self.vars["TxFIFO"][0]
                # shift the whole TxFIFO
                for t in range(self.vars["TxFIFO_count"]):
                    self.vars["TxFIFO"][t] = self.vars["TxFIFO"][t+1]
                # there is now one less data item in the TxFIFO
                self.vars["TxFIFO_count"] -= 1
                # the number of bits shifted out of the OSR is 0
                self.vars["OSR_shift_counter"] = 0
            else:
                # block: do not go to next instruction
                self.skip_increase_pc = True

    def execute_instruction(self, instruction):
        """ Execute the PIO insruction """
        # get the three bits that encode the instruction type
        instruction_type = (instruction & 0xE000) >> 13
        # get the five delay/side-set bits
        instruction_del_ss = (instruction & 0x1F00) >> 8
        # print("delay/side-set=", instruction_del_ss)
        # TODO: for now assume side-set is not used -> all bits are a delay
        self.vars["delay"] = instruction_del_ss

        is_pull = 1 if (instruction & (1 << 7)) > 0 else 0

        # determine which function to execute based on the instruction_type
        if instruction_type == 0:                   # its a 'jmp'
            self.execute_jmp(instruction)
        elif instruction_type == 1:                 # its a 'wait'
            self.execute_wait(instruction)
        elif instruction_type == 2:                 # its a 'in'
            self.execute_in(instruction)
        elif instruction_type == 3:                 # its a 'out'
            self.execute_out(instruction)
        elif instruction_type == 4 and is_pull:     # its a 'pull'
            self.execute_pull(instruction)
        elif instruction_type == 4 and not is_pull:  # its a 'push'
            self.execute_push(instruction)
        elif instruction_type == 5:                 # its a 'mov'
            self.execute_mov(instruction)
        elif instruction_type == 6:                 # its a 'irq'
            self.execute_irq(instruction)
        elif instruction_type == 7:                 # its a 'set'
            self.execute_set(instruction)
        else:                                       # its an error!
            print("Error: execute_instruction: unknown instruction, continuing anyway")

    def execute_jmp(self, instruction):
        """ execute a jmp instruction """
        jmp_condition = (instruction & 0x00E0) >> 5
        addr = instruction & 0x001F
        do_jump = False
        if jmp_condition == 0:      # always
            do_jump = True
        elif jmp_condition == 1:    # !x
            if self.vars["x"] == 0:
                do_jump = True
        elif jmp_condition == 2:    # x--
            if self.vars["x"] != 0:
                self.vars["x"] -= 1
                do_jump = True
        elif jmp_condition == 3:    # !y
            if self.vars["y"] != 0:
                do_jump = True
        elif jmp_condition == 4:    # y--
            if self.vars["y"] != 0:
                self.vars["y"] -= 1
                do_jump = True
        elif jmp_condition == 5:    # x!=y
            if self.vars["y"] != self.vars["x"]:
                do_jump = True
        elif jmp_condition == 6:    # pin
            if self.rp2040.GPIO[self.settings["jmp_pin"]] == 1:
                do_jump = True
        elif jmp_condition == 7:    # !OSRE
            if self.vars["OSR_shift_counter"] < 32:
                do_jump = True
        if do_jump:
            # save the address to jump to for when the pc is set for the new instruction
            self.jmp_to = addr
            # the address is given, so, no increase of one for the pc
            self.skip_increase_pc = True

    def execute_wait(self, instruction):
        """ execute a wait instruction """
        polarity = 1 if (instruction & (1 << 7)) > 0 else 0
        source = (instruction & 0x0060) >> 5
        index = instruction & 0x1F
        is_not_met = False
        if source == 0:             # GPIO
            if self.rp2040.GPIO[index] != polarity:
                is_not_met = True
        elif source == 1:           # pin
            if self.rp2040.GPIO[self.settings["PINCTRL_IN_BASE"]+index] != polarity:
                is_not_met = True
        elif source == 2:           # IRQ
            MSB = 1 if (instruction & (1 << 4)) > 0 else 0
            if MSB:
                irq = (instruction & 0x07 + self.sm_number) % 4
            else:
                irq = instruction & 0x07
            if self.rp2040.PIO[0].sm_irq[irq] != polarity:
                is_not_met = True
        else:
            print("Error: wait unknown source")

        if is_not_met:
            # condition has not been met, keep waiting
            self.skip_increase_pc = True
            # wait with the delay until condition has been met
            self.delay_delay = True

    def execute_in(self, instruction):
        """ execute an in instruction """
        source = (instruction & 0x00E0) >> 5
        bit_count = instruction & 0x001F
        if bit_count == 0:
            bit_count = 32
        value = 0
        mask = (1 << bit_count) - 1
        if source == 0:     # PINS
            for pin in range(self.settings["in_count"]):
                value |= (self.rp2040.GPIO[(
                    self.settings["in_base"] + pin) % 32] << pin)
        elif source == 1:   # X
            value = self.vars["x"] & mask
        elif source == 2:   # Y
            value = self.vars["y"] & mask
        elif source == 3:   # NULL
            value = 0
        elif source == 4:   # reserved
            print("Error: execute_mov: unknown source, skipping")
            return
        elif source == 5:   # reserved
            print("Error: execute_mov: unknown source, skipping")
            return
        elif source == 6:   # ISR
            value = self.vars["ISR"] & mask
        elif source == 7:   # OSR
            value = self.vars["OSR"] & mask
        else:               # Error
            print("Error: execute_mov: unknown source, skipping")
            return
        self.vars["ISR_shift_counter"] += bit_count
        if self.settings["in_shift_direction"] == 0:  # shift right
            self.vars["ISR"] >>= bit_count
            self.vars["ISR"] |= value << (32-bit_count)
        else:  # shift left
            self.vars["ISR"] <<= bit_count
            self.vars["ISR"] |= value

    def execute_out(self, instruction):
        """ execute an out instruction """
        destination = (instruction & 0x00E0) >> 5
        bit_count = instruction & 0x001F
        if bit_count == 0:
            bit_count = 32

        if self.settings["out_shift_direction"] == 1:  # shift to the left
            # take the bit_count MSB by making a mask and shifting it left
            mask = (1 << bit_count)-1
            # shift them (32-bit_count) to the left
            mask <<= (32-bit_count)
            # get the bits from the OSR
            value = self.vars["OSR"] & mask
            # shift value back
            value >>= (32-bit_count)
            # shift the OSR bit_count to the right
            self.vars["OSR"] <<= bit_count
            # make sure it the OSR stays 32 bit
            self.vars["OSR"] &= 0xFFFFFFFF
        else:   # shift to the right
            # take the bit_count LSB
            mask = (1 << bit_count)-1
            value = self.vars["OSR"] & mask
            # shift the OSR bit_count to the left
            self.vars["OSR"] >>= bit_count
            # make sure it the OSR stays 32 bit
            self.vars["OSR"] &= 0xFFFFFFFF
        # adjust the shift counter
        self.vars["OSR_shift_counter"] += bit_count

        # put the result in the destination
        if destination == 0:     # PINS
            for pin in range(bit_count):
                self.rp2040.GPIO[(
                    self.settings["out_base"] + pin) % 32] = value & (1 << pin)
        elif destination == 1:   # X
            self.vars["x"] = value
        elif destination == 2:   # Y
            self.vars["y"] = value
        elif destination == 3:   # NULL
            pass
        elif destination == 4:   # PINDIRS
            for pin in range(bit_count):
                self.rp2040.GPIO_pindirs[(
                    self.settings["out_base"] + pin) % 32] = value & (1 << pin)
        elif destination == 5:   # PC
            # save the address to jump to for when the pc is set for the new instruction
            self.jmp_to = value & 0x1F
            # the address is given, so, no increase of one for the pc
            self.skip_increase_pc = True
        elif destination == 6:   # ISR
            self.vars["ISR"] = value
            self.vars["ISR_shift_counter"] += bit_count
        elif destination == 7:   # EXEC
            # TODO:
            pass
        else:                   # Error
            print("Error: execute_out: unknown destination, skipping")
            return

    def execute_push(self, instruction):
        """ execute a push instruction """
        # TODO: ifE implementation
        ifE = 1 if (instruction & (1 << 6)) > 0 else 0
        Blk = 1 if (instruction & (1 << 5)) > 0 else 0

        if self.vars["RxFIFO_count"] < 4:
            self.vars["RxFIFO"][self.vars["RxFIFO_count"]] = self.vars["ISR"]
            self.vars["RxFIFO_count"] += 1
            self.vars["ISR_shift_counter"] = 0
            self.vars["ISR"] = 0
        else:
            # if blocking do not go to next instruction
            self.FDEBUG_RXSTALL = 1
            if Blk:
                self.skip_increase_pc = True
            else:
                self.vars["ISR"] = 0

    def execute_pull(self, instruction):
        """ execute a pull instruction """
        # TODO: ifE implementation
        ifE = 1 if (instruction & (1 << 6)) > 0 else 0
        Blk = 1 if (instruction & (1 << 5)) > 0 else 0

        if self.vars["TxFIFO_count"] != 0:
            # there is data in the TxFIFO, place the first item in the OSR
            self.vars["OSR"] = self.vars["TxFIFO"][0]
            # shift the whole TxFIFO
            for t in range(self.vars["TxFIFO_count"]):
                self.vars["TxFIFO"][t] = self.vars["TxFIFO"][t+1]
            # there is now one less data item in the TxFIFO
            self.vars["TxFIFO_count"] -= 1
            # the number of bits shifted out of the OSR is 0
            self.vars["OSR_shift_counter"] = 0
        else:
            # if blk do not go to next instruction
            if Blk:
                self.skip_increase_pc = True
            else:
                # "A nonblocking PULL on an empty FIFO has
                # the same effect as MOV OSR, X"
                self.vars["OSR"] = self.vars["x"]

    def execute_mov(self, instruction):
        """ execute a mov instruction """
        destination = (instruction & 0x00E0) >> 5
        operation = (instruction & 0x0018) >> 3
        source = (instruction & 0x0007)
        value = -1

        # get the source (i.e. set 'value')
        if source == 0:     # PINS
            value = 0
            for pin in range(self.settings["in_count"]):
                value |= (self.rp2040.GPIO[(
                    self.settings["in_base"] + pin) % 32] << pin)
        elif source == 1:   # X
            value = self.vars["x"]
        elif source == 2:   # Y
            value = self.vars["y"]
        elif source == 3:   # NULL
            value = 0
        elif source == 4:   # reserved
            print("Error: execute_mov: unknown source, skipping")
            return
        elif source == 5:   # status
            value = self.settings["status"]
        elif source == 6:   # ISR
            value = self.vars["ISR"]
        elif source == 7:   # OSR
            value = self.vars["OSR"]
        else:               # Error
            print("Error: execute_mov: unknown source, skipping")

        # apply the operation on value
        if operation == 1:      # invert (bitwise complement)
            value = 0xFFFFFFFF - value
        elif operation == 2:    # bit-reversed
            reversed_value = 0
            for i in range(32):
                reversed_value <<= 1
                reversed_value |= value & 1
                value >>= 1
            value = reversed_value

        # place value in destination
        # get the source (i.e. set 'value')
        if destination == 0:     # PINS
            for pin in range(self.settings["out_count"]):
                self.rp2040.GPIO[(
                    self.settings["out_base"] + pin) % 32] = value & (1 << pin)
        elif destination == 1:   # X
            self.vars["x"] = value
        elif destination == 2:   # Y
            self.vars["y"] = value
        elif destination == 3:   # reserved
            print("Error: execute_mov: unknown destination, skipping")
            return
        elif destination == 4:   # EXEC
            # TODO:
            pass
        elif destination == 5:   # PC
            # save the address to jump to for when the pc is set for the new instruction
            self.jmp_to = value & 0x1F
            # the address is given, so, no increase of one for the pc
            self.skip_increase_pc = True

        elif destination == 6:   # ISR
            self.vars["ISR"] = value
            self.vars["ISR_shift_counter"] = 0
        elif destination == 7:   # OSR
            self.vars["OSR"] = value
            self.vars["OSR_shift_counter"] = 0
        else:               # Error
            print("Error: execute_mov: unknown destination, skipping")
            return

    def execute_irq(self, instruction):
        """ execute an irq instruction """
        Clr = 1 if (instruction & (1 << 6)) > 0 else 0
        Wait = 1 if (instruction & (1 << 5)) > 0 else 0
        MSB = 1 if (instruction & (1 << 4)) > 0 else 0
        if MSB:
            irq = (instruction & 0x07 + self.sm_number) % 4
        else:
            irq = instruction & 0x07
        print("c=", Clr, "Wait=", Wait, "irq=", irq)
        if Clr:
            # clear the irq
            self.rp2040.PIO[0].sm_irq[irq] = 0
        elif Wait:
            # check if irq is set, if so, wait
            if self.rp2040.PIO[0].sm_irq[irq] == 0:
                self.skip_increase_pc = True
                self.delay_delay = True
        else:
            # set the irq
            self.rp2040.PIO[0].sm_irq[irq] = 1

    def execute_set(self, instruction):
        """ execute a set instruction """
        destination = (instruction & 0x00E0) >> 5
        data = instruction & 0x001F
        if destination == 0:        # PINS
            if self.settings["set_base"] == -1:
                print("Error: no set_pin_base is set")
            else:
                for pin in range(self.settings["set_count"]):
                    self.rp2040.GPIO[(
                        self.settings["set_base"] + pin) % 32] = data & (1 << pin)
        elif destination == 1:      # X
            self.vars["x"] = data
        elif destination == 2:      # Y
            self.vars["y"] = data
        elif destination == 4:      # PINDIRS
            for pin in range(self.settings["set_count"]):
                self.rp2040.GPIO_pindirs[(
                    self.settings["set_base"] + pin) % 32] = data & (1 << pin)
        else:
            print("Error: execute_set: unknown destination")
