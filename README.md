# Content

This repository contains bits and pieces that I made while trying to figure out how the Raspberry Pi Pico state machines and the Programmable Input/Output (PIO) works.

Here I have written down the reusable 'tricks' I use in several of the projects listed below.

The following projects are contained in this repository:

## Two independently running state machines 
This is just an example of two state machines running independently. Nothing special about it, but I had to do it.

## Two independently running state machines, one gets disabled temporarily
This is an example of two state machines running independently, but one gets disabled temporarily.

## Two independently running state machines, synchronized via irq, one gets disabled temporarily
This is an example of two state machines synchronized via setting and clearing an irq, one gets disabled temporarily.

## State machine writes into a buffer via DMA
This is an example of a state machine using DMA (Direct Memory Access) to write into a buffer.

## State Machine -> DMA -> State Machine -> DMA -> Buffer
This is an example where one state machine writes via DMA to another state machine whose output is put into a buffer via another DMA channel.

## Communicating values between state machines 
The [RP2040 Datasheet](https://datasheets.raspberrypi.org/rp2040/rp2040-datasheet.pdf) states that "State machines can not communicate data". Or can they ... Yes they can, in several ways, including via GPIO pins.

## Use the ISR for rotational shifting
Normally if the ISR shifts via the `IN` instruction, the bits that come out of the ISR go to cyber space, never to be heard from again. Sometimes it is handy to have rotational shifting. Right shifting works fine, but left shifting needs some trickery.

## 4x4 button matrix using PIO code
This code reads a 4x4 button matrix using PIO code for the Raspberry Pico and returns the button pressed.

## Button debouncer using PIO code
When using a GPIO to read noisy input, such as a mechanical button, a software debouncer makes sure that only after the input signal has stabilized, the code will read the new value. 

## PWM input using PIO code
Most microcontrollers have hardware to produce Pulse Width Modulation (PWM) signals. But sometimes it is useful to be able to read PWM signals and determine the period, pulse width and duty cycle.

## Rotary encoder using PIO code
This software reads an optical rotary encoder with very clean signals on its output using PIO code.

# HC-SR04 using the PIO code
This code reads the HC-SR04, an ultrasonic distance measurement unit.
