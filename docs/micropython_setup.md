



## Setup

MicroPython is an application of the Python 3 language that runs on microcontrollers like the Pro Micro - RP2350 that allows you to easily interact with the board directly over a USB serial interface through either a command line interface or your preferred Python IDE such as [Thonny](https://thonny.org/). We'll do a quick overview of how to download MicroPython firmware on to the Pro Micro - RP2350 but for a complete overview of using MicroPython with this and other RP2350 boards, head over to Raspberry Pi's documentation by clicking the button below:

<center>
    [Raspberry Pi MicroPython Docs](https://www.raspberrypi.com/documentation/microcontrollers/micropython.html){ .md-button .md-button--primary}
</center>

## UF2 Firmware

As of this writing, we're waiting on the next release of MicroPython to include UF2 files for the Pro Micro - RP2350 so to get users started before then, you can download the "early release" of the UF2 firmware file by clicking the button below.

<center>
    [Pro Micro - RP2350 UF2 Firmware](https://cdn.sparkfun.com/assets/6/3/c/f/d/firmware_pro_micro_rp2350.uf2){ .md-button .md-button--primary}
</center>

Once downloaded, put the Pro Micro - RP2350 into UF2 bootloader mode and open the location the board appeared as a USB storage device (it should appear as "RP2350"). Next, simply drag and drop the UF2 file into the RP2350 folder and the board should reboot. Next, open up your preferred MicroPython IDE (or command line interface) and you can start interacting with your Pro Micro. Read on to the MicroPython examples section for a few quick examples to make sure everything is working properly.