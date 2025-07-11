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

typedef struct ClockP_Struct_
{
    StaticTimer_t timerObj;
    TimerHandle_t timerHndl;
    ClockP_FxnCallback callback;
    void *args;

} ClockP_Struct;


static void ClockP_sleepTicks(uint32_t ticks);

ClockP_Control gClockCtrl;

void ClockP_timerTickIsr(void *args)
{
    void vPortTimerTickHandler(void);

    /* increment the systick counter */
    gClockCtrl.ticks++;
    

    vPortTimerTickHandler();

    ClockP_timerClearOverflowInt(gClockConfig.timerBaseAddr);
}

void ClockP_timerCallbackFunction( TimerHandle_t xTimer )
{
    ClockP_Struct *pTimer = (ClockP_Struct *) pvTimerGetTimerID(xTimer);

    if((pTimer != NULL) && (pTimer->callback))
    {
        pTimer->callback((ClockP_Object*)pTimer, pTimer->args);
    }
}

int32_t ClockP_construct(ClockP_Object *handle, ClockP_Params *params)
{
    ClockP_Struct *pTimer = (ClockP_Struct*)handle;
    UBaseType_t uxAutoReload = pdFALSE;
    int32_t status;

    DebugP_assert(sizeof(ClockP_Struct) <= sizeof(ClockP_Object));

    if(params->period == 0U)
    {
        uxAutoReload = pdFALSE;
    }
    else
    {
        /* timeout and period MUST match for auto-reloading to take effect */
        DebugP_assert(params->timeout == params->period);

        uxAutoReload = pdTRUE;
    }

    pTimer->callback = params->callback;
    pTimer->args = params->args;

    pTimer->timerHndl = xTimerCreateStatic(
                            params->name,
                            params->timeout,
                            uxAutoReload,
                            pTimer,
                            ClockP_timerCallbackFunction,
                            &pTimer->timerObj
                            );
    if(pTimer->timerHndl == NULL)
    {
        status = SystemP_FAILURE;
    }
    else
    {
        if(params->start != 0U)
        {
            (void) xTimerStart(pTimer->timerHndl, portMAX_DELAY);
        }
        status = SystemP_SUCCESS;
    }
    return status;
}

void ClockP_destruct(ClockP_Object *handle)
{
    ClockP_Struct *pTimer = (ClockP_Struct*)handle;

    (void) xTimerDelete(pTimer->timerHndl, portMAX_DELAY);
}

uint64_t ClockP_usecToTicks(uint64_t usecs)
{
    return (usecs / (uint64_t)gClockCtrl.usecPerTick);
}

uint64_t ClockP_ticksToUsec(uint32_t ticks)
{
    return ((uint64_t)ticks * gClockCtrl.usecPerTick);
}

uint32_t ClockP_getTicks(void)
{
    uint32_t ticks;
    if (HwiP_inISR() != 0U)
    {
        ticks = (uint32_t)xTaskGetTickCountFromISR();
    }
    else
    {
        ticks = (uint32_t)xTaskGetTickCount();
    }
    return (ticks);
}

uint32_t ClockP_getTimeout(ClockP_Object *handle)
{
    ClockP_Struct *pTimer = (ClockP_Struct*)handle;
    uint32_t value = 0;

    if(xTimerIsTimerActive(pTimer->timerHndl) != 0)
    {
        value = xTimerGetExpiryTime( pTimer->timerHndl ) - xTaskGetTickCount();
    }
    return value;
}

uint32_t ClockP_isActive(ClockP_Object *handle)
{
    ClockP_Struct *pTimer = (ClockP_Struct*)handle;

    return (uint32_t)xTimerIsTimerActive(pTimer->timerHndl);
}

void ClockP_Params_init(ClockP_Params *params)
{
    params->start = 0;
    params->timeout = 0;
    params->period = 0;
    params->callback = 0;
    params->args = NULL;
    params->name = "Clock (DPL)";
}

