// Include MicroPython API.
#include "py/runtime.h"

// Declare the function we'll make available in Python as cppexample.cppfunc().
// FIXME: start shouldn't be necessary (see other files)
extern mp_obj_t start();
extern mp_obj_t debounce_gpio(mp_obj_t mp_gpio);
extern mp_obj_t set_debounce_time(mp_obj_t mp_gpio, mp_obj_t mp_debounce_time);
extern mp_obj_t read(mp_obj_t mp_gpio);
extern mp_obj_t undebounce_gpio(mp_obj_t mp_gpio);
