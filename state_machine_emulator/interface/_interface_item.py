from tkinter import Text, Label, Listbox
from interface._tooltips import CreateToolTip

"""
All the classes here are an element of the GUI. They all have a setup (__init__), a method to make a string to be used and update.
"""

class Interface_Item:
    def __init__(self, frame, display_name, row, col, width):
        self.display_name = display_name
        self.row = row
        self.col = col
        self.width = width
        # make the Text widget for the name of the variable to be displayed
        label = Text(frame, height=1, width=12)
        label.insert("end", self.display_name)
        label.configure(font="TkFixedFont", state="disabled")
        label.grid(row=self.row, column=self.col, padx=(5, 5), sticky='W')
        # make the Text widget for the value of the variable
        self.value_label = Text(frame, height=1, width=self.width)
        self.value_label.insert("end", self.value_string(0))
        self.value_label.configure(font="TkFixedFont", state="disabled")
        self.value_label.grid(row=self.row, column=self.col, padx=(5, 5), sticky='E')
        # make tags to be used in showing which characters have changed, and which have not
        self.value_label.tag_config("changed", foreground="Blue")
        self.value_label.tag_config("unchanged", foreground="Black")

    def update(self, clock):
        # save the current value
        old_value = self.value_label.get("1.0", "end").strip()
        # allow the widget text to be changed, and delete the existing content
        self.value_label.configure(state="normal")
        self.value_label.delete("1.0", "end")
        # determine the new text for the widget, and insert it
        new_value = self.value_string(clock)
        self.value_label.insert("end", new_value)
        # disallow changing the text, justify right
        self.value_label.configure(state="disabled")
        self.value_label.tag_configure("j_right", justify='right')
        self.value_label.tag_add("j_right", 1.0, "end")
        # now apply colors, using tags "changed and "unchanged", e.g. to the last 32 characters (the binary representation)
        len_string = min(len(new_value), len(old_value), 32)
        for i in range(-1,-len_string-1, -1):
            if old_value[i] != new_value[i]:
                self.value_label.tag_add("changed", f"{1}.{len(new_value)+i}") 
            else:
                self.value_label.tag_add("unchanged", f"{1}.{len(new_value)+i}") 

            
class Var_Bits_32(Interface_Item):
    def __init__(self, display_name, var_name, frame, row, col, var, var_index):
        self.var_name = var_name
        self.var = var
        self.var_index = var_index
        super().__init__(frame, display_name, row, col, 45)

    def value_string(self, clock):
        value_string = str(self.var[clock][self.var_index][self.var_name] & 0xFFFFFFFF) + " = "
        value = self.var[clock][self.var_index][self.var_name]
        # extend the value_string based on bits in 'value' 
        for i in reversed(range(32)):
            value_string += "0" if (value & (1 << i)) == 0 else "1"
        return value_string


class Pin_Settings_32(Interface_Item):
    def __init__(self, display_name, base_name, count_name, frame, row, col, var, var_index):
        self.base_name = base_name
        self.count_name = count_name
        self.var = var
        self.var_index = var_index
        super().__init__(frame, display_name, row, col, 40)

    def value_string(self, clock):
        base = self.var[clock][self.var_index][self.base_name]
        count = 0
        if self.count_name:
            count = self.var[clock][self.var_index][self.count_name]
            # dirty hack: sideset_count includes the sideset_opt bit, but this does not set a pin!
            if self.count_name == "sideset_count" and self.var[clock][self.var_index]["sideset_opt"]:
                count -= 1
        value_string_list = ['.' for i in range(32)]
        if base >= 0:
            value_string_list[31-base] = 'B'
        for i in range(count-1):
            value_string_list[31-(base+1+i) % 32] = 'C'
        value_string = ''.join(value_string_list)
        return value_string


class Var_List_IRQ(Interface_Item):
    def __init__(self, display_name, frame, row, col, var, var_index):
        self.var = var
        self.var_index = var_index
        super().__init__(frame, display_name, row, col, 40)

    def value_string(self, clock):
        value_string = ""
        for v in reversed(self.var[clock][self.var_index]):
            value_string += "1" if v==1 else "0" if v==0 else "."
        return value_string

class Var_List(Interface_Item):
    def __init__(self, display_name, frame, row, col, var, var_index):
        self.var = var
        self.var_index = var_index
        super().__init__(frame, display_name, row, col, 40)

    def value_string(self, clock):
        value_string = ""
        for v in reversed(self.var[clock][0][self.var_index]):
            value_string += "1" if v==1 else "0" if v==0 else "."
        return value_string


class Interface_Item_Listbox_Bits:
    def __init__(self, display_name, var_name, frame, row, col, var, clock):
        self.display_name = display_name
        self.var_name = var_name
        self.var = var
        label = Label(frame, text=display_name + ' \u24D8')
        CreateToolTip(label, \
            "The data in TxFIFO is transmitted from the normal core to the state machine.\n"
            "The data in RxFIFO is transmitted from the state machine to the normal core. ")
        label.grid(row=row, column=col, padx=(5, 5), sticky='W')
        self.value_listbox = Listbox(frame, height=4, width=45, justify="right", exportselection=0)
        for index in range(4):
            self.value_listbox.insert("end", self.value_string(index, clock))
        self.value_listbox.grid(row=row+1, column=0, padx=(5, 5))

    def update(self, clock):
        for index in range(4):
            self.value_listbox.delete(0)
        for index in range(4):
            self.value_listbox.insert("end", self.value_string(index, clock))

    def value_string(self, index, clock):
        value_string = str(self.var[clock][1][self.var_name][index] & 0xFFFFFFFF) + " = "
        value = self.var[clock][1][self.var_name][index]
        for i in reversed(range(32)):
            value_string += "0" if (value & (1 << i)) == 0 else "1"
        return value_string


class Interface_Item_Listbox_Time:
    def __init__(self, display_name, frame, row, col, var):
        self.display_name = display_name
        self.var = var
        label = Label(frame, text=display_name)
        label.grid(row=row, column=col, padx=(5, 5), sticky='W')
        self.value_listbox = Listbox(frame, height=13, width=45, exportselection=0)
        for index in range(len(var)):
            self.value_listbox.insert("end", self.value_string(index))
        self.value_listbox.grid(row=row+1, column=0, padx=(5, 5))

    def update(self):
        for index in range(4):
            self.value_listbox.delete(0)
        for index in range(4):
            self.value_listbox.insert("end", self.value_string(index))

    def value_string(self, index):
        value_string = str(self.var[index][0]) + " : " + self.var[index][1]
        first = True
        for l in self.var[index][2:]:
            value_string += "=" if first else ", "
            first = False
            value_string += str(l)
        return value_string
