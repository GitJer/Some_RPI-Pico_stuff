#include "stdio.h"
#include "pico/stdlib.h"
#include "hardware/pio.h"
#include "hardware/vreg.h"
// the .pio.h file also defines (in this order): D0 (8 bits), A0 (8 bits), RW, WR, DIR, OE
#include "Z80.pio.h"

// PIO and state machine
PIO pio;
uint sm_rd;
uint sm_wr;
uint offset_rd;
uint offset_wr;
// Configure the pio state machine
void configure_pio_sm()
{
    // pio 0 is used
    pio = pio0;
    // state machine 0 is used.
    sm_rd = 0;
    sm_wr = 1;

    // load the sm_rd program into the pio memory
    offset_rd = pio_add_program(pio, &Z80_read_program);
    // load the sm program into the pio memory
    offset_wr = pio_add_program(pio, &Z80_write_program);
    // make a sm_rd config
    pio_sm_config smc_rd = Z80_read_program_get_default_config(offset_rd);
    // make a sm_wr config
    pio_sm_config smc_wr = Z80_write_program_get_default_config(offset_wr);

    // function select: this allows pio to set output on a gpio
    for (int i = D0; i <= OE; i++)
        pio_gpio_init(pio, i);

    // TODO: for testing purposes, set data and address lines to 0 via pull down, normally this would be set externally
    for (int i = D0; i < RD; i++)
        gpio_set_pulls(i, false, true);

    // set initial pindirs: D0 - D7 are (also) output
    pio_sm_set_consecutive_pindirs(pio, sm_rd, D0, 8, true);
    pio_sm_set_consecutive_pindirs(pio, sm_wr, D0, 8, true);
    // set initial pindirs: A0 - A7 are input
    pio_sm_set_consecutive_pindirs(pio, sm_rd, A0, 8, false);
    pio_sm_set_consecutive_pindirs(pio, sm_wr, A0, 8, false);
    // set initial pindirs: RD, WR are input
    pio_sm_set_consecutive_pindirs(pio, sm_rd, RD, 1, false);
    pio_sm_set_consecutive_pindirs(pio, sm_wr, RD, 1, false);
    pio_sm_set_consecutive_pindirs(pio, sm_rd, WR, 1, false);
    pio_sm_set_consecutive_pindirs(pio, sm_wr, WR, 1, false);
    // set initial pindirs: DIR and OE are output
    pio_sm_set_consecutive_pindirs(pio, sm_rd, DIR, 1, true);
    pio_sm_set_consecutive_pindirs(pio, sm_wr, DIR, 1, true);
    pio_sm_set_consecutive_pindirs(pio, sm_rd, OE, 1, true);
    pio_sm_set_consecutive_pindirs(pio, sm_wr, OE, 1, true);

    // pio 'in' pins: inputs start at the first data bit (D0)
    sm_config_set_in_pins(&smc_rd, D0);
    // pio 'out' pins: data D0-D7 can also be output
    sm_config_set_out_pins(&smc_rd, D0, 8);
    // pio 'set' pins: DIR (LSB) and OE (MSB)
    sm_config_set_set_pins(&smc_rd, DIR, 2);
    // Reading from RxFIFO: Shift to left, autopull disabled
    sm_config_set_in_shift(&smc_rd, false, false, 32);
    // Writing to TxFIFO: Shift to right, autopull disabled
    sm_config_set_out_shift(&smc_rd, true, false, 32);
    // pio 'in' pins: inputs start at the first data bit (D0)
    sm_config_set_in_pins(&smc_wr, D0);
    // pio 'out' pins: data D0-D7 can also be output
    sm_config_set_out_pins(&smc_wr, D0, 8);
    // pio 'set' pins: DIR (LSB) and OE (MSB)
    sm_config_set_set_pins(&smc_wr, DIR, 2);
    // Reading from RxFIFO: Shift to left, autopull disabled
    sm_config_set_in_shift(&smc_wr, false, false, 32);
    // Writing to TxFIFO: Shift to right, autopull disabled
    sm_config_set_out_shift(&smc_wr, true, false, 32);
    // set clock to about 4Mhz TODO: set correct frequency
    // sm_config_set_clkdiv(&smc, 31);
    // sm_config_set_clkdiv(&smc_rd, 16);
    // sm_config_set_clkdiv(&smc_wr, 16);
    // init the pio sm with the config
    pio_sm_init(pio, sm_rd, offset_rd, &smc_rd);
    pio_sm_init(pio, sm_wr, offset_wr, &smc_wr);
    // enable the sm
    pio_sm_set_enabled(pio, sm_rd, true);
    pio_sm_set_enabled(pio, sm_wr, true);
}

// in the current version the address bus is only 8 bits
// the first half is ROM, the second half is RAM
uint8_t ROM[128];
uint8_t RAM[128];

int main()
{
    // set the voltage a bit higher than default
    vreg_set_voltage(0b1100); // 1.15v
    // overclock to 270MHz
    set_sys_clock_khz(270000, true);

    // fill the ROM and RAM with numbers
    uint8_t num;
    for (num=0; num<128; num++) {
        ROM[num] = num;
        RAM[num] = 128+num;
    }

    // needed for printf
    stdio_init_all();

    // initialize the state machine
    configure_pio_sm();

    // print the gpio assignments
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
    pio_sm_exec(pio, sm_rd, offset_rd + Z80_read_offset_set_default);
    pio_sm_exec(pio, sm_wr, offset_wr + Z80_write_offset_set_default);

    uint bitoffs;
    const uint32_t mask = PIO_FLEVEL_RX0_BITS >> PIO_FLEVEL_RX0_LSB;

    while (1)
    {
        // test if there is something in RxFIFO of sm0 (a read)
        bitoffs = PIO_FLEVEL_RX0_LSB + sm_rd * (PIO_FLEVEL_RX1_LSB - PIO_FLEVEL_RX0_LSB);
        if ((pio->flevel >> bitoffs) & mask > 0)
        {
            // get the data from the RxFIFO
            addr_data = pio->rxf[sm_rd];
            // the lowest 8 bits are the data and the next 8 bits are the address
            // data = addr_data & 0xFF;
            address = addr_data >> 8;

            // determine if it is ROM or RAM, then get the value from ROM or RAM
            if (address<128) {
                data = ROM[address];
                printf("read ROM address %d results in %d\n", address, data);
            } else {
                data = RAM[address-128];
                printf("read RAM address %d results in %d\n", address-128, data);
            }
            // send the data to the pio for writing it to the data bits
            pio->txf[sm_rd] = data;
        }

        // test if there is something in RxFIFO of sm1 (a write)
        bitoffs = PIO_FLEVEL_RX0_LSB + sm_wr * (PIO_FLEVEL_RX1_LSB - PIO_FLEVEL_RX0_LSB);
        if ((pio->flevel >> bitoffs) & mask > 0)
        {
            // get the data from the RxFIFO
            addr_data = pio->rxf[sm_wr];
            data = addr_data & 0xFF;
            address = addr_data >> 8;
            if (address<128) {
                printf("ROM not writable\n");
            } else {
                RAM[address-128] = data;
                printf("Wrote %d to RAM address %d\n", data, address-128);
            }

        }
    }
}