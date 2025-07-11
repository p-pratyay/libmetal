/*
 *  Copyright (C) 2018-2021 Texas Instruments Incorporated
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

#include <kernel/freertos/dpl/common/ClockP_freertos_priv.h>
#include <kernel/dpl/TimerP.h>
#include <kernel/dpl/ClockP.h>
#define LPRINTF(format, ...) printf(format, ##__VA_ARGS__)
/* ----------- ClockP ----------- */
#define MCU_TIMER0_CLOCK_SRC_MUX_ADDR (0x4508060u)
#define MCU_TIMER0_CLOCK_SRC_MCU_HFOSC0 (0x0u)
#define MCU_TIMER0_BASE_ADDR     (0x4800000u)


ClockP_Config gClockConfig = {
    .timerBaseAddr = MCU_TIMER0_BASE_ADDR,
    .timerHwiIntNum = 28,
    .timerInputClkHz = 25000000,
    .timerInputPreScaler = 1,
    .usecPerTick = 1000,
};

void ClockP_timerClearOverflowInt(uint32_t timerBaseAddr)
{
    TimerP_clearOverflowInt(timerBaseAddr);
}

uint32_t ClockP_getTimerCount(uint32_t timerBaseAddr)
{
    return TimerP_getCount(timerBaseAddr);
}

void ClockP_init(void)
{
    TimerP_Params timerParams;
    HwiP_Params timerHwiParams;

    /* These MUST not be 0 */
    DebugP_assert( gClockConfig.timerInputPreScaler != 0U);
    DebugP_assert( gClockConfig.timerInputClkHz != 0U);
    DebugP_assert( gClockConfig.usecPerTick != 0U);
    DebugP_assert( gClockConfig.timerBaseAddr != 0U);

    /* init internal data structure */
    gClockCtrl.ticks = 0;
    gClockCtrl.usecPerTick = gClockConfig.usecPerTick;
    gClockCtrl.timerBaseAddr = gClockConfig.timerBaseAddr;

    /* Check if tick period set in FreeRTOS config matches the value that is passed to this function
     * A mistmatch will affect when pdMS_TO_TICKS to calculate delays
     */
    if( pdMS_TO_TICKS( TIME_IN_MILLI_SECONDS ) != ClockP_usecToTicks( TIME_IN_MICRO_SECONDS ) )
    {
        DebugP_logWarn("FreeRTOS configTICK_RATE_HZ (%d), does not match ClockP tick rate Hz (%d)\r\n",
            configTICK_RATE_HZ,
            TIME_IN_MICRO_SECONDS / (gClockConfig.usecPerTick)
            );
    }

    /* setup timer but dont start it */
    TimerP_Params_init(&timerParams);
    timerParams.inputPreScaler    = gClockConfig.timerInputPreScaler;
    timerParams.inputClkHz        = gClockConfig.timerInputClkHz;
    timerParams.periodInUsec      = gClockConfig.usecPerTick;
    timerParams.oneshotMode       = 0;
    timerParams.enableOverflowInt = 1;
    TimerP_setup(gClockCtrl.timerBaseAddr, &timerParams);

    /* Get timer reload count, we will use this later to compute current time in usecs */
    gClockCtrl.timerReloadCount = TimerP_getReloadCount(gClockCtrl.timerBaseAddr);

    /* setup ISR and enable it */
    HwiP_Params_init(&timerHwiParams);
    timerHwiParams.intNum = gClockConfig.timerHwiIntNum;
    timerHwiParams.callback = ClockP_timerTickIsr;
    timerHwiParams.isPulse = 0;
    (void) HwiP_construct(&gClockCtrl.timerHwiObj, &timerHwiParams);

    /* start the tick timer */
    TimerP_start(gClockCtrl.timerBaseAddr);
    // LPRINTF("completed the clockP_init function\r\n");
}

