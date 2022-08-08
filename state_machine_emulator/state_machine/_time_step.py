def time_step(self):
    """ emulate one time step """
    # prepare for warning messages
    self.sm_warning_messages = []
    # flag to indicate we're dealing with a delayed delay
    skip_due_to_delay_delay = False
    # check if delay is active: for some instructions (wait, irq) delay needs to wait till after the instruction has finished
    if not self.delay_delay:
        # check if a delay is being executed: if so, do nothing
        if self.vars["delay"] > 0:
            self.vars["delay"] -= 1
            skip_due_to_delay_delay = True
    
    # if delayed delay: skip normal pc increase and instruction execution
    if skip_due_to_delay_delay == False:
        self.delay_delay = False
        # check if (e.g. due to a jmp or wait) the pc does not need to be changed
        if self.skip_increase_pc:
            # yes: do not increase the pc
            self.skip_increase_pc = False # clear flag
            if self.jmp_to >= 0:
                # the skip is due to a jmp, set pc to the address to jmp to
                self.vars["pc"] = self.jmp_to
                self.jmp_to = -1 #  not set
        else:
            # add one to the program counter
            self.vars["pc"] += 1
            # check if the pc should wrap
            if self.vars["pc"] == self.wrap+1:
                self.vars["pc"] = self.wrap_target
        # get the new instruction and execute it
        instruction = int(self.program[self.vars["pc"]][0], 16)
        # execute the instruction
        self.execute_instruction(instruction)
        # set the 'status' depending on RxFIFO or TxFIFO count
        if self.settings['status_sel'] == 0:
            # check TxFIFO level
            if self.vars["TxFIFO_count"] < self.settings['FIFO_level_N']:
                self.vars["status"] = -1; # binary all ones 
            else:
                self.vars["status"] = 0;  # binary all zeroes
        else: # self.settings['status_sel'] == 1
            # check RxFIFO level
            if self.vars["RxFIFO_count"] < self.settings['FIFO_level_N']:
                self.vars["status"] = -1; # binary all ones
            else:
                self.vars["status"] = 0; # binary all zeroes
    # set the GPIOs according to the changes in this time step
    # note: even when skip_due_to_delay_delay is True, there may still be 
    # changes due to the pin_ or c_program
    self.set_all_GPIO()
    # increase the system level clock
    self.clock += 1
    # return warnings
    return self.sm_warning_messages
