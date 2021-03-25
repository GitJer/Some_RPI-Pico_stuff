
#include <stdio.h>
#include <stdlib.h>

#include "pico/stdlib.h"
#include "hardware/pio.h"

#include "rotational_shift_ISR.pio.h"

// handy function to print the value of 'number' in bits
void printBits(uint32_t number)
{ // go through each of the bits, starting with the Most Significant Bit (MSB)
    for (int i = 31; i >= 0; i--)
    {
        // test of that bit is set, print "1" if set, or "0" if not
        if ((number & (1 << i)) > 0)
        {
            printf("1");
        }
        else
        {
            printf("0");
        }
    }
}

int main()
{
    // needed for printf
    stdio_init_all();
    // pio 0 is used
    PIO pio = pio0;
    // state machine 0 and 1
    uint sm = 0;
    // load the sm0 program into the pio memory
    uint offset = pio_add_program(pio, &rotational_shift_ISR_program);
    // make a sm config
    pio_sm_config c = rotational_shift_ISR_program_get_default_config(offset);
    // set shift direction
    // NOTE: IT IS SET TO SHIFT RIGHT! EVEN THOUGH THE PIO CODE SHIFTS RIGHT AND LEFT
    sm_config_set_in_shift(&c, true, false, 0);
    // init the pio sm with the config
    pio_sm_init(pio, sm, offset, &c);
    // enable the sm
    pio_sm_set_enabled(pio, sm, true);

    while (true)
    {
        // push a random number into the Tx FIFO
        pio_sm_put(pio, sm, rand());
        // read it back (first wait until something is written into the Rx FIFO)
        while (pio_sm_is_rx_fifo_empty(pio, sm))
            ;
        printf("S)\t");
        printBits(pio_sm_get(pio, sm));
        printf("\n");

        // let the PIO code do 32 shifts to the right
        for (int i = 1; i <= 32; i++)
        {
            while (pio_sm_is_rx_fifo_empty(pio, sm))
                ;
            printf("R%d)\t", i);
            printBits(pio_sm_get(pio, sm));
            printf("\n");
        }

        // let the PIO code do 32 shifts to the left
        for (int i = 1; i <= 32; i++)
        {
            while (pio_sm_is_rx_fifo_empty(pio, sm))
                ;
            printf("L%d)\t", i);
            printBits(pio_sm_get(pio, sm));
            printf("\n");
        }

        printf("----------------------------------------------------------\n");
    }
}