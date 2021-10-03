/*

This code shows how a pio state machine can work with led panels. It is made for
two 64x64 led panels connected to form a 64 row x 128 column panel. There are 16
brightness levels for each Red, Green and Blue of each pixel, allowing many 
colors. In its present form it updates the panel at about 144 Hz at standard
clock settings (=125MHz.)

The main function has code for three test patterns.

The user prepares an image (variable 'image') and then encodes it to a
version that is suitable for the pio state machine (sm) to display.
The pio state machine (sm) continuously updates the display via direct 
memory access (DMA).

Setting the overall brightness:
- the panel works by setting RGB values on the LEDs of one row and letting them
  shine for a little bit of time. The variable 'overall_brightness' controls
  overall brightness (in addition to the 16 brightness levels of each red, 
  green, and blue pixel values). It can take on values from 0 to 7.
- If a higher brightness value is needed, the pio program can be changed: it has
  a delay loop with three 'nop [3]' statements (do nothing for 4 clock cycles).
  More 'nop' statements can be added. But this will also lower the update rate
  of the display.

*/
#include "stdio.h"
#include "pico/stdlib.h"
#include "hardware/pio.h"
#include "hardware/dma.h"
#include "hardware/irq.h"
#include "ledpanel.pio.h"

/******************************************************************************
 * Display settings
 *****************************************************************************/

// The display configuration:
//      I use two 64x64 ledpanels. to use a smaller configuration: just let part 
//      of the image blank (0 brightness)
// Do not change:
#define num_of_displays 2
#define columns_of_display 64
#define rows_of_display 64
#define half_rows_of_display 32

/* 
  The pin assignment has been chosen such that the PIO sm can use 'out PINS' 
  to immediately set both the pixel data and address data to the pins

  panel:            pin assignment:     cable (=mirror of pin assignment):
  -------------     -------------       -------------
  | R1  | G1  |     |  2  |  3  |       |  3  |  2  |
  -------------     -------------       -------------
  | B1  | GND |     |  4  | GND |       | GND |  4  |
  -------------     -------------       -------------
  | R2  | G2  |     |  5  |  6  |       |  6  |  5  |
  -------------     -------------       -------------
    B2  |  E  |        7  | 12  |       | 12  |  7  ||
  -------------     -------------       -------------
     A  |  B  |        8  |  9  |       |  9  |  8  ||
  -------------     -------------       -------------
  |  C  |  D  |     | 10  | 11  |       | 11  | 10  |
  -------------     -------------       -------------
  | CLK | LAT |     | 14  | 13  |       | 13  | 14  |
  -------------     -------------       -------------
  | OE  | GND |     | 15  | GND |       | GND | 15  |
  -------------     -------------       -------------

  Note: OE = output enable = inverse of blank
*/

// The pin assignments
#define panel_r1 2
#define panel_g1 3
#define panel_b1 4
#define panel_r2 5
#define panel_g2 6
#define panel_b2 7
#define address_a 8
#define address_b 9
#define address_c 10
#define address_d 11
#define address_e 12
#define latch 13
#define clock_p 14
#define blank 15

/******************************************************************************
 * Image and its encoding for the state machine
 *****************************************************************************/

// Overall brightness: a number from 0 (dim) to 7 (brightest)
//      the brightness levels of the pixels is 4 bits
//      the total data send to the sm should be 11 bits
//      the overall brightness shifts the 4 bits, so the max shift is 7 (11-4)
uint overall_brightness = 7;

// Just for easy use: assign numbers 0, 1 and 2 to Red, Green, and Blue
#define R 0
#define G 1
#define B 2
// the image to be displayed: image[row][column][RBG]=brightness level
uint image[rows_of_display][num_of_displays * columns_of_display][3];

// the image is encoded for output to the sm in the variable "encoded_image"
// Because of the construction of the led panel, you always send (x,y) and (x+32, y) pixels.
// Additionally, with each set of pixels the address is given
// So, from MSB to LSB: E, F, D, C, B, A, B2, G2, R2, B1, G1, R1
// This is 11 bits, so: two of these fit in a uint32_t (with room to spare: 10 bits)
//
// How brightness levels are made:
// see https://www.galliumstudio.com/2020/04/07/stm32-episode-3-color-tricks-with-led-panels/
// but here I've done the 'pwm' per row and not for a whole frame. Also, there are
// 16 brightness levels (4 bits).

