/*
 *  Copyright (C) 2021-2025 Texas Instruments Incorporated
 *
 *  Redistribution and use in source and binary forms, with or without
 *  modification, are permitted provided that the following conditions
 *  are met:
 *
 *    Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 *
 *    Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the
 *    distribution.
 *
 *    Neither the name of Texas Instruments Incorporated nor the names of
 *    its contributors may be used to endorse or promote products derived
 *    from this software without specific prior written permission.
 *
 *  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 *  "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 *  LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
 *  A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
 *  OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
 *  SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
 *  LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
 *  DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
 *  THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 *  (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 *  OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */
/*
 * Auto generated file
 */
#include <stdio.h>
#include <drivers/soc.h>
#include <kernel/dpl/AddrTranslateP.h>
#include "ti_dpl_config.h"
#include "ti_drivers_config.h"
#include <drivers/hw_include/am62ax/cslr_soc_defines.h>
#include <drivers/soc/am62ax/soc.h>



/* ----------- HwiP ----------- */
// HwiP_Config gHwiConfig = {
//     .intcBaseAddr = 0x7FF0000u,
// };

/* ----------- ClockP ----------- */
#define MCU_TIMER0_CLOCK_SRC_MUX_ADDR (0x4508060u)
#define MCU_TIMER0_CLOCK_SRC_MCU_HFOSC0 (0x0u)
#define MCU_TIMER0_BASE_ADDR     (0x4800000u)

// ClockP_Config gClockConfig = {
//     .timerBaseAddr = MCU_TIMER0_BASE_ADDR,
//     .timerHwiIntNum = 28,
//     .timerInputClkHz = 25000000,
//     .timerInputPreScaler = 1,
//     .usecPerTick = 1000,
// };

/* ----------- DebugP ----------- */
// void putchar_(char character)
// {
//     /* Output to memory trace buffer */
//     DebugP_memLogWriterPutChar(character);
//     /* Output to UART console */
//     DebugP_uartLogWriterPutChar(character);
// }

/* DebugP log buffer memory and size
 * - This log can be viewed via ROV in CCS
 * - When linux is enabled, this log can also be viewed via linux debugfs
 */
// char gDebugMemLog[DebugP_MEM_LOG_SIZE] __attribute__ ((section (".bss.debug_mem_trace_buf"), aligned (128)));
// uint32_t gDebugMemLogSize = DebugP_MEM_LOG_SIZE;


/* ----------- AddrTranslateP ----------- */
#define CONFIG_ADDR_TRANSLATE_RAT_BASE_ADDR  (0x07FE0000u)
#define CONFIG_ADDR_TRANSLATE_REGIONS  (3u)

AddrTranslateP_RegionConfig gAddrTranslateRegionConfig[CONFIG_ADDR_TRANSLATE_REGIONS] = 
{
    {
        .localAddr = 0x9B000000u,
        .systemAddr = 0x9B000000u,
        .size = AddrTranslateP_RegionSize_16M,
    },
    {
        .localAddr = 0x9C000000u,
        .systemAddr = 0x9C000000u,
        .size = AddrTranslateP_RegionSize_16M,
    },
    {
        .localAddr = 0xA0000000u,
        .systemAddr = 0xA0000000u,
        .size = AddrTranslateP_RegionSize_16M,
    },
};

/* ----------- CacheP ----------- */
// CacheP_Config gCacheConfig = {
//     .enable = 1,
//     .enableForceWrThru = 0,
// };

/* ----------- MpuP_armv7 ----------- */
#define CONFIG_MPU_NUM_REGIONS  (8u)

// MpuP_Config gMpuConfig = {
//     .numRegions = CONFIG_MPU_NUM_REGIONS,
//     .enableBackgroundRegion = 0,
//     .enableMpu = 1,
// };

