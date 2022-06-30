from tkinter import Frame, Label, Listbox, Text


def build_output_frame(self):
    """ build the frame with the output (of c-statements) of the emulation """
    # the output frame
    self.output_frame = Frame(self.root, width=425, height=725)
    self.output_frame.grid(row=1, column=3, padx=0, pady=2)
    self.output_frame.grid_propagate(0)

    # program output label
    self.program_label = Label(self.output_frame, text="Highlight is just produced output from RxFIFO:")
    self.program_label.grid(row=2, column=3, padx=(5, 5), sticky='W')

    # program output is in a listbox
    self.output_listbox = Listbox(self.output_frame, height=26, width=50, justify="right", exportselection=0)
    self.output_listbox.grid(row=3, column=3, padx=(5, 5), sticky='E')

    # put the c-program into the listbox (in update_output_frame the current line will be highlighted (selected) in the GUI)
    for line in self.emulation_output_c_program:
        self.output_listbox.insert("end", line)

    # messages label
    self.program_label = Label(self.output_frame, text="Messages")
    self.program_label.grid(row=4, column=3, padx=(5, 5), sticky='W')

    # messages
    self.messages_text = Text(self.output_frame, height=11, width=50, exportselection=0)
    self.messages_text.grid(row=5, column=3, padx=(5, 5), sticky='E')


def update_output_frame(self):
    """ update the panel with the output (of c-statements) """
    # highlight the just executed output
    self.output_listbox.selection_clear(0, "end")
    for index in self.emulation_results[self.current_clock][6]:
        self.output_listbox.selection_set(index)
        self.output_listbox.see(index)

    # put the messages produced at the current clock here
    self.messages_text.delete("1.0", "end")
    for line in self.emulation_results[self.current_clock][7]:
        self.messages_text.insert("end", "-> " + line)
