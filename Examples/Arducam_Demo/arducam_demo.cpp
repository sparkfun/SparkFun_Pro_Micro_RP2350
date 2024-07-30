/*
    SPDX-License-Identifier: MIT
    
    Copyright (c) 2024 SparkFun Electronics

    This example is a modified version of Arducam_MINI_5MP_Plus_Videostreaming:
    https://github.com/ArduCAM/PICO_SPI_CAM/tree/master/C/Examples/

    Ensure you have the Pico SDK set up on your computer, then use the following
    to build this project:

    mkdir build
    cd build
    cmake .. -DPICO_PLATFORM=rp2350 -DPICO_BOARD=sparkfun_promicro_rp2350
    make

    Then upload the .uf2 file to the Pro Micro RP2350 with the Arducam attached.
    Run the provided Processing script to see the output.
*/

// Include the headers needed to run this example
#include <stdio.h>
#include "pico/stdlib.h"
#include "ArduCAM.h"
#include "ov5642_regs.h"
#include "tusb.h"
#include "hardware/spi.h"
#include "sfe_pico_alloc/sfe_pico_alloc.h"

// Chip select pin for camera
const uint8_t cameraCsPin = 5;

// Create an instance of the camera
ArduCAM myCAM(OV5642, cameraCsPin);

// Resolution of image, this is set by user input over USB serial
const sensor_reg* resolution;
uint32_t nRows;
uint32_t nCols;

// Buffer to store the image from the camera. This gets allocated with malloc()
// in main(), which is setup to use PSRAM
uint8_t *imageBuf;

// Pin to select whether to run the image processing algorithm. Will be setup in
// main() to use an internal pull up resisotr
uint32_t imageProcessPin = 29;

// Function definitions, implementations are defined after main()
int SerialUsbRead(void) ;
int SerialUSBAvailable(void);
void SerialUsb(uint8_t* buf, uint32_t length);
bool selectResolution();
void beginCamera(const sensor_reg* resolution);
void triggerCamera();
void readCameraImage(ArduCAM myCAM, uint8_t* buffer, uint32_t length);

// Main program entry point
int main() 
{
  // Standard initialization for Pico SDK
  stdio_init_all();

  // Get user input to select resolution to use
  if(selectResolution() == false)
  {
    // Invalid resolution selected
    printf("Invalid resolution! Exiting example\n");
    return 1;
  }

  // Create buffer to store image. In this demo, malloc() has been configure to
  // use the PSRAM of the SparkFun Pro Micro RP2350, so you don't need to do
  // anything else to use the PSRAM!
  imageBuf = (uint8_t*) malloc(nRows * nCols);
  if (!imageBuf)
  {
    // Always good practice to verify that malloc() worked
    printf("Malloc failed! Exiting example\n");
    return 1;
  }

  // Connect and initialize the camera
  beginCamera(resolution);

  // Configure image processing pin as an input with internal pullup resistor
  gpio_init(imageProcessPin);
  gpio_set_dir(imageProcessPin, GPIO_IN);
  gpio_pull_up(imageProcessPin);

  // Init TinyUSB, which is used for sending the image data since it's more
  // reliable than printf()
  tusb_init();

  // Main loop
  while (1)
  {
    // Trigger next frame capture from camera
    printf("Triggering\n");
    triggerCamera();

    // Read image from camera into image buffer
    printf("Reading\n");
    readCameraImage(myCAM, imageBuf, nRows * nCols);
    
    // Check whether image processing has been requested by user
    if(gpio_get(imageProcessPin))
    {
      // Run simple thresholding algorithm
      printf("Processing\n");
      for(int i = 0; i < nRows * nCols; i++)
      {
        imageBuf[i] = imageBuf[i] < 127 ? 0 : 255;
      }
    }
    
    // Print raw image data using SerialUsb, which behaves better than printf
    // and fwrite in this example. Need to send it in chunks, otherwise the
    // transmission can get desynced for some reason
    printf("Sending\n");
    int n = 512;
    for(int i = 0; i < (nRows * nCols) / n; i++)
    {
      SerialUsb(imageBuf + i*n, n);
    }

    // Ensure everything finishes sending
    sleep_ms(10);
  }
}

void readCameraImage(ArduCAM myCAM, uint8_t* buffer, uint32_t length)
{
  // Read raw image data
  myCAM.CS_LOW();
  myCAM.set_fifo_burst();
  spi_read_blocking(SPI_PORT, BURST_FIFO_READ, buffer, length);
  myCAM.CS_HIGH();
}

