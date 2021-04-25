from tkinter import *


def build_mid_frame(self):
    """ build the panel with the sm variables and settings """
    from interface.interface_item import Interface_Item_Var_Bits_32, Interface_Item_Var_List, Interface_Item_Pin_Settings_32

    # In the middle
    self.mid_frame = Frame(self.root, width=460, height=700)
    self.mid_frame.grid(row=1, column=1, padx=10, pady=2)
    self.mid_frame.grid_propagate(0)
    self.mid_frame.grid_columnconfigure(1, weight=1)

    grid_row = 2

    # RP2040 clock
    self.clock_label = Label(
        self.mid_frame, text="clock=0")
    self.clock_label.grid(row=grid_row, column=1, padx=(10, 10), sticky=W)
    grid_row += 1

    # IRQ
    self.IRQ_label = Interface_Item_Var_List(
        "IRQ", self.mid_frame, grid_row, 1, self.emulation_results, 4)
    grid_row += 1

    # PIO pc
    self.pc_label = Label(
        self.mid_frame, text="pc=0")
    self.pc_label.grid(row=grid_row, column=1, padx=(10, 10), sticky=W)
    grid_row += 1

    # Wait cycles to complete
    self.delay_label = Label(self.mid_frame, text="delay=0")
    self.delay_label.grid(row=grid_row, column=1, padx=(10, 10), sticky=W)
    grid_row += 1

    # TODO: status (not used in other parts of the code!)
    self.status_label = Label(self.mid_frame, text="status=0")
    self.status_label.grid(row=grid_row, column=1, padx=(10, 10), sticky=W)
    grid_row += 1

    # OSR
    self.OSR_label = Interface_Item_Var_Bits_32(
        "OSR", "OSR", self.mid_frame, grid_row, 1, self.emulation_results, 2)
    grid_row += 1

    # ISR
    self.ISR_label = Interface_Item_Var_Bits_32(
        "ISR", "ISR", self.mid_frame, grid_row, 1, self.emulation_results, 2)
    grid_row += 1

    # x
    self.X_label = Interface_Item_Var_Bits_32(
        "X", "x", self.mid_frame, grid_row, 1, self.emulation_results, 2)
    grid_row += 1

    # y
    self.Y_label = Interface_Item_Var_Bits_32(
        "Y", "y", self.mid_frame, grid_row, 1, self.emulation_results, 2)
    grid_row += 1

    # GPIO # TODO: indicate externally or internally driven?
    # e.g. -1 (not driven), H (externally driven), 1 (internally driven), L (external), 0 (internal), E (conflict between internal and external)
    self.GPIO_label = Interface_Item_Var_List(
        "GPIO", self.mid_frame, grid_row, 1, self.emulation_results, 0)
    grid_row += 1

    # Label to indicate pin configuration
    self.pin_config_label = Label(self.mid_frame, text="Pin configuration:")
    self.pin_config_label.grid(row=grid_row, column=1, padx=(10, 10), sticky=W)
    grid_row += 1

    # GPIO pindirs
    self.GPIO_pindirs_label = Interface_Item_Var_List(
        "GPIO pindir", self.mid_frame, grid_row, 1, self.emulation_results, 1)
    grid_row += 1

    # IN_pins
    self.in_pins_label = Interface_Item_Pin_Settings_32(
        "in pins", "in_base", None, self.mid_frame, grid_row, 1, self.emulation_results, 3)
    grid_row += 1

    # OUT_pins
    self.out_pins_label = Interface_Item_Pin_Settings_32(
        "out pins", "out_base", "out_count", self.mid_frame, grid_row, 1, self.emulation_results, 3)
    grid_row += 1

    # SET_pins
    self.set_pins_label = Interface_Item_Pin_Settings_32(
        "set pins", "set_base", "set_count", self.mid_frame, grid_row, 1, self.emulation_results, 3)
    grid_row += 1

    # SIDESET_pins
    self.sideset_pins_label = Interface_Item_Pin_Settings_32(
        "sideset pins", "sideset_base", "sideset_count", self.mid_frame, grid_row, 1, self.emulation_results, 3)
    grid_row += 1

    # JMP_PIN
    self.jmp_pin_label = Interface_Item_Pin_Settings_32(
        "JMP pin", "jmp_pin", None, self.mid_frame, grid_row, 1, self.emulation_results, 3)
    grid_row += 1

    # settings_box/list
    # TODO: update
    # only some settings are to be displayed
    list_of_settings_to_be_displayed = [
        "in_shift_right", "in_shift_autopush", "push_threshold", "out_shift_right", "out_shift_autopull", "pull_threshold", "sideset_opt", "sideset_pindirs"]
    temp = Label(self.mid_frame, text="Settings=")
    temp.grid(row=grid_row, column=1, padx=(10, 10), sticky=W)
    self.settings_listbox = Listbox(self.mid_frame, height=10, width=40)
    settings = self.emulation_results[0][3]
    for k, v in settings.items():
        if k in list_of_settings_to_be_displayed:
            self.settings_listbox.insert(END, k + " = " + str(v))
    grid_row += 1
    self.settings_listbox.grid(row=grid_row, column=1, padx=(10, 10))


def update_mid_frame(self):
    """ update the frame with the sm variables and settings """
    vars = self.emulation_results[self.current_clock][2]
    self.clock_label['text'] = "clock=" + str(self.current_clock)
    self.pc_label['text'] = "pc=" + str(vars['pc'])
    self.delay_label['text'] = "delay=" + str(vars['delay'])
    self.OSR_label.update(self.current_clock)
    self.ISR_label.update(self.current_clock)
    self.X_label.update(self.current_clock)
    self.Y_label.update(self.current_clock)
    self.GPIO_label.update(self.current_clock)
    self.GPIO_pindirs_label.update(self.current_clock)
    self.IRQ_label.update(self.current_clock)
    self.in_pins_label.update(self.current_clock)
    self.out_pins_label.update(self.current_clock)
    self.set_pins_label.update(self.current_clock)
    self.sideset_pins_label.update(self.current_clock)
    self.jmp_pin_label.update(self.current_clock)
