
#include "stdio.h"
#include "pico/stdlib.h"
#include "hardware/pio.h"
#include "hardware/dma.h"
#include "hardware/irq.h"
#include "hardware/interp.h"
#include "ledpanel.pio.h"
#include "pico/multicore.h"
#include "ledpanel.h"

// Overall brightness: a number from 0 (dim) to 7 (brightest)
uint overall_brightness = 7;

// the image to be displayed (a pointer) and the two actual variables for double buffering
uint currently_drawing = 1;
uint (*image)[num_of_displays * columns_of_display];
uint image1[rows_of_display][num_of_displays * columns_of_display];
uint image2[rows_of_display][num_of_displays * columns_of_display];

// 0 = no image ready or processing finished
// 1 = producer (core 0, in this file) has finished producing image 1
// 2 = producer (core 0)) has finished producing image 2
uint image_ready = 0;
uint image_processing = 0;

/******************************************************************************
 * some simple routines to draw the image for the test patterns
 *****************************************************************************/

// set the color + brightness for a pixel
void set_pixel(uint row, uint column, uint c, uint brightness)
{
    // leave the other two colors untouched to allow mixing
    if (c == R) // Red: do not shift
        image[row][column] |= brightness;
    else if (c == G) // Green: shift over 8 bits
        image[row][column] |= brightness << 8;
    else // Blue: shift 4 bits
        image[row][column] |= brightness << 4;
}

// set the individual colors + brightnesses for a pixel
void set_pixel_c(uint row, uint column, uint R_brightness, uint G_brightness, uint B_brightness)
{
    image[row][column] = R_brightness | G_brightness << 8 | B_brightness << 4;
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
    for (uint x = 0; x < rows_of_display; x++)
        for (uint y = 0; y < num_of_displays * columns_of_display; y++)
            image[x][y] = 0;
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

// switch image buffers to be drawn (dubbelbuffering)
// However, wait if the process on core1 is still busy with a buffer
void switch_buffer()
{
    image_ready = currently_drawing;
    if (currently_drawing == 1)
    {
        while (image_processing == 2)
            ;
        image = image2;
        currently_drawing = 2;
    }
    else
    {
        while (image_processing == 1)
            ;
        image = image1;
        currently_drawing = 1;
    }
}

/******************************************************************************
 * the main function: initializes everything and starts a loop of test patterns
 *****************************************************************************/

int main()
{

    // start the dubbel buffer with image1
    image = image1;
    currently_drawing = 1;

    // TODO: remove (only for testing purposes)
    gpio_init(timing_pin_build_image);
    gpio_set_dir(timing_pin_build_image, GPIO_OUT);

    // needed for printf
    stdio_init_all();

    // start with an empty image
    clear_image();

    // Start core 1. Core 1 continuously transcodes the image
    // to a format that can be used with the pio code on the
    // state machine..
    multicore_launch_core1(core1_worker);

    // show the test patterns forever
    while (true)
    {

        // /*
        //
        // Test pattern 3: make the rainbow pattern that shifts with time
        //

        for (int current_t = 0; current_t < 255; current_t++)
        {
            gpio_put(timing_pin_build_image, 1); // TODO: remove (only for testing purposes)
            uint r, g, b;
            // make a rainbow pattern
            for (uint row = 0; row < rows_of_display; row++)
                for (uint column = 0; column < num_of_displays * columns_of_display; column++)
                {
                    wheel((current_t + row + column) % 256, &r, &g, &b);
                    // Note: green is a bit too strong on my panels -> lower the multiplication factor
                    set_pixel_c(row, column, (int)(r * 0.0627), (int)(g * 0.04), (int)(b * 0.0627));
                }

            gpio_put(timing_pin_build_image, 0); // TODO: remove (only for testing purposes)
            // signal to core 1 that the image is ready, and switch image buffer
            switch_buffer();
        }
        // */

        // /*
        //
        // Test pattern 2: draw red and blue planes with 16 different intensity levels and change the overall brightness
        //

        // change the overall brightness
        for (uint b = 0; b <= 7; b++)
        {
            gpio_put(timing_pin_build_image, 1); // TODO: remove (only for testing purposes)

            clear_image();

            uint prev_fraction = 0;
            uint next_fraction = 0;
            // Red color planes with brightness in 16 levels (4 bits)
            for (uint fraction = 1; fraction <= 16; fraction++)
            {
                next_fraction = (int)((float)fraction / 16. * (num_of_displays * columns_of_display));
                for (uint current_y = prev_fraction; current_y < next_fraction; current_y++)
                    column_line(current_y, 0, rows_of_display, R, fraction - 1);
                prev_fraction = next_fraction;
            }

            prev_fraction = 0;
            // Blue color planes with brightness in 16 levels (4 bits)
            for (uint fraction = 1; fraction <= 16; fraction++)
            {
                next_fraction = (int)((float)fraction / 16. * rows_of_display);
                for (uint current_x = prev_fraction; current_x < next_fraction; current_x++)
                    row_line(current_x, 0, num_of_displays * columns_of_display, B, fraction - 1);
                prev_fraction = next_fraction;
            }

            overall_brightness = b;
            gpio_put(timing_pin_build_image, 0); // TODO: remove (only for testing purposes)
            // signal to core 1 that the image is ready, and switch image buffer
            switch_buffer();
            // pause 1 seconds
            sleep_ms(1000);
        }
        //    */

        //   /*
        //
        // Test pattern 1: moving lines
        //

        // repeat this test pattern a number of times
        for (int count = 0; count < 5; count++)
        {
            for (uint current_t = 0; current_t < num_of_displays * columns_of_display; current_t++)
            {
                gpio_put(timing_pin_build_image, 1); // TODO: remove (only for testing purposes)
                uint current_x = current_t % rows_of_display;
                uint current_y = current_t;
                // prepare an empty image
                clear_image();
                // draw red, green and blue lines at high brightness
                column_line(current_y, 0, rows_of_display, R, 15);
                column_line((current_y + 64) % (num_of_displays * columns_of_display), 0, rows_of_display, G, 15);
                row_line(current_x, 0, num_of_displays * columns_of_display, B, 15);
                gpio_put(timing_pin_build_image, 0); // TODO: remove (only for testing purposes)
                // signal to core 1 that the image is ready, and switch image buffer
                switch_buffer();
            }
        }
        // */
    }
}
