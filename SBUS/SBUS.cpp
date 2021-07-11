/*

    Read SBUS protocol, typically used in Radio Controlled cars, drones, etc.

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

int main()
{
    // needed for printf
    stdio_init_all();

    // Set up the state machine we're going to use to receive RC SBUS data.
    PIO pio = pio0;
    uint sm = 0;
    uint offset = pio_add_program(pio, &sbus_program);
    sbus_program_init(pio, sm, offset, PIO_RX_PIN, SERIAL_BAUD);

    // a SBUS data packet consists of 25 8-bit bytes starting with 0x0F and ending with two 0xFF
    // The signal is inverted, has an even parity bit and two stop-bits, https://github.com/bolderflight/sbus
    uint8_t prev1 = 0xFF;
    uint8_t prev2 = 0xFF;
    uint8_t index = 0;
    uint8_t data[25];

    while (true)
    {
        // get the SBUS data from the pio
        // Note: there is sufficient time because the PIO runs slow and the RxFIFO is joined with the TxFIFO (i.e. 8 bytes deep)
        uint8_t data_item = (unsigned int)sbus_program_getc(pio, sm);
        // search for 0x00, 0x00, 0x0f (two end indicators, and one start indicator)
        if ((data_item == 0x0f) && (prev1 == 0x00) && (prev2 == 0x00))
        {
            index = 0;
        }
        // store the received data and shift prev2 <- prev1 <- data_item
        data[index++] = data_item;
        prev2 = prev1;
        prev1 = data_item;

        // a new set of data has already been started (the two end signals and a 0x0f has have been received), process the previous data
        uint16_t channels[16];
        if (index == 1)
        {
            // see https://platformio.org/lib/show/5622/Bolder%20Flight%20Systems%20SBUS

            channels[0] = (uint16_t)((data[1 + 0] | data[1 + 1] << 8) & 0x07FF);
            channels[1] = (uint16_t)((data[1 + 1] >> 3 | data[1 + 2] << 5) & 0x07FF);
            channels[2] = (uint16_t)((data[1 + 2] >> 6 | data[1 + 3] << 2 | data[1 + 4] << 10) & 0x07FF);
   	    channels[3] = (uint16_t)((data[1 + 4] >> 1 | data[1 + 5] << 7) & 0x07FF);
   	    channels[4] = (uint16_t)((data[1 + 5] >> 4 | data[1 + 6] << 4) & 0x07FF);
   	    channels[5] = (uint16_t)((data[1 + 6] >> 7 | data[1 + 7] << 1 | data[1 + 8] << 9) & 0x07FF);
   	    channels[6] = (uint16_t)((data[1 + 8] >> 2 | data[1 + 9] << 6) & 0x07FF);
   	    channels[7] = (uint16_t)((data[1 + 9] >> 5 | data[1 + 10] << 3) & 0x07FF);
   	    channels[8] = (uint16_t)((data[1 + 11] | data[1 + 12] << 8) & 0x07FF);
   	    channels[9] = (uint16_t)((data[1 + 12] >> 3 | data[1 + 13] << 5) & 0x07FF);
   	    channels[10] = (uint16_t)((data[1 + 13] >> 6 | data[1 + 14] << 2 | data[1 + 15] << 10) & 0x07FF);
   	    channels[11] = (uint16_t)((data[1 + 15] >> 1 | data[1 + 16] << 7) & 0x07FF);
   	    channels[12] = (uint16_t)((data[1 + 16] >> 4 | data[1 + 17] << 4) & 0x07FF);
   	    channels[13] = (uint16_t)((data[1 + 17] >> 7 | data[1 + 18] << 1 | data[1 + 19] << 9) & 0x07FF);
   	    channels[14] = (uint16_t)((data[1 + 19] >> 2 | data[1 + 20] << 6) & 0x07FF);
   	    channels[15] = (uint16_t)((data[1 + 20] >> 5 | data[1 + 21] << 3) & 0x07FF);
            for (int i=0; i<16; i++) {
                printf("%d \t", channels[i]);
            }
            printf("\n");
        }
    }
}
