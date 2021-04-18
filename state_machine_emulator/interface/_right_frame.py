from tkinter import *


def build_right_frame(self):
    """ build the panel with the pio program """
    # on the right side
    self.right_frame = Frame(self.root, width=460, height=700)
    self.right_frame.grid(row=1, column=2, padx=10, pady=2)
    self.right_frame.grid_propagate(0)

    self.program_label = Label(
        self.right_frame, text="Highlight is just executed pio-code:")
    self.program_label.grid(row=1, column=2, padx=(10, 10), sticky=W)
    # PIO program
    self.code_listbox = Listbox(self.right_frame, height=32, width=40)
    for p in self.pio_program:
        self.code_listbox.insert(END, p[1])
    self.code_listbox.grid(row=2, column=2, padx=(10, 10), sticky=E)


def update_right_frame(self):
    """ update the panel on the right with the pio program """
    self.code_listbox.selection_clear(0, END)
    self.code_listbox.selection_set(
        self.emulation_results[self.current_clock][2]['pc'])