void ClockP_setTimeout(ClockP_Object *handle, uint32_t timeout)
{
    ClockP_Struct *pTimer = (ClockP_Struct*)handle;

    if(HwiP_inISR() != 0U)
    {
        BaseType_t xHigherPriorityTaskWoken = 0;

        (void) xTimerChangePeriodFromISR(pTimer->timerHndl, timeout, &xHigherPriorityTaskWoken);
        portYIELD_FROM_ISR((uint32_t)xHigherPriorityTaskWoken);
    }
    else
    {
        (void) xTimerChangePeriod( pTimer->timerHndl, timeout, portMAX_DELAY);
    }
}

void ClockP_start(ClockP_Object *handle)
{
    ClockP_Struct *pTimer = (ClockP_Struct*)handle;

    if(HwiP_inISR() != 0U)
    {
        BaseType_t xHigherPriorityTaskWoken = 0;

        (void) xTimerStartFromISR(pTimer->timerHndl, &xHigherPriorityTaskWoken);
        portYIELD_FROM_ISR((uint32_t)xHigherPriorityTaskWoken);
    }
    else
    {
        (void) xTimerStart(pTimer->timerHndl, portMAX_DELAY);
    }
}

void ClockP_stop(ClockP_Object *handle)
{
    ClockP_Struct *pTimer = (ClockP_Struct*)handle;

    if(HwiP_inISR() != 0U)
    {
        BaseType_t xHigherPriorityTaskWoken = 0;

        (void) xTimerStopFromISR(pTimer->timerHndl, &xHigherPriorityTaskWoken);
        portYIELD_FROM_ISR((uint32_t)xHigherPriorityTaskWoken);
    }
    else
    {
        (void) xTimerStop(pTimer->timerHndl, portMAX_DELAY);
    }
}

void ClockP_sleep(uint32_t sec)
{
    uint64_t ticks = (uint64_t)sec * TIME_IN_MICRO_SECONDS / (uint64_t)gClockCtrl.usecPerTick;

    ClockP_sleepTicks((uint32_t)ticks);
}

void ClockP_usleep(uint64_t usec)
{
    uint64_t curTime, endTime;
    uint32_t ticksToSleep;

    curTime = ClockP_getTimeUsec();
    endTime = curTime + usec;

    if (usec >= gClockCtrl.usecPerTick) {
        ticksToSleep = (uint32_t)usec / gClockCtrl.usecPerTick;
        ClockP_sleepTicks(ticksToSleep);
    }
    else
    {
        curTime = ClockP_getTimeUsec();
        while (curTime < endTime) {
            curTime = ClockP_getTimeUsec();
        }
    }
}

/*
 *  Get the current time in microseconds.
 */
uint64_t ClockP_getTimeUsec(void)
{
    uint64_t ts = 0U;
    uint32_t timerCount;
    uint64_t ticks1;
    uint64_t ticks2;

    do {
        ticks1 = gClockCtrl.ticks;
        timerCount = ClockP_getTimerCount(gClockCtrl.timerBaseAddr);
        ticks2 = gClockCtrl.ticks;
    } while (ticks1 != ticks2);

    /* Get the current time in microseconds */
    ts = (ticks2 * (uint64_t)gClockCtrl.usecPerTick)
             + (uint64_t) ( /* convert timer count to usecs */
                (uint64_t)(((timerCount - gClockCtrl.timerReloadCount)*gClockCtrl.usecPerTick)/(MAX_TIMER_COUNT_VALUE - gClockCtrl.timerReloadCount))
                );

    return (ts);
}

/*
 *  Sleep for a given number of ClockP ticks.
 */
static void ClockP_sleepTicks(uint32_t ticks)
{
    vTaskDelay(ticks);
}

/*
 *  De-initialize the clock module.
 */
void ClockP_deinit()
{
    /* Stop the tick timer and clear any pending interrupts */
    TimerP_stop(gClockCtrl.timerBaseAddr);
    TimerP_clearOverflowInt(gClockCtrl.timerBaseAddr);

    /* Disable and destroy the HWI of the timer used by clock module */
    HwiP_destruct(&gClockCtrl.timerHwiObj);
}
