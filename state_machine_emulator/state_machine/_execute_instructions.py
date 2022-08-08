def execute_instruction(self, instruction):
    """ Execute the PIO instruction """
    # get the three bits that encode the instruction type
    instruction_type = (instruction & 0xE000) >> 13
    # get the five delay/side-set bits
    instruction_delay_sideset = (instruction & 0x1F00) >> 8

    # determine the (optional) delay
    # the bits for delay is 5 minus the number of sideset pins
    # NOTE: the number of sideset pins specified in the pio.h file includes the opt-bit
    bits_for_delay = 5 - self.settings["sideset_count"]
    # delay is the bits_for_delay LSB of instruction_del_ss
    self.vars["delay"] = instruction_delay_sideset & ((1 << bits_for_delay) - 1)
    self.do_sideset(instruction_delay_sideset)

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
        self.sm_warning_messages.append("Warning: unknown instruction, continuing\n")

    # when not pull or mov, do autopull (if enabled) (datasheet 3.5.4):
    if self.settings["out_shift_autopull"] and not ((instruction_type == 4 and is_pull) or (instruction_type == 5)):
        if self.vars["OSR_shift_counter"] >= self.settings["pull_threshold"]:
            if self.vars["TxFIFO_count"] > 0:
                self.pull_from_TxFIFO()
                self.pull_is_stalling = False


def execute_jmp(self, instruction):
    """ execute a jmp instruction """
    # get instruction parameters
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
            do_jump = True
        # x--
        self.vars["x"] = (self.vars["x"] - 1) & 0xffffffff
    elif jmp_condition == 3:    # !y
        if self.vars["y"] == 0:
            do_jump = True
    elif jmp_condition == 4:    # y--
        if self.vars["y"] != 0:
            do_jump = True
        # y--
        self.vars["y"] = (self.vars["y"] - 1) & 0xffffffff
    elif jmp_condition == 5:    # x!=y
        if self.vars["y"] != self.vars["x"]:
            do_jump = True
    elif jmp_condition == 6:    # pin
        if self.settings["jmp_pin"] == -1:
            self.sm_warning_messages.append("Warning: 'jmp_pin' isn't set before use in JMP instruction, continuing\n")
        if self.GPIO_data["GPIO"][self.settings["jmp_pin"]] == 1:
            do_jump = True
    elif jmp_condition == 7:    # !OSRE
        if self.vars["OSR_shift_counter"] < 32:
            do_jump = True
    if do_jump:
        # save the address to jump to for when the pc is set for the new instruction
        self.jmp_to = addr
        # the address is given by self.jmp_to, so, no increase of the pc
        self.skip_increase_pc = True


def execute_wait(self, instruction):
    """ execute a wait instruction """
    # get instruction parameters
    polarity = 1 if (instruction & (1 << 7)) > 0 else 0
    source = (instruction & 0x0060) >> 5
    index = instruction & 0x1F
    
    is_not_met = False
    if source == 0:             # GPIO
        if self.GPIO_data["GPIO"][index] != polarity:
            is_not_met = True
    elif source == 1:           # pin
        if self.settings["in_base"] == -1:
            self.sm_warning_messages.append("Warning: 'in_base' isn't set before use in WAIT instruction, continuing\n")
        if self.GPIO_data["GPIO"][(self.settings["in_base"]+index) % 32] != polarity:
            is_not_met = True
    elif source == 2:           # IRQ
        MSB = 1 if (instruction & (1 << 4)) > 0 else 0
        if MSB:
            irq = (instruction & 0x07 + self.sm_number) % 4
        else:
            irq = instruction & 0x07
        if self.sm_irq[irq] != polarity:
            is_not_met = True
    else:
        self.sm_warning_messages.append("Warning: WAIT has unknown source, continuing\n")

    if is_not_met:
        # condition has not been met, keep waiting
        self.skip_increase_pc = True
        # wait with the delay until condition has been met
        self.delay_delay = True


