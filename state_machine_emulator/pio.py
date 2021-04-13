import state_machine


class pio:
    """ this class emulates the PIO (Programmable IO) """

    def __init__(self, name, program_definitions, rp2040):
        """ init makes the state machines (sm) and the interrupts """
        self.name = name
        self.rp2040 = rp2040
        self.program_definitions = program_definitions
        self.sm_irq = [0 for i in range(8)]
        self.sm = [state_machine.state_machine(
            i, self.program_definitions, self.rp2040) for i in range(4)]

    def time_step(self):
        """ do one time step """
        self.sm[0].time_step()
