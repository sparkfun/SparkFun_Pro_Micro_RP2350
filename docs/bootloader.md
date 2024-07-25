The Pro Micro - RP2350 ships with a UF2 (USB Flashing Format) bootloader. This bootloader allows the board to enumerate as a removable storage device when connected over USB on Windows, Mac, and Linux <i>without</i> having to install any drivers. 

### What is UF2?

UF2 stands for USB Flashing Format, which was developed by Microsoft for PXT (now known as MakeCode) for flashing microcontrollers over the Mass Storage Class (MSC), just like a removable flash drive. The file format is unique, so unfortunately, you cannot simply drag and drop a compiled binary or hex file onto the board. Instead, the format of the file has extra information to tell the processor where the data goes, in addition to the data itself. For more information about UF2, you can read more from the [MakeCode blog](https://makecode.com/blog/one-chip-to-flash-them-all), as well as the [UF2 file format specification](https://github.com/Microsoft/uf2).

### Pro Micro RP2350 UF2 Bootloader

Setting the board into bootloader mode requires a few simple steps using the BOOT and RESET buttons on the Pro Micro. Start by pressing down and holding the BOOT button. Next, press and release the RESET button and the finally release the BOOT button. The Pro Micro should appear as a removable storage device on your computer named "RP2350".