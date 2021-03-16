# State Machine -> DMA -> State Machine -> DMA -> Buffer
This is an example where one state machine writes via DMA to another state machine whose output is put into a buffer via another DMA channel.

The sm0 counts up from 0, and sends its current value into its Rx FIFO. A DMA channel reads this Rx FIFO and puts it into the Tx FIFO of sm1. The sm1 does nothing more than put what it receives into its Rx FIFO. A DMA channel reads the sm1 Rx FIFO and puts the values into a buffer (of length 100).

In the code the chain is as follows:
```
sm0 -> dma_chan_2 -> sm1 -> dma_chan_1 -> buffer
```

I had some problems getting this to work. The main reason was to set the data request (DREQ) signal correctly. It turns out to depend on which sm is the slowest. That one has to determine the speed.

In the PIO code sm0 runs a program (tester0) that is 3 instructions long while sm1 runs a program (tester1) that is 4 instructions. Therefore, sm1 should determine the DREQ:
``` 
channel_config_set_dreq(&dma_conf_2, pio_get_dreq(pio, sm1, true)); 
```
If I artificially make tester0 longer, e.g. by uncommenting the ';[31]', then sm0 should determine the DREQ:
```
channel_config_set_dreq(&dma_conf_2, pio_get_dreq(pio, sm0, false));

```
In addition, starting the state machines and DMA channels in the correct order is important. In this case:
```
// enable the state machines
pio_sm_set_enabled(pio, sm1, true);
pio_sm_set_enabled(pio, sm0, true);

// start the dma channel from sm0 to sm1: start producing data
dma_channel_start(dma_chan_2);

// wait for dma channel from sm1 to the buffer to finish
dma_channel_wait_for_finish_blocking(dma_chan_1);
```