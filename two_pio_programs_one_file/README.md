This example contains two pio programs.
It was actually part of getting the 1-wire protocol working, and that requires an external pull up resistor on the pin used by both set and side-set
But for now, this is mostly an example of how two pio programs in one .pio file can be used.

The first program produces a short 0 (two cycles) and a long 1 (nine cycles) on the output
the second program produces a short 1 (two cycles) and a long 0 (nine cycles) on the output

Note that in the generated file two_p_one_f.pio.h file the two programs are 
considered completely separate and independent of where in memory they will end up!
Even the 'jmp again1' and 'jmp again2' are both encoded as a 'jmp 0'. This means
that when loading a program into pio memory, some trans-coding is necessary to make
the jmp statements point to the right memory address.
