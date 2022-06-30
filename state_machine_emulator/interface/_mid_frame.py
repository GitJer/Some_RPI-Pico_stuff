from tkinter import Frame, Label, Text
from config import STATEMACHINE_NUMBER

def build_mid_frame(self):
    """ build the panel with the sm variables and settings """
    from interface._interface_item import Var_Bits_32, Var_List_IRQ, Var_List, Pin_Settings_32
    from interface._tooltips import CreateToolTip

    # In the middle
    self.mid_frame = Frame(self.root, width=460, height=725)
    self.mid_frame.grid(row=1, column=1, padx=0, pady=2)
    self.mid_frame.grid_propagate(0)

    grid_row = 2

    # make a separate frame for some data items (clock, pc, delay, status)
    self.clk_pc_delay_stat_frame = Frame(self.mid_frame, width=460, height=25)
    self.clk_pc_delay_stat_frame.grid(row=grid_row, column=1, padx=0, pady=5)
    self.clk_pc_delay_stat_frame.grid_propagate(0)
    self.clk_pc_delay_stat_frame.grid_columnconfigure(index=0, weight=1)
    self.clk_pc_delay_stat_frame.grid_columnconfigure(index=1, weight=1)
    self.clk_pc_delay_stat_frame.grid_columnconfigure(index=2, weight=1)
    self.clk_pc_delay_stat_frame.grid_columnconfigure(index=3, weight=1)
    inset_col=0
    # clock
    self.clock_label = Label(self.clk_pc_delay_stat_frame, text="clock=0", font=("Arial", 14))
    self.clock_label.grid(row=1, column=inset_col, padx=(5, 5), sticky='W')
    inset_col += 1
    # PIO pc
    self.pc_label = Label(self.clk_pc_delay_stat_frame, text="pc=0")
    self.pc_label.grid(row=1, column=inset_col, padx=(5, 5), sticky='W')
    inset_col += 1
    # Wait cycles to complete
    self.delay_label = Label(self.clk_pc_delay_stat_frame, text="delay=0")
    self.delay_label.grid(row=1, column=inset_col, padx=(5, 5), sticky='W')
    inset_col += 1
    # status
    self.status_label = Label(self.clk_pc_delay_stat_frame, text="status=0")
    self.status_label.grid(row=1, column=inset_col, padx=(5, 5), sticky='W')

    grid_row += 1

    # make a separate frame for some data items (OSR and ISR shift counters)
    self.OSR_ISR_shift_counters_frame = Frame(self.mid_frame, width=460, height=25)
    self.OSR_ISR_shift_counters_frame.grid(row=grid_row, column=1, padx=0, pady=0)
    self.OSR_ISR_shift_counters_frame.grid_propagate(0)
    self.OSR_ISR_shift_counters_frame.grid_columnconfigure(index=0, weight=1)
    self.OSR_ISR_shift_counters_frame.grid_columnconfigure(index=1, weight=1)
    self.OSR_ISR_shift_counters_frame.grid_columnconfigure(index=2, weight=0)
    self.OSR_ISR_shift_counters_frame.grid_columnconfigure(index=3, weight=0)
    inset_col=0
    # OSR shift counter
    self.OSR_shift_counter_label = Label(self.OSR_ISR_shift_counters_frame, text="OSR shift counter=0")
    self.OSR_shift_counter_label.grid(row=1, column=inset_col, padx=(5, 5), sticky='W')
    inset_col += 1
    # ISR shift counter
    self.ISR_shift_counter_label = Label(self.OSR_ISR_shift_counters_frame, text="ISR shift counter=0")
    self.ISR_shift_counter_label.grid(row=1, column=inset_col, padx=(5, 5), sticky='W')
    
    # Now place the other data items in rows
    grid_row += 1
    # OSR
    self.OSR_label = Var_Bits_32("OSR", "OSR", self.mid_frame, grid_row, 1, self.emulation_results, 1)
    grid_row += 1
    # ISR
    self.ISR_label = Var_Bits_32("ISR", "ISR", self.mid_frame, grid_row, 1, self.emulation_results, 1)
    grid_row += 1
    # x
    self.X_label = Var_Bits_32("X", "x", self.mid_frame, grid_row, 1, self.emulation_results, 1)
    grid_row += 1
    # y
    self.Y_label = Var_Bits_32("Y", "y", self.mid_frame, grid_row, 1, self.emulation_results, 1)
    grid_row += 1
    # empty row
    self.empty_label1 = Label(self.mid_frame)
    self.empty_label1.grid(row=grid_row, column=1, padx=(5, 5), sticky='W')
    grid_row += 1
    # IRQ
    self.IRQ_label = Var_List_IRQ("IRQ (sm="+str(STATEMACHINE_NUMBER)+")", self.mid_frame, grid_row, 1, self.emulation_results, 3)
    grid_row += 1

    # pin configuration section label
    self.pin_config_label = Label(self.mid_frame, text="\nPin configuration: \u24D8")
    self.pin_config_label.grid(row=grid_row, column=1, padx=(5, 5), sticky='W')
    self.pin_config_label_ttp = CreateToolTip(self.pin_config_label, \
        "The GPIO output (first row below) is determined by (lowest priority) 'out' and 'set', or (mid priority) 'sideset'.\n"
        "The 'out', 'set' and 'sideset' also require pindir to be an output (a '0' in the third row below).\n"
        "The highest priority is an external signal (GPIO ext). This wins from any internal signal (and may damage the RPI pico!)")
    grid_row += 1
    # GPIO 
    self.GPIO_label = Var_List("GPIO", self.mid_frame, grid_row, 1, self.emulation_results, "GPIO")
    grid_row += 1
    # GPIO_external 
    self.GPIO_ext_label = Var_List("GPIO ext", self.mid_frame, grid_row, 1, self.emulation_results, "GPIO_external")
    grid_row += 1
    # GPIO pindirs
    self.GPIO_pindirs_label = Var_List("pindir", self.mid_frame, grid_row, 1, self.emulation_results, "GPIO_pindirs")
    grid_row += 1
    # SIDESET_pins
    self.sideset_vals_label = Var_List("sideset vals", self.mid_frame, grid_row, 1, self.emulation_results, "GPIO_sideset")
    grid_row += 1
    self.sideset_pins_label = Pin_Settings_32("sideset pins", "sideset_base", "sideset_count", self.mid_frame, grid_row, 1, self.emulation_results, 2)
    grid_row += 1
    # OUT_pins
    self.out_vals_label = Var_List("out vals", self.mid_frame, grid_row, 1, self.emulation_results, "GPIO_out")
    grid_row += 1
    self.out_pins_label = Pin_Settings_32("out pins", "out_base", "out_count", self.mid_frame, grid_row, 1, self.emulation_results, 2)
    grid_row += 1
    # SET_pins
    self.set_vals_label = Var_List("set vals", self.mid_frame, grid_row, 1, self.emulation_results, "GPIO_set")
    grid_row += 1
    self.set_pins_label = Pin_Settings_32("set pins", "set_base", "set_count", self.mid_frame, grid_row, 1, self.emulation_results, 2)
    grid_row += 1
    # IN_pins
    self.in_pins_label = Pin_Settings_32("in pins", "in_base", None, self.mid_frame, grid_row, 1, self.emulation_results, 2)
    grid_row += 1
    # JMP_PIN
    self.jmp_pin_label = Pin_Settings_32("jmp pin", "jmp_pin", None, self.mid_frame, grid_row, 1, self.emulation_results, 2)
    grid_row += 1
    # settings
    temp = Label(self.mid_frame, text="\nSettings:")
    temp.grid(row=grid_row, column=1, padx=(5, 5), sticky='W')
    grid_row += 1
    # place the settings that are to be displayed
    settings = self.emulation_results[0][2]
    self.list_of_settings_to_be_displayed = ["in_shift_right", "in_shift_autopush", "push_threshold", "out_shift_right", "out_shift_autopull", "pull_threshold", "sideset_opt", "sideset_pindirs"]
    self.settings_labels = [None]*len(self.list_of_settings_to_be_displayed)
    for i, s in enumerate(self.list_of_settings_to_be_displayed):
        label = Text(self.mid_frame, height=1, width=20)
        label.insert("end", s)
        label.configure(font="TkFixedFont", state="disabled")
        label.grid(row=grid_row, column=1, padx=(5, 5), sticky='W')
        self.settings_labels[i] = Text(self.mid_frame, height=1, width=35)
        self.settings_labels[i].insert("end", str(settings[s]))
        self.settings_labels[i].configure(font="TkFixedFont", state="disabled")
        self.settings_labels[i].grid(row=grid_row, column=1, padx=(5, 5), sticky='E')
        # make tags to be used in showing which characters have changed, and which have not
        self.settings_labels[i].tag_config("changed", foreground="Blue")
        self.settings_labels[i].tag_config("unchanged", foreground="Black")
        grid_row += 1


