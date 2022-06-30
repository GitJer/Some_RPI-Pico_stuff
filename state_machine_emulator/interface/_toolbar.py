from tkinter import Frame, Button

"""
    build the toolbar with associated call back functions and key bindings
"""

def enable_disable_buttons(self):
    """ enable or disable buttons and keybindings depending on time step and max time step """

    
    # disable 50 step back button if not possible
    self.step_50_back_button["state"] = "disabled" if self.current_clock < 50 else "normal"

    # disable 10 step back button and keybinding if not possible, otherwise enable
    self.step_10_back_button["state"] = "disabled" if self.current_clock < 10 else "normal"

    # disable step back button and keybinding if not possible, otherwise enable 
    self.step_back_button["state"] = "disabled" if self.current_clock == 0 else "normal"

    # disable step button and keybinding if not possible, otherwise enable 
    self.step_button["state"] = "normal" if self.current_clock < self.max_clock-1 else "disabled"

    # disable 10 step button if not possible
    self.step_10_button["state"] = "normal" if self.current_clock < self.max_clock-10 else "disabled"

    # disable 50 step button if not possible
    self.step_50_button["state"] = "normal" if self.current_clock < self.max_clock-51 else "disabled"


def reload_callback(self, event):
    """ if the 'reload' button is pressed, start from the beginning, reloading all files """
    self.reload_flag = True
    self.__del__()


def restart_callback(self, event):
    """ if the 'restart' button is pressed, start from t=0 """
    self.current_clock = 0
    self.enable_disable_buttons()
    self.update_display()


def quit_callback(self, event):
    """ if the 'quit' button is pressed, quit the program """
    self.__del__()


def step_50_back_callback(self):
    """ take 50 steps back in time (if possible) """
    if self.current_clock >= 50:
        self.current_clock -= 50
    else:
        self.current_clock = 0
    self.enable_disable_buttons()
    self.update_display()


def step_10_back_callback(self, event):
    """ take 10 steps back in time (if possible) """
    if self.current_clock >= 10:
        self.current_clock -= 10
    else:
        self.current_clock = 0
    self.enable_disable_buttons()
    self.update_display()


def step_back_callback(self, event):
    """ take one step back in time (if possible) """
    self.current_clock -= 1 if self.current_clock > 0 else 0 
    self.enable_disable_buttons()
    self.update_display()


def step_callback(self, event):
    """ take one step forward in time (if possible) """
    self.current_clock += 1 if self.current_clock < self.max_clock-1 else 0
    self.enable_disable_buttons()
    self.update_display()


def step_10_callback(self, event):
    """ take 10 steps forward in time (if possible) """
    if self.current_clock < self.max_clock-10:
        self.current_clock += 10
    else:
        self.current_clock = self.max_clock-1
    self.enable_disable_buttons()
    self.update_display()


def step_50_callback(self):
    """ take 50 steps forward in time (if possible) """
    if self.current_clock < self.max_clock-50:
        self.current_clock += 50
    else:
        self.current_clock = self.max_clock-1
    self.enable_disable_buttons()
    self.update_display()


def build_toolbar(self):
    """ build the toolbar """

    # build the toolbar
    self.toolbar = Frame(self.root, borderwidth=2, relief='raised')
    self.toolbar.grid(row=0, columnspan=4, padx=0, pady=2, sticky="EW")

    # build the buttons in the toolbar
    self.reload_button = Button(self.toolbar, text="reLoad", command=lambda: self.reload_callback(None))
    self.reload_button.pack(side="left", fill="none")

    self.restart_button = Button(self.toolbar, text="Restart", command=lambda: self.restart_callback(None))
    self.restart_button.pack(side="left", fill="none")

    self.step_50_back_button = Button(self.toolbar, text="back 50", command=lambda: self.step_50_back_callback())
    self.step_50_back_button.pack(side="left", fill="none")

    self.step_10_back_button = Button(self.toolbar, text="back 10\u2193", command=lambda: self.step_10_back_callback(None))
    self.step_10_back_button.pack(side="left", fill="none")

    self.step_back_button = Button(self.toolbar, text="back\u2190", command=lambda: self.step_back_callback(None))
    self.step_back_button.pack(side="left", fill="none")

    self.step_button = Button(self.toolbar, text="step\u2192", command=lambda: self.step_callback(None))
    self.step_button.pack(side="left", fill="none")

    self.step_10_button = Button(self.toolbar, text="step 10\u2191", command=lambda: self.step_10_callback(None))
    self.step_10_button.pack(side="left", fill="none")

    self.step_50_button = Button(self.toolbar, text="step 50", command=lambda: self.step_50_callback())
    self.step_50_button.pack(side="left", fill="none")

    self.quit_button = Button(self.toolbar, text="Quit", command=lambda: self.quit_callback(None))
    self.quit_button.pack(side="right", fill="none")

    # prepare key binding and unbinding of back button and step button (in 'enable_disable_buttons()')
    self.root.bind('<Down>', self.step_10_back_callback)
    self.root.bind('<Left>', self.step_back_callback)
    self.root.bind('<Right>', self.step_callback)
    self.root.bind('<Up>', self.step_10_callback)

    # bind key 'q' and 'Q' to quit
    self.root.bind('q', self.quit_callback)
    self.root.bind('Q', self.quit_callback)
    # bind key 'r' and 'R' to restart
    self.root.bind('r', self.restart_callback)
    self.root.bind('R', self.restart_callback)
    # bind key 'l' and 'L' to reload
    self.root.bind('l', self.reload_callback)
    self.root.bind('L', self.reload_callback)

    # enable or disable buttons depending on the current time step and max time step
    self.enable_disable_buttons()