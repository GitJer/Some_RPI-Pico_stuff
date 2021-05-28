# Content

This repository contains bits and pieces that I made while trying to figure out how the Raspberry Pi Pico state machines and the Programmable Input/Output (PIO) work.

[Here](https://github.com/GitJer/Some_RPI-Pico_stuff/tree/main/handy_bits_and_pieces) I have written down the reusable 'tricks' I use in several of the projects listed below.

The following projects are contained in this repository:

## State machine emulator
The problem with the state machines is that debuggers do not give the insight I need when writing code for a sm. I typically write some code, upload it to the pico and find that it doesn't do what I want it to do. Instead of guessing what I do wrong, I would like to see the values of e.g. the registers when the sm is executing. So, I made an [emulator](https://github.com/GitJer/Some_RPI-Pico_stuff/tree/main/state_machine_emulator).

## Two independently running state machines 
[This](https://github.com/GitJer/Some_RPI-Pico_stuff/tree/main/Two_sm_simple) is just an example of two state machines running independently. Nothing special about it, but I had to do it.

## Two independently running state machines, one gets disabled temporarily
[This](https://github.com/GitJer/Some_RPI-Pico_stuff/tree/main/Two_sm_one_disabled) is an example of two state machines running independently, but one gets disabled temporarily.

## Two independently running state machines, synchronized via irq, one gets disabled temporarily
[This](https://github.com/GitJer/Some_RPI-Pico_stuff/tree/main/Two_sm_one_disabled_with_irq) is an example of two state machines synchronized via setting and clearing an irq, one gets disabled temporarily.

## State machine writes into a buffer via DMA
[This](https://github.com/GitJer/Some_RPI-Pico_stuff/tree/main/sm_to_dma_to_buffer) is an example of a state machine using DMA (Direct Memory Access) to write into a buffer.

## State Machine -> DMA -> State Machine -> DMA -> Buffer
[This](https://github.com/GitJer/Some_RPI-Pico_stuff/tree/main/sm_to_dma_to_sm_to_dma_to_buffer) is an example where one state machine writes via DMA to another state machine whose output is put into a buffer via another DMA channel.

## Communicating values between state machines 
The [RP2040 Datasheet](https://datasheets.raspberrypi.org/rp2040/rp2040-datasheet.pdf) states that "State machines can not communicate data". Or can they ... Yes they can, in several ways, [including via GPIO pins](https://github.com/GitJer/Some_RPI-Pico_stuff/tree/main/Value_communication_between_two_sm_via_pins).

## Use the ISR for rotational shifting
Normally if the ISR shifts via the `IN` instruction, the bits that come out of the ISR go to cyber space, never to be heard from again. Sometimes it is handy to have rotational shifting. [Right shifting works fine, but left shifting needs some trickery](https://github.com/GitJer/Some_RPI-Pico_stuff/tree/main/Rotational_shift_ISR).

## 4x4 button matrix using PIO code
[This code](https://github.com/GitJer/Some_RPI-Pico_stuff/tree/main/button_matrix_4x4) reads a 4x4 button matrix using PIO code for the Raspberry Pico and returns the button pressed.

## Button debouncer using PIO code
When using a GPIO to read noisy input, such as a mechanical button, a [software debouncer](https://github.com/GitJer/Some_RPI-Pico_stuff/tree/main/Button-debouncer) makes sure that only after the input signal has stabilized, the code will read the new value. 

## PWM input using PIO code
Most microcontrollers have hardware to produce Pulse Width Modulation (PWM) signals. But sometimes it is useful to be able to [read PWM signals](https://github.com/GitJer/Some_RPI-Pico_stuff/tree/main/PwmIn) and determine the period, pulse width and duty cycle.

## Rotary encoder using PIO code
[This software](https://github.com/GitJer/Some_RPI-Pico_stuff/tree/main/Rotary_encoder) reads an optical rotary encoder with very clean signals on its output using PIO code.

## HC-SR04 using the PIO code
[This code](https://github.com/GitJer/Some_RPI-Pico_stuff/tree/main/HCSR04) reads the HC-SR04, an ultrasonic distance measurement unit.

## Ws2812 led strip with 120 pixels 
[This code](https://github.com/GitJer/Some_RPI-Pico_stuff/tree/main/ws2812_led_strip_120) is my take on how to control a ws2812 led strip with 120 pixels

## multiply two numbers 
[This code](https://github.com/GitJer/Some_RPI-Pico_stuff/tree/main/multiplication) multiplies two numbers.

## Two pio programs in one file
I wanted to see how I could use two pio programs in one file and use them from within the c/c++ program, see [here.](https://github.com/GitJer/Some_RPI-Pico_stuff/tree/main/two_pio_programs_one_file) 

## 1-wire protocol for one device 
[This code](https://github.com/GitJer/Some_RPI-Pico_stuff/tree/main/Limited_1_wire) is a pio implementation of the 1-wire protocol.

## Blow out a(n) LED
[This code](https://github.com/GitJer/Some_RPI-Pico_stuff/tree/main/blow_out_a_LED) is a remake of the wonderfull little thingy made by Paul Dietz: blow on a LED to make it go out! Really!

## Counting pulses in a pulse train separated by a pause 

[This code](https://github.com/GitJer/Some_RPI-Pico_stuff/tree/main/count_pulses_with_pause) can be used for protocols where the data is encoded by a number of pulses in a pulse train followed by a pause.
E.g. the LMT01 temperature sensor uses this, [see](https://www.reddit.com/r/raspberrypipico/comments/nis1ew/made_a_pulse_counter_for_the_lmt01_temperature/).

