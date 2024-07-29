/**

 */

// #include "hardware/flash.h"
#include "hardware/address_mapped.h"
#include "hardware/flash.h"
#include "hardware/gpio.h"
#include "hardware/regs/addressmap.h"
#include "hardware/spi.h"
#include "hardware/structs/qmi.h"
#include "hardware/structs/xip_ctrl.h"
#include "hardware/sync.h"
#include "pico/binary_info.h"
#include "pico/flash.h"
#include "pico/stdlib.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// our allocator - tlsf
#include "tlsf/tlsf.h"

#include "sfe_pico_alloc.h"
#include "sfe_pico_boards.h"

static tlsf_t _mem_heap = NULL;
static pool_t _mem_sram_pool = NULL;
static pool_t _mem_psram_pool = NULL;

static size_t _psram_size = 0;

static bool _bInitalized = false;

// The ID check is from the Circuit Python code that was downloaded from:
// https://github.com/raspberrypi/pico-sdk-rp2350/issues/12#issuecomment-2055274428
//
// in the file supervisor/port.c -- the function setup_psram()

// NOTE: The PSRAM IC used by the Circuit Python example is:
//      https://www.adafruit.com/product/4677
//
// The PSRAM IC used by the SparkFun Pro Micro is: apmemory APS6404L-3SQR-ZR
// https://www.mouser.com/ProductDetail/AP-Memory/APS6404L-3SQR-ZR?qs=IS%252B4QmGtzzpDOdsCIglviw%3D%3D
//
// The datasheets from both these IC's are almost identical (word for word), with the first being ESP32 branded
//