// The size of the coded image for tranmission to the sm follows from:
//      128 lines
//      32 rows (two rows are transmitted simultaneously)
//      4 brightness level
//      2 pixels (and the address) per uint32_t
// -> 128*32*4 / 2 = 8192
//      additionally the overall brightness is set for each of the 32 rows and each of the 4 brightness levels
// -> 8192 + 32*4 = 8320
#define MAX_ITEMS 8320
uint32_t encoded_image[MAX_ITEMS];

// number of items to send to the sm via dma
uint num_of_items_to_dma;

// encode the image to be suitable for sending it to the sm
void encode_image()
{
    // variable to hold the data to be send to the sm
    uint32_t address_and_pixels;
    // counter to keep track of the number of items to be send
    uint num_of_items = 0;

    // 64 rows, but row i and i+1 are drawn simultaneously -> 32 rows
    for (uint8_t row = 0; row < half_rows_of_display; row++)
    {
        // 4 brightness level bits (8, 4, 2, 1)
        for (uint b = 8; b >= 1; b >>= 1)
        {
            for (uint8_t i = 0; i < num_of_displays * columns_of_display; i += 2)
            {
                // LED displays put the x and x+32 rows on the display at the same time.
                // Two pixels fit in one byte to be displayed. So, code the y and y+1 pixels

                // pixels (x,y) and (x+32, y)
                address_and_pixels = ((image[row][i][R] & b) > 0 ? 1 : 0);            // r1
                address_and_pixels |= ((image[row][i][B] & b) > 0 ? 1 : 0) << 1;      // g1
                address_and_pixels |= ((image[row][i][G] & b) > 0 ? 1 : 0) << 2;      // b1
                address_and_pixels |= ((image[row + 32][i][R] & b) > 0 ? 1 : 0) << 3; // r2
                address_and_pixels |= ((image[row + 32][i][B] & b) > 0 ? 1 : 0) << 4; // g2
                address_and_pixels |= ((image[row + 32][i][G] & b) > 0 ? 1 : 0) << 5; // b2
                // add the row address
                address_and_pixels |= row << 6;
                // pixels (x,y+1) and (x+32, y+1)
                address_and_pixels |= ((image[row][i + 1][R] & b) > 0 ? 1 : 0) << 11;      // r1
                address_and_pixels |= ((image[row][i + 1][B] & b) > 0 ? 1 : 0) << 12;      // g1
                address_and_pixels |= ((image[row][i + 1][G] & b) > 0 ? 1 : 0) << 13;      // b1
                address_and_pixels |= ((image[row + 32][i + 1][R] & b) > 0 ? 1 : 0) << 14; // r2
                address_and_pixels |= ((image[row + 32][i + 1][B] & b) > 0 ? 1 : 0) << 15; // g2
                address_and_pixels |= ((image[row + 32][i + 1][G] & b) > 0 ? 1 : 0) << 16; // b2
                // add the row address
                address_and_pixels |= row << 17;
                // add the data to the encoded image array to be sent to the pio sm
                if (num_of_items < MAX_ITEMS)
                    encoded_image[num_of_items++] = address_and_pixels;
            }
            // This controlls the overall brightness of the panels
            // In order not to disturb the 22 bit autopull, we send it twice
            address_and_pixels = (b << overall_brightness);
            address_and_pixels |= address_and_pixels << 11;
            if (num_of_items < MAX_ITEMS)
                encoded_image[num_of_items++] = address_and_pixels;
        }
    }
    // set the number of dataitems to be sent to the sm via dma
    num_of_items_to_dma = num_of_items;
}

/******************************************************************************
 * configuration of the sm and dma
 *****************************************************************************/

// variables for the pio and state machine (sm) to be used
PIO pio;
uint sm;
// the sm program offset and sm configuration
uint sm_offset;
pio_sm_config smc;
// the dma channel
uint dma_chan;

