// -------------------------------------------------- //
// This file is autogenerated by pioasm; do not edit! //
// -------------------------------------------------- //

#pragma once

#if !PICO_NO_HARDWARE
#include "hardware/pio.h"
#endif

// --------- //
// push_pull //
// --------- //

#define push_pull_wrap_target 0
#define push_pull_wrap 1

static const uint16_t push_pull_program_instructions[] = {
            //     .wrap_target
    0x6025, //  0: out    x, 5                       
    0x4023, //  1: in     x, 3                       
            //     .wrap
};

#if !PICO_NO_HARDWARE
static const struct pio_program push_pull_program = {
    .instructions = push_pull_program_instructions,
    .length = 2,
    .origin = -1,
};

static inline pio_sm_config push_pull_program_get_default_config(uint offset) {
    pio_sm_config c = pio_get_default_sm_config();
    sm_config_set_wrap(&c, offset + push_pull_wrap_target, offset + push_pull_wrap);
    return c;
}
#endif

