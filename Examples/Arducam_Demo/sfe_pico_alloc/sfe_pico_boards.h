
#ifndef _SFE_PICO_BOARDS_H_
#define _SFE_PICO_BOARDS_H_

// Define the CS pin for the PSRAM on sparkfun boards. This uses the
// board #define in the pico SDK board files to determine the PSRAM settings

#if defined(SPARKFUN_PROMICRO_RP2350)

// For the pro micro rp2350
#define SFE_RP2350_XIP_CSI_PIN 19

#elif defined(SPARKFUN_THINGPLUS_RP2350)

// For the Thing plus rp2350
#define SFE_RP2350_XIP_CSI_PIN 19

#endif
#endif