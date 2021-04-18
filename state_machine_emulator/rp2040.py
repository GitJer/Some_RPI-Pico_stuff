import pio


class rp2040:
    """ this class emulates the rp2040 """

    def __init__(self, program_definitions):
        """ init makes the PIOs, GPIOs, and pindirs """

        # the Pico has two PIOs each with 4 sm
        self.PIO = [pio.pio("pio0", program_definitions, self),
                    pio.pio("pio1", program_definitions, self)]
        # the Pico has 32 GPIO
        self.GPIO = [0 for i in range(32)]
        # each GPIO has pin direction
        # TODO: should this be a character that can be set to 'I' and 'O' and '.' for not set?
        self.GPIO_pindirs = [-1 for i in range(32)]
        # RP2040 clock
        self.clock = 0

    def time_step(self):
        """ do one time step """
        self.PIO[0].time_step()
        self.clock += 1