def execute_in(self, instruction):
    """ execute an in instruction """

    # rp2040-datasheet.pdf 3.2.4 Stalling: An IN instruction when autopush is enabled, ISR reaches its shift threshold, and the RxFIFO is full
    if self.push_is_stalling:
        self.sm_warning_messages.append("Warning: push is stalling in IN\n")
        return

    # get instruction parameters
    source = (instruction & 0x00E0) >> 5
    bit_count = instruction & 0x001F

    if bit_count == 0:
        bit_count = 32
    value = 0
    mask = (1 << bit_count) - 1

    # get data from the source
    if source == 0:     # PINS
        if self.settings["in_base"] == -1:
            self.sm_warning_messages.append("Warning: 'in_base' isn't set before use in IN instruction, continuing\n")
        for pin in range(bit_count):
            value |= (self.GPIO_data["GPIO"][(self.settings["in_base"] + pin) % 32] << pin)
    elif source == 1:   # X
        value = self.vars["x"] & mask
    elif source == 2:   # Y
        value = self.vars["y"] & mask
    elif source == 3:   # NULL
        value = 0
    elif source == 4:   # reserved
        self.sm_warning_messages.append("Warning: IN has unknown source, continuing\n")
        return
    elif source == 5:   # reserved
        self.sm_warning_messages.append("Warning: IN has unknown source, continuing\n")
        return
    elif source == 6:   # ISR
        value = self.vars["ISR"] & mask
    elif source == 7:   # OSR
        value = self.vars["OSR"] & mask
    else:               # Error
        self.sm_warning_messages.append("Warning: IN has unknown source, continuing\n")
        return

    # shift data into the ISR
    if self.settings["in_shift_right"]:  # shift right
        self.vars["ISR"] >>= bit_count
        self.vars["ISR"] |= value << (32-bit_count)
    else:  # shift left
        self.vars["ISR"] <<= bit_count
        self.vars["ISR"] |= value
    # make sure it the ISR stays 32 bit
    self.vars["ISR"] &= 0xFFFFFFFF
    # adjust the shift counter
    self.vars["ISR_shift_counter"] += bit_count
    if (self.vars["ISR_shift_counter"]) > 32:
        self.vars["ISR_shift_counter"] = 32

    # (if enabled) autopush or stall
    if self.settings["in_shift_autopush"] and (self.vars["ISR_shift_counter"] >= self.settings["push_threshold"]):
        if self.vars["RxFIFO_count"] < 4:
            self.push_to_RxFIFO()
            self.push_is_stalling = False
        else:
            # block: do not go to next instruction
            self.skip_increase_pc = True
            self.delay_delay = True
            self.push_is_stalling = True

def execute_out(self, instruction):
    """ execute an out instruction """

    # do autopull (datasheet 3.5.4):
    if self.settings["out_shift_autopull"]:
        if self.vars["OSR_shift_counter"] >= self.settings["pull_threshold"]:
            if self.vars["TxFIFO_count"] > 0:
                self.pull_from_TxFIFO()
            # stall
            self.skip_increase_pc = True
            self.delay_delay = True
            self.pull_is_stalling = True
            self.sm_warning_messages.append("Warning: pull is stalling in OUT\n")
            return
    
    # get instruction parameters
    destination = (instruction & 0x00E0) >> 5
    bit_count = instruction & 0x001F

    if bit_count == 0:
        bit_count = 32

    # shift to the right
    if self.settings["out_shift_right"]:
        # take the bit_count LSB
        mask = (1 << bit_count)-1
        value = self.vars["OSR"] & mask
        # shift the OSR bit_count to the right
        self.vars["OSR"] >>= bit_count
    else:   # shift to the left
        # take the bit_count MSB by making a mask and shifting it left
        mask = (1 << bit_count)-1
        # shift them (32-bit_count) to the left
        mask <<= (32-bit_count)
        # get the bits from the OSR
        value = self.vars["OSR"] & mask
        # shift value back
        value >>= (32-bit_count)
        # shift the OSR bit_count to the left
        self.vars["OSR"] <<= bit_count
    # make sure it the OSR stays 32 bit
    self.vars["OSR"] &= 0xFFFFFFFF
    # adjust the shift counter
    self.vars["OSR_shift_counter"] += bit_count

    # put the result in the destination
    if destination == 0:     # PINS
        if self.settings["out_base"] == -1:
            self.sm_warning_messages.append("Warning: 'out_base' isn't set before use in OUT instruction, continuing\n")
        for pin in range(bit_count):
            self.GPIO_data["GPIO_out"][(self.settings["out_base"] + pin) % 32] = 1 if value & (1 << pin) else 0
    elif destination == 1:   # X
        self.vars["x"] = value
    elif destination == 2:   # Y
        self.vars["y"] = value
    elif destination == 3:   # NULL
        pass
    elif destination == 4:   # PINDIRS
        if self.settings["out_base"] == -1:
            self.sm_warning_messages.append("Warning: 'out_base' isn't set before use in OUT instruction, continuing\n")
        for pin in range(bit_count):
            self.GPIO_data["GPIO_pindirs"][(self.settings["out_base"] + pin) % 32] = 1 if value & (1 << pin) else 0
    elif destination == 5:   # PC
        # save the address to jump to for when the pc is set for the new instruction
        self.jmp_to = value & 0x1F
        # the address is given, so, no increase of one for the pc
        self.skip_increase_pc = True
    elif destination == 6:   # ISR
        self.vars["ISR"] = value
        self.vars["ISR_shift_counter"] += bit_count
    elif destination == 7:   # EXEC
        # TODO?
        self.sm_warning_messages.append("Warning: OUT EXEC functionality isn't implemented, continuing\n")
    else:                   # Error
        self.sm_warning_messages.append("Warning: OUT has unknown destination, continuing\n")
        return






