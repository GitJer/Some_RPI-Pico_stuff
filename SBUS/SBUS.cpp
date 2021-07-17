/*
    Read SBUS protocol, typically used in Radio Controlled cars, drones, etc.
    SBUS data packets consists of 25 8-bit bytes starting with 0x0F and ending with two 0x00
    The signal is inverted, has an even parity bit and two stop-bits, 
    see: https://github.com/bolderflight/sbus
*/
#include <stdio.h>

#include "pico/stdlib.h"
#include "hardware/pio.h"
#include "hardware/clocks.h"

#include "SBUS.pio.h"

// The baud rate for the SBUS protocol is 100000
#define SERIAL_BAUD 100000
// The RC receiver is attached to pin 5
#define PIO_RX_PIN 5
// the max amount of data in a SBUS data packet
#define MAX_DATA_ITEMS 25

// function to decode the received SBUS data to actual steering commands
void decode(uint8_t *data)
{
    uint16_t channels[16];
    // see https://platformio.org/lib/show/5622/Bolder%20Flight%20Systems%20SBUS
    channels[0] = (uint16_t)((data[0] | data[1] << 8) & 0x07FF);
    channels[1] = (uint16_t)((data[1] >> 3 | data[2] << 5) & 0x07FF);
    channels[2] = (uint16_t)((data[2] >> 6 | data[3] << 2 | data[4] << 10) & 0x07FF);
    channels[3] = (uint16_t)((data[4] >> 1 | data[5] << 7) & 0x07FF);
    channels[4] = (uint16_t)((data[5] >> 4 | data[6] << 4) & 0x07FF);
    channels[5] = (uint16_t)((data[6] >> 7 | data[7] << 1 | data[8] << 9) & 0x07FF);
    channels[6] = (uint16_t)((data[8] >> 2 | data[9] << 6) & 0x07FF);
    channels[7] = (uint16_t)((data[9] >> 5 | data[10] << 3) & 0x07FF);
    channels[8] = (uint16_t)((data[11] | data[12] << 8) & 0x07FF);
    channels[9] = (uint16_t)((data[12] >> 3 | data[13] << 5) & 0x07FF);
    channels[10] = (uint16_t)((data[13] >> 6 | data[14] << 2 | data[15] << 10) & 0x07FF);
    channels[11] = (uint16_t)((data[15] >> 1 | data[16] << 7) & 0x07FF);
    channels[12] = (uint16_t)((data[16] >> 4 | data[17] << 4) & 0x07FF);
    channels[13] = (uint16_t)((data[17] >> 7 | data[18] << 1 | data[19] << 9) & 0x07FF);
    channels[14] = (uint16_t)((data[19] >> 2 | data[20] << 6) & 0x07FF);
    channels[15] = (uint16_t)((data[20] >> 5 | data[21] << 3) & 0x07FF);
    for (int i = 0; i < 16; i++)
    {
        printf("%d \t", channels[i]);
    }
    printf("\n");
}

// Main function
int main()
{
    // needed for printf
    stdio_init_all();

    // Set up the state machine to receive RC SBUS data
    PIO pio = pio0;
    uint sm = 0;
    uint offset = pio_add_program(pio, &sbus_program);
    pio_sm_config c = sbus_program_get_default_config(offset);

    // configure the pin to receive the SBUS data
    pio_sm_set_consecutive_pindirs(pio, sm, PIO_RX_PIN, 1, false);
    pio_gpio_init(pio, PIO_RX_PIN);
    gpio_pull_down(PIO_RX_PIN);
    sm_config_set_in_pins(&c, PIO_RX_PIN); // for WAIT, IN
    sm_config_set_jmp_pin(&c, PIO_RX_PIN); // for JMP
    // Shift to right, autopull disabled
    sm_config_set_in_shift(&c, true, false, 32);
    // Shift to left, autopull disabled
    sm_config_set_out_shift(&c, false, false, 32);
    // Deeper FIFO as we're not doing any TX
    sm_config_set_fifo_join(&c, PIO_FIFO_JOIN_RX);
    // SM transmits 1 bit per 8 execution cycles.
    float div = (float)clock_get_hz(clk_sys) / (8 * SERIAL_BAUD);
    sm_config_set_clkdiv(&c, div);
    // init and enable the sm
    pio_sm_init(pio, sm, offset, &c);
    pio_sm_set_enabled(pio, sm, true);

    uint8_t index = 0;
    uint8_t data[MAX_DATA_ITEMS];

    // continuously get the SBUS data from the pio and decode it
    while (true)
    {
        // Note: 
        // Although there is sufficient time for receiving and decoding because the clkdiv 
        // and join (see above), too much printing will cause data loss
        while (pio_sm_is_rx_fifo_empty(pio, sm))
            tight_loop_contents();
        uint8_t data_item = pio_sm_get(pio, sm) >> 24;
        // search for 0x0f: the start marker
        if (data_item == 0x0f)
            index = 0;
        else if (index < MAX_DATA_ITEMS)
        {
            // test if the first end marker is read
            if (data_item == 0)
            {
                // read the second end marker
                while (pio_sm_is_rx_fifo_empty(pio, sm))
                    tight_loop_contents();
                data_item = pio_sm_get(pio, sm) >> 24;
                // the second end marker should also be 0
                if (data_item != 0)
                    printf("Error, second end marker not found\n");
                else
                    // if all data is received, decode it
                    if (index == 22)
                        decode(data);
            }
            else
                // if not start or end marker, add it to the received data
                data[index++] = data_item;
        }
        else
            printf("error\n");
    }
}
