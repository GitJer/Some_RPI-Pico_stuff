# State machine writes into a buffer via DMA

This is an example of a state machine (sm) using DMA (Direct Memory Access) to write into a buffer.

The sm0 counts up from 0, and sends its current value into the Rx FIFO. A DMA channel reads the Rx FIFO and puts it into a buffer (of length 100).

