#include <stdio.h>

#include "pico/stdlib.h"
#include "hardware/irq.h"
#include "hardware/pio.h"

#include "count_pulses_with_pause.pio.h"

/*
This class can be used for protocols where the data is encoded by a number of pulses in a pulse train followed by a pause.
E.g. the LMT01 temperature sensor uses this (see https://www.reddit.com/r/raspberrypipico/comments/nis1ew/made_a_pulse_counter_for_the_lmt01_temperature/)

The class itself only starts the state machine and, when called, read_pulses() gives the data the state machine has put in the Rx FIFO
*/

class count_pulses_with_pause
{
public:
    // input = pin that receives the pulses.
    count_pulses_with_pause(uint input)
    {
        // pio 0 is used
        pio = pio0;
        // state machine 0
        sm = 0;
        // configure the used pin
        pio_gpio_init(pio, input);
        // load the pio program into the pio memory
        uint offset = pio_add_program(pio, &count_pulses_with_pause_program);
        // make a sm config
        pio_sm_config c = count_pulses_with_pause_program_get_default_config(offset);
        // set the 'jmp' pin
        sm_config_set_jmp_pin(&c, input);
        // set the 'wait' pin (uses 'in' pins)
        sm_config_set_in_pins(&c, input);
        // set shift direction
        sm_config_set_in_shift(&c, false, false, 0);
        // init the pio sm with the config
        pio_sm_init(pio, sm, offset, &c);
        // enable the sm
        pio_sm_set_enabled(pio, sm, true);
    }

    // read the number of pulses in a pulse train
    uint32_t read_pulses(void)
    {
        // clear the FIFO: do a new measurement
        pio_sm_clear_fifos(pio, sm);
        // wait for the FIFO to contain a data item
        while (pio_sm_get_rx_fifo_level(pio, sm) < 1)
            ;
        // read the Rx FIFO and return the value: the number of pulses measured
        return (pio_sm_get(pio, sm));
    }

private:
    // the pio instance
    PIO pio;
    // the state machine
    uint sm;
};

int main()
{
    // needed for printf
    stdio_init_all();
    // the instance of the count_pulses_with_pause. Note the input pin is 28 in this example
    count_pulses_with_pause my_count_pulses_with_pause(28);
    // infinite loop to print pulse measurements
    while (true)
    {
        int num_pulses = my_count_pulses_with_pause.read_pulses();
        printf("number of pulses = %zu\n", num_pulses);
        sleep_ms(100);
    }
}
