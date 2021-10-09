# HC-SR04 using the PIO code
The HC-SR04 is an ultrasonic distance measurement unit ([datasheet](https://cdn.sparkfun.com/datasheets/Sensors/Proximity/HCSR04.pdf)). Basically, it works by sending a puls to the Trig pin, and measuring the pulse on the Echo pin. The length of the Echo pulse represents the distance to an object in front of the device.

The HC-SR04 uses 5V while the Pico uses 3V3. Thus the Echo pin requires a [resistive divider](https://hackaday.com/2016/12/05/taking-it-to-another-level-making-3-3v-and-5v-logic-communicate-with-level-shifters/) to be save. 

## The algorithm
The pio code to read the HC-SR04 has the following steps:
* Give a pulse on the Trig pin of the HC-SR04 to start the measurement. The datasheet indicates that the length of this pulse should be 10 us
* Measure the length of the pulse on the Echo pin
* Wait 60 ms before making a new measurement to be sure that no echos from previous measurements are found

Setting and reading pins are (if you've done it before) straight forward, but making and reading pulses of specific length weren't for me.

Making the HC-SR04 algorithm work involves two types of timing:
* a delay to make the trigger pulse
* measuring the length of the echo pulse
* a delay to ensure no echos of a previous measurement are received

The PIO doesn't seem to have timers, but each instruction takes one pio clock cycle, and these can be used to measure time.

## Delay in PIO
According to the datasheet, the trigger pulse should be 10 us. The pio clock runs at 125 MHz, so, the pulse should be 1250 cycles long. Here it is assumed that the clock divider is set to 1. If this has a different value, some of the timing calculations below will change. Creating the required delay can be achieved in the following way:
* 1250 cycles in binary is 10011100010
* assuming the pulse width doesn't need to be very precise, this can be rounded down to 10011000000
* this can be split into the 5 most significant digits (10011) and 6 additional 0 bits
* place this number in the x register in the following way (note that for this to work correctly, the shift direction needs to be set with`sm_config_set_in_shift(&c, false, false, 0);`):
``` pio
    set x 19        ; set x to 10011 (and clear the higher bits)
    mov ISR x       ; copy x to ISR 
    in NULL 6       ; shift in 6 more 0 bits
    mov x ISR       ; move the ISR to x (which now contains 10011000000)
``` 
* Count down to 0 in a tight loop using:
``` pio
delay1:
    jmp x-- delay1
```
This results in a delay of almost 10 us.

This same approach can be used to create a clock-cycle precise delay for any number between 0x00000000 and 0xFFFFFFFF. This may involve setting x and shifting it into the ISR, 5 bits at a time, several times. If the timing doesn't have to be precise, rounding down (or up) after the 5 most significant bits and shifting in further 0's, as done above, can save instructions.

The second delay that is needed is 60 ms. The same trick as above is used to create this delay. But now with the following values:
7500000 clock cycles are needed. In binary this is 11100100111000011100000. This is rounded down to 11100 + 18 * 0.

## Measuring duration of a pulse

To measure the duration of some event, in this case the length of the Echo pulse which represents the distance to an object, can be done by starting the x (or y) scratch register with 0xFFFFFFFF and counting down, testing for the stop criterion each iteration. 

The counting down loop with test for the stop criterion looks like this:
```pio
timer:
    jmp x-- test    ; count down
    jmp timerstop   ; timer has reached 0, stop count down
test:
    jmp pin timer   ; test if the echo pin is still 1, if so, continue counting down
timerstop:          ; echo pulse is over (or timer has reached 0)
    mov ISR ~x      ; move the bit-inverted value of x to the ISR
    push noblock    ; push the ISR into the RX FIFO
```
If x isn't 0, the statement `jmp x-- test` jumps to testing for the stop criterion (the Echo pin going low). If the stop criterion is true, or if the x register is 0 when arriving at the jmp statement, the bit-inverted value of x is moved to the ISR, which makes it positive, and is then pushed to the Rx FIFO. 

## Calculation of distance
In the C/C++ code the value in the x scratch register is received. Each loop has two instructions (`jmp` with decrement, and `jmp` testing the pin) the amount of pio clock cycles becomes `2 * pio_sm_get(pio, sm)`.

For the calculation of the distance three more things are needed:
* one pio clock cycle takes 1 / 125 MHz = 0.000000008 s
* The speed of sound in air is about 340 m/s = 34000 cm/s
* The sound travels from the HC-SR04 to the object and back (twice the distance)

If those are factored in, the code to calculate the distance in cm becomes:
``` c
    clock_cycles = 2*pio_sm_get(pio, sm);
    cm = (float)clock_cycles * 0.000136;
```


