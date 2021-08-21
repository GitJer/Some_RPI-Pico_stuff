#include "stdio.h"
#include "pico/stdlib.h"
#include "hardware/pio.h"
// the .pio.h file also defines (in this order): D0 (8 bits), A0 (8 bits), RW, WR, DIR, OE
#include "Z80.pio.h"

// PIO and state machine
PIO pio;
uint sm;
// the offset of the pio program in pio memory
uint offset;

// Configure the pio state machine
void configure_pio_sm()
{
    // pio 0 is used
    pio = pio0;
    // state machine 0 is used.
    sm = 0;

    // function select: this allows pio to set output on a gpio
    for (int i = D0; i <= OE; i++)
        pio_gpio_init(pio, i);

    // TODO: for testing purposes, set data and address lines to 0 via pull down, normally this would be set externally
    for (int i = D0; i < RD; i++)
        gpio_set_pulls(i, false, true);

    // load the sm program into the pio memory
    offset = pio_add_program(pio, &Z80_program);
    // make a sm config
    pio_sm_config smc = Z80_program_get_default_config(offset);

    // set initial pindirs: D0 - D7 are (also) output
    pio_sm_set_consecutive_pindirs(pio, sm, D0, 8, true);
    // set initial pindirs: A0 - A7 are input
    pio_sm_set_consecutive_pindirs(pio, sm, A0, 8, false);
    // set initial pindirs: RD, WR are input
    pio_sm_set_consecutive_pindirs(pio, sm, RD, 1, false);
    pio_sm_set_consecutive_pindirs(pio, sm, WR, 1, false);
    // set initial pindirs: DIR and OE are output
    pio_sm_set_consecutive_pindirs(pio, sm, DIR, 1, true);
    pio_sm_set_consecutive_pindirs(pio, sm, OE, 1, true);

    // pio 'in' pins: inputs start at the first data bit (D0)
    sm_config_set_in_pins(&smc, D0);
    // pio 'out' pins: data D0-D7 can also be output
    sm_config_set_out_pins(&smc, D0, 8);
    // pio 'set' pins: DIR (LSB) and OE (MSB)
    sm_config_set_set_pins(&smc, DIR, 2);
    // Reading from RxFIFO: Shift to left, autopull disabled
    sm_config_set_in_shift(&smc, false, false, 32);
    // Writing to TxFIFO: Shift to right, autopull disabled
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

    // initialize the state machine
    configure_pio_sm();

    // print the important gpio assignments
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
    // used for checking whether the sm has returned to the default state
    uint8_t current_pc;

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
        // NOTE: whatever is set on the data pins (D0-D7) is maintained by the RPI pico,
        //       so the next read will return whatever was set!

        // wait for the pio program to return to the default state
        current_pc = pio_sm_get_pc(pio, sm) - (offset + Z80_offset_set_default);
        while (current_pc > 1)
            current_pc = pio_sm_get_pc(pio, sm) - (offset + Z80_offset_set_default);

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

        // wait for the pio program to return to the default state
        current_pc = pio_sm_get_pc(pio, sm) - (offset + Z80_offset_set_default);
        while (current_pc > 1)
            current_pc = pio_sm_get_pc(pio, sm) - (offset + Z80_offset_set_default);
    }
}