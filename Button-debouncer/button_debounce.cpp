#include <stdio.h>

#include "pico/stdlib.h"
#include "hardware/pio.h"
#include "hardware/clocks.h"

#include "button_debounce.pio.h"
#include "button_debounce.h"

// indicate if errors and warnings should print a message
// comment out for no messages
#define PRINT_ERRORS

// indicator that something is not used or not set
#define UNUSED -10

/* 
 * class that debounces gpio using the PIO state machines.
 * up to 8 gpios can be debounced at any one time.
 * the debounce time for each gpio can be set individually, default it is set a 10ms.
 */
Debounce::Debounce(void)
{
    // indicate that currently there are no gpios debounced
    for (int i = 0; i < 32; i++)
    {
        gpio_debounced[i] = UNUSED;
        pio_debounced[i] = (PIO)NULL;
        sm_debounced[i] = UNUSED;
        offset[i] = UNUSED;
        // conf[i] = (pio_sm_config) 0;
    }
    num_of_debounced = 0;
    pio0_already_set = UNUSED;
    pio1_already_set = UNUSED;
}

/* 
 * Request to debounce the gpio
 * @param gpio: the gpio that needs to be debounced
 *              the value must be [0, 28] excluding 23, 24 and 25. 
 */
int Debounce::debounce_gpio(uint gpio)
{

    // check if the gpio is valid
    if ((gpio > 28) || gpio == 23 || gpio == 24 || gpio == 25)
    {
#ifdef PRINT_ERRORS
        printf("debounce error: gpio should be 0 to 28 excluding 23, 24 and 25\n");
#endif
        return -1;
    }
    // check that the gpio is unused
    if (gpio_debounced[gpio] != UNUSED)
    {
#ifdef PRINT_ERRORS
        printf("debounce warning: gpio is already debounced\n");
#endif
        return -1;
    }
    // check if there are still sm available (there are 8, but other programs could also be using sm, which is checked later)
    if (num_of_debounced == 8)
    {
#ifdef PRINT_ERRORS
        printf("debounce error: max 8 gpios can be debounced (no state machine available)\n");
#endif
        return -1;
    }

    // Find a pio and sm:
    // start with trying to use pio0
    PIO pio = pio0;
    // claim a state machine, no panic if non is available
    uint sm = pio_claim_unused_sm(pio, false);
    // check if this is a valid sm
    if (sm == -1)
    {
        // pio0 did not deliver a sm, try pio1
        pio = pio1;
        // claim a state machine, no panic if non is available
        sm = pio_claim_unused_sm(pio, false);
        // check if this is a valid sm
        if (sm == -1)
        {
            // also no sm from pio1, return an error
#ifdef PRINT_ERRORS
            printf("debounce error: no state machine available\n");
#endif
            return -1;
        }
    }

    pio_debounced[gpio] = pio;
    sm_debounced[gpio] = sm;
    gpio_debounced[gpio] = gpio;
    num_of_debounced += 1;

    // check if the pio program has already been loaded:
    if ((pio_debounced[gpio] == pio0) && (pio0_already_set != UNUSED))
    {
        offset[gpio] = offset[pio0_already_set];
        conf[gpio] = conf[pio0_already_set];
    }
    else if ((pio_debounced[gpio] == pio1) && (pio1_already_set != UNUSED))
    {
        offset[gpio] = offset[pio1_already_set];
        conf[gpio] = conf[pio1_already_set];
    }
    else
    {
        // load the pio program into the pio memory
        offset[gpio] = pio_add_program(pio_debounced[gpio], &button_debounce_program);
        // make a sm config
        conf[gpio] = button_debounce_program_get_default_config(offset[gpio]);
        // set the initial clkdiv to 10ms
        sm_config_set_clkdiv(&conf[gpio], 10.);
        if (pio_debounced[gpio] == pio0)
            pio0_already_set = gpio;
        else
            pio1_already_set = gpio;
    }
    // set the 'wait' gpios
    sm_config_set_in_pins(&conf[gpio], gpio); // for WAIT, IN
    // set the 'jmp' gpios
    sm_config_set_jmp_pin(&conf[gpio], gpio); // for JMP
    // init the pio sm with the config
    pio_sm_init(pio_debounced[gpio], sm_debounced[gpio], offset[gpio], &conf[gpio]);

    // enable the sm
    pio_sm_set_enabled(pio_debounced[gpio], sm_debounced[gpio], true);
    return 0;
};

/* 
 * Request to debounce the gpio
 * @param gpio: the gpio that needs to be debounced
 *              the value must be a uint in the range [0, 28] excluding 23, 24 and 25. 
 * @param debounce_time: the float debounce_time in milliseconds in the range [0.5, 30.]
 */

