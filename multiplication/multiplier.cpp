#include <stdio.h>

#include "pico/stdlib.h"
#include "hardware/pio.h"

#include "multiplier.pio.h"

// the pio instance
PIO pio;
// the state machine
uint sm;

void pio_mul(int a, int b)
{
    pio_sm_put(pio, sm, a);
    pio_sm_put(pio, sm, b);
    printf("%d * %d = %d\n", a, b, pio_sm_get_blocking(pio, sm));
}

int main()
{
    // needed for printf
    stdio_init_all();

    // pio 0 is used
    pio = pio0;
    // state machine 0
    sm = 0;
    // load the pio program into the pio memory
    uint offset = pio_add_program(pio, &multiplier_program);
    // make a sm config
    pio_sm_config c = multiplier_program_get_default_config(offset);
    // init the pio sm with the config
    pio_sm_init(pio, sm, offset, &c);
    // enable the sm
    pio_sm_set_enabled(pio, sm, true);

    pio_mul(1, 1);
    pio_mul(0, 1);
    pio_mul(5, 0);
    pio_mul(1, 2);
    pio_mul(2, 1);
    pio_mul(3, 2);
    pio_mul(2, 3);
    pio_mul(4, 5);
    pio_mul(5, 4);
    pio_mul(1, 10);
    pio_mul(5, 100);
    pio_mul(12, 12);
    pio_mul(100, 101);
    pio_mul(1001, 1000);

    while (true)
    {
    }
}