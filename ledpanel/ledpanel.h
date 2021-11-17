#ifndef LEDPANELH
#define LEDPANELH

/*

This code shows how a pio state machine can work with led panels. It is made for
two 64x64 led panels connected to form a 64 row x 128 column panel. There are 16
brightness levels for each Red, Green and Blue of each pixel, allowing many 
colors. In addition there is a 8 level overall brightness. 
In its present form it updates the panel at about 144 Hz at standard
clock settings (=125MHz.)

Concept of operation
The user prepares an image. In this code this happens on core 0. 
The main function has code for three test patterns, that each produce several 
images to make a small animation.
These images are transcoded on core 1 to a datastructure suitable for the pio 
state machine (sm) to control the display. The pio state machine (sm) 
continuously updates the display via direct memory access (DMA).


*/

#include "stdio.h"
#include "pico/stdlib.h"
#include "hardware/pio.h"
#include "hardware/dma.h"
#include "hardware/irq.h"
#include "ledpanel.pio.h"
#include "pico/multicore.h"

/******************************************************************************
 * Display settings
 *****************************************************************************/

// The display configuration:
//      I use two 64x64 ledpanels. to use a smaller configuration: just let part
//      of the image blank (0 brightness)
// Do not change (the PIO code assumes this configuration hard coded):
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

// TODO: remove (only for testing purposes)
#define timing_pin_DMA 16
#define timing_pin_convert 17
#define timing_pin_build_image 18

/******************************************************************************
 * Image and its encoding for the state machine
 *****************************************************************************/

/* 
Color and brightness 

Each pixel in a ledpanel has three LEDS: a red, green and blue LED. The panel 
works by setting a 0 or 1 for each RGB LED for each pixel for one row and letting 
them shine for a little bit of time. Each color for each pixel in this code has 
4 bits. This means that each row is written 4 times, where the 4th bit of each 
color is written first, then the 3th, etc. This results in a (sort of) 'Pulse 
Width Modulation' (PWM), 
see https://www.galliumstudio.com/2020/04/07/stm32-episode-3-color-tricks-with-led-panels/.
Note that here I've done the 'pwm' per row and not for a whole frame. 
The time the LEDs are on is the longest for the 4th bit and the shortest for
the 1st bit.


Setting the overall brightness:
The variable 'overall_brightness' controls overall brightness in addition to the 
16 brightness levels of each red, green, and blue pixel values. It does this by
bit-shifting the delay time to higher values. It can take on values from 0 to 7.
Taking the 4 bits encoding of the color, and shifting it by a maximum of 7 bits 
results in a 11-bit number. This number is used as the counter for the delay loop.

If a higher brightness value is needed, the pio program can be changed: it has
a delay loop with three 'nop [3]' statements (do nothing for 4 clock cycles).
More 'nop' statements can be added. But this will also lower the update rate
of the display.

*/
extern uint overall_brightness;

// Just for ease of use: assign numbers 0, 1 and 2 to Red, Green, and Blue
#define R 0
#define G 1
#define B 2

/*
The image to be displayed

The RGB values for each pixel are stored in an image variable:
image[row][column]=RGB_brightness 
For each R, G and B 4 bits are used: Red (no shift), Blue (shift 4 bits), 
Green (shift 8 bits). So the values in image look like: 
g4, g3, g2, g1, b4, b3, b2, b1, r4, r3, r2, r1
where g4 is the highest brightness bit of green, g1 is the lowest brightness 
bit of green, similar for blue and red

Double buffering is used, so two image variables:
*/
extern uint image1[rows_of_display][num_of_displays * columns_of_display];
extern uint image2[rows_of_display][num_of_displays * columns_of_display];
// the pointer to one of the two above image variables
extern uint (*image)[num_of_displays * columns_of_display];

/*
Both cores of the Pico are used:
    core 0: generates images
    core 1: processes images to be send to the PIO state machine (SM), and controls the DMA to the SM
Some coordination is required to make sure that all images that are prepared by
core 0 are actually displayed by core 1. Also some coordination is needed to 
make sure that the image data is only overwritten by core 0 after it has been 
processed for displaying by core 1. 
*/
// function to be called from core 0
extern void core1_worker();
// indicates which image is ready (set by core 0)
extern uint image_ready;
// indicates which image is being processed on core 1
extern uint image_processing;

#endif 
