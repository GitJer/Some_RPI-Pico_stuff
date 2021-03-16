#include <stdio.h>

#include "pico/stdlib.h"
#include "hardware/pio.h"

#include "two_sm_simple.pio.h"

int main()
{
    // needed for printf
    stdio_init_all();

    // pio 0 is used
    PIO pio = pio0;
    // state machine 0 and 1
    uint sm0 = 0;
    uint sm1 = 1;
    // load the sm0 program into the pio memory
    uint offset0 = pio_add_program(pio, &tester0_program);
    // load the sm1 program into the pio memory
    uint offset1 = pio_add_program(pio, &tester1_program);
    // make a sm config
    pio_sm_config c0 = tester0_program_get_default_config(offset0);
    pio_sm_config c1 = tester1_program_get_default_config(offset1);
    // init the pio sm0 with the config
    pio_sm_init(pio, sm0, offset0, &c0);
    pio_sm_init(pio, sm1, offset1, &c1);
    // enable the sm
    pio_sm_set_enabled(pio, sm0, true);
    pio_sm_set_enabled(pio, sm1, true);

    while (true)
    {
        printf("0 = %d \t\t1 = %d\n", pio_sm_get(pio, sm0), pio_sm_get(pio, sm1));
    }
}