def update_mid_frame(self):
    """ update the frame with the sm variables and settings """
    vars = self.emulation_results[self.current_clock][1]

    self.clock_label['text'] = "clock=" + str(self.current_clock)
    self.pc_label['text'] = "pc=" + str(vars['pc'])
    self.delay_label['text'] = "delay=" + str(vars['delay'])
    self.status_label['text'] = "status=" + str(vars['status'])
    self.OSR_shift_counter_label['text'] = "OSR shift counter="+str(vars['OSR_shift_counter'])
    self.ISR_shift_counter_label['text'] = "ISR shift counter="+str(vars['ISR_shift_counter'])

    self.OSR_label.update(self.current_clock)
    self.ISR_label.update(self.current_clock)
    self.X_label.update(self.current_clock)
    self.Y_label.update(self.current_clock)
    self.GPIO_label.update(self.current_clock)
    self.GPIO_ext_label.update(self.current_clock)
    self.GPIO_pindirs_label.update(self.current_clock)
    self.IRQ_label.update(self.current_clock)
    self.in_pins_label.update(self.current_clock)
    self.out_pins_label.update(self.current_clock)
    self.out_vals_label.update(self.current_clock)
    self.set_vals_label.update(self.current_clock)
    self.set_pins_label.update(self.current_clock)
    self.sideset_pins_label.update(self.current_clock)
    self.sideset_vals_label.update(self.current_clock)
    self.jmp_pin_label.update(self.current_clock)


    # update settings
    settings = self.emulation_results[self.current_clock][2]
    for i, s in enumerate(self.list_of_settings_to_be_displayed):
        # save the current value for comparison with the new value
        old_value = self.settings_labels[i].get("1.0", "end").strip()
        # allow the widget text to be changed, and delete the existing content
        self.settings_labels[i].configure(state="normal")
        self.settings_labels[i].delete("1.0", "end")
        # determine the new text for the widget, and insert it
        self.settings_labels[i].insert("end", str(settings[s]))
        # disallow changing the text
        self.settings_labels[i].configure(state="disabled")

        # apply tags to give color to the changed data items
        if old_value != str(settings[s]):
            self.settings_labels[i].tag_add("changed", '1.0', '1.'+str(len(str(settings[s])))) 
        else:
            self.settings_labels[i].tag_add("unchanged", '1.0', '1.'+str(len(str(settings[s]))))