// configure the state machine
void configure_pio_sm()
{
    // pio to use
    pio = pio0;
    // set the GPIO to be used by the pio
    for (uint pin = panel_r1; pin <= blank; pin++)
        pio_gpio_init(pio, pin);
    // state machine to use
    sm = 0;
    // load the sm program into the pio memory
    sm_offset = pio_add_program(pio, &ledpanel_program);
    // make a sm config
    smc = ledpanel_program_get_default_config(sm_offset);
    // set pindirs all pins are under control of the PIO sm as output
    pio_sm_set_consecutive_pindirs(pio, sm, panel_r1, 14, true);
    // set out pin (r1, g1, b1, r2, g2, b2, a, b, c, d, e)
    sm_config_set_out_pins(&smc, panel_r1, 11);
    // set side-set pins (latch, clock, blank)
    sm_config_set_sideset_pins(&smc, latch);
    // set the correct ISR shift direction (see the .pio file)
    sm_config_set_in_shift(&smc, true, false, 0);
    // set autopull for out: NOTE: one set of pixels is 11 bits,
    // there are two sets per doubleword (32 bits) send to the Tx FIFO
    sm_config_set_out_shift(&smc, true, true, 22);
    // init the pio sm with the config
    pio_sm_init(pio, sm, sm_offset, &smc);
    // enable the state machines
    pio_sm_set_enabled(pio, sm, true);
}

// interrupt handler for the direct memory access
void dma_handler()
{
    // Clear the interrupt request
    dma_hw->ints0 = 1u << dma_chan;
    // set the number of data items to transfer
    dma_channel_set_trans_count(dma_chan, num_of_items_to_dma, false);
    // set the read address, this will trigger the DMA to start again
    dma_channel_set_read_addr(dma_chan, encoded_image, true);
}

// configure the direct memory access
void configure_dma()
{
    // init variable to hold the number of items to send via dma
    num_of_items_to_dma = 0;
    // Get free DMA channels, panic() if there are none
    dma_chan = dma_claim_unused_channel(true);
    // make DMA configs
    dma_channel_config dma_conf = dma_channel_get_default_config(dma_chan);
    // transfer uint32_t
    channel_config_set_transfer_data_size(&dma_conf, DMA_SIZE_32);
    // the buffer increment of read pointer
    channel_config_set_read_increment(&dma_conf, true);
    // the TxFIFO is fixed in memory -> no increment write pointer
    channel_config_set_write_increment(&dma_conf, false);
    // let the sm of pio determine the speed
    channel_config_set_dreq(&dma_conf, pio_get_dreq(pio, sm, true));
    // configure the dma channel to write to sm TxFIFO from buffer
    dma_channel_configure(dma_chan, &dma_conf, &pio0_hw->txf[0], NULL, 0, false);
    // enable IRQ for the channel
    dma_channel_set_irq0_enabled(dma_chan, true);
    // set the handler
    irq_set_exclusive_handler(DMA_IRQ_0, dma_handler);
    // enable IRQ
    irq_set_enabled(DMA_IRQ_0, true);
}

/******************************************************************************
 * some simple routines to draw the image for the test patterns
 *****************************************************************************/

// set the color + brightness for a pixel
void set_pixel(uint row, uint column, uint c, uint brightness)
{
    image[row][column][c] = brightness;
    // leave the other two colors untouched to allow mixing
}

// set the individual colors + brightnesses for a pixel
void set_pixel_c(uint row, uint column, uint R_brightness, uint G_brightness, uint B_brightness)
{
    image[row][column][R] = R_brightness;
    image[row][column][G] = G_brightness;
    image[row][column][B] = B_brightness;
}

// draw a horizontal line
void row_line(uint row, uint column_start, uint column_end, uint c, uint brightness)
{
    for (uint column = column_start; column < column_end; column++)
        set_pixel(row, column, c, brightness);
}

// draw a vertical line
void column_line(uint column, uint row_start, uint row_end, uint c, uint brightness)
{
    for (uint row = row_start; row < row_end; row++)
        set_pixel(row, column, c, brightness);
}

