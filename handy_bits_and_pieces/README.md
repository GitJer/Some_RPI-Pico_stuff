# Handy bits and pieces of PIO code 

On this page I list some of the things that I found out while making the projects. 

If you know of better ways of doing things, or if you have additional 'tricks', please let me know!

## Setting the x (or y) scratch registers to 0xFFFFFFFF

The x register can be used for counting, but PIO code only has one instruction (`jmp x-- label`) that can actually subtract one from the value in x. 
Setting the x (or y) registers to a value from 0 to 31 can be done with the `set` command. That command has 5 bits for a number. 
For counters it is handy to have much higher values to start with. One approach is to start the x with the highest value available in 32 bits: 0xFFFFFFFF and count down.

This can be achieved with the instruction:
```
mov x ~NULL
```
If the C-program needs a number counting down from 0xFFFFFFFF to 0, you can use:
```
start:
    mov x ~NULL	    ; set the x register to 0xFFFFFFFF
push_it:
    mov ISR x		; copy x to the ISR
    push block		; wait for the C-program to read it from the Rx FIFO
    jmp x-- push_it	; decrement x and loop if not zero
    jmp start		; x = 0 -> start over
```

## Counting up instead of down
As stated above, PIO only has one instruction to count down: `jmp x-- label` (which can also use y). Counting up isn't possible. However, bitwise inverting the value in x results in its complement. Thus we can start a loop at 0xFFFFFFFF and count down, but use the inverted value in x, which results in counting from 0 to 0xFFFFFFFF.

If the C-program needs a number counting up from 0 to 0xFFFFFFFF, you can use:
```
start:
    mov x ~NULL	    ; set the x register to 0xFFFFFFFF
push_it:
    mov ISR ~x		; copy its complement to the ISR
    push block		; wait for the C-program to read it from the Rx FIFO
    jmp x-- push_it	; decrement x and loop if not zero
    jmp start		; x = 0 -> start over
```
Note the tilde '~' before the x in the line `mov ISR ~x`.

## Delay
If you need to have a small delay in PIO code the delay_value can be set to a number of cycles between 0 and 31. If, however, a (much) larger delay is required, the following can be used:

A counter in the x (or y) register can be started at the required number of delay cycles, and then counted down in a tight loop. Note that you need to subtract the amount of cycles that setting up the counter takes.

This same approach can be used to create a clock-cycle precise delay for any number between 0x00000000 and 0xFFFFFFFF. This can be done by setting x and shifting it into the ISR, 5 bits at a time, several times. For example, a delay loop of 682 instructions is needed (in binary: 1010101010), this can be achieved in the following way:
```
    set x 21             ; in binary 10101, the first 5 bits of 1010101010
    mov ISR x            ; copy x into the ISR
    set x 10             ; in binary 01010, the 5 least significant bits of 1010101010
    in x 5               ; shift the least significant 5 bits of x into the ISR
    mov x ISR            ; place the ISR in x as the start value of the counter
            
delay_loop:              ; the actual delay loop
    jmp x-- delay_loop
```
Note that setting up the starting value in the x register took 5 instructions, so in total this will result in a delay of 682+5 = 687 clock cycles. Also note that left-shifting is used.

If the timing doesn't have to be precise, rounding down (or up) after the 5 most significant bits and shifting in further 0's can save instructions.
For example if a delay of approximately 7500000 clock cycles is needed (in binary 11100100111000011100000), using only the first 5 most significant digits and setting the rest (18 bits) to 0, may suffice. This results in:
```
    set x 28             ; set x to 11100
    mov ISR x            ; copy x into the ISR
    in NULL 18           ; shift in 18 more 0 bits
    mov x ISR            ; move the ISR to x 

delay_loop:              ; the delay loop
    jmp x-- delay_loop   
```

## Measuring 'time'
To measure the duration of some event, can be done by starting the x (or y) scratch register with 0xFFFFFFFF and counting down, testing for the stop criterion each iteration. 

The counting down loop with test for the stop criterion looks like this:
```pio
    mov x ~NULL     ; start with 0xFFFFFFFF
timer:
    jmp x-- test    ; count down, and jump to testing
    jmp timerstop   ; timer has reached 0, stop count down
test:
    jmp pin timer   ; <---- insert the test criterion here
timerstop:          ; The test criterion was met (or timer has reached 0)
    mov ISR ~x      ; move the bit inverted value of x to the ISR
    push noblock    ; push the ISR into the RX FIFO
```
The `jmp x-- test` tests if x is zero, and if not decrements it and jumps to testing for the stop criterion (here the `jmp` pin going low). If the criterion fails, a jmp is  made to `timer`. If the criterion is met, or if the x register becomes 0, the bit-inverted value of x is moved to the ISR, and is then pushed to the Rx FIFO. 

Here the counting down and the test itself only take two instructions (`jmp x-- test` and `jmp pin timer`). Therefore the resolution of measuring the time with this code is 2 * pio clock tick, which for the standard clock is `2 / 125 MHz = 0.000000016` seconds. If the test takes more than one instruction, the resolution becomes lower.


## Change shifting direction in PIO code
When configuring a sm from c-code, you can configure the shift direction of the ISR:
for left shifting:
```
sm_config_set_in_shift(&c, false, false, 0);
```
for right shifting:
```
sm_config_set_in_shift(&c, true, false, 0);
```
If you need to change the shifting direction in PIO code, you can use:
```
mov ISR :: ISR
in ISR 1
mov ISR :: ISR
```
The `::` symbol reverses the bit order. In this example, shifting is only for one bit, but other bit counts are also possible: `in ISR Bit_count`, with Bit_count between 1 and 32, 32 is encoded as 00000.

