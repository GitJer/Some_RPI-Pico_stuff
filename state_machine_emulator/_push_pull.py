def do_auto_push_pull(self):
    """ take care of the auto push and pull """
    # do auto push
    if self.settings["in_shift_autopush"] and (self.vars["ISR_shift_counter"] >= self.settings["push_threshold"]):
        if self.vars["RxFIFO_count"] < 4:
            self.push_to_RxFIFO()
            self.push_is_stalling = False
        else:
            # block: do not go to next instruction
            self.skip_increase_pc = True
            self.delay_delay = True
            self.push_is_stalling = True
    # do auto pull
    # TODO: check if this adheres to section 3.5.4.2
    if self.settings["out_shift_autopull"] and (self.vars["OSR_shift_counter"] >= self.settings["pull_threshold"]):
        if self.vars["TxFIFO_count"] > 0:
            self.pull_from_TxFIFO()
            self.pull_is_stalling = False
        else:
            # block: do not go to next instruction
            self.skip_increase_pc = True
            self.delay_delay = True
            self.pull_is_stalling = True


def push_to_RxFIFO(self):
    """ push data to the RxFIFO (it has already been checked that there is space!) """
    # place the new item after the data already in the FIFO
    self.vars["RxFIFO"][self.vars["RxFIFO_count"]] = self.vars["ISR"]
    # there is now one more item
    self.vars["RxFIFO_count"] += 1 
    # clear the shift counter and the ISR itself
    self.vars["ISR_shift_counter"] = 0
    self.vars["ISR"] = 0


def pull_from_TxFIFO(self):
    """ pull data from TxFIFO (it has already been checked that there is data) """
    # pull the first item in the TxFIFO and place it in the OSR
    self.vars["OSR"] = self.vars["TxFIFO"][0]
    # shift the whole TxFIFO
    for t in range(self.vars["TxFIFO_count"]-1):
        self.vars["TxFIFO"][t] = self.vars["TxFIFO"][t+1]
    # set the now open space in the TxFIFO to 0
    self.vars["TxFIFO"][self.vars["TxFIFO_count"]-1] = 0
    # there is now one less data item in the TxFIFO
    self.vars["TxFIFO_count"] -= 1
    # the number of bits shifted out of the OSR is 0
    self.vars["OSR_shift_counter"] = 0
