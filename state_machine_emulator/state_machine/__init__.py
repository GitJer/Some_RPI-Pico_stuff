from config import STATEMACHINE_NUMBER

class state_machine:
    """ this class emulates a state machine (sm) """

    from ._time_step import time_step
    from ._push_pull import push_to_RxFIFO, pull_from_TxFIFO
    from ._do_sideset import do_sideset
    from ._set_all_GPIO import set_all_GPIO
    from ._execute_instructions import execute_instruction, execute_jmp, execute_wait, execute_in, execute_out, execute_push, execute_pull, execute_mov, execute_irq, execute_set


    def __init__(self, program_definitions):
        """ make the variables (such as x and y registers) and settings
            and make some class variables needed for correct execution
        """
        # Note: the Pico has two PIOs with 4 state machines each, but this emulator only has one state machine!
        # You can still set the state_machine_number (in config.py) because it is used in some irq operations

        # the sm number (used in irq calculations)
        self.sm_number = STATEMACHINE_NUMBER
        # the pio program and important parameters
        self.program = program_definitions['pio_program']
        self.wrap_target = program_definitions['pio_program_wrap_target']
        self.wrap = program_definitions['pio_program_wrap']
        # RP2040 clock
        self.clock = 0
        # define the irq
        self.sm_irq = [0 for _ in range(8)]

        # data related to the GPIO (the GPIO themselves, the pindir, external driven GPIO, out, set and sideset)
        self.GPIO_data = {}
        # the Pico has 32 GPIO
        self.GPIO_data["GPIO"] = [-1 for _ in range(32)]
        # pindir = 1 means Input (default); the '1' looks like an 'I' from 'Input'  
        # pindir = 0 means output; the '0' looks like an 'O' from 'Output'
        self.GPIO_data["GPIO_pindirs"] = [1 for _ in range(32)]
        # externally driven pins
        self.GPIO_data["GPIO_external"] = [-1 for _ in range(32)]
        # pins driven by 'out'
        self.GPIO_data["GPIO_out"] = [-1 for _ in range(32)]
        # pins driven by 'set'
        self.GPIO_data["GPIO_set"] = [-1 for _ in range(32)]
        # pins driven by 'sideset'
        self.GPIO_data["GPIO_sideset"] = [-1 for _ in range(32)]
        
        # the variables used by the sm
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
        self.vars["OSR_shift_counter"] = 32
        self.vars["pc"] = -1 # before executing any instruction a '+= 1' is done in time_step()
        self.vars["delay"] = 0
        self.vars["status"] = -1

        # settings that control how sm functions are executed
        self.settings = {}
        self.settings["in_shift_right"] = True      # right
        self.settings["in_shift_autopush"] = False
        self.settings["push_threshold"] = 32        # false
        self.settings["out_shift_right"] = False    # left (True = right)
        self.settings["out_shift_autopull"] = False
        self.settings["pull_threshold"] = 32        # false
        self.settings["in_base"] = -1       # indicate it is not set
        self.settings["jmp_pin"] = -1       # indicate it is not set
        self.settings["set_base"] = -1      # indicate it is not set
        self.settings["set_count"] = 0      # no pins assigned
        self.settings["out_base"] = -1      # indicate it is not set
        self.settings["out_count"] = 0      # no pins assigned
        self.settings["sideset_base"] = -1  # indicate it is not set
        self.settings["sideset_count"] = 0  # no pins assigned
        self.settings["sideset_opt"] = False
        self.settings["sideset_pindirs"] = False
        self.settings["FIFO_level_N"] = 0
        self.settings["status_sel"] = 0 # 0 => status ~0x0 if TxFIFO_count < FIFO_LEVEL_N
                                        # 1 => status ~0x0 if RxFIFO_count < FIFO_LEVEL_N

        # for e.g. jmp the pc should not be updated after an execution step since it is set by the jmp
        # also for e.g. wait statements the pc should remain unchanged
        self.skip_increase_pc = False # clear flag
        # for some statements the delay should be postponed to after the instruction (e.g. wait) has finished
        self.delay_delay = False
        # the jmp statement changes the vars["pc"] immediately when issued. But I want to keep the 'old' pc until it is time to change the pc in 'time_step()'
        self.jmp_to = -1 # not set
        # flags that indicate whether the sm is waiting because either pull or push is stalling (TxFIFO empty / RxFIFO full)
        self.pull_is_stalling = False
        self.push_is_stalling = False
        # indicates if the irq instruction is already waiting for clearing of the irq
        self.irq_is_waiting = False