static size_t __no_inline_not_in_flash_func(setup_psram)(uint psram_cs_pin)
{
    if (!psram_cs_pin)
        return 0;
    gpio_set_function(psram_cs_pin, GPIO_FUNC_XIP_CS1);

    size_t psram_size = 0;
    uint32_t intr_stash = save_and_disable_interrupts();

    // Try and read the PSRAM ID via direct_csr.
    qmi_hw->direct_csr = 30 << QMI_DIRECT_CSR_CLKDIV_LSB | QMI_DIRECT_CSR_EN_BITS;
    // Need to poll for the cooldown on the last XIP transfer to expire
    // (via direct-mode BUSY flag) before it is safe to perform the first
    // direct-mode operation
    while ((qmi_hw->direct_csr & QMI_DIRECT_CSR_BUSY_BITS) != 0)
    {
    }

    // Exit out of QMI in case we've inited already
    qmi_hw->direct_csr |= QMI_DIRECT_CSR_ASSERT_CS1N_BITS;
    // Transmit as quad.
    qmi_hw->direct_tx = QMI_DIRECT_TX_OE_BITS | QMI_DIRECT_TX_IWIDTH_VALUE_Q << QMI_DIRECT_TX_IWIDTH_LSB | 0xf5;
    while ((qmi_hw->direct_csr & QMI_DIRECT_CSR_BUSY_BITS) != 0)
    {
    }
    (void)qmi_hw->direct_rx;
    qmi_hw->direct_csr &= ~(QMI_DIRECT_CSR_ASSERT_CS1N_BITS);

    // Read the id
    qmi_hw->direct_csr |= QMI_DIRECT_CSR_ASSERT_CS1N_BITS;
    uint8_t kgd = 0;
    uint8_t eid = 0;
    for (size_t i = 0; i < 7; i++)
    {
        if (i == 0)
        {
            qmi_hw->direct_tx = 0x9f;
        }
        else
        {
            qmi_hw->direct_tx = 0xff;
        }
        while ((qmi_hw->direct_csr & QMI_DIRECT_CSR_TXEMPTY_BITS) == 0)
        {
        }
        while ((qmi_hw->direct_csr & QMI_DIRECT_CSR_BUSY_BITS) != 0)
        {
        }
        if (i == 5)
        {
            kgd = qmi_hw->direct_rx;
        }
        else if (i == 6)
        {
            eid = qmi_hw->direct_rx;
        }
        else
        {
            (void)qmi_hw->direct_rx;
        }
    }
    // Disable direct csr.
    qmi_hw->direct_csr &= ~(QMI_DIRECT_CSR_ASSERT_CS1N_BITS | QMI_DIRECT_CSR_EN_BITS);

    if (kgd != 0x5D)
    {
        printf("Invalid PSRAM ID: %x\n", kgd);
        restore_interrupts(intr_stash);
        return psram_size;
    }

    // Enable quad mode.
    qmi_hw->direct_csr = 30 << QMI_DIRECT_CSR_CLKDIV_LSB | QMI_DIRECT_CSR_EN_BITS;
    // Need to poll for the cooldown on the last XIP transfer to expire
    // (via direct-mode BUSY flag) before it is safe to perform the first
    // direct-mode operation
    while ((qmi_hw->direct_csr & QMI_DIRECT_CSR_BUSY_BITS) != 0)
    {
    }

    // RESETEN, RESET and quad enable
    for (uint8_t i = 0; i < 3; i++)
    {
        qmi_hw->direct_csr |= QMI_DIRECT_CSR_ASSERT_CS1N_BITS;
        if (i == 0)
        {
            qmi_hw->direct_tx = 0x66;
        }
        else if (i == 1)
        {
            qmi_hw->direct_tx = 0x99;
        }
        else
        {
            qmi_hw->direct_tx = 0x35;
        }
        while ((qmi_hw->direct_csr & QMI_DIRECT_CSR_BUSY_BITS) != 0)
        {
        }
        qmi_hw->direct_csr &= ~(QMI_DIRECT_CSR_ASSERT_CS1N_BITS);
        for (size_t j = 0; j < 20; j++)
        {
            asm("nop");
        }
        (void)qmi_hw->direct_rx;
    }
    // Disable direct csr.
    qmi_hw->direct_csr &= ~(QMI_DIRECT_CSR_ASSERT_CS1N_BITS | QMI_DIRECT_CSR_EN_BITS);

    qmi_hw->m[1].timing =
        QMI_M1_TIMING_PAGEBREAK_VALUE_1024 << QMI_M1_TIMING_PAGEBREAK_LSB | // Break between pages.
        3 << QMI_M1_TIMING_SELECT_HOLD_LSB | // Delay releasing CS for 3 extra system cycles.
        1 << QMI_M1_TIMING_COOLDOWN_LSB | 1 << QMI_M1_TIMING_RXDELAY_LSB |
        16 << QMI_M1_TIMING_MAX_SELECT_LSB |  // In units of 64 system clock cycles. PSRAM says 8us max. 8 / 0.00752 /64
                                              // = 16.62
        7 << QMI_M1_TIMING_MIN_DESELECT_LSB | // In units of system clock cycles. PSRAM says 50ns.50 / 7.52 = 6.64
        2 << QMI_M1_TIMING_CLKDIV_LSB;
    qmi_hw->m[1].rfmt = (QMI_M1_RFMT_PREFIX_WIDTH_VALUE_Q << QMI_M1_RFMT_PREFIX_WIDTH_LSB |
                         QMI_M1_RFMT_ADDR_WIDTH_VALUE_Q << QMI_M1_RFMT_ADDR_WIDTH_LSB |
                         QMI_M1_RFMT_SUFFIX_WIDTH_VALUE_Q << QMI_M1_RFMT_SUFFIX_WIDTH_LSB |
                         QMI_M1_RFMT_DUMMY_WIDTH_VALUE_Q << QMI_M1_RFMT_DUMMY_WIDTH_LSB |
                         QMI_M1_RFMT_DUMMY_LEN_VALUE_24 << QMI_M1_RFMT_DUMMY_LEN_LSB |
                         QMI_M1_RFMT_DATA_WIDTH_VALUE_Q << QMI_M1_RFMT_DATA_WIDTH_LSB |
                         QMI_M1_RFMT_PREFIX_LEN_VALUE_8 << QMI_M1_RFMT_PREFIX_LEN_LSB |
                         QMI_M1_RFMT_SUFFIX_LEN_VALUE_NONE << QMI_M1_RFMT_SUFFIX_LEN_LSB);
    qmi_hw->m[1].rcmd = 0xeb << QMI_M1_RCMD_PREFIX_LSB | 0 << QMI_M1_RCMD_SUFFIX_LSB;
    qmi_hw->m[1].wfmt = (QMI_M1_WFMT_PREFIX_WIDTH_VALUE_Q << QMI_M1_WFMT_PREFIX_WIDTH_LSB |
                         QMI_M1_WFMT_ADDR_WIDTH_VALUE_Q << QMI_M1_WFMT_ADDR_WIDTH_LSB |
                         QMI_M1_WFMT_SUFFIX_WIDTH_VALUE_Q << QMI_M1_WFMT_SUFFIX_WIDTH_LSB |
                         QMI_M1_WFMT_DUMMY_WIDTH_VALUE_Q << QMI_M1_WFMT_DUMMY_WIDTH_LSB |
                         QMI_M1_WFMT_DUMMY_LEN_VALUE_NONE << QMI_M1_WFMT_DUMMY_LEN_LSB |
                         QMI_M1_WFMT_DATA_WIDTH_VALUE_Q << QMI_M1_WFMT_DATA_WIDTH_LSB |
                         QMI_M1_WFMT_PREFIX_LEN_VALUE_8 << QMI_M1_WFMT_PREFIX_LEN_LSB |
                         QMI_M1_WFMT_SUFFIX_LEN_VALUE_NONE << QMI_M1_WFMT_SUFFIX_LEN_LSB);
    qmi_hw->m[1].wcmd = 0x38 << QMI_M1_WCMD_PREFIX_LSB | 0 << QMI_M1_WCMD_SUFFIX_LSB;

    psram_size = 1024 * 1024; // 1 MiB
    uint8_t size_id = eid >> 5;
    if (eid == 0x26 || size_id == 2)
    {
        psram_size *= 8;
    }
    else if (size_id == 0)
    {
        psram_size *= 2;
    }
    else if (size_id == 1)
    {
        psram_size *= 4;
    }

    // Mark that we can write to PSRAM.
    xip_ctrl_hw->ctrl |= XIP_CTRL_WRITABLE_M1_BITS;
    restore_interrupts(intr_stash);
    // printf("PSRAM ID: %x %x\n", kgd, eid);
    return psram_size;
}

