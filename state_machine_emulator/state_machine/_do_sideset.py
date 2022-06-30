def do_sideset(self, instruction_delay_sideset):
    """ handle sideset """
    # TODO: is 3.5.1. fully covered? Especially the 4 points at the end of the section.
    # determine and execute the (optional) sideset
    # if sideset is optional the MSB is set when sideset should be executed:
    if self.settings["sideset_opt"]:
        # check if the MSB is set
        if instruction_delay_sideset & 0x10:
            # do the side step for sideset_count bits after the MSB
            # note: sideset_count includes the optional bit!
            for i in range(self.settings["sideset_count"]-1):
                test_ss_bit = 4-i-1
                value = 1 if instruction_delay_sideset & (1 << test_ss_bit) else 0
                # do the side step for sideset_base + i
                if self.settings["sideset_pindirs"]:
                    self.GPIO_data["GPIO_pindirs"][(self.settings["sideset_base"] + i) % 32] = value
                else:
                    self.GPIO_data["GPIO_sideset"][(self.settings["sideset_base"] + i) % 32] = value
    else:  # sideset is mandatory
        for i in range(self.settings["sideset_count"]):
            test_ss_bit = 5-i-1
            # if the bit is set, the GPIO (or pindir) has to be set; if not: clear the GPIO/pindir
            value = 1 if instruction_delay_sideset & (1 << test_ss_bit) else 0
            # do the side step for sideset_base + i
            if self.settings["sideset_pindirs"]:
                self.GPIO_data["GPIO_pindirs"][(self.settings["sideset_base"] + i) % 32] = value
            else:
                self.GPIO_data["GPIO_sideset"][(self.settings["sideset_base"] + i) % 32] = value
