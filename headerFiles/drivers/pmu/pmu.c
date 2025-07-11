/*
 *  Copyright (C) 2025 Texas Instruments Incorporated
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

/**
 *  \file pmu.c
 *
 *  \brief File containing PMU Driver APIs implementation
 *
 */

/* ========================================================================== */
/*                             Include Files                                  */
/* ========================================================================== */

#include <stdint.h>
#include <string.h>
#include <kernel/dpl/SystemP.h>
#include <drivers/pmu.h>
#include <drivers/hw_include/tistdtypes.h>

/* ========================================================================== */
/*                           Macros & Typedefs                                */
/* ========================================================================== */

#define PMU_MAX_LOG_ENTRIES (64U)
/* R5 PMU supports 3 event counters */
#define PMU_MAX_EVENT_COUNTERS (3U)

/* ========================================================================== */
/*                         Structures and Enums                               */
/* ========================================================================== */

typedef struct {
    const char* name;
    uint32_t type;
    uint32_t value;

} PMU_Event;


typedef struct {
    PMU_Event events[PMU_MAX_EVENT_COUNTERS];
    PMU_Event cycleCount;
    const char *name;

} PMU_ProfilePoint;

typedef struct {

    uint32_t logIndex;
    uint32_t bCycleCounter;
    uint32_t numEvents;
    PMU_ProfilePoint point[PMU_MAX_LOG_ENTRIES];

} PMU_ProfileObject;


/* ========================================================================== */
/*                 Internal Function Declarations                             */
/* ========================================================================== */

static void PMU_resetCounters(void);
static void PMU_enableAllCounters(uint32_t numCounters);
static void PMU_disableAllCounters(uint32_t numCounters);

/* ========================================================================== */
/*                            Global Variables                                */
/* ========================================================================== */

PMU_ProfileObject gProfileObject;

/* ========================================================================== */
/*                          Function Definitions                              */
/* ========================================================================== */

int32_t PMU_init(PMU_Config *cfg)
{
    uint32_t i, j;
    /* Initialize profile */
    memset((void *)&gProfileObject, 0U, sizeof(gProfileObject));
    gProfileObject.logIndex = 0U;
    gProfileObject.bCycleCounter = cfg->bCycleCounter;
    DebugP_assert(cfg->numEventCounters <= 3U);
    gProfileObject.numEvents = cfg->numEventCounters;

    /* Fill in event counter details to profile object */
    for(i = 0; i < PMU_MAX_LOG_ENTRIES; i++)
    {
        PMU_ProfilePoint *p = &gProfileObject.point[i];
        for(j = 0; j < cfg->numEventCounters; j++)
        {
            p->events[j].name = cfg->eventCounters[j].name;
            p->events[j].type = cfg->eventCounters[j].type;
        }
    }

    /* Set core PMU registers */
    uint32_t numCount = PmuP_GetNumCntrs();
    /* R5F PMU only has 3 event counters */
    DebugP_assert(numCount == cfg->numEventCounters);

    /* User mode enable */
    PmuP_Config(0U, 0U ,1U);

    /* Configure event counters */
    for(i = 0; i < cfg->numEventCounters; i++)
    {
        PmuP_ConfigCounter(i, (cfg->eventCounters[i].type & 0xFFU));
    }

    /* Disable counter overflow interrupt */
    for(i = 0; i < cfg->numEventCounters; i++)
    {
        PmuP_EnableCounterOverflowInterrupt(i, 0U);
    }
    if(cfg->bCycleCounter == TRUE)
    {
        PmuP_EnableCounterOverflowInterrupt(PmuP_PMU_CYCLE_COUNTER_NUM, 0U);
    }

    /* Reset counters */
    PmuP_ResetCounters();
    PmuP_ResetCycleCnt();

    /* Enable Counters */
    PMU_enableAllCounters(cfg->numEventCounters);

    return SystemP_SUCCESS;
}

