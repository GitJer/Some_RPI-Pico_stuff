/*

The goal of this program is to show how DMA can be used to transfer data from one state machine to the next and from that second state machine to a buffer.

state machine 0 (sm0) produces numbers from 0 counting up.
state machien 1 (sm1) accepts input and immedeately pushes it as output

In between sm0 and sm1 there is a DMA channel, and between sm1 and the buffere there is another DMA channel: 

sm0 -> dma_chan_2 -> sm1 -> dma_chan_1 -> buffer

*/

#include <stdio.h>

#include "pico/stdlib.h"
#include "hardware/pio.h"
#include "hardware/dma.h"

#include "sm_to_dma_to_sm_to_dma_to_buffer.pio.h"

int main()
{
    uint32_t num_samples = 1000;
    // buffer to write to
    uint32_t buffer[num_samples];

    // needed for printf
    stdio_init_all();

    // pio 0 is used
    PIO pio = pio0;
    // state machines 0 and 1
    uint sm0 = 0;
    uint sm1 = 1;
    // load the sm0 program into the pio memory
    uint offset0 = pio_add_program(pio, &tester0_program);
    // load the sm1 program into the pio memory
    uint offset1 = pio_add_program(pio, &tester1_program);
    // make a sm config
    pio_sm_config smc0 = tester0_program_get_default_config(offset0);
    pio_sm_config smc1 = tester1_program_get_default_config(offset1);
    // init the pio sm0 with the config
    pio_sm_init(pio, sm0, offset0, &smc0);
    pio_sm_init(pio, sm1, offset1, &smc1);
    // disable the state machines
    pio_sm_set_enabled(pio, sm0, false);
    pio_sm_set_enabled(pio, sm1, false);
    // clear the FIFOs
    pio_sm_clear_fifos(pio, sm0);
    pio_sm_clear_fifos(pio, sm1);

    // DMA channel from sm1 to a buffer in memory
    // NOTES:
    // - this channel is paced (dreq) by reading output from sm1 (is_tx is false)
    // - this channel may start immediately

    // Get a free DMA channel, panic() if there are none
    int dma_chan_1 = dma_claim_unused_channel(true);
    // make a dma config
    dma_channel_config dma_conf_1 = dma_channel_get_default_config(dma_chan_1);
    // transfer uint32_t
    channel_config_set_transfer_data_size(&dma_conf_1, DMA_SIZE_32);
    // a FIFO is read -> no increment of read pointer
    channel_config_set_read_increment(&dma_conf_1, false);
    // a buffer in memory is written to -> increment write pointer
    channel_config_set_write_increment(&dma_conf_1, true);
    // let the sm0 of pio determine the speed
    channel_config_set_dreq(&dma_conf_1, pio_get_dreq(pio, sm1, false));
    // configure the dma channel to read num_samples uint32_t from sm1 to the buffer
    dma_channel_configure(dma_chan_1, &dma_conf_1,
                          buffer,         // Destinatinon pointer
                          &pio->rxf[sm1], // Source pointer
                          num_samples,    // Number of transfers
                          true            // Start immediately
    );

    // DMA channel from sm0 output to sm1 input
    // NOTES:
    // - this channel is paced (dreq) by writing to sm1 (is_tx is true).
    //   The PIO program of sm1 is 4 instructions, sm0 is 3 instructions
    //   If sm0 is made slower than sm1, then dreq should depend on sm0 (is_tx is false)
    // - this channel should not start immediately. First enable the state machines

    // Get a free channel, panic() if there are none
    int dma_chan_2 = dma_claim_unused_channel(true);
    // make a dma config
    dma_channel_config dma_conf_2 = dma_channel_get_default_config(dma_chan_2);
    // transfer uint32_t
    channel_config_set_transfer_data_size(&dma_conf_2, DMA_SIZE_32);
    // a FIFO is read -> no increment of read pointer
    channel_config_set_read_increment(&dma_conf_2, false);
    // a FIFO is written to -> no increment of write pointer
    channel_config_set_write_increment(&dma_conf_2, false);
    // let writing to sm1 determine the speed if sm1 is slower than sm0
    channel_config_set_dreq(&dma_conf_2, pio_get_dreq(pio, sm1, true));
    // let writing to sm0 determine the speed if sm0 is slower than sm1
    // channel_config_set_dreq(&dma_conf_2, pio_get_dreq(pio, sm0, false));
    // configure the dma channel to read num_samples uint32_t from sm0 to sm1
    dma_channel_configure(dma_chan_2, &dma_conf_2,
                          &pio->txf[sm1], // Destinatinon pointer
                          &pio->rxf[sm0], // Source pointer
                          num_samples,    // Number of transfers
                          false           // Start immediately
    );

    // enable the state machines
    pio_sm_set_enabled(pio, sm1, true);
    pio_sm_set_enabled(pio, sm0, true);

    // start the dma channel from sm0 to sm1: start producing data
    dma_channel_start(dma_chan_2);
    // wait for dma channel from sm1 to the buffer to finish
    dma_channel_wait_for_finish_blocking(dma_chan_1);

    // print the result in the buffer
    for (int i = 0; i < num_samples; i++)
    {
        printf("buffer = %d\n", buffer[i]);
    }
    while (true)
        ;
}