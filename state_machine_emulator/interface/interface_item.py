from tkinter import *

"""
All the classes here are an element of the GUI. They all have a setup (__init__) and update.
"""


class Interface_Item_Var_Bits_32:
    def __init__(self, displ_name, var_name, frame, row, col, var, var_index):
        self.displ_name = displ_name
        self.var_name = var_name
        self.var = var
        self.var_index = var_index
        label = Label(frame, text=displ_name + " = ")
        label.grid(row=row, column=col, padx=(10, 10), sticky=W)
        self.value_label = Label(frame, text=self.value_string(0))
        self.value_label.grid(row=row, column=col, padx=(10, 10), sticky=E)

    def update(self, clock):
        self.value_label['text'] = self.value_string(clock)

    def value_string(self, clock):
        value_string = str(
            self.var[clock][self.var_index][self.var_name]) + " = "
        value = self.var[clock][self.var_index][self.var_name]
        for i in reversed(range(32)):
            if (value & (1 << i)) == 0:
                value_string += "0"
            else:
                value_string += "1"
        return value_string


class Interface_Item_Var_List:
    def __init__(self, displ_name, frame, row, col, var, var_index):
        self.displ_name = displ_name
        self.var = var
        self.var_index = var_index
        label = Label(frame, text=displ_name + " = ")
        label.grid(row=row, column=col, padx=(10, 10), sticky=W)
        self.value_label = Label(frame, text=self.value_string(0))
        self.value_label.grid(row=row, column=col, padx=(10, 10), sticky=E)

    def update(self, clock):
        self.value_label['text'] = self.value_string(clock)

    def value_string(self, clock):
        value_string = ""
        for v in reversed(self.var[clock][self.var_index]):
            if v == -1:
                value_string += "x"
            elif v == 0:
                value_string += "0"
            else:
                value_string += "1"
        return value_string


class Interface_Item_Listbox_Bits:
    def __init__(self, displ_name, var_name, frame, row, col, var, clock):
        self.displ_name = displ_name
        self.var_name = var_name
        self.var = var
        label = Label(frame, text=displ_name)
        label.grid(row=row, column=col, padx=(10, 10), sticky=W)
        self.value_listbox = Listbox(
            frame, height=4, width=45, justify=RIGHT, exportselection=0)
        for index in range(4):
            self.value_listbox.insert(END, self.value_string(index, clock))
        self.value_listbox.grid(row=row+1, column=0, padx=(10, 10))

    def update(self, clock):
        for index in range(4):
            self.value_listbox.delete(0)
        for index in range(4):
            self.value_listbox.insert(END, self.value_string(index, clock))

    def value_string(self, index, clock):
        value_string = str(self.var[clock][2][self.var_name][index]) + " = "
        value = self.var[clock][2][self.var_name][index]
        for i in reversed(range(32)):
            if (value & (1 << i)) == 0:
                value_string += "0"
            else:
                value_string += "1"
        return value_string


class Interface_Item_Listbox_Time:
    def __init__(self, displ_name, frame, row, col, var):
        self.displ_name = displ_name
        self.var = var
        label = Label(frame, text=displ_name)
        label.grid(row=row, column=col, padx=(10, 10), sticky=W)
        self.value_listbox = Listbox(
            frame, height=10, width=45, exportselection=0)
        for index in range(len(var)):
            self.value_listbox.insert(END, self.value_string(index))
        self.value_listbox.grid(row=row+1, column=0, padx=(10, 10))

    def update(self):
        for index in range(4):
            self.value_listbox.delete(0)
        for index in range(4):
            self.value_listbox.insert(END, self.value_string(index))

    def value_string(self, index):
        value_string = str(self.var[index][0]) + \
            " : " + self.var[index][1] + "("
        first = True
        for l in self.var[index][2:]:
            value_string += "" if first else ", "
            first = False
            value_string += str(l)
        value_string += ")"
        return value_string