int32_t PMU_profileStart(const char *name)
{
    uint32_t i = gProfileObject.logIndex;
    uint32_t j;
    uint32_t numEvents = gProfileObject.numEvents;
    uint32_t bCCnt = gProfileObject.bCycleCounter;

    PMU_ProfilePoint *p = &gProfileObject.point[i];
    
    p->name = name;

    /* Reset counters */
    PMU_resetCounters();

    /* Set cycle count value */
    if(bCCnt == TRUE)
    {
        p->cycleCount.value = PmuP_ReadCounter(PmuP_PMU_CYCLE_COUNTER_NUM);
    }
    for(j = 0U; j < numEvents; j++)
    {
        p->events[j].value = PmuP_ReadCounter(j);
    }

    return SystemP_SUCCESS;
}

int32_t PMU_profileEnd(const char *name)
{
    int32_t status = SystemP_SUCCESS;
    /* Read all three counters + cycle counter first thing */
    uint32_t ccount = PmuP_ReadCounter(PmuP_PMU_CYCLE_COUNTER_NUM);
    uint32_t count0 = PmuP_ReadCounter(0);
    uint32_t count1 = PmuP_ReadCounter(1);
    uint32_t count2 = PmuP_ReadCounter(2);
    uint32_t counts[3] = {count0, count1, count2};

    uint32_t i = gProfileObject.logIndex;
    uint32_t j;
    uint32_t numEvents = gProfileObject.numEvents;
    uint32_t bCCnt = gProfileObject.bCycleCounter;

    PMU_ProfilePoint *p = &gProfileObject.point[i];

    if(strcmp(name, p->name) != 0)
    {
        status = SystemP_FAILURE;
    }

    if(status == SystemP_SUCCESS)
    {
        /* Set cycle count value */
        if(bCCnt == TRUE)
        {
            p->cycleCount.value = ccount - p->cycleCount.value;
        }
        for(j = 0U; j < numEvents; j++)
        {
            p->events[j].value = counts[j] - p->events[j].value;
        }
        gProfileObject.logIndex++;
    }

    return status;
}

void PMU_profilePrintEntry(const char *name)
{
    uint32_t i, j;
    for(i = 0U; i < gProfileObject.logIndex; i++)
    {
        PMU_ProfilePoint *p = &gProfileObject.point[i];

        if(strcmp(name, p->name) == 0)
        {
            DebugP_log("Profile Point: %-32s\r\n", p->name);
            DebugP_log("Cycle Count: %lu\r\n", p->cycleCount.value);
            for(j = 0U; j < PMU_MAX_EVENT_COUNTERS; j++)
            {
                DebugP_log("%s Count: %lu\r\n", p->events[j].name, p->events[j].value);
            }
            DebugP_log("\r\n");
            break;
        }
    }
}

void PMU_profilePrint(void)
{
    uint32_t i, j;
    for(i = 0U; i < gProfileObject.logIndex; i++)
    {
        PMU_ProfilePoint *p = &gProfileObject.point[i];
        DebugP_log("Profile Point: %-32s\r\n", p->name);
        DebugP_log("Cycle Count: %lu\r\n", p->cycleCount.value);
        for(j = 0U; j < PMU_MAX_EVENT_COUNTERS; j++)
        {
            DebugP_log("%s Count: %lu\r\n", p->events[j].name, p->events[j].value);
        }
        DebugP_log("\r\n");
    }
}

static void PMU_resetCounters(void)
{
    PmuP_ResetCycleCnt();
    PmuP_ResetCounters();
}

static void PMU_enableAllCounters(uint32_t numCounters)
{
    uint32_t i;

    /* Enable all counters */
    PmuP_EnableAllCounters(1U);

    /* Enable specific counters */
    for(i = 0U; i < numCounters; i++)
    {
        PmuP_enableCounter(i, 1U);
    }
    PmuP_enableCounter(PmuP_PMU_CYCLE_COUNTER_NUM, 1U);
}

static void PMU_disableAllCounters(uint32_t numCounters)
{
    uint32_t i;
    
    /* Enable all counters */
    PmuP_EnableAllCounters(0U);

    /* Enable specific counters */
    for(i = 0U; i < numCounters; i++)
    {
        PmuP_enableCounter(i, 0U);
    }
    PmuP_enableCounter(PmuP_PMU_CYCLE_COUNTER_NUM, 0U);
}