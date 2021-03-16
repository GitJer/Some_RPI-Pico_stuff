#include <stdio.h>

#include "pico/stdlib.h"
#include "hardware/pio.h"
#include "hardware/clocks.h"

#include "button_debounce.pio.h"


// class that debounces a gpio pin
class Debounce {
  public:
    // constructor to debounce the gpio
    Debounce(uint gpio) {
      // instantiate a pio, for now use pio0
      // TODO: if no sm in pio0 are available, try pio1
      PIO pio = pio0;
      // claim a state machine
      sm = pio_claim_unused_sm(pio, false); 
      // check if this is a valid sm
      if (sm == -1) {
        valid = 0;
      } else {
        valid = 1;
      }
      // load the pio program into the pio memory
      offset = pio_add_program(pio, &button_debounce_program);
      // make a sm config
      pio_sm_config c = button_debounce_program_get_default_config(offset);
      // set the 'wait' pins
      sm_config_set_in_pins(&c, gpio); // for WAIT, IN
      // set the 'jmp' pins
      sm_config_set_jmp_pin(&c, gpio); // for JMP
      // set the clock divisor to set a reasonable debounce time
      // TODO: let the user set the debounce time in ms, calculate clock divisor for pio delay of 31.
      sm_config_set_clkdiv(&c, 11);
      // init the pio sm with the config
      pio_sm_init(pio, sm, offset, &c);
      // enable the sm
      pio_sm_set_enabled(pio, sm, true);
    };

    // return the value of the debounced gpio
    uint read(void){
      // read the program counter
      uint pc = pio_sm_get_pc(pio0, sm);
      // if it is at or beyond the "wait 0 pin 0" it has value 1, else 0
      if (pc >= offset+6) {
        return 1;
      }else {
        return 0;
      }
    };

    // if no sm was available the debounce does not work
    uint is_valid(void) {
      return valid;
    }

  private:
    // the pio instance 
    PIO pio;
    // the state machine
    uint sm;
    // the location of the pio program in the memory
    uint offset;
    // indicator if the sm is valid
    uint valid = 0;
};


/*
 * The main program: instantiate the debouncer and keep printing the value
 */
int main()
{
  // necessary for printf
  stdio_init_all();
  // instantiate the debouncer for a gpio
  Debounce my_pin(5);
  // check if it is valid (i.e. there is an active state machine)
  printf("my_pin validity = %d\n", my_pin.is_valid());
  // loop that reads and prints the gpio value
  while (true) {
    printf("value=%d\n", my_pin.read());
  }
}
