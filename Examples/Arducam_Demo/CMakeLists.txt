cmake_minimum_required(VERSION 3.13)

# use our own allocator
set(SKIP_PICO_MALLOC 1)
include(pico_sdk_import.cmake)

project(arducam_demo C CXX ASM)
set(CMAKE_C_STANDARD 11)
set(CMAKE_CXX_STANDARD 17)
pico_sdk_init()

add_executable(arducam_demo arducam_demo.cpp)

include_directories(PICO_SPI_CAM/C/ArduCAM)
add_subdirectory(PICO_SPI_CAM/C/ArduCAM)

pico_enable_stdio_usb(arducam_demo 1)
pico_enable_stdio_uart(arducam_demo 0)

add_subdirectory(sfe_pico_alloc)

# pull in common dependencies and additional spi hardware support
target_link_libraries(arducam_demo 
	pico_stdlib
	hardware_dma
	hardware_i2c
	hardware_pwm
	ArduCAM
	sfe_pico_alloc
)

# create map/bin/hex file etc.
pico_add_extra_outputs(arducam_demo)
