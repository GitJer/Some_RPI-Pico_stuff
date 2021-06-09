/*
    This is the C SDK code needed to start the accompanying pio code.
    It only defines a pin (for out and set) as output, and starts the sm
*/

#include <stdio.h>

#include "pico/stdlib.h"
#include "hardware/pio.h"

#include "subroutine.pio.h"

int main()
{
    // the pio instance
    PIO pio = pio0;
    // the state machine
    uint sm = 0;

    // needed for printf
    stdio_init_all();

    int tx_pin = 16;

    // let pio control tx_pin
    pio_gpio_init(pio, tx_pin);
    // load the pio program into the pio memory
    uint offset = pio_add_program(pio, &subroutine_program);
    // make a sm config
    pio_sm_config c = subroutine_program_get_default_config(offset);
    // set the 'out' pin
    sm_config_set_out_pins(&c, tx_pin, 1);
    // set the 'set' pin
    sm_config_set_set_pins(&c, tx_pin, 1);
    // set the pin to output
    pio_sm_set_consecutive_pindirs(pio, sm, tx_pin, 1, true);
    // OUT shifts to right, no autopull
    sm_config_set_out_shift(&c, true, false, 32);
    // just for testing purposes (I have a crappy logic analyzer): slow down the clock
    sm_config_set_clkdiv(&c, 10);
    // init the pio sm with the config
    pio_sm_init(pio, sm, offset, &c);
    // enable the sm
    pio_sm_set_enabled(pio, sm, true);
    // do nothing
    while (1)
        ;
}