# Read the SBUS protocol with pio code

The SBUS protocol is typically used in Radio Controlled cars, drones, etc.
If you want to read this protocol from a RC receiver in order to manipulate the data before setting motors and servos, you can use this code.

The SBUS protocol is basically an uart Rx with inverted input, 100000 baud rate, a parity bit, and two stop bits, see [here](https://github.com/bolderflight/sbus).
The basis for the PIO code is the RPI pico example for [pio rx](https://github.com/raspberrypi/pico-examples/blob/master/pio/uart_rx/uart_rx.pio).
The parsing of the received data is done following to [this](https://platformio.org/lib/show/5622/Bolder%20Flight%20Systems%20SBUS).
It even does parity checking in the pio code!
