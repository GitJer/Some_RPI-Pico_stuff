#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include "pico/stdlib.h"
#include "hardware/pio.h"
#include "hardware/clocks.h"
#include "ws2812_led_strip_120.pio.h"

static inline void put_pixel(uint32_t pixel_grb)
{
    pio_sm_put_blocking(pio0, 0, pixel_grb << 8u);
}

static inline uint32_t urgb_u32(uint8_t r, uint8_t g, uint8_t b)
{
    return ((uint32_t)(r) << 8) |
           ((uint32_t)(g) << 16) |
           (uint32_t)(b);
}

void all_red()
{
    for (uint i = 0; i < 120; ++i)
        put_pixel(urgb_u32(0x80, 0, 0));
}

void all_green()
{
    for (uint i = 0; i < 120; ++i)
        put_pixel(urgb_u32(0, 0x80, 0));
}

void all_blue()
{
    for (uint i = 0; i < 120; ++i)
        put_pixel(urgb_u32(0, 0, 0x80));
}

void white(int level)
{
    for (uint i = 0; i < 120; ++i)
        put_pixel(urgb_u32(level, level, level));
}

const int PIN_TX = 2;

int main()
{
    // needed for printf
    stdio_init_all();
    // the pio instance
    PIO pio;
    // the state machine
    uint sm;
    // pio 0 is used
    pio = pio0;
    // state machine 0
    sm = 0;
    // configure the used pins
    pio_gpio_init(pio, PIN_TX);
    // load the pio program into the pio memory
    uint offset = pio_add_program(pio, &ws2812_led_strip_120_program);
    // make a sm config
    pio_sm_config c = ws2812_led_strip_120_program_get_default_config(offset);
    // set the 'set' pin
    sm_config_set_set_pins(&c, PIN_TX, 1);
    // set the pindirs to output
    pio_sm_set_consecutive_pindirs(pio, sm, PIN_TX, 1, true);
    // set out shift direction
    sm_config_set_out_shift(&c, false, true, 24);
    // set in shift direction
    sm_config_set_in_shift(&c, false, false, 0);
    // join the FIFOs
    sm_config_set_fifo_join(&c, PIO_FIFO_JOIN_TX);
    // init the pio sm with the config
    pio_sm_init(pio, sm, offset, &c);
    // enable the sm
    pio_sm_set_enabled(pio, sm, true);

    // int maxRows = 11;
    // int maxCols = 10;
    //     bool transition[11][10] = {
    //         {0, 0, 0, 0, 0, 0, 0, 0, 0, 0}, // 10
    //         {0, 0, 0, 0, 0, 0, 0, 0, 0, 1}, // 9
    //         {0, 0, 0, 0, 1, 0, 0, 0, 0, 1}, // 8
    //         {0, 0, 0, 1, 0, 0, 1, 0, 0, 1}, // 7
    //         {0, 0, 1, 0, 1, 0, 0, 1, 0, 1}, // 6
    //         {0, 1, 0, 1, 0, 1, 0, 1, 0, 1}, // 5
    //         {0, 1, 0, 1, 1, 0, 1, 0, 1, 1}, // 4
    //         {0, 1, 1, 0, 1, 1, 0, 1, 1, 1}, // 3
    //         {0, 1, 1, 1, 1, 0, 1, 1, 1, 1}, // 2
    //         {0, 1, 1, 1, 1, 1, 1, 1, 1, 1}, // 1
    //         {1, 1, 1, 1, 1, 1, 1, 1, 1, 1}, // 0
    //     };

    int maxRows = 13;
    int maxCols = 12;
    bool transition[13][12] = {
        {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0}, // 12
        {1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0}, // 11
        {1, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0}, // 10
        {1, 0, 0, 0, 1, 0, 0, 0, 1, 0, 0, 0}, // 9
        {1, 0, 0, 1, 0, 0, 1, 0, 0, 1, 0, 0}, // 8
        {1, 0, 1, 0, 1, 0, 0, 1, 0, 1, 0, 0}, // 7
        {1, 0, 1, 0, 1, 0, 1, 0, 1, 0, 1, 0}, // 6
        {0, 1, 0, 1, 0, 1, 1, 0, 1, 0, 1, 1}, // 5
        {0, 1, 1, 0, 1, 1, 0, 1, 1, 0, 1, 1}, // 4
        {0, 1, 1, 1, 0, 1, 1, 1, 0, 1, 1, 1}, // 3
        {0, 1, 1, 1, 1, 1, 0, 1, 1, 1, 1, 1}, // 2
        {0, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1}, // 1
        {1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1}, // 0
    };

    int maxLevel = 20;
    while (1)
    {
        // without dithering
        for (int level = 0; level < maxLevel; level++)
            // next loop is just to make it equal to the dithered version below
            for (int row = 0; row < maxRows * maxCols; row++) 
                white(level);
        for (int level = maxLevel; level > 0; level--)
            // next loop is just to make it equal to the dithered version below
            for (int row = maxRows * maxCols - 1; row >= 0; row--) 
                white(level);
        // with dithering
        for (int level = 0; level < maxLevel; level++)
            for (int row = 0; row < maxRows; row++)
                for (int col = 0; col < maxCols; col++)
                    white(level + transition[row][col]);
        for (int level = maxLevel; level > 0; level--)
            for (int row = maxRows - 1; row >= 0; row--)
                for (int col = 0; col < maxCols; col++)
                    white(level + transition[row][col]);
    }
}