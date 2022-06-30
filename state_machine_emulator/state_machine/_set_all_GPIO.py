def set_all_GPIO(self):
    """ sets the GPIO according to the changes in the time step; there is a specific priority order to out, set, sideset and external """
    # first 'out' and 'set' values (lowest priority)
    for pin in range(self.settings["out_base"], self.settings["out_count"] + self.settings["out_base"]):
        if self.GPIO_data["GPIO_out"][pin] != -1:
            if self.GPIO_data["GPIO_pindirs"][pin] == 0: # pindir must be an output (0)
                self.GPIO_data["GPIO"][pin] = self.GPIO_data["GPIO_out"][pin]
            else:
                self.sm_warning_messages.append("Warning: GPIO "+str(pin)+" set by 'out' is not an output, continuing\n")

    for pin in range(self.settings["set_base"], self.settings["set_count"]+self.settings["set_base"]):
        if self.GPIO_data["GPIO_set"][pin] != -1:
            if self.GPIO_data["GPIO_pindirs"][pin] == 0: # pindir must be an output (0)
                self.GPIO_data["GPIO"][pin] = self.GPIO_data["GPIO_set"][pin]
            else:
                self.sm_warning_messages.append("Warning: GPIO "+str(pin)+" set by 'set' is not an output, continuing\n")

    # now sideset, medium priority
    # first check if the GPIO or the pindirs have to be set (here only GPIO)
    if not self.settings["sideset_pindirs"]:
        # dirty hack: sideset_count includes the sideset_opt bit, but this does not set a pin!
        count_sideset = self.settings["sideset_count"]
        if self.settings["sideset_opt"]:
            count_sideset -= 1
        # now set the pins
        for pin in range(self.settings["sideset_base"], count_sideset + self.settings["sideset_base"]):
            if self.GPIO_data["GPIO_sideset"][pin] != -1:
                if self.GPIO_data["GPIO_pindirs"][pin] == 0: # pindir must be an output (0)
                    self.GPIO_data["GPIO"][pin] = self.GPIO_data["GPIO_sideset"][pin]
                else:
                    self.sm_warning_messages.append("Warning: GPIO "+str(pin)+" set by 'sideset' is not an output, continuing\n")

    # finally, externally driven pins, highest priority
    for pin in range(32):
        if self.GPIO_data["GPIO_external"][pin] != -1:
            # external input always wins
            self.GPIO_data["GPIO"][pin] = self.GPIO_data["GPIO_external"][pin]
            if self.GPIO_data["GPIO_pindirs"][pin] != 1:
                # the pin is configured as an output! Warning!
                self.sm_warning_messages.append("Warning: external input applied to GPIO "+str(pin)+" but it is configured as output (external wins!), continuing\n")