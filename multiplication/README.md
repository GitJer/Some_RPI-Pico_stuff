# multiply two numbers 

In the [RP2040 datasheet](https://datasheets.raspberrypi.org/rp2040/rp2040-datasheet.pdf) there is an example of how a sm can be used to add two numbers. If you can add, you can multiply! So, this is a pio program that can multiply. Not very useful, but interesting nonetheless. 

If the two numbers to be multiplied are called m1 and m2, this multiplication works by adding (actually subtracting) one for m2 times m1 times.

A possibly better implementation would perform the multiplication in a different way: by shifting and adding.