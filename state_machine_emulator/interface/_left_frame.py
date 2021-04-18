from tkinter import *


def build_left_frame(self):
    """ build the panel on the left with the c-program, pin_program, and the TxFIFO and RxFIFO """
    from interface.interface_item import Interface_Item_Listbox_Bits, Interface_Item_Listbox_Time

    # on the left side
    self.left_frame = Frame(self.root, width=460, height=700)
    self.left_frame.grid(row=1, column=0, padx=10, pady=2)

    # start on row=2 with placing widgets
    grid_row = 2

    # pin states program
    self.pin_program_listbox = Interface_Item_Listbox_Time(
        "Highlight is just executed pin-program", self.left_frame, grid_row, 0, self.pin_program)
    grid_row += 2

    # c program
    self.c_program_listbox = Interface_Item_Listbox_Time(
        "Highlight is just executed C-program", self.left_frame, grid_row, 0, self.c_program)
    grid_row += 2

    # TxFIFO
    self.TxFIFO_listbox = Interface_Item_Listbox_Bits(
        "TxFIFO", "TxFIFO", self.left_frame, grid_row, 0, self.emulation_results, self.current_clock)
    grid_row += 2

    # RxFIFO
    self.RxFIFO_listbox = Interface_Item_Listbox_Bits(
        "RxFIFO", "RxFIFO", self.left_frame, grid_row, 0, self.emulation_results, self.current_clock)


def update_left_frame(self):
    """ update the left frame """
    # update the Tx and Rx FIFOs
    self.TxFIFO_listbox.update(self.current_clock)
    self.RxFIFO_listbox.update(self.current_clock)

    # highlight the just executed pin_program statements
    self.pin_program_listbox.value_listbox.selection_clear(0, END)
    # make sure the first highlighted item is visible by using 'see'
    first = True
    for index in self.emulation_results[self.current_clock][5]:
        self.pin_program_listbox.value_listbox.selection_set(index)
        if first:
            self.pin_program_listbox.value_listbox.see(index)
            first = False

    # highlight the just executed c_program statements
    self.c_program_listbox.value_listbox.selection_clear(0, END)
    # make sure the first highlighted item is visible by using 'see'
    first = True
    for index in self.emulation_results[self.current_clock][6]:
        self.c_program_listbox.value_listbox.selection_set(index)
        if first:
            self.c_program_listbox.value_listbox.see(index)
            first = False
