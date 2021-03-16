#include <stdio.h>

#include "pico/stdlib.h"
#include "hardware/pio.h"

#include "PwmIn.pio.h"

// class that sets up and reads PWM pulses: PwmIn. It has three functions:
// read_period (in seconds)
// read_pulsewidth (in seconds)
// read_dutycycle (between 0 and 1)
class PwmIn
{
public:
    // constructor
    // input = pin that receives the PWM pulses.
    PwmIn(uint input)
    {
        // pio 0 is used
        pio = pio0;
        // state machine 0
        sm = 0;
        // configure the used pins
        pio_gpio_init(pio, input);
        // load the pio program into the pio memory
        uint offset = pio_add_program(pio, &PwmIn_program);
        // make a sm config
        pio_sm_config c = PwmIn_program_get_default_config(offset);
        // set the 'jmp' pin
        sm_config_set_jmp_pin(&c, input);
        // set shift direction
        sm_config_set_in_shift(&c, false, false, 0);
        // init the pio sm with the config
        pio_sm_init(pio, sm, offset, &c);
        // enable the sm
        pio_sm_set_enabled(pio, sm, true);
    }

    // read_period (in seconds)
    float read_period(void)
    {
        if (read() == -1)
        {
            return -1;
        }
        // one clock cycle is 1/125000000 seconds
        return (period * 0.000000008);
    }

    // read_pulsewidth (in seconds)
    float read_pulsewidth(void)
    {
        if (read() == -1)
        {
            return -1;
        }
        // one clock cycle is 1/125000000 seconds
        return (pulsewidth * 0.000000008);
    }

    // read_dutycycle (between 0 and 1)
    float read_dutycycle(void)
    {
        if (read() == -1)
        {
            return -1;
        }
        return ((float)pulsewidth / (float)period);
    }

private:
    // read the period and pulsewidth
    float read(void)
    {
        // clear the FIFO: do a new measurement
        pio_sm_clear_fifos(pio, sm);
        // wait for the FIFO to contain two data items: pulsewidth and period
        while (pio_sm_get_rx_fifo_level(pio, sm) < 2)
            ;
        // read pulse width from the FIFO
        uint32_t t1 = pio_sm_get(pio, sm);
        // read period from the FIFO
        uint32_t t2 = pio_sm_get(pio, sm);
        // since data is continuously added to the FIFO, sometimes the period/pulse data is read reversed
        if (t1 > t2)
        {
            period = t1;
            pulsewidth = t2;
        }
        else
        {
            period = t2;
            pulsewidth = t1;
        }
        // the measurements are taken with 2 clock cycles per timer tick
        pulsewidth = 2 * pulsewidth;
        // calculate the period in clock cycles:
        period = 2 * period;
        // return as successful
        return 0;
    }

    // the pio instance
    PIO pio;
    // the state machine
    uint sm;
    // data about the PWM input measured in pio clock cycles
    uint32_t pulsewidth, period;
};

int main()
{
    // needed for printf
    stdio_init_all();
    // the instance of the PwmIn
    PwmIn my_PwmIn(16);
    // infinite loop to print PWM measurements
    while (true)
    {
        printf("pw=%.8f \tp=%.8f \tdc=%.8f\n", my_PwmIn.read_pulsewidth(), my_PwmIn.read_period(), my_PwmIn.read_dutycycle());
    }
}
