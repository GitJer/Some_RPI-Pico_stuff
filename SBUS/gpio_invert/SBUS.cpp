/*
    Read SBUS protocol, typically used in Radio Controlled cars, drones, etc.
    SBUS data packets consists of 25 8-bit bytes starting with 0x0F and ending with two 0x00
    The signal is inverted, has an even parity bit and two stop-bits, 
    see: https://github.com/bolderflight/sbus
*/
#include <stdio.h>

#include "pico/stdlib.h"
#include "hardware/uart.h"

// the max amount of data in a SBUS data packet (including start marker 0x0F and end marker 0x00)
#define MAX_DATA_ITEMS 25
// the baud rate for the SBUS protocol is 100000
#define SERIAL_BAUD 100000
// the RC receiver is attached to pin 5
#define RX_PIN 5
// the UART output (not used)
// #define TX_PIN 4
// the uart (there are two: uart0 and uart1, pin5 belongs to uart1)
#define UART_ID uart1
// number of data bits; SBUS has 8
#define DATA_BITS 8
// number of stop bits; SBUS has 2
#define STOP_BITS 2
// parity setting; SBUS has even parity
#define PARITY UART_PARITY_EVEN

// function to decode the received SBUS data to actual steering commands
void decode(uint8_t *data)
{
    // the SBUS data encodes 16 channels
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
    // initialize the uart
    uart_init(UART_ID, SERIAL_BAUD);
    // set the gpio function to uart
    gpio_set_function(RX_PIN, GPIO_FUNC_UART);
    // set data bits, stop_bits and the parity of the uart
    uart_set_format(UART_ID, DATA_BITS, STOP_BITS, PARITY);
    // set the input gpio pin to inverted
    gpio_set_inover(RX_PIN, GPIO_OVERRIDE_INVERT);

    // index of read SBUS data item
    uint8_t index = 0;
    // array to store read SBUS data
    uint8_t data[MAX_DATA_ITEMS];

    // continuously get the SBUS data from the pio and decode it
    while (true)
    {   // read a SBUS data item
        uint8_t data_item = (uint8_t)uart_getc(UART_ID);
        // store it
        data[index++] = data_item;
        // check for the start markter
        if (data_item == 0x0F)
        {
            // if start marker has been found: decode the existing SBUS data
            decode(data);
            // start over
            index = 0;
        }
    }
}
