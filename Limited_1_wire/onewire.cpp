/*

This code uses pio code to read a single DS18B20 temperature sensor.
It is not suited to read more than one sensor: it does not use the parts
of the 1-wire protocol that allowes more than one sensor.

*/

#include <stdio.h>

#include "pico/stdlib.h"
#include "hardware/pio.h"

#include "onewire.pio.h"

// the pin to which the sensor is connected.
// Note that an external pullup resistor of 4.7k is needed on this pin
#define OW_PIN 15

// ---------------------------------------------------------------------------------
// The 1-Wire CRC scheme is described in Maxim Application Note 27:
// "Understanding and Using Cyclic Redundancy Checks with Maxim iButton Products"
//
// Dow-CRC using polynomial X^8 + X^5 + X^4 + X^0
// Tiny 2x16 entry CRC table created by Arjen Lentz
// See http://lentz.com.au/blog/calculating-crc-with-a-tiny-32-entry-lookup-table
//
// Copied from https://github.com/PaulStoffregen/OneWire/blob/master/OneWire.cpp
static const uint8_t dscrc2x16_table[] = {
    0x00, 0x5E, 0xBC, 0xE2, 0x61, 0x3F, 0xDD, 0x83,
    0xC2, 0x9C, 0x7E, 0x20, 0xA3, 0xFD, 0x1F, 0x41,
    0x00, 0x9D, 0x23, 0xBE, 0x46, 0xDB, 0x65, 0xF8,
    0x8C, 0x11, 0xAF, 0x32, 0xCA, 0x57, 0xE9, 0x74};
// ---------------------------------------------------------------------------------

class OneWire
{
public:
    OneWire(uint onewire_pin)
    {
        pio = pio0;
        // state machine 0
        sm = 0;
        // configure the used pins
        pio_gpio_init(pio, OW_PIN);
        // load the pio programs into the pio memory
        offset_wait = pio_add_program(pio, &onewire_wait_program);
        offset_reset = pio_add_program(pio, &onewire_reset_program);
        offset_write_byte = pio_add_program(pio, &onewire_write_byte_program);
        offset_read_byte = pio_add_program(pio, &onewire_read_byte_program);

        // make a sm config
        pio_sm_config c = pio_get_default_sm_config();
        // set the set pin
        sm_config_set_set_pins(&c, OW_PIN, 1);
        // set the out pin
        sm_config_set_out_pins(&c, OW_PIN, 1);
        // configure side set to be 1 pin, optional (hence 2 bits) and controls pindirs
        sm_config_set_sideset(&c, 2, true, true);
        sm_config_set_sideset_pins(&c, OW_PIN);
        // set the in pin
        sm_config_set_in_pins(&c, OW_PIN);
        // set shift to right: bits shifted by 'in' are ordered as least
        // significant bit (LSB) first, no autopush/pull
        sm_config_set_in_shift(&c, true, false, 0);
        sm_config_set_out_shift(&c, true, false, 0);
        // one clock cycle is 10 us
        sm_config_set_clkdiv(&c, 1250);
        // init the pio sm with the config, start with the wait program
        pio_sm_init(pio, sm, offset_wait, &c);
        // enable the sm
        pio_sm_set_enabled(pio, sm, true);
    }

    int reset()
    {
        // start the reset program and check if a sensor (worker) responds
        pio_sm_exec(pio, sm, offset_reset);
        // read the return value: 0 means there are at least one workers
        int no_workders_detected = pio_sm_get_blocking(pio, sm);
        if (no_workders_detected == 1)
            // indeed: no workers detected
            return -1;
        return 1;
    }

    int reset_and_check()
    {
        // this method checks if there are workers (i.e. DS18B20 sensors)
        // and then - assuming only one is present - checks if it can read
        // this sensor properly by asking for its unique ID and doing a CRC.

        // start the reset program
        pio_sm_exec(pio, sm, offset_reset);
        // read the return value: 0 means there are (at least one) workers
        int no_workders_detected = pio_sm_get_blocking(pio, sm);
        if (no_workders_detected == 1)
            // indeed: no workers detected
            return -1;
        else
        {
            // for stability reasons give it some time
            sleep_ms(10);
            // send the command to get the address of the worker
            send_byte(0x33);
            // for stability reasons give it some time
            sleep_ms(10);
            // read the results
            read_bytes(8);
            // the eighth byte is the crc
            if (crc8(results, 7) != results[7])
            {
                printf("crc of ROM is wrong!!!!!!!!!!!\n");
                return -1;
            }
            return 1;
        }
    }

