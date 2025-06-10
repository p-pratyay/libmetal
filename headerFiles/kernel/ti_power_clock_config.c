/*
 *  Copyright (C) 2021 Texas Instruments Incorporated
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
 #include "kernel/dpl/ti_power_clock_config.h"
#include <kernel/dpl/DebugP.h>
#include <drivers/soc.h>
#define LPRINTF(format, ...) printf(format, ##__VA_ARGS__)
#define TISCI_DEV_MCU_UART0 149U
#define TISCI_DEV_MCU_UART0_FCLK_CLK 0
#define TISCI_DEV_MCU_UART0_VBUSP_CLK 3

#define SOC_MODULES_END     (0xFFFFFFFFu)

typedef struct {

    uint32_t moduleId;
    uint32_t clkId;
    uint32_t clkRate;

} SOC_ModuleClockFrequency;

uint32_t gSocModules[] = {
    TISCI_DEV_MCU_UART0,

    SOC_MODULES_END,
};

SOC_ModuleClockFrequency gSocModulesClockFrequency[] = {
    { TISCI_DEV_MCU_UART0, TISCI_DEV_MCU_UART0_FCLK_CLK, 48000000 },

    { SOC_MODULES_END, SOC_MODULES_END, SOC_MODULES_END },
};


void Module_clockEnable(void)
{
    int32_t status;
    uint32_t i = 0;
    LPRINTF("inside Module_clockEnable function\r\n");
    while(gSocModules[i]!=SOC_MODULES_END)
    {
         LPRINTF("inside the while loop of Module_ClockEnable\r\n");
        status = SOC_moduleClockEnable(gSocModules[i], 1);
        DebugP_assertNoLog(status == SystemP_SUCCESS);
        i++;
    }
    LPRINTF("inside Module_clockEnable function reached the end \r\n");
}

void Module_clockDisable(void)
{
    int32_t status;
    uint32_t i = 0;

    while(gSocModules[i]!=SOC_MODULES_END)
    {
        status = SOC_moduleClockEnable(gSocModules[i], 0);
        DebugP_assertNoLog(status == SystemP_SUCCESS);
        i++;
    }
}

void Module_clockSetFrequency(void)
{
    int32_t status;
    uint32_t i = 0;

    while(gSocModulesClockFrequency[i].moduleId!=SOC_MODULES_END)
    {
         LPRINTF("inside the while loop of Module_Clockset______Freq\r\n");
        status = SOC_moduleSetClockFrequency(
                    gSocModulesClockFrequency[i].moduleId,
                    gSocModulesClockFrequency[i].clkId,
                    gSocModulesClockFrequency[i].clkRate
                    );
        LPRINTF("the value of return value is %d\r\n", status);
        DebugP_assertNoLog(status == SystemP_SUCCESS);
        i++;
    }
     LPRINTF("inside Module_clockset____FReq function reached the end \r\n");
}


void PowerClock_init(void)
{
    LPRINTF("inside the PowerClock_init function \r\n");
    Module_clockEnable();
    LPRINTF("inside the PowerClock_init function after Module_clockEnable \r\n");
    Module_clockSetFrequency();
    LPRINTF("inside the PowerClock_init function after Module_clockSetFrequency \r\n");
}

void PowerClock_deinit()
{
    Module_clockDisable();
}