int Debounce::set_debounce_time(uint gpio, float debounce_time)
{
    // check if the gpio is valid
    if ((gpio > 28) || gpio == 23 || gpio == 24 || gpio == 25)
    {
#ifdef PRINT_ERRORS
        printf("debounce error: gpio should be 0 to 28 excluding 23, 24 and 25\n");
#endif
        return -1;
    }
    if (debounce_time < 0.5)
    {
#ifdef PRINT_ERRORS
        printf("debounce error: debounce time must be > 0 ms\n");
#endif
        return -1;
    }
    if (debounce_time > 30)
    {
#ifdef PRINT_ERRORS
        printf("debounce error: the maximum is 30 ms, if you need longer debounce times: instructions are in the code\n");
#endif
        return -1;
    }

    /* 
        calculate clkdiv based on debounce time:
        Note: the resulting debounce time will not be very precise, but probably within 5 to 10%

        In the pio code it becomes clear that the debounce time is about 31*2 = 62 instructions.
        The time in seconds when a clkdiv is applied then becomes: clkdiv * 62 / 125000000
        In microseconds this is: clkdiv * 62/125, which is about half the clkdiv value.
        Conversely: the clkdiv for a given debounce_time in miliseconds is: 2 * 1000 * debounce_time
        The minimum clkdiv value is 1, the corresponding debounce time is about 500 microseconds
        The maximum clkdiv value is 65535, the corresponding debounce time is about 33 milliseconds
        
        If a longer debounce time is required, the pio code must be adapted to add some pauses. This is
        indicated in the pio code.
     */

    // stop the sm
    pio_sm_set_enabled(pio_debounced[gpio], sm_debounced[gpio], false);
    // calculate the clkdiv (see explanation above)
    float clkdiv = 2. * debounce_time * 1000.;
    // check that the clkdiv has a valid value
    if (clkdiv < 1.0)
        clkdiv = 1.0;
    else if (clkdiv > 65535.)
        clkdiv = 65535.;
    // set the clkdiv for both pio
    sm_config_set_clkdiv(&conf[gpio], clkdiv);
    // do the init of the pio/sm
    pio_sm_init(pio_debounced[gpio], sm_debounced[gpio], offset[gpio], &conf[gpio]);
    // enable the sm
    pio_sm_set_enabled(pio_debounced[gpio], sm_debounced[gpio], true);
    return 0;
};

/* 
 * Read the current value of the debounced the gpio
 * @param gpio: the gpio whose value (low, high) is read
 *              the gpio must have previously been debounced using debounce_gpio()
 */
int Debounce::read(uint gpio)
{
    // check if the gpio is valid
    if ((gpio > 28) || gpio == 23 || gpio == 24 || gpio == 25)
    {
#ifdef PRINT_ERRORS
        printf("debounce error: gpio should be 0 to 28 excluding 23, 24 and 25\n");
#endif
        return -1;
    }
    // check that this gpio is indeed being debounced
    if (gpio_debounced[gpio] == UNUSED)
    {
#ifdef PRINT_ERRORS
        printf("debounce error: gpio is not debounced\n");
#endif
        return -1;
    }
    // read the program counter
    uint pc = pio_sm_get_pc(pio_debounced[gpio], sm_debounced[gpio]);
    // if it is at or beyond the "wait 0 pin 0" it has value 1, else 0
    // in the pio code a public define called 'border' is set at that position
    if (pc >= (offset[gpio] + button_debounce_border))
        return 1;
    else
        return 0;
};

/* 
 * undebounce a previously debounced gpio
 * @param gpio: the gpio that is no longer going to be debounced
 *              the gpio must have previously been debounced using debounce_gpio()
 */
int Debounce::undebounce_gpio(uint gpio)
{
    // check if the gpio is valid
    if ((gpio > 28) || gpio == 23 || gpio == 24 || gpio == 25)
    {
#ifdef PRINT_ERRORS
        printf("debounce error: gpio should be 0 to 28 excluding 23, 24 and 25\n");
#endif
        return -1;
    }
    // check that this gpio is indeed being debounced
    if (gpio_debounced[gpio] == UNUSED)
    {
#ifdef PRINT_ERRORS
        printf("debounce error: gpio is not debounced\n");
#endif
        return -1;
    }

    // disable the pio
    pio_sm_set_enabled(pio_debounced[gpio], sm_debounced[gpio], false);
    // save pio, sm and offset to - if possible - unclaim the sm and remove the program from pio memory
    PIO pio_used = pio_debounced[gpio];
    uint sm_used = sm_debounced[gpio];
    int offset_used = offset[gpio];
    // indicate that the gpio is not debounced
    gpio_debounced[gpio] = UNUSED;
    pio_debounced[gpio] = (PIO)NULL;
    sm_debounced[gpio] = UNUSED;
    offset[gpio] = UNUSED;

    // unclaim the sm
    pio_sm_unclaim(pio_used, sm_used);

    // if this is the last gpio of a pio: remove the program, set pioX_already_set to UNUSED
    int i;
    for (i = 0; i < 32; i++)
    {
        // check if the pio is still in use (i.e. one of the sm belongs to this pio)
        if (pio_debounced[i] == pio_used)
            break;
    }
    // if i==32 it means that no other debounced gpio uses this pio
    if (i == 32)
    {
        // remove the program
        pio_remove_program(pio_used, &button_debounce_program, offset_used);
        // indicate that the pio (either pio0 or pio1) is not set
        if (pio_used == pio0)
            pio0_already_set = UNUSED;
        else
            pio1_already_set = UNUSED;
    }
    // there is one less gpio being debounced
    num_of_debounced--;

    return 0;
}
