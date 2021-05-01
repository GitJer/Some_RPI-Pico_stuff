# 1-wire protocol for one device 

This code is a pio implementation of the [1-wire protocol](https://en.wikipedia.org/wiki/1-Wire). The C++ implementation is rather limited: it is meant for only one device (the DS18B20 temperature sensor) and uses the default settings (12-bit conversion). 
To make this I have used the [datasheet](https://datasheets.maximintegrated.com/en/ds/DS18B20.pdf) as well as the implementation of [Paul Stoffregen](https://github.com/PaulStoffregen/OneWire) and [this pio implementation](https://www.raspberrypi.org/forums/viewtopic.php?t=304511#p1851030)

