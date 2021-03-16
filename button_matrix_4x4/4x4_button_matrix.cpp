#include <stdio.h>

#include "pico/stdlib.h"
#include "hardware/pio.h"

#include "4x4_button_matrix.pio.h"

// class that sets up and reads the 4x4 button matrix
class button_matrix_4x4
{
public:
    // constructor
    // base_input is the starting gpio for the 4 input pins
    // base_output is the starting gpio for the 4 output pins
    button_matrix_4x4(uint base_input, uint base_output)
    {
        // pio 0 is used
        pio = pio0;
        // state machine 0
        sm = 0;
        // configure the used pins
        for (int i = 0; i < 4; i++)
        {
            // output pins
            pio_gpio_init(pio, base_output + i);
            // input pins with pull down
            pio_gpio_init(pio, base_input + i);
            gpio_pull_down(base_input + i);
        }
        // load the pio program into the pio memory
        uint offset = pio_add_program(pio, &button_matrix_program);
        // make a sm config
        pio_sm_config c = button_matrix_program_get_default_config(offset);
        // set the 'in' pins
        sm_config_set_in_pins(&c, base_input);
        // set the 4 output pins to output
        pio_sm_set_consecutive_pindirs(pio, sm, base_output, 4, true);
        // set the 'set' pins
        sm_config_set_set_pins(&c, base_output, 4);
        // set shift such that bits shifted by 'in' end up in the lower 16 bits
        sm_config_set_in_shift(&c, 0, 0, 0);
        // init the pio sm with the config
        pio_sm_init(pio, sm, offset, &c);
        // enable the sm
        pio_sm_set_enabled(pio, sm, true);
    }

    // read the 4x4 matrix
    int read(void)
    {
        // value is used to read from the fifo
        uint32_t value = 0;
        // clear the FIFO, we only want a currently pressed key
        pio_sm_clear_fifos(pio, sm);
        // give the sm some time to fill the FIFO if a key is being pressed
        sleep_ms(1);
        // check that the FIFO isn't empty
        if (pio_sm_is_rx_fifo_empty(pio, sm))
        {
            return -1;
        }
        // read one data item from the FIFO
        value = pio_sm_get(pio, sm);
        // translate from a bit position in value to a key number from 0 to 15.
        for (int i = 0; i < 16; i++)
        {
            // test a bit to see if it is set
            if ((value & (0x1 << i)) != 0)
            {
                //the bit is set -> return 'i' as the key number
                return i;
            }
        }
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
    // the instance of the button matrix: input gpio: 10, 11, 12, and 13, output gpio: 18, 19, 20, and 21
    button_matrix_4x4 my_matrix(10, 18);
    // infinite loop to print pressed keys
    while (true)
    {
        // read the matrix of buttons
        int key = my_matrix.read();
        if (key >= 0)
        { // a key was pressed: print its number
            printf("key pressed = %d\n", key);
        }
        sleep_ms(1000);
    }
}