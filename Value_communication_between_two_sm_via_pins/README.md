# Sending values between state machines 

The [RP2040 Datasheet](https://datasheets.raspberrypi.org/rp2040/rp2040-datasheet.pdf) states that "State machines can not communicate data". Or can they ...

Of course they can, but not directly from within the state machines (sm). They can communicate via their FIFOs if the c program reads the Rx FIFO of one sm and puts it into the Tx FIFO of the other sm. They can even do this via DMA, which is another example in this repository.

But they can also communicate using the GPIO pins.

In this example, one sm sets a GPIO pin to a value, and the other reads that pin to obtain the value. They use an irq to make sure that they work synchronized. In this example only one GPIO pin is used, but they could of course use more pins to communicate more bits at one time.