// MpuP_RegionConfig gMpuRegionConfig[CONFIG_MPU_NUM_REGIONS] =
// {
//     {
//         .baseAddr = 0x0u,
//         .size = MpuP_RegionSize_2G,
//         .attrs = {
//             .isEnable = 1,
//             .isCacheable = 0,
//             .isBufferable = 0,
//             .isSharable = 1,
//             .isExecuteNever = 1,
//             .tex = 0,
//             .accessPerm = MpuP_AP_ALL_RW,
//             .subregionDisableMask = 0x0u
//         },
//     },
//     {
//         .baseAddr = 0x0u,
//         .size = MpuP_RegionSize_32K,
//         .attrs = {
//             .isEnable = 1,
//             .isCacheable = 1,
//             .isBufferable = 1,
//             .isSharable = 0,
//             .isExecuteNever = 0,
//             .tex = 1,
//             .accessPerm = MpuP_AP_ALL_RW,
//             .subregionDisableMask = 0x0u
//         },
//     },
//     {
//         .baseAddr = 0x41010000u,
//         .size = MpuP_RegionSize_32K,
//         .attrs = {
//             .isEnable = 1,
//             .isCacheable = 1,
//             .isBufferable = 1,
//             .isSharable = 0,
//             .isExecuteNever = 0,
//             .tex = 1,
//             .accessPerm = MpuP_AP_ALL_RW,
//             .subregionDisableMask = 0x0u
//         },
//     },
//     {
//         .baseAddr = 0x79100000u,
//         .size = MpuP_RegionSize_512K,
//         .attrs = {
//             .isEnable = 1,
//             .isCacheable = 1,
//             .isBufferable = 1,
//             .isSharable = 0,
//             .isExecuteNever = 0,
//             .tex = 1,
//             .accessPerm = MpuP_AP_ALL_RW,
//             .subregionDisableMask = 0x0u
//         },
//     },
//     {
//         .baseAddr = 0x60000000u,
//         .size = MpuP_RegionSize_256M,
//         .attrs = {
//             .isEnable = 1,
//             .isCacheable = 1,
//             .isBufferable = 1,
//             .isSharable = 0,
//             .isExecuteNever = 0,
//             .tex = 1,
//             .accessPerm = MpuP_AP_ALL_RW,
//             .subregionDisableMask = 0x0u
//         },
//     },
//     {
//         .baseAddr = 0x9B000000u,
//         .size = MpuP_RegionSize_16M,
//         .attrs = {
//             .isEnable = 1,
//             .isCacheable = 0,
//             .isBufferable = 0,
//             .isSharable = 1,
//             .isExecuteNever = 0,
//             .tex = 1,
//             .accessPerm = MpuP_AP_ALL_RW,
//             .subregionDisableMask = 0x0u
//         },
//     },
//     {
//         .baseAddr = 0xA0000000u,
//         .size = MpuP_RegionSize_8M,
//         .attrs = {
//             .isEnable = 1,
//             .isCacheable = 0,
//             .isBufferable = 0,
//             .isSharable = 1,
//             .isExecuteNever = 1,
//             .tex = 1,
//             .accessPerm = MpuP_AP_ALL_RW,
//             .subregionDisableMask = 0x0u
//         },
//     },
//     {
//         .baseAddr = 0x9C000000u,
//         .size = MpuP_RegionSize_32M,
//         .attrs = {
//             .isEnable = 1,
//             .isCacheable = 0,
//             .isBufferable = 0,
//             .isSharable = 1,
//             .isExecuteNever = 0,
//             .tex = 1,
//             .accessPerm = MpuP_AP_ALL_RW,
//             .subregionDisableMask = 0x0u
//         },
//     },
// };


// #define BOOT_SECTION __attribute__((section(".text.boot")))

/* This function is called by _c_int00 */
// void BOOT_SECTION __mpu_init()
// {
//     CacheP_init();
//     MpuP_init();
// }


#define LPRINTF(format, ...) printf(format, ##__VA_ARGS__)
void Dpl_init(void)
{
    LPRINTF("[Dpl_init] Start DPL initialization\n");

    /* initialize Hwi but keep interrupts disabled */
    LPRINTF("[Dpl_init] Initializing HwiP...\n");
    HwiP_init();

    /* address trnaslate MUST happen early to make sure, the memory space is mapped for later modules */
    /* initialize Address Translate (RAT) Module */
    {
        AddrTranslateP_Params addrTranslateParams;

        LPRINTF("[Dpl_init] Initializing Address Translate (RAT) Module...\n");
        AddrTranslateP_Params_init(&addrTranslateParams);
        addrTranslateParams.numRegions = CONFIG_ADDR_TRANSLATE_REGIONS;
        addrTranslateParams.ratBaseAddr = CONFIG_ADDR_TRANSLATE_RAT_BASE_ADDR;
        addrTranslateParams.regionConfig = &gAddrTranslateRegionConfig[0];

        AddrTranslateP_init(&addrTranslateParams);
        LPRINTF("[Dpl_init] Address Translate (RAT) Module initialized\n");
    }

    /* init debug log zones early */
    /* Debug log init */
    LPRINTF("[Dpl_init] Enabling DebugP log zones...\n");
    DebugP_logZoneEnable(DebugP_LOG_ZONE_ERROR);
    DebugP_logZoneEnable(DebugP_LOG_ZONE_WARN);
    /* Initialize linux trace log writer */
    LPRINTF("[Dpl_init] Initializing DebugP memory log writer...\n");
    DebugP_memLogWriterInit(CSL_CORE_ID_MCU_R5FSS0_0);
    /* UART console to use for reading input */
    LPRINTF("[Dpl_init] Setting UART console driver index...\n");
    DebugP_uartSetDrvIndex(CONFIG_UART0);

    /* set timer clock source */
    LPRINTF("[Dpl_init] Setting timer clock source...\n");
    SOC_controlModuleUnlockMMR(SOC_DOMAIN_ID_MCU, 2);
    *(volatile uint32_t*)(MCU_TIMER0_CLOCK_SRC_MUX_ADDR) = MCU_TIMER0_CLOCK_SRC_MCU_HFOSC0;
    SOC_controlModuleLockMMR(SOC_DOMAIN_ID_MCU, 2);

    /* initialize Clock */
    // LPRINTF("[Dpl_init] Initializing ClockP...\n");
    // ClockP_init();

    /* Enable interrupt handling */
    LPRINTF("[Dpl_init] Enabling interrupts...\n");
    HwiP_enable();

    LPRINTF("[Dpl_init] DPL initialization complete\n");
}

void Dpl_deinit(void)
{
    /* de-initialize Clock */
    ClockP_deinit();
    /* Disable interrupt handling */
    HwiP_disable();
}