    void send_byte(uint32_t to_send)
    {
        // put the byte in the TxFIFO
        pio_sm_put(pio, sm, to_send);
        // start the sm program that writes a byte 
        pio_sm_exec(pio, sm, offset_write_byte);
    }

    uint32_t read_byte()
    {
        // start the sm program that reads a byte
        pio_sm_exec(pio, sm, offset_read_byte);
        // wait for the result
        return pio_sm_get_blocking(pio, sm) >> 24;
    }

    void read_bytes(int n)
    {
        // read n bytes from the sensor
        for (int i = 0; i < n; i++)
            results[i] = (uint8_t)read_byte() & 0xFF;
    }

    float read_temperature()
    {
        // put the sensor in a known state
        int workers = reset();
        if (workers < 0)
            return -1;
        // for stability reasons give it some time
        sleep_ms(10);
        // address the family (not one specific sensor)
        send_byte(0xCC);
        // for stability reasons give it some time
        sleep_ms(5);
        // ask for a temperature conversion
        send_byte(0x44);
        // a temperature conversion at 12 bit resolution takes 750ms
        sleep_ms(800);
        // put the sensor in a known state
        workers = reset();
        if (workers < 0)
            return -1;
        // for stability reasons give it some time
        sleep_ms(10);
        // address the family (not one specific sensor)
        send_byte(0xCC);
        // for stability reasons give it some time
        sleep_ms(5);
        // ask for the results
        send_byte(0xBE);
        // for stability reasons give it some time
        sleep_ms(10);
        // read the results
        read_bytes(9);
        // the ninth byte is the crc
        if (crc8(results, 8) != results[8])
        {
            printf("crc of data is wrong!!!!!!!!!!!\n");
            return -1;
        }
        // convert the temperature results to a float
        // the bits in results[0] indicate temperature as follows:
        // bit 7 to 0: 2^3, 2^2, 2^1, 2^0, 2^-1, 2^-2, 2^-3, 2^-4
        uint8_t Tlow = results[0];
        float T = 0;
        for (int bit = 0; bit < 8; bit++)
            if ((Tlow & 1 << bit) > 0)
                T += 1 << bit;
        // above counting started as if 2^-4 is 1, so correct by dividing by 16
        T /= 16.;
        // the bits in results[1] indicate temperature as follows:
        // bit 7 to 0: S, S, S, S, S, 2^6, 2^5, 2^4; where S = sign
        // note that negative numbers start at -128
        uint8_t Thigh = results[1];
        for (int bit = 0; bit < 3; bit++)
            if ((Thigh & 1 << bit) > 0)
                T += 1 << (bit + 4);
        // check the sign using bit 3
        if ((Thigh & 1 << 3) > 0)
            T = -128.+T;
        return T;
    }

    // ---------------------------------------------------------------------------------
    // Copied (and adapted) from https://github.com/PaulStoffregen/OneWire/blob/master/OneWire.cpp
    uint8_t crc8(const uint8_t *addr, uint8_t len)
    {
        uint8_t crc = 0;
        while (len--)
        {
            crc = *addr++ ^ crc; // just re-using crc as intermediate
            crc = dscrc2x16_table[(crc & 0x0f)] ^
                  dscrc2x16_table[16 + ((crc >> 4) & 0x0f)];
        }
        return crc;
    }
    // ---------------------------------------------------------------------------------

private:
    // the pio instance
    PIO pio;
    // the state machine
    uint sm;
    // the offsets (origins of the PIO programs)
    uint offset_wait;
    uint offset_reset;
    uint offset_write_byte;
    uint offset_read_byte;
    // the result of reading the sensor
    uint8_t results[9];
};

int main()
{
    // needed for printf
    stdio_init_all();
    // the instance of the OneWire
    OneWire DS18B20(OW_PIN);
    // reset and determine if there are workers
    int workers = DS18B20.reset_and_check();

    if (workers > 0)
        while (true)
            printf("Temperature = %f\n", DS18B20.read_temperature());
    else
        while (true)
            ;
}