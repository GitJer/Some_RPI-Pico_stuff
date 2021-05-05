# Blow out a(n) LED
This is remake of the wonderfull little thingy made by Paul Dietz. See this [Hackaday article](https://hackaday.com/2018/08/21/an-led-you-can-blow-out-with-no-added-sensor/) and the [github code](https://github.com/paulhdietz/LEDSensors/blob/master/_07_BlowOutLED/_07_BlowOutLED.ino).

It lights a LED and when you blow on it, it switches off for one second, then lights again. Imagine blowing out a LED!

From the hackaday article:
"Turning the LED on warms it up and blowing on it cools it off, causing measurable 
changes in the voltage drop across the device. The change isn’t much — only a 
handful of millivolts — but the effect is consistent and can be measured. "

Where his code is rather simple and elegant, mine is convoluted.
Why? Because I wanted to play with the DMA sniffer!

The DMA sniffer allowes summing of all values that pass through the DMA.
So, there is no need to sum the values yourself afterwards.

The code does the following:
- it turns the LED on for at least 1s to warm it up
- it starts a repeat timer to call "repeating_timer_callback"
- in that callback the adc is read NUM_ADC_SAMPLES times through dma, 
  the samples are summed by the dma sniffer
- if the read summed value deviates sufficiently from the average, a flag is set to switch the LED off
- an array of obtained summed values is kept up to date to determine the average of the sums
- in the main loop the flag is checked and when set the LED goes off for 1s and then 1s on to warm it up