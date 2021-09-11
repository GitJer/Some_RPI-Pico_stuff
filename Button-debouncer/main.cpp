#include <stdio.h>

#include "pico/stdlib.h"
#include "button_debounce.h"

/*
  This code shows how to use the button debouncer that uses the PIO state machines.
  It shows all functionality:
 
  Instantiate the debouncer, e.g.: Debounce debouncer;
  Request to debounce the gpio, e.g. gpio 3: debouncer.debounce_gpio(3)
  set the debounce time for a gpio, e.g. set to 1ms: debouncer.set_debounce_time(3, 1);
  Read the current value of the debounced the gpio, e.g. gpio 3: int v = debouncer.read(3);
  undebounce (rebounce?) a previously debounced gpio, e.g. gpio 3: debouncer.undebounce_gpio(3);

  This example code first debounces gpio 3 to 10, then in an infinite loop reads the current 
  value of the debounced gpios. During this loop one by one the gpios are undebounced and then 
  one by one debounced again.
 */

int main()
{
    // necessary for printf
    stdio_init_all();
    printf("Start of debounce example\n");
    // instantiate the debouncer
    Debounce debouncer;

    // debounce 8 gpio
    debouncer.debounce_gpio(3);
    debouncer.debounce_gpio(4);
    debouncer.debounce_gpio(5);
    debouncer.debounce_gpio(6);
    debouncer.debounce_gpio(7);
    debouncer.debounce_gpio(8);
    debouncer.debounce_gpio(9);
    debouncer.debounce_gpio(10);

    // set different debounce times
    // Note: an external puls generator that can vary the puls widts and an logic analyser 
    // was used during testing to verify that this indeed works.
    // debouncer.set_debounce_time(3, 1);
    // debouncer.set_debounce_time(4, 2);
    // debouncer.set_debounce_time(5, 3);
    // debouncer.set_debounce_time(6, 4);
    // debouncer.set_debounce_time(7, 5);
    // debouncer.set_debounce_time(8, 6);
    // debouncer.set_debounce_time(9, 7);
    // debouncer.set_debounce_time(10, 8);

    // infinite loop that continues to show the debounced value of the gpios
    // in the first part of the loop the debounced gpios are one by one UNdebounced (rebounced?)
    // in the second part of the loop the gpios are again all debounced (redebounced?)
    int sm;
    while (true)
    {
        tight_loop_contents();
        // loop that reads and prints the current values of the debounced gpios (or print an X if not debounced)
        // one by one the gpios are undebounced
        for (int stop_debounce = 3; stop_debounce <= 10; stop_debounce++)
        {
            printf("\nValue:\t");
            for (int gpio = 3; gpio <= 10; gpio++)
            {
                int v = debouncer.read(gpio);
                if (v != -1)
                    printf("%d\t", v);
                else 
                    printf("X\t");
            }
            sleep_ms(250);
            // one by one UNdebounce the gpios
            debouncer.undebounce_gpio(stop_debounce);
        }

        // loop that reads and prints the current values of the debounced gpios (or print an X if not debounced)
        // one by one the gpios are debounced again
        for (int debounce_again = 3; debounce_again <= 10; debounce_again++)
        {
            printf("\nValue:\t");
            for (int gpio = 3; gpio <= 10; gpio++)
            {
                int v = debouncer.read(gpio);
                if (v != -1)
                    printf("%d\t", v);
                else 
                    printf("X\t");
            }
            sleep_ms(250);
            // one by one debounce the gpios
            debouncer.debounce_gpio(debounce_again);
        }
    }
}