def execute_push(self, instruction):
    """ execute a push instruction """
    # get instruction parameters
    ifF = 1 if (instruction & (1 << 6)) > 0 else 0
    Blk = 1 if (instruction & (1 << 5)) > 0 else 0

    # check if there is space in the FIFO
    if self.vars["RxFIFO_count"] < 4:
        if ifF:
            # only push if counter >= the threshold
            if (self.vars["ISR_shift_counter"] >= self.settings["push_threshold"]):
                self.push_to_RxFIFO()
        else:
            # push independent of counter and threshold
            self.push_to_RxFIFO()
    else:
        # there's no space to push: block or continue?
        if Blk:
            # blocking: do not go to next instruction
            self.skip_increase_pc = True
            self.delay_delay = True
            self.push_is_stalling = True
        else:
            # continue, but clear ISR
            self.push_is_stalling = False
            self.vars["ISR"] = 0


def execute_pull(self, instruction):
    """ execute a pull instruction """
    # get instruction parameters
    ifE = 1 if (instruction & (1 << 6)) > 0 else 0
    Blk = 1 if (instruction & (1 << 5)) > 0 else 0

    if self.vars["TxFIFO_count"] != 0:
        if ifE:
            # only pull if counter is larger than threshold
            if (self.vars["OSR_shift_counter"] >= self.settings["pull_threshold"]):
                self.pull_from_TxFIFO()
        else:
            # pull independent of counter and threshold
            self.pull_from_TxFIFO()
    else:
        # there's no data to pull: block or continue
        if Blk:
            # blocking: do not go to next instruction
            self.skip_increase_pc = True
            self.delay_delay = True
            self.pull_is_stalling = True
        else:
            # "A non-blocking PULL on an empty FIFO has
            # the same effect as MOV OSR, X"
            self.vars["OSR"] = self.vars["x"]
            self.pull_is_stalling = False
            self.sm_warning_messages.append("Note: a non-blocking PULL on an empty FIFO has the same effect as 'MOV OSR, X', continuing\n")