//-------------------------------------------------------------------------------
// Allocator Routines
//-------------------------------------------------------------------------------

// Location /address where PSRAM starts
// TODO - is there a define in the pico sdk for this?
#define PSRAM_LOCATION _u(0x11000000)

// Internal function that init's the allocator
static bool sfe_pico_alloc_init()
{
    if (_bInitalized)
        return true;

    // First, our sram pool. External heap symbols from rpi pico-sdk
    extern unsigned char __heap_start;
    extern unsigned char __heap_end;
    // size
    size_t sram_size = (size_t)(&__heap_end - &__heap_start) * sizeof(uint32_t);

    _mem_heap = tlsf_create_with_pool((void *)&__heap_start, sram_size, 64 * 1024 * 1024);
    _mem_sram_pool = tlsf_get_pool(_mem_heap);

#ifndef SFE_RP2350_XIP_CSI_PIN
    printf("PSRAM CS pin not defined - check board file or specify board on build. unable to use PSRAM\n");
#else
    // Setup PSRAM if we have it.
    _psram_size = setup_psram(SFE_RP2350_XIP_CSI_PIN);
    // printf("PSRAM setup complete. PSRAM size 0x%lX (%d)\n", _psram_size, _psram_size);
    if (_psram_size > 0)
        _mem_psram_pool = tlsf_add_pool(_mem_heap, (void *)PSRAM_LOCATION, _psram_size);
#endif
    _bInitalized = true;
    return true;
}

// Our allocator interface -- same signature as the stdlib malloc/free/realloc/calloc

void *sfe_mem_malloc(size_t size)
{
    if (!sfe_pico_alloc_init())
        return NULL;
    return tlsf_malloc(_mem_heap, size);
}

void sfe_mem_free(void *ptr)
{
    if (!sfe_pico_alloc_init())
        return;
    tlsf_free(_mem_heap, ptr);
}

void *sfe_mem_realloc(void *ptr, size_t size)
{
    if (!sfe_pico_alloc_init())
        return NULL;
    return tlsf_realloc(_mem_heap, ptr, size);
}

void *sfe_mem_calloc(size_t num, size_t size)
{
    if (!sfe_pico_alloc_init())
        return NULL;
    void *ptr = tlsf_malloc(_mem_heap, num * size);
    if (ptr)
        memset(ptr, 0, num * size);
    return ptr;
}

static bool max_free_walker(void *ptr, size_t size, int used, void *user)
{
    size_t *max_size = (size_t *)user;
    if (!used && *max_size < size)
    {
        *max_size = size;
    }
    return true;
}
size_t sfe_mem_max_free_size(void)
{
    if (!sfe_pico_alloc_init())
        return 0;
    size_t max_free = 0;
    tlsf_walk_pool(_mem_sram_pool, max_free_walker, &max_free);
    if (_mem_psram_pool)
        tlsf_walk_pool(_mem_psram_pool, max_free_walker, &max_free);

    return max_free;
}

// Wrappers for the standard malloc/free/realloc/calloc routines - set the wrapper functions
// in the cmake file ...

void *__wrap_malloc(size_t size)
{
    return sfe_mem_malloc(size);
}
void __wrap_free(void *ptr)
{
    sfe_mem_free(ptr);
}
void *__wrap_realloc(void *ptr, size_t size)
{
    return sfe_mem_realloc(ptr, size);
}
void *__wrap_calloc(size_t num, size_t size)
{
    return sfe_mem_calloc(num, size);
}