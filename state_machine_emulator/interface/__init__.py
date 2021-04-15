import sys

from tkinter import *
from tkinter import filedialog
from PIL import Image, ImageTk

import rp2040
import emulation


class RPI_PICO_PIO_interface():
    """ This class builds the GUI to show the results of the emulation of the sm of a RP2040 """

    # Imported methods
    from ._toolbar import build_toolbar, step_callback, quit_callback, step_10_callback, step_50_callback, restart_callback, step_10_back_callback, step_50_back_callback, step_back_callback, reload_callback
    from ._left_frame import build_left_frame, update_left_frame
    from ._mid_frame import build_mid_frame, update_mid_frame
    from ._right_frame import build_right_frame, update_right_frame
    from ._output_frame import build_output_frame, update_output_frame

    def __init__(self, pio_program, pin_program, c_program, emulation_results, emulation_output_c_program):
        """ build and run the GUI """
        # the flag that can signal to the main function to reload all files
        self.reload_flag = False
        self.pio_program = pio_program
        self.pin_program = pin_program
        self.c_program = c_program
        self.emulation_results = emulation_results
        self.emulation_output_c_program = emulation_output_c_program
        self.current_clock = 0
        self.max_clock = len(self.emulation_results)
        # Make the window
        self.root = Tk()
        # Make the title that will appear in the top left
        self.root.wm_title("Raspberry PI PICO PIO Emulation")
        # set background color to white
        self.root.config(background="#FFFFFF")

        # this is just a simple activity to allow ctrl-c to function
        self.after_id = self.root.after(50, self.check)

        # make the frames: left for controls, right for graphs
        self.build_toolbar()
        self.build_left_frame()
        self.build_mid_frame()
        self.build_right_frame()
        self.build_output_frame()

        # update the display to reflect the data of self.current_clock=0
        self.update_display()
        # start the monitoring and updating
        self.root.mainloop()

    def check(self):
        """ this is just a simple activity to allow ctrl-c to function """
        self.after_id = self.root.after(50, self.check)  # 50 stands for 50 ms.

    def __del__(self):
        """ clean up at exit """
        self.root.after_cancel(self.after_id)
        try:
            self.root.quit()
            self.root.destroy()
        except:
            pass

    def get_reload_flag(self):
        return self.reload_flag

    def update_display(self):
        """ update the display at every step to show the emulation results for the new step """
        self.update_mid_frame()
        self.update_right_frame()
        self.update_left_frame()
        self.update_output_frame()