// make an empty image
void clear_image()
{
    uint8_t x, y, c;
    for (x = 0; x < 64; x++)
        for (y = 0; y < 128; y++)
            for (c = 0; c < 3; c++)
                image[x][y][c] = 0;
}

// color wheel (from Adafruit strand test)
//      Input a value 0 to 255 to get a color value.
//      The colors returned are a transition r - g - b - back to r.
void wheel(uint wheel_pos, uint *RC, uint *GC, uint *BC)
{
    if (wheel_pos < 85)
    {
        *RC = wheel_pos * 3;
        *GC = 255 - wheel_pos * 3;
        *BC = 0;
    }
    else if (wheel_pos < 170)
    {
        wheel_pos -= 85;
        *BC = wheel_pos * 3;
        *RC = 255 - wheel_pos * 3;
        *GC = 0;
    }
    else
    {
        wheel_pos -= 170;
        *GC = wheel_pos * 3;
        *BC = 255 - wheel_pos * 3;
        *RC = 0;
    }
}

/******************************************************************************
 * the main function: initializes everything and starts a loop of test patterns
 *****************************************************************************/

int main()
{
    // needed for printf
    stdio_init_all();
    // prepare the sm
    configure_pio_sm();
    // prepare the dma
    configure_dma();
    // prepare an empty image
    clear_image();
    // convert the image to data for the pio sm
    encode_image();
    // start the dma
    dma_handler();

    // show the test patterns forever
    while (true)
    {
        //
        // Test pattern 3: make the rainbow pattern shift with time
        //

        for (int current_t = 0; current_t < 255; current_t++)
        {
            uint r, g, b;
            // make a rainbow pattern
            for (uint row = 0; row < rows_of_display; row++)
                for (uint column = 0; column < num_of_displays * columns_of_display; column++)
                {
                    wheel((current_t + row + column) % 256, &r, &g, &b);
                    set_pixel_c(row, column, (int)(r * 0.0627), (int)(g * 0.0627), (int)(b * 0.0627));
                }
            // encode and draw the image
            encode_image();
        }

        //
        // Test pattern 2: draw red and blue planes with 16 different intensity levels and change the overall brightness
        //

        // prepare an empty image
        clear_image();
        uint prev_fraction = 0;
        uint next_fraction = 0;
        // brightness has 16 levels (4 bits)
        for (uint fraction = 1; fraction <= 16; fraction++)
        {
            next_fraction = (int)((float)fraction / 16. * (num_of_displays * columns_of_display));
            for (uint current_y = prev_fraction; current_y < next_fraction; current_y++)
                column_line(current_y, 0, rows_of_display, R, fraction - 1);
            prev_fraction = next_fraction;
        }

        prev_fraction = 0;
        // brightness has 16 levels (4 bits)
        for (uint fraction = 1; fraction <= 16; fraction++)
        {
            next_fraction = (int)((float)fraction / 16. * rows_of_display);
            for (uint current_x = prev_fraction; current_x < next_fraction; current_x++)
                row_line(current_x, 0, num_of_displays * columns_of_display, B, fraction - 1);
            prev_fraction = next_fraction;
        }
        // change the overall brightness
        for (uint b = 0; b <= 7; b++)
        {
            overall_brightness = b;
            // encode and draw the image
            encode_image();
            // pause 1 seconds
            sleep_ms(1000);
        }

        //
        // Test pattern 1: moving lines
        //

        for (uint current_t = 0; current_t < num_of_displays * columns_of_display; current_t++)
        {
            uint current_x = current_t % rows_of_display;
            uint current_y = current_t;
            // prepare an empty image
            clear_image();
            // draw red, green and blue lines at high brightness
            column_line(current_y, 0, rows_of_display, R, 15);
            column_line((current_y+64)%(num_of_displays * columns_of_display), 0, rows_of_display, G, 15);
            row_line(current_x, 0, num_of_displays * columns_of_display, B, 15);
            // encode and draw the image
            encode_image();
            // pause for a brief time
            sleep_ms(100);
        }
    }
}
