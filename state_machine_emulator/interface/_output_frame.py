from tkinter import *


def build_output_frame(self):
    """ build the frame with the output (of c-statements) of the emulation """
    # the output frame
    self.output_frame = Frame(self.root, width=520, height=700)
    self.output_frame.grid(row=1, column=3, padx=10, pady=2)
    self.output_frame.grid_propagate(0)

    # label
    self.program_label = Label(
        self.output_frame, text="Output from RxFIFO:")
    self.program_label.grid(row=2, column=3, padx=(10, 10), sticky=W)

    # output is in listbox
    self.output_listbox = Listbox(
        self.output_frame, height=32, width=50, justify=RIGHT, exportselection=0)
    self.output_listbox.grid(row=3, column=3, padx=(10, 10), sticky=E)

    for line in self.emulation_output_c_program:
        self.output_listbox.insert(END, line)


def update_output_frame(self):
    """ update the panel with the output (of c-statements) """
    # highlight the just executed output
    self.output_listbox.selection_clear(0, END)
    for index in self.emulation_results[self.current_clock][7]:
        # print("update_output_frame index=", index)
        self.output_listbox.selection_set(index)
        self.output_listbox.see(index)