def execute_mov(self, instruction):
    """ execute a mov instruction """
    # get instruction parameters
    destination = (instruction & 0x00E0) >> 5
    operation = (instruction & 0x0018) >> 3
    source = (instruction & 0x0007)

    # the value to be moved
    value = -1
    # get the source (i.e. set 'value')
    if source == 0:     # PINS
        if self.settings["in_base"] == -1:
            self.sm_warning_messages.append("Warning: 'in_base' isn't set before use in MOV instruction, continuing\n")
        value = 0
        for pin in range(32):
            if self.GPIO_data["GPIO"][(self.settings["in_base"] + pin) % 32] == -1:
                self.sm_warning_messages.append(str("Warning: a pin ("+str((self.settings["in_base"] + pin) % 32)+") with undefined state is read, 0 is used, continuing\n"))
                # not necessary to 'or' value with a 0
            else:
                value |= (self.GPIO_data["GPIO"][(self.settings["in_base"] + pin) % 32] << pin)
    elif source == 1:   # X
        value = self.vars["x"]
    elif source == 2:   # Y
        value = self.vars["y"]
    elif source == 3:   # NULL
        value = 0
    elif source == 4:   # reserved
        self.sm_warning_messages.append("Warning: MOV has unknown source, continuing\n")
        return
    elif source == 5:   # status 
        value = self.vars["status"]
    elif source == 6:   # ISR
        value = self.vars["ISR"]
    elif source == 7:   # OSR
        value = self.vars["OSR"]
    else:               # Error
        self.sm_warning_messages.append("Warning: MOV has unknown source, continuing\n")

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
    # PINS TODO: check if it is correct that only out_count bits are output (not 32)
    if destination == 0:
        if self.settings["out_base"] == -1:
            self.sm_warning_messages.append("Warning: 'out_base' isn't set before use in MOV instruction, continuing\n")
        if self.settings["out_count"] == -1:
            self.sm_warning_messages.append("Warning: 'out_count' isn't set before use in MOV instruction, continuing\n")
        for pin in range(self.settings["out_count"]):
            self.GPIO_data["GPIO_out"][(self.settings["out_base"] + pin) % 32] = 1 if value & (1 << pin) else 0
            # self.set_GPIO('out', (self.settings["out_base"] + pin) % 32, value & (1 << pin))
    elif destination == 1:   # X
        self.vars["x"] = value
    elif destination == 2:   # Y
        self.vars["y"] = value
    elif destination == 3:   # reserved
        self.sm_warning_messages.append("Warning: MOV has unknown destination, continuing\n")
        return
    elif destination == 4:   # EXEC
        # TODO?
        self.sm_warning_messages.append("Warning: MOV EXEC functionality isn't implemented, continuing\n")
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
    else:                    # Error
        self.sm_warning_messages.append("Warning: MOV has unknown destination, continuing\n")
        return


def execute_irq(self, instruction):
    """ execute an irq instruction """
    # get instruction parameters
    Clr = 1 if (instruction & (1 << 6)) > 0 else 0
    Wait = 1 if (instruction & (1 << 5)) > 0 else 0
    MSB = 1 if (instruction & (1 << 4)) > 0 else 0

    # add sm number and do modulo 4 if MSB is set
    if MSB:
        irq = (instruction & 0x07 + self.sm_number) % 4
    else:
        irq = instruction & 0x07
    
    # the irq statement is already waiting for clearing 
    if self.irq_is_waiting:
        # check if irq is set, if so, wait
        if self.sm_irq[irq] == 1:
            self.skip_increase_pc = True
            self.delay_delay = True
        else:
            self.irq_is_waiting = False
            return
    
    if Clr:
        # clear the irq
        self.sm_irq[irq] = 0
    else:
        # set the irq
        self.sm_irq[irq] = 1
        if Wait:
            self.irq_is_waiting = True
            self.skip_increase_pc = True
            self.delay_delay = True

def execute_set(self, instruction):
    """ execute a set instruction """
    # get instruction parameters
    destination = (instruction & 0x00E0) >> 5
    data = instruction & 0x001F

    if destination == 0:        # PINS
        if self.settings["set_base"] == -1:
            self.sm_warning_messages.append("Warning: 'set_base' isn't set before use in SET instruction, continuing\n")
        if self.settings["set_count"] == -1:
            self.sm_warning_messages.append("Warning: 'set_count' isn't set before use in SET instruction, continuing\n")
        else:
            for pin in range(self.settings["set_count"]):
                self.GPIO_data["GPIO_set"][(self.settings["set_base"] + pin) % 32] = 1 if data & (1 << pin) else 0
                # self.set_GPIO('set', ((self.settings["set_base"] + pin) % 32), data & (1 << pin))
    elif destination == 1:      # X
        self.vars["x"] = data
    elif destination == 2:      # Y
        self.vars["y"] = data
    elif destination == 4:      # PINDIRS
        if self.settings["set_base"] == -1:
            self.sm_warning_messages.append("Warning: 'set_base' isn't set before use in SET instruction, continuing\n")
        if self.settings["set_count"] == -1:
            self.sm_warning_messages.append("Warning: 'set_count' isn't set before use in SET instruction, continuing\n")
        for pin in range(self.settings["set_count"]):
            self.GPIO_data["GPIO_pindirs"][(self.settings["set_base"] + pin) % 32] = 1 if data & (1 << pin) else 0
    else:
        self.sm_warning_messages.append("Warning: SET has unknown destination, continuing\n")
