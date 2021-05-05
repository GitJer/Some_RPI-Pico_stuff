# PWM input using the Raspberry Pi Pico PIO 

Most microcontrollers have hardware to produce Pulse Width Modulation (PWM) signals. But sometimes it is useful to be able to read PWM signals and determine the period, pulse width and duty cycle.

Although the RP2040 seems to be capable of measuring pulse width and period, this takes more CPU cycles than letting a PIO state machine take care of it.

Based on the method to measure pulses with PIO code as described [here](https://github.com/GitJer/Some_RPI-Pico_stuff/tree/main/HCSR04), a PWM Input can be made.

## Algorithm

In pseudo-code the algorithm is as follows:

```
    loop:
        reset y, the 'timer' for the pulsewidth (high period)
        reset x, the 'timer' for the low period. (high + low = period)
        wait for a 0, then wait for a 1: this is the rising edge
        loop: 
           decrement y timer
           test for falling edge 
        y timer value is the pulse width (actually, (0xFFFFFFFF - x)*2/125MHz is the pulse width)
        loop:
           test for rising edge
           decrement x timer
        x timer is the low period (actually, (0xFFFFFFFF - x)*2/125MHz is the low period)
        push both y and x to the Rx FIFO
```
