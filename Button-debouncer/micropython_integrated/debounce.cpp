extern "C" {
#include <debouncemodule.h>
#include "button_debounce.h"
#include <stdio.h>
#include "py/obj.h"

Debounce debounce;

// FIXME: this is due to micropython not running the constructor again at re-importing the module
mp_obj_t start() {
    // mp_printf(&mp_plat_print, "start\n");
    debounce.start();
    return mp_obj_new_int(1);
}

mp_obj_t debounce_gpio(mp_obj_t mp_gpio) {
    const auto gpio = mp_obj_get_int(mp_gpio);
    // mp_printf(&mp_plat_print, "debounce_gpio\n");
    return mp_obj_new_int(debounce.debounce_gpio(gpio));
}

mp_obj_t set_debounce_time(mp_obj_t mp_gpio, mp_obj_t mp_debounce_time) {
    const auto gpio = mp_obj_get_int(mp_gpio);
    const auto debounce_time = mp_obj_get_float(mp_debounce_time);
    // mp_printf(&mp_plat_print, "set_debounce_time\n");
    return mp_obj_new_int(debounce.set_debounce_time(gpio, debounce_time));
}

mp_obj_t read(mp_obj_t mp_gpio) {
    const auto gpio = mp_obj_get_int(mp_gpio);
    // mp_printf(&mp_plat_print, "read\n");
    return mp_obj_new_int(debounce.read(gpio));
}

mp_obj_t undebounce_gpio(mp_obj_t mp_gpio) {
    const auto gpio = mp_obj_get_int(mp_gpio);
    // mp_printf(&mp_plat_print, "undebounce_gpio\n");
    return mp_obj_new_int(debounce.undebounce_gpio(gpio));
}


}
