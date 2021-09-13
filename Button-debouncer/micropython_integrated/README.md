# Button debouncer using the Raspberry Pico PIO integrated into MicroPython

## Note
This is a version of the button debounce program that has to be compiled into MicroPython. 
This is not what I want, but this worked and I'm still working on a version that is an external .mpy file that can be imported.

## How-to compile this code into MicroPython
This is a how-to for integrating cpp programs such as the button_debounce class into MicroPython.

Starting point:
- there is a working micropython environment for the RPI Pico
- there is a working cpp program (in this case: button_debounce)

Go to the rp2 port in the micropython directory:
```
cd /YOUR_PATH/micropython/ports/rp2
```

Test that compiling it produces a working micropython binary:
```
make clean
make
ls -l build-PICO/
```
The file 'firmware.uf2' should be present (and just built)

The module to be integrated in micropython will be located in the examples directory
```
cd /YOUR_PATH/micropython/examples/usercmodule
mkdir button_debounce
cd button_debounce
```
**The next text isn't needed if you use the files in this github repository! Skip to 'Add to overall cmake file for user-c-modules'**

Copy the cpp files to this directory. Do not forget to include the generated .pio.h file if needed:
```
cp /PATH_TO_ORIGINAL_CPP_CODE/button_debounce.* .
```

Now make the following files:
```
touch debounce.cpp
touch debouncemodule.h
touch debouncemodule.c
touch micropython.cmake
```
The content of these files is:
debounce.cpp:
```
extern "C" {
#include <debouncemodule.h>
#include "button_debounce.h"

Debounce debounce;

mp_obj_t debounce_gpio(mp_obj_t mp_gpio) {
    const auto gpio = mp_obj_get_int(mp_gpio);
    return mp_obj_new_int(debounce.debounce_gpio(gpio));
}

mp_obj_t set_debounce_time(mp_obj_t mp_gpio, mp_obj_t mp_debounce_time) {
    const auto gpio = mp_obj_get_int(mp_gpio);
    const auto debounce_time = mp_obj_get_float(mp_debounce_time);
    return mp_obj_new_int(debounce.set_debounce_time(gpio, debounce_time));
}

mp_obj_t read(mp_obj_t mp_gpio) {
    const auto gpio = mp_obj_get_int(mp_gpio);
    return mp_obj_new_int(debounce.read(gpio));
}

mp_obj_t undebounce_gpio(mp_obj_t mp_gpio) {
    const auto gpio = mp_obj_get_int(mp_gpio);
    return mp_obj_new_int(debounce.undebounce_gpio(gpio));
}
}
```
debouncemodule.h:
```
// Include MicroPython API.
#include "py/runtime.h"

// Declare the function we'll make available in Python as cppexample.cppfunc().
extern mp_obj_t debounce_gpio(mp_obj_t mp_gpio);
extern mp_obj_t set_debounce_time(mp_obj_t mp_gpio, mp_obj_t mp_debounce_time);
extern mp_obj_t read(mp_obj_t mp_gpio);
extern mp_obj_t undebounce_gpio(mp_obj_t mp_gpio);
```
debouncemodule.c:
```
#include <debouncemodule.h>

// Define a Python reference to the function we'll make available.
// See example.cpp for the definition.
STATIC MP_DEFINE_CONST_FUN_OBJ_1(debounce_gpio_obj, debounce_gpio);
STATIC MP_DEFINE_CONST_FUN_OBJ_2(set_debounce_time_obj, set_debounce_time);
STATIC MP_DEFINE_CONST_FUN_OBJ_1(read_obj, read);
STATIC MP_DEFINE_CONST_FUN_OBJ_1(undebounce_gpio_obj, undebounce_gpio);

// Define all properties of the module.
// Table entries are key/value pairs of the attribute name (a string)
// and the MicroPython object reference.
// All identifiers and strings are written as MP_QSTR_xxx and will be
// optimized to word-sized integers by the build system (interned strings).
STATIC const mp_rom_map_elem_t debounce_module_globals_table[] = {
    { MP_ROM_QSTR(MP_QSTR___name__), MP_ROM_QSTR(MP_QSTR_debounce) },
    { MP_ROM_QSTR(MP_QSTR_debounce_gpio), MP_ROM_PTR(&debounce_gpio_obj) },
    { MP_ROM_QSTR(MP_QSTR_set_debounce_time), MP_ROM_PTR(&set_debounce_time_obj) },
    { MP_ROM_QSTR(MP_QSTR_read), MP_ROM_PTR(&read_obj) },
    { MP_ROM_QSTR(MP_QSTR_undebounce_gpio), MP_ROM_PTR(&undebounce_gpio_obj) },
};
STATIC MP_DEFINE_CONST_DICT(debounce_module_globals, debounce_module_globals_table);

// Define module object.
const mp_obj_module_t debounce_user_cmodule = {
    .base = { &mp_type_module },
    .globals = (mp_obj_dict_t *)&debounce_module_globals,
};

// Register the module to make it available in Python.
// Note: the "1" in the third argument means this module is always enabled.
// This "1" can be optionally replaced with a macro like MODULE_CPPEXAMPLE_ENABLED
// which can then be used to conditionally enable this module.
MP_REGISTER_MODULE(MP_QSTR_debounce, debounce_user_cmodule, 1);
```
micropython.cmake:
```
add_library(usermod_debounce INTERFACE)
target_sources(usermod_debounce INTERFACE
    ${CMAKE_CURRENT_LIST_DIR}/button_debounce.cpp
    ${CMAKE_CURRENT_LIST_DIR}/debounce.cpp
    ${CMAKE_CURRENT_LIST_DIR}/debouncemodule.c
)
target_include_directories(usermod_debounce INTERFACE
    ${CMAKE_CURRENT_LIST_DIR}
)
target_link_libraries(usermod INTERFACE usermod_debounce)
```
## Add to overall cmake file for user-c-modules
In the directory with usercmodules add the button_debounce to the micropython.cmake file (you can use any editor, I used vi):
```
vi /YOUR_PATH/micropython/examples/usercmodule/micropython.cmake
```
This file now looks like:
```
# This top-level micropython.cmake is responsible for listing
# the individual modules we want to include.
# Paths are absolute, and ${CMAKE_CURRENT_LIST_DIR} can be
# used to prefix subdirectories.

# Add the C example.
# include(${CMAKE_CURRENT_LIST_DIR}/cexample/micropython.cmake)
# Add the CPP example.
# include(${CMAKE_CURRENT_LIST_DIR}/cppexample/micropython.cmake)
# Add the module.
include(${CMAKE_CURRENT_LIST_DIR}/button_debounce/micropython.cmake)
```
## Compiling
Go to the rp2 port in the micropython directory and make with the option to include the user_c_modules:
```
cd /YOUR_PATH/micropython/ports/rp2
make USER_C_MODULES=../../examples/usercmodule/micropython.cmake
```

This should result in a firmware.uf2 that includes the button_debounce module. 

## Running
The firmware must be uploaded to the pico (turn off, button pressed, turn on, file explorer pops up, move firmware.uf2 into it).
Thonny can be used to interact with micropython. 

I made the following script and uploaded it to the pico:
```
import debounce
import time
debounce.start()
debounce.debounce_gpio(15)
while True:
    print(debounce.read(15))
    time.sleep(0.1)
```
Note the 'debounce.start'. This is due to a problem I haven't solved yet: micropython loads the module with 'import debounce' but at a restart of a script using it, it doesn't run the constructor of the Debounce class again, therefore not initializing the variables. Thus, I've made a function 'start' to explicitly initialize the class variables.

