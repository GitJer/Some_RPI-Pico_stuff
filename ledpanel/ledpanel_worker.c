#include "stdio.h"
#include "pico/stdlib.h"
#include "hardware/pio.h"
#include "hardware/dma.h"
#include "hardware/irq.h"
#include "ledpanel.pio.h"
#include "pico/multicore.h"
#include "hardware/interp.h"

#include "ledpanel.h"

// local (i.e. core 1) pointer to the image variable that contains the image information
uint (*image_to_encode)[num_of_displays * columns_of_display];

// the image is encoded for output to the sm in the variable "encoded_image"
// Because of the construction of the led panel, you always send (x,y) and (x+32, y) pixels.
// Additionally, with each set of pixels the address is given.
// So, from MSB to LSB: E, F, D, C, B, A, g(row+32), b(row+32), r(row+32), g(row), b(row), r(row)
//      where E,F,D,C,B and A encode the row address.
//      and where g, b and r are bits for a brightness level, each color is encoded with 4 bits. See the
//      loop 'for (int b = 3; b >= 0; b--)' in the code below.
// This is 11 bits, so: two of these fit in a uint32_t (with room to spare: 10 bits)

// The size of the transcoded image for tranmission to the sm follows from:
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
    gpio_put(timing_pin_convert, 1);// TODO: remove (only for testing purposes)

    // variable to hold the data to be send to the sm
    uint32_t address_and_pixels;
    // counter to keep track of the number of items to be send
    uint num_of_items = 0;

    /* 

    YES YES YES! I finally found a good use for the interpolator hardware!!!!

    It may not be what it was intended for, but hey: it works!
    And it is faster than a lot of 'and', 'bit shift' and 'or' operations!

    The following bit shifting/reordering is needed to transcode the RGB values as
    stored in the image variables to the format required by the PIO sm to send
    to the ledpanel:
    The images contain pixel information in the format:
        g4, g3, g2, g1, b4, b3, b2, b1, r4, r3, r2, r1
    but the sm needs the bits to be in a different order: 
    for the highest brightness bits (b=3): 
        g4, b4, r4
    down to the lowest brightness color information (b=0):
        g1, b1, r1

    So, the interpolator is configured as follows:
    - the input is the pixel color information shifted over b bits
    - The Base 2 variable contains the b'th bit for red
        see the lines in the code below:
            uint value = image_to_encode[row][i] >> b;
            interp0->base[2] = value & 0x01;
        so, the first bit in interpolator's 'Result 2' is the b'th bit for red
    - Lane 0 shifts the pixel info three bits right and masks the result in
      such a way that the b'th bit of blue is now the second bit in 'Result 2'
    - Lane 1 shifts the pixel info 6 bits right and masks the result in such
      a way that the b'th bit of green is now the third bit in 'Result 2'
    */

    // Initialise lane 0 on interp0 on this core
    interp_config cfg0 = interp_default_config();
    // shift right 3 bits
    interp_config_set_shift(&cfg0, 3);
    // mask lets bit 1 (the 2nd bit) pass
    interp_config_set_mask(&cfg0, 1, 1);
    interp_set_config(interp0, 0, &cfg0);

    // Initialise lane 1 on interp0 on this core
    interp_config cfg1 = interp_default_config();
    // shift right 6 bits
    interp_config_set_shift(&cfg1, 6);
    // mask lets bit 2 (the 3rd bit) pass
    interp_config_set_mask(&cfg1, 2, 2);
    interp_set_config(interp0, 1, &cfg1);

    // 64 rows, but row i and i+1 are drawn simultaneously -> 32 rows
    for (uint8_t row = 0; row < half_rows_of_display; row++)
    {
        // 4 brightness level bits for color in each pixel
        for (int b = 3; b >= 0; b--)
        {
            // all columns
            for (uint8_t i = 0; i < num_of_displays * columns_of_display; i += 2)
            {
                // ledpanel displays put the x and x+32 rows on the display at the same time.
                // Additionally, two pixels fit in one byte to be displayed. So, code the y and y+1 pixels

                // pixel (x,y)
                uint value = image_to_encode[row][i] >> b;
                // printf("1 image_to_encode=%d value=%d\n", image_to_encode[row][i], value);
                interp0->base[2] = value & 0x01;
                interp0->accum[0] = value;
                interp0->accum[1] = value;
                address_and_pixels = interp0->peek[2];
                // pixel (x+32, y)
                value = image_to_encode[row + 32][i] >> b;
                // printf("2 image_to_encode=%d value=%d\n", image_to_encode[row + 32][i], value);
                interp0->base[2] = value & 0x01;
                interp0->accum[0] = value;
                interp0->accum[1] = value;
                address_and_pixels |= interp0->peek[2] << 3;
                // add the row address
                address_and_pixels |= row << 6;
                // pixel (x,y+1)
                value = image_to_encode[row][i + 1] >> b;
                // printf("3 image_to_encode=%d value=%d\n",image_to_encode[row][i + 1], value);
                interp0->base[2] = value & 0x01;
                interp0->accum[0] = value;
                interp0->accum[1] = value;
                address_and_pixels |= interp0->peek[2] << 11;
                // pixel (x+32, y+1)
                value = image_to_encode[row + 32][i + 1] >> b;
                // printf("4 image_to_encode=%d value=%d\n",image_to_encode[row + 32][i + 1], value);
                interp0->base[2] = value & 0x01;
                interp0->accum[0] = value;
                interp0->accum[1] = value;
                address_and_pixels |= interp0->peek[2] << 14;
                // add the row address
                address_and_pixels |= row << 17;
                // add the data to the encoded image array to be sent to the pio sm
                if (num_of_items < MAX_ITEMS)
                    encoded_image[num_of_items++] = address_and_pixels;
            }
            // This controlls the overall brightness of the panels
            // In order not to disturb the 22 bit autopull, it is sent twice
            address_and_pixels = 1 << (overall_brightness + b);
            address_and_pixels |= address_and_pixels << 11;
            if (num_of_items < MAX_ITEMS)
                encoded_image[num_of_items++] = address_and_pixels;
        }
    }
    // set the number of dataitems to be sent to the sm via dma
    num_of_items_to_dma = num_of_items;

    gpio_put(timing_pin_convert, 0);// TODO: remove (only for testing purposes)
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
    gpio_put(timing_pin_DMA, 1);// TODO: remove (only for testing purposes)
    // Clear the interrupt request
    dma_hw->ints0 = 1u << dma_chan;
    // set the number of data items to transfer
    dma_channel_set_trans_count(dma_chan, num_of_items_to_dma, false);
    // set the read address, this will trigger the DMA to start again
    dma_channel_set_read_addr(dma_chan, encoded_image, true);
    gpio_put(timing_pin_DMA, 0);// TODO: remove (only for testing purposes)
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

void core1_worker()
{
    // TODO: remove (only for testing purposes)
    gpio_init(timing_pin_DMA);
    gpio_set_dir(timing_pin_DMA, GPIO_OUT);
    gpio_init(timing_pin_convert);
    gpio_set_dir(timing_pin_convert, GPIO_OUT);

    // wait for an image to be finished by core 0
    while (image_ready == 0)
        ;
    if (image_ready == 1)
    {
        // the image in variable image1 is ready for transcoding
        image_to_encode = image1;
        // indicate that image1 is what this core is working on
        image_processing = image_ready;
    }
    else
    {
        // the image in variable image2 is ready for transcoding
        image_to_encode = image2;
        // indicate that image2 is what this core is working on
        image_processing = image_ready;
    }

    // prepare the sm
    configure_pio_sm();
    // prepare the dma
    configure_dma();
    // convert the image to data for the pio sm
    encode_image();
    // start the dma
    dma_handler();
    image_processing = 0;

    while (true)
    {
        // if image1 is ready
        if (image_ready == 1)
        {   
            // the image in variable image1 is ready for transcoding
            // indicate that image1 is what this core is working on
            image_processing = image_ready;
            // indicate that core 0 can continue with generating a new image
            image_ready = 0;
            image_to_encode = image1;
            // do the encoding
            encode_image();
            // indicate transcoding has finished
            image_processing = 0;
        }
        else if (image_ready == 2)
        {
            // the image in variable image2 is ready for transcoding
            // indicate that image2 is what this core is working on
            image_processing = image_ready;
            image_ready = 0;
            // indicate that core 0 can continue with generating a new image
            image_to_encode = image2;
            // do the encoding
            encode_image();
            // indicate transcoding has finished
            image_processing = 0;
        }
    }
}
