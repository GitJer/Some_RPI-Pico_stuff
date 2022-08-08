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
