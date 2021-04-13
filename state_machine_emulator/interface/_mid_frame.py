from tkinter import *


def build_mid_frame(self):
    """ build the panel with the sm variables and settings """
    from interface.interface_item import Interface_Item_Var_Bits_32, Interface_Item_Var_List

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

    # status
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

    # GPIO
    self.GPIO_label = Interface_Item_Var_List(
        "GPIO", self.mid_frame, grid_row, 1, self.emulation_results, 0)
    grid_row += 1

    # GPIO pindirs
    self.GPIO_pindirs_label = Interface_Item_Var_List(
        "GPIO pindir", self.mid_frame, grid_row, 1, self.emulation_results, 1)
    grid_row += 1

    # IN_pins
    temp = Label(self.mid_frame, text="IN_pins=")
    temp.grid(row=grid_row, column=1, padx=(10, 10), sticky=W)
    self.IN_pins_label = Label(
        self.mid_frame, text="0 = 00000000000000000000000000000000")
    self.IN_pins_label.grid(row=grid_row, column=1,
                            padx=(10, 10), sticky=E)
    grid_row += 1

    # OUT_PINS
    temp = Label(self.mid_frame, text="OUT_PINS=")
    temp.grid(row=grid_row, column=1, padx=(10, 10), sticky=W)
    self.OUT_PINS_label = Label(
        self.mid_frame, text="0 = 00000000000000000000000000000000")
    self.OUT_PINS_label.grid(
        row=grid_row, column=1, padx=(10, 10), sticky=E)
    grid_row += 1

    # SET_PINS
    temp = Label(self.mid_frame, text="SET_PINS=")
    temp.grid(row=grid_row, column=1, padx=(10, 10), sticky=W)
    self.SET_PINS_label = Label(
        self.mid_frame, text="0 = 00000000000000000000000000000000")
    self.SET_PINS_label.grid(
        row=grid_row, column=1, padx=(10, 10), sticky=E)
    grid_row += 1

    # settings_box/list
    # TODO: update
    temp = Label(self.mid_frame, text="Settings=")
    temp.grid(row=grid_row, column=1, padx=(10, 10), sticky=W)
    # self.previous_selected_settings = None
    self.settings_listbox = Listbox(self.mid_frame, height=20, width=40)
    settings = self.emulation_results[0][3]
    for k, v in settings.items():
        self.settings_listbox.insert(END, k + " = " + str(v))
    grid_row += 1
    self.settings_listbox.grid(row=grid_row, column=1, padx=(10, 10))


def update_mid_frame(self):
    """ update the frame with the sm variables and settings """
    vars = self.emulation_results[self.current_clock][2]
    self.clock_label['text'] = "clock=" + str(self.current_clock)
    self.pc_label['text'] = "pc=" + str(vars['pc'])
    self.OSR_label.update(self.current_clock)
    self.ISR_label.update(self.current_clock)
    self.X_label.update(self.current_clock)
    self.Y_label.update(self.current_clock)
    self.GPIO_label.update(self.current_clock)
    self.GPIO_pindirs_label.update(self.current_clock)
    self.IRQ_label.update(self.current_clock)
