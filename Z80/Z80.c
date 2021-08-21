#include "stdio.h"
#include "pico/stdlib.h"
#include "hardware/pio.h"
#include "hardware/dma.h"
#include "hardware/irq.h"
#include "Z80.pio.h"

// PIO and state machine
PIO pio;
uint sm;
uint offset;

// TODO: move these definitions to the pio program (as globally visible). But can those defines do math?

// The starting point of the 8 bit Data D0 - D8
// NOTE: I started at 2 because the first two pins are used by picoprobe.
#define D0 2
// The starting point of the 16 bit Address A0 - A15
#define A0 (D0 + 8)
#define RD (A0 + 8 + 0)
#define WR (A0 + 8 + 1)
#define DIR (A0 + 8 + 2)
#define OE (A0 + 8 + 3)

void configure_pio_sm()
{
    // pio 0 is used
    pio = pio0;
    // state machine 0 is used.
    sm = 0;

    for (int i = D0; i <= OE; i++)
    {
        pio_gpio_init(pio, i);
    }
    // TODO: for testing purposes, set data and address lines to 0 via pull down, normally this would be set externally
    for (int i = D0; i < RD; i++)
    {
        gpio_set_pulls(i, false, true);
    }

    // load the sm program into the pio memory
    offset = pio_add_program(pio, &Z80_program);
    // make a sm config
    pio_sm_config smc = Z80_program_get_default_config(offset);
    // set initial pindirs
    pio_sm_set_consecutive_pindirs(pio, sm, D0, 8, true);
    pio_sm_set_consecutive_pindirs(pio, sm, A0, 8, true);
    pio_sm_set_consecutive_pindirs(pio, sm, RD, 1, false);
    pio_sm_set_consecutive_pindirs(pio, sm, WR, 1, false);
    pio_sm_set_consecutive_pindirs(pio, sm, DIR, 1, true);
    pio_sm_set_consecutive_pindirs(pio, sm, OE, 1, true);

    // inputs start at the first data bit (D0)
    sm_config_set_in_pins(&smc, D0);
    // data can also be output
    sm_config_set_out_pins(&smc, D0, 8);
    // use set pins for DIR (LSB) and OE (MSB)
    sm_config_set_set_pins(&smc, DIR, 2);
    // Shift to left, autopull disabled
    sm_config_set_in_shift(&smc, false, false, 32);
    // Shift to left, autopull disabled
    sm_config_set_out_shift(&smc, true, false, 32);
    // set clock to about 4Mhz TODO: set correct frequency
    // sm_config_set_clkdiv(&smc, 31);
    sm_config_set_clkdiv(&smc, 16);
    // init the pio sm with the config
    pio_sm_init(pio, sm, offset, &smc);
    // enable the sm
    pio_sm_set_enabled(pio, sm, true);
}

int main()
{
    // needed for printf
    stdio_init_all();

    configure_pio_sm();

    printf("D0=%d\n", D0);
    printf("A0=%d\n", A0);
    printf("RD=%d\n", RD);
    printf("WR=%d\n", WR);
    printf("DIR=%d\n", DIR);
    printf("OE=%d\n", OE);

    // a counter to simulate the data in memory (or after some calculations)
    uint16_t counter = 0;
    // the received data from pio is a combination of the data and the address
    uint32_t addr_data;
    // the data part
    uint8_t data;
    // the address part
    uint16_t address;

    // start in the default situation
    // Note: if this is in reality set by an external system (e.g. via pullup/down resistors, ths can be removed)
    pio_sm_exec(pio, sm, offset + Z80_offset_set_default);

    while (1)
    {
        // *********************************************
        // read:
        // the pio receives an address
        // the c code determines a data value
        // the pio sets this data value on the data bus
        // return to the default situation
        // *********************************************

        // start the read program
        pio_sm_exec(pio, sm, offset + Z80_offset_read_data);
        // wait for address in the RxFIFO
        addr_data = pio_sm_get_blocking(pio, sm);
        // the lowest 8 bits are the data and the next 8 bits are the address
        data = addr_data & 0xFF;
        address = addr_data >> 8;
        printf("read_data: raw=%zu address=%d data=%d\n", addr_data, address, data);

        // simulate getting the data from memory (or doing some calculations) by using a counter
        counter++;
        if (counter == 256)
            counter = 0;
        printf("setting data to %d\n", counter);
        // send the data to the pio for writing it to the data bits
        pio_sm_put(pio, sm, counter);
        // NOTE: whatever is set on the data bus is maintained by the RPI pico,
        //       so the next read will return whatever was set!

        // TODO: can this sleep be removed?
        // wait some time for the pio to finish the read
        sleep_ms(1);

        // *********************************************
        // write:
        // the pio receives an address and the data
        // the c code does something useful with them
        // return to the default situation
        // *********************************************

        // start the write program
        pio_sm_exec(pio, sm, offset + Z80_offset_write_data);
        // wait for address in the RxFIFO
        addr_data = pio_sm_get_blocking(pio, sm);
        // the lowest 8 bits are the data and the next 8 bits are the address
        data = addr_data & 0xFF;
        address = addr_data >> 8;
        printf("write_data: raw=%zu address=%d data=%d\n", addr_data, address, data);

        // TODO: can this sleep be removed?
        // wait some time for the pio to finish the write
        sleep_ms(1);
    }
}