#include <stdio.h>

#include "pico/stdlib.h"
#include "hardware/pio.h"

#include "two_p_one_f.pio.h"

#define TEST_PIN 16

/*
This program shows how you can use two pio programs in one file (two_p_one_f)
For this test it is assumed that the TEST_PIN is externally pulled high via e.g. a 10k resistor
*/

int main()
{
    // needed for printf
    stdio_init_all();
    // the pio instance is 0
    PIO pio;
    pio = pio0;
    // the state machine instance is 0
    uint sm;
    sm = 0;
    // configure the used pin
    pio_gpio_init(pio, TEST_PIN);

    // load the pio programs into the pio memory
    uint offset_1 = pio_add_program(pio, &two_p_one_f_1_program);
    uint offset_2 = pio_add_program(pio, &two_p_one_f_2_program);
    // make the sm config for both programs
    pio_sm_config c = pio_get_default_sm_config();
    // set the pin used by set
    sm_config_set_set_pins(&c, TEST_PIN, 1);
    // sideset (bitcount=2: one bit + one bit because sideset is optional, optional=true, pindirs=true)
    sm_config_set_sideset(&c, 2, true, true);
    // set the pin used by sideset (same pin as for SET, two lines above)
    sm_config_set_sideset_pins(&c, TEST_PIN);
    // one clock cycle is 10 us
    sm_config_set_clkdiv(&c, 1250);

    // init the pio sm with the config, start with two_p_one_f_1
    pio_sm_init(pio, sm, offset_1, &c);
    // enable the sm
    pio_sm_set_enabled(pio, sm, true);

    while (true)
    {
        // switch to two_p_one_f_1
        pio_sm_exec(pio, sm, 0x0000 + offset_1);
        sleep_ms(1000);
        // switch to two_p_one_f_2
        pio_sm_exec(pio, sm, 0x0000 + offset_2);
        sleep_ms(1000);
    }
}
