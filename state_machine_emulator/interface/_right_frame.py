from tkinter import Frame, Label, Listbox


def build_right_frame(self):
    """ build the panel with the pio program """
    # on the right side
    self.right_frame = Frame(self.root, width=305, height=725)
    # self.right_frame = Frame(self.root, width=345, height=675)
    self.right_frame.grid(row=1, column=2, padx=0, pady=2)
    self.right_frame.grid_propagate(0)
    # self.right_frame.grid_columnconfigure(1, weight=1)

    self.program_label = Label(self.right_frame, text="Highlight is just executed pio-code:")
    self.program_label.grid(row=1, column=2, padx=(5, 5), sticky='W')
    # PIO program
    self.code_listbox = Listbox(self.right_frame, height=38, width=40)
    for index, p in enumerate(self.pio_program):
        if index == self.program_definitions['pio_program_wrap']:
            self.code_listbox.insert("end", p[1] + ' (wrap)')
        elif index == self.program_definitions['pio_program_wrap_target']:
            self.code_listbox.insert("end", p[1] + ' (wrap target)')
        elif index == self.program_definitions['pio_program_origin']:
            self.code_listbox.insert("end", p[1] + ' (origin)')
        else:
            self.code_listbox.insert("end", p[1])
    self.code_listbox.grid(row=2, column=2, padx=(5, 5), sticky='E')


def update_right_frame(self):
    """ update the panel on the right with the pio program """
    self.code_listbox.selection_clear(0, "end")
    self.code_listbox.selection_set(self.emulation_results[self.current_clock][1]['pc'])
