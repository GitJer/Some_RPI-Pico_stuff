# Ws2812 led strip with 120 pixels 

This is my take on how to control a ws2812 led strip with 120 pixels. It differs from the [pio example code](https://github.com/raspberrypi/pico-examples/tree/master/pio/ws2812) for the ws2812 in two ways.

## clkdiv
I wanted to have the sm run at its normal speed (125 MHz). The timing restrictions of the ws2812 chip then requires to use delay cycles. It happens that the maximum (31) is just enough delay to make it work. In the pio example code, a side set is used to toggle the GPIO that drives the signal to the ws2812 pixels. This results in neat code, but if side set is used, however, less bits are left over to specify a delay. In this case not enough to make it work.

## reset period
To make the ws2812 accept a new set of pixel data, a reset period has to be used. In my code I count how many pixel RGB values have been sent out and then start a delay loop. The led strip I use has 120 pixels, so, the pio code is specific to that amount of pixels. The delay loop is such that it waits long enough for the reset of the ws2812.

## Dithering
I wanted to try dithering for low brightness values to make the transitions more smooth. In the code I have used a simple table with values to use for dithering. I have no idea how others do this, so maybe there are more clever ways. In the code first normal brightness changes are programmed, followed by dithered brightness changes.
