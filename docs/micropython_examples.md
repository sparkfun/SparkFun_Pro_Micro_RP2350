---
icon simple/micropython
---

## Verify MicroPython Firmware & Hardware

First, to make sure everything is running properly, use the <code>sys</code> module to verify the firmware version and machine/board running. Open your preferred interface and enter the following prompts and you should see something similar to the printout below:

``` py
>>> import sys
>>> sys.implementation
(name='micropython', version=(1, 24, 0, 'preview'), _machine='SparkFun Pro Micro RP2350 with RP2350', _mpy=7942)
```

## Verify Memory (Internal and PSRAM)

Next, we can verify the total free memory on the Pro Micro which includes the built-in memory on the RP2350 as well as the 8MB PSRAM. We'll use the <code>gc</code> module for this so type in the prompt below and you should see a response close to the value below:

``` py
>>> import gc
>>> gc.mem_free()
8640352
```



## WS2812 LED Control

Finally, we'll make sure we can properly control the WS2812 LED on the Pro Micro using the <code>machine</code> and <code>neopixel</code> classes. The WS2812 Data In pin connects to I/O 25 on the RP2350 so we'll create a pin for it as an OUTPUT and assign it to the LED object. Next, we'll set the color to red and finally write the color values to the LED. The code below goes through all these steps so try copying it on your machine and you should see the WS2812 LED turn red.

``` py
>>> import machine, neopixel
>>> pin = machine.Pin(25, machine.Pin.OUT)
>>> led = neopixel.NeoPixel(pin, 1)
>>> n[0] = (255, 0, 0)
>>> n.write()
```

Try playing around with other values between 0 and 255 for the three colors (R, G, B) and then writing the changes to switch the LED's displayed color.