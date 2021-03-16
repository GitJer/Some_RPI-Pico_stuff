
#include <stdio.h>

#include "pico/stdlib.h"
#include "hardware/pio.h"

#include "value_communication_between_two_sm_via_pins.pio.h"

int main()
{
    // needed for printf
    stdio_init_all();
    // pio 0 is used
    PIO pio = pio0;
    // state machine 0 and 1
    uint sm0 = 0;
    uint sm1 = 1;
    // use pin 15 for both output of sm0 and input to sm1
    uint pin = 15;
    pio_gpio_init(pio, pin);
    pio_sm_set_consecutive_pindirs(pio, sm0, pin, 1, true);
    // These pins are used by sm0 and sm1.
    // As it turns out, the pin needs to be set once: to output!
    // even though sm1 uses them as input.

    // load the sm0 program into the pio memory
    uint offset0 = pio_add_program(pio, &tester0_program);
    // load the sm1 program into the pio memory
    uint offset1 = pio_add_program(pio, &tester1_program);
    // make a sm config
    pio_sm_config c0 = tester0_program_get_default_config(offset0);
    pio_sm_config c1 = tester1_program_get_default_config(offset1);

    // set shift direction
    sm_config_set_in_shift(&c0, false, false, 0);
    // set shift direction
    sm_config_set_in_shift(&c1, false, false, 0);

    // sm0 produces output using 'set', sm1 reads it using 'in'
    // set the 'set' pins for sm0
    sm_config_set_set_pins(&c0, pin, 1);
    // set the 'in' pins for sm1
    sm_config_set_in_pins(&c1, pin);

    // init the pio sm0 with config c0
    pio_sm_init(pio, sm0, offset0, &c0);
    // init the pio sm1 with config c1
    pio_sm_init(pio, sm1, offset1, &c1);
    // enable the state machines
    pio_sm_set_enabled(pio, sm0, true);
    pio_sm_set_enabled(pio, sm1, true);

    // end with infinite loop of printing what is send by sm0 and received by sm1
    while (true)
    {
        printf("value send by sm0 = %d\n", pio_sm_get(pio, sm0));
        printf("value received by sm1 = %d\n", pio_sm_get(pio, sm1));
    }
}