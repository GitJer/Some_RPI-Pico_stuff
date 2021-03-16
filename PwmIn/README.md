# PWM input using the Raspberry Pi Pico PIO 

Most microcontrollers have hardware to produce Pulse Width Modulation (PWM) signals. But sometimes it is useful to be able to read PWM signals and determine the period, pulse width and duty cycle.

Based on the method to measure pulses with PIO code as described [here](https://github.com/GitJer/Some_RPI-Pico_stuff/tree/main/HCSR04), a PWM Input can be made.

## Algorithm

In pseudo-code the algorithm is as follows:

```
    loop:
       reset the 'timer'
       loop: 
          decrement timer
          test for falling edge 
       record timer value as pulse width (actually, (0xFFFFFFFF - x)*2*1/125MHz is the pulse width)
       loop:
          test for rising edge
          decrement timer
       record the timer value as period (actually, (0xFFFFFFFF - x)*2*1/125MHz is the period)
```
