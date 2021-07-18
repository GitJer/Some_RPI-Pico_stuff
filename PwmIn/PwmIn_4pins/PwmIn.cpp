#include <stdio.h>

#include "pico/stdlib.h"
#include "hardware/pio.h"
#include "hardware/irq.h"

#include "PwmIn.h"
#include "PwmIn.pio.h"

// class that reads PWM pulses from up to 4 pins
PwmIn::PwmIn(uint *pin_list, uint num_of_pins)
{
    _num_of_pins = num_of_pins;
    // pio 0 is used
    pio = pio0;
    // load the pio program into the pio memory
    uint offset = pio_add_program(pio, &PwmIn_program);
    // start num_of_pins state machines
    for (int i = 0; i < num_of_pins; i++)
    {
        // prepare state machine i
        pulsewidth[i] = 0;
        period[i] = 0;

        // configure the used pins (pull down, controlled by PIO)
        gpio_pull_down(pin_list[i]);
        pio_gpio_init(pio, pin_list[i]);
        // make a sm config
        pio_sm_config c = PwmIn_program_get_default_config(offset);
        // set the 'jmp' pin
        sm_config_set_jmp_pin(&c, pin_list[i]);
        // set the 'wait' pin (uses 'in' pins)
        sm_config_set_in_pins(&c, pin_list[i]);
        // set shift direction
        sm_config_set_in_shift(&c, false, false, 0);
        // init the pio sm with the config
        pio_sm_init(pio, i, offset, &c);
        // enable the sm
        pio_sm_set_enabled(pio, i, true);
    }
    // set the IRQ handler
    irq_set_exclusive_handler(PIO0_IRQ_0, pio_irq_handler);
    // enable the IRQ
    irq_set_enabled(PIO0_IRQ_0, true);
    // allow irqs from the low 4 state machines
    pio0_hw->inte0 = PIO_IRQ0_INTE_SM0_BITS | PIO_IRQ0_INTE_SM1_BITS | PIO_IRQ0_INTE_SM2_BITS | PIO_IRQ0_INTE_SM3_BITS ;
};

// read the period and pulsewidth
void PwmIn::read_PWM(float *readings, uint pin)
{
    if (pin < _num_of_pins)
    {

        // determine whole period
        period[pin] += pulsewidth[pin];
        // the measurements are taken with 2 clock cycles per timer tick
        // hence, it is 2*0.000000008
        *(readings + 0) = (float)pulsewidth[pin] * 2 * 0.000000008;
        *(readings + 1) = (float)period[pin] * 2 * 0.000000008;
        *(readings + 2) = ((float)pulsewidth[pin] / (float)period[pin]);
        pulsewidth[pin] = 0;
        period[pin] = 0;
    }
};

// read only the duty cycle
float PwmIn::read_DC(uint pin)
{
    return ((float)pulsewidth[pin] / (float)period[pin]);
}

// read only the period
float PwmIn::read_P(uint pin)
{
    // the measurements are taken with 2 clock cycles per timer tick
    // hence, it is 2*0.000000008
    return ((float)period[pin] * 0.000000016);
}

float PwmIn::read_PW(uint pin)
{
    // the measurements are taken with 2 clock cycles per timer tick
    // hence, it is 2*0.000000008
    return ((float)pulsewidth[pin] * 0.000000016);
};

uint32_t PwmIn::pulsewidth[4];
uint32_t PwmIn::period[4];
PIO PwmIn::pio;
