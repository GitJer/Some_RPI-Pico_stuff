#include <debouncemodule.h>

// Define a Python reference to the function we'll make available.
// See example.cpp for the definition.
// FIXME: start shouldn't be necessary (see other files)
STATIC MP_DEFINE_CONST_FUN_OBJ_0(start_obj, start);
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
// FIXME: start shouldn't be necessary (see other files)
    { MP_ROM_QSTR(MP_QSTR_start), MP_ROM_PTR(&start_obj) },
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

