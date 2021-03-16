#include <stdio.h>

#include "pico/stdlib.h"
#include "hardware/pio.h"
#include "hardware/dma.h"

#include "sm_to_dma_to_buffer.pio.h"

int main()
{
    // buffer to write to
    uint32_t buffer[100];

    // needed for printf
    stdio_init_all();

    // pio 0 is used
    PIO pio = pio0;
    // state machine 0
    uint sm0 = 0;
    // load the sm0 program into the pio memory
    uint offset0 = pio_add_program(pio, &sm_to_dma_to_buffer_program);
    // make a sm config
    pio_sm_config smc0 = sm_to_dma_to_buffer_program_get_default_config(offset0);
    // init the pio sm0 with the config
    pio_sm_init(pio, sm0, offset0, &smc0);
    // disable the sm
    pio_sm_set_enabled(pio, sm0, false);
    // make sure the FIFOs are empty
    pio_sm_clear_fifos(pio, sm0);

    // Get a free DMA channel, panic() if there are none
    int dma_chan = dma_claim_unused_channel(true);
    // make a default dma config
    dma_channel_config dma_conf = dma_channel_get_default_config(dma_chan);
    // transfer uint32_t
    channel_config_set_transfer_data_size(&dma_conf, DMA_SIZE_32);
    // a FIFO is read -> no increment of read pointer
    channel_config_set_read_increment(&dma_conf, false);
    // a buffer in memory is written to -> increment write pointer
    channel_config_set_write_increment(&dma_conf, true);
    // let the sm0 of pio determine the speed
    channel_config_set_dreq(&dma_conf, pio_get_dreq(pio, sm0, false));
    // configure the dma channel to read 100 uint32_t from sm0 to the buffer
    dma_channel_configure(dma_chan, &dma_conf,
                          buffer,         // Destinatinon pointer: the buffer in memory
                          &pio->rxf[sm0], // Source pointer: the output of sm0
                          100,            // Number of transfers
                          true            // Start immediately
    );
    // enable the sm
    pio_sm_set_enabled(pio, sm0, true);
    // let the dma channel do its stuff
    dma_channel_wait_for_finish_blocking(dma_chan);

    // print the result in the buffer
    for (int i = 0; i < 100; i++)
    {
        printf("buffer = %d\n", buffer[i]);
    }
    // endless loop to end this program
    while (true)
        ;
}