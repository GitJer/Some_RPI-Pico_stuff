/*

This is remake of the wonderful little thingy made by Paul Dietz. See:
https://hackaday.com/2018/08/21/an-led-you-can-blow-out-with-no-added-sensor/
https://github.com/paulhdietz/LEDSensors/blob/master/_07_BlowOutLED/_07_BlowOutLED.ino

From the hackaday article:
"Turning the LED on warms it up and blowing on it cools it off, causing measurable 
changes in the voltage drop across the device. The change isn’t much — only a 
handful of millivolts — but the effect is consistent and can be measured. "

Where his code is rather simple and elegant, mine is convoluted.
Why? Because I wanted to play with the DMA sniffer!

The DMA sniffer allows summing of all values that pass through the DMA.
So, there is no need to sum the values yourself afterwards.

The code does the following:
- it turns the LED on for at least 1s to warm it up
- it starts a repeat timer to call "repeating_timer_callback"
- in that callback the adc is read NUM_ADC_SAMPLES times through dma, 
  the samples are summed by the dma sniffer
- an array of the obtained sums is kept up to date to determine the average of the sums
- if the read summed value deviates sufficiently from the average, a flag is set to switch the LED off
- in the main loop the flag is checked and when set the LED goes off for 1s and then 1s on to warm it up
*/

#include <stdio.h>
#include <math.h>
#include "pico/stdlib.h"
#include "hardware/gpio.h"
#include "hardware/adc.h"
#include "hardware/dma.h"

// LED Connections: PLUS pin - resistor - ADC pin - LED - GND
#define PLUS 22
// pin for ADC goes between resistor and LED
#define MEASURE 26
// Number of adc samples to take. Only used in dma_channel_configure
#define NUM_ADC_SAMPLES 512
// Number of samples to determine the average
#define NUM_AV_SAMPLES 64
// Minimum jump for blow out
// depends on the resistor value and type of LED
#define BLOWN 400

// the array with NUM_ADC_SAMPLES summed adc samples
float summed_adc_values[NUM_AV_SAMPLES];
// keeps track of where in the summed_adc_values array the new value is to be written
uint8_t sample_num;
// dma channel number
uint dma_chan;
// the total of the summed_adc_values array
float total;
// flag to indicate it the led should be turned off
bool light_out;
// the variable into which the dma channel dumps all adc readings!
uint32_t temp;

// the callback function that is called automatically (see add_repeating_timer_ms below)
bool repeating_timer_callback(struct repeating_timer *t)
{
    // start the sniff sum at 0
    dma_hw->sniff_data = 0;
    // start the dma_channel. NOTE: it doesn't write to a buffer, it writes to a single variable
    // the samples are summed by the sniffer functionality!
    dma_channel_set_write_addr(dma_chan, &temp, true);
    // wait for it to finish
    dma_channel_wait_for_finish_blocking(dma_chan);

    // check whether the deviations of the current sample deviates more than BLOWN from the average
    // the value of BLOWN depends on many things, e.g. resistor, type of LED, NUM_AV_SAMPLES, and NUM_ADC_SAMPLES
    // the print statement can be used to determine which BLOWN value will work for you
    // printf("%f\n", (total / 64.) - summed_adc_values[sample_num]);
    if (((total / 64.) - dma_hw->sniff_data) > BLOWN)
        light_out = true;

    // save the summed adc values in the summed_adc_values array
    // total is the sum of all elements in that array
    total -= summed_adc_values[sample_num];
    summed_adc_values[sample_num] = dma_hw->sniff_data;
    total += summed_adc_values[sample_num];

    // make the array a ring
    if (++sample_num == NUM_AV_SAMPLES)
        sample_num = 0;

    return true;
}

int main()
{
    // needed for printf
    stdio_init_all();

    // switch on the LED and let it warm up
    gpio_init(PLUS);
    gpio_set_dir(PLUS, GPIO_OUT);
    gpio_put(PLUS, 1);
    sleep_ms(1000);

    // initialize some variables
    light_out = false;
    sample_num = 0;
    total = 0;
    for (int i = 0; i < NUM_AV_SAMPLES; i++)
        summed_adc_values[i] = 0;

    // ========== ADC setup ===================
    adc_init();
    adc_fifo_setup(
        true,  // Write each completed conversion to the sample FIFO
        true,  // Enable DMA data request (DREQ)
        1,     // DREQ (and IRQ) asserted when at least 1 sample present
        false, // disable ERR bit
        false  // Do not shift each sample to 8 bits when pushing to FIFO, i.e. keep it 12 bit resolution
    );
    // Divisor of 0 -> full speed.
    adc_set_clkdiv(0);
    // start the adc
    adc_run(true);

    // ========== DMA setup ===================
    // Set up the DMA to start transferring data as soon as it appears in FIFO
    dma_chan = dma_claim_unused_channel(true);
    dma_channel_config cfg = dma_channel_get_default_config(dma_chan);
    // data size is 32 bits (it should be at least 12)
    channel_config_set_transfer_data_size(&cfg, DMA_SIZE_32);
    // Reading from AND WRITING to a constant address
    channel_config_set_read_increment(&cfg, false);
    channel_config_set_write_increment(&cfg, false);
    // Pace transfers based on availability of ADC samples
    channel_config_set_dreq(&cfg, DREQ_ADC);
    // configure sniff to automatically sum (mode=0xf) all the data
    dma_sniffer_enable(dma_chan, 0xf, true);
    // enable sniff
    channel_config_set_sniff_enable(&cfg, true);
    // configure the channel
    dma_channel_configure(dma_chan, &cfg,
                          NULL,            // dst
                          &adc_hw->fifo,   // src
                          NUM_ADC_SAMPLES, // transfer count
                          false            // start immediately
    );

    // ===========TIMER setup =================
    struct repeating_timer timer;
    add_repeating_timer_ms(10, repeating_timer_callback, NULL, &timer);
    // ========================================

    while (true)
    {
        if (light_out)
        { // switch the LED off
            gpio_put(PLUS, 0);
            sleep_ms(1000);
            // switch it back on and let it warm up
            gpio_put(PLUS, 1);
            sleep_ms(1000);
            // clear flag
            light_out = false;
        }
    }
}