void triggerCamera()
{
  // Clear camera's buffer and flag
  myCAM.flush_fifo();
  myCAM.clear_fifo_flag();

  // Start capture and wait for it to finish
  myCAM.start_capture();
  while(myCAM.get_bit(ARDUCHIP_TRIG, CAP_DONE_MASK) == 0)
  {
    // Just waiting until this bit gets set, nothing to do
  }
}

void beginCamera(const sensor_reg* resolution)
{
  uint8_t vid, pid;
  uint8_t cameraCommand;

	myCAM.Arducam_init();	
  gpio_init(cameraCsPin);
  gpio_set_dir(cameraCsPin, GPIO_OUT);
  gpio_put(cameraCsPin, 1);
  //Reset the CPLD
  myCAM.write_reg(0x07, 0x80);
  sleep_ms(100);
  myCAM.write_reg(0x07, 0x00);
  sleep_ms(100);
	while (1) 
  {
			//Check if the ArduCAM SPI bus is OK
			myCAM.write_reg(ARDUCHIP_TEST1, 0x55);
			cameraCommand = myCAM.read_reg(ARDUCHIP_TEST1);
			if (cameraCommand != 0x55) {
				printf(" SPI interface Error!\n");
				sleep_ms(1000); continue;
			} else {
				printf("ACK CMD SPI interface OK.END\n"); break;
			}
	}

  while (1) 
  {
    //Check if the camera module type is OV5640
    myCAM.wrSensorReg16_8(0xff, 0x01);
    myCAM.rdSensorReg16_8(OV5642_CHIPID_HIGH, &vid);
    myCAM.rdSensorReg16_8(OV5642_CHIPID_LOW, &pid);
    if((vid != 0x56) || (pid != 0x42))
    {
      printf("Can't find OV5642 module!\n");
      sleep_ms(1000); continue;
    }
    else 
    {
      printf("OV5642 detected.END\n"); break;
    }
  }

  //Change to RAW capture mode and initialize the OV5642 module
  myCAM.set_format(RAW);
  myCAM.InitCAM();
  myCAM.write_reg(ARDUCHIP_TIM, VSYNC_LEVEL_MASK);   //VSYNC is active HIGH
  myCAM.wrSensorRegs16_8(resolution);
  sleep_ms(1000);
  myCAM.clear_fifo_flag();
  myCAM.write_reg(ARDUCHIP_FRAMES,0x00);
}

bool selectResolution()
{
  // Clear any data that happens to be in the serial buffer
  while(SerialUSBAvailable() > 0)
  {
    SerialUsbRead();
  }

  // Wait for input from host
  while(SerialUSBAvailable() == 0)
  {
    // Prompt user for input
    printf("\n");
    printf("Please select a resolution:\n");
    printf("1 - 320x240\n");
    printf("2 - 640x480\n");
    printf("3 - 1280x960\n");
    sleep_ms(1000);
  }

  // Wait a brief moment to ensure all bytes are received
  sleep_ms(10);

  // Get input
  char input = SerialUsbRead();

  printf("\n");
  printf("Selected resolution: ");
  switch(input)
  {
    case '1':
      nRows = 240;
      nCols = 320;
      resolution = ov5642_320x240;
      printf("320x240\n");
      break;
    case '2':
      nRows = 480;
      nCols = 640;
      resolution = ov5642_640x480;
      printf("640x480\n");
      break;
    case '3':
      nRows = 960;
      nCols = 1280;
      resolution = ov5642_1280x960;
      printf("1280x960\n");
      break;
    default:
      return false;
      break;
  }

  return true;
}

void SerialUsb(uint8_t* buf, uint32_t length)
{
  static uint64_t last_avail_time;
  int i = 0;
  if (tud_cdc_connected()) 
  {
    for (int i = 0; i < length;) 
    {
      int n = length - i;
      int avail = tud_cdc_write_available();
      if (n > avail) n = avail;
      if (n) 
      {
        int n2 = tud_cdc_write(buf + i, n);
        tud_task();
        tud_cdc_write_flush();
        i += n2;
        last_avail_time = time_us_64();
      } 
      else 
      {
        tud_task();
        tud_cdc_write_flush();
        if (!tud_cdc_connected() ||
          (!tud_cdc_write_available() && time_us_64() > last_avail_time + 1000000 /* 1 second */)) {
          break;
        }
      }
    }
  } 
  else 
  {
    // reset our timeout
    last_avail_time = 0;
  }
}

int SerialUSBAvailable(void)
{
  return tud_cdc_available();
} 

int SerialUsbRead(void) 
{
  if (tud_cdc_connected() && tud_cdc_available()) 
  {
    return tud_cdc_read_char();
  }
  return -1;
}
