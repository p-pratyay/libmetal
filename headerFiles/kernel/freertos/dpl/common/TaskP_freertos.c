/*
 *  Copyright (C) 2018-2023 Texas Instruments Incorporated
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

#include <kernel/dpl/TaskP.h>
#include <kernel/dpl/ClockP.h>
#include <FreeRTOS.h>
#include <task.h>
#include <stdbool.h>

#define TaskP_LOAD_CPU_LOAD_SCALE   (10000U)
#define TaskP_REGISTRY_MAX_ENTRIES  (16u)
#define TaskP_STACK_SIZE_MIN        (128U)

void vApplicationLoadHook(void);

typedef struct TaskP_Struct_ {
    StaticTask_t taskObj;
    TaskHandle_t taskHndl;
    uint32_t     lastRunTime;
    uint64_t     accRunTime;
} TaskP_Struct;

#ifdef SMP_FREERTOS
typedef struct {

    TaskP_Struct *taskRegistry[TaskP_REGISTRY_MAX_ENTRIES];
    uint32_t lastTotalTime;
    uint64_t accTotalTime;
    uint32_t idleTsk1LastRunTime;
    uint64_t idleTsk1AccRunTime;
    uint32_t idleTsk2LastRunTime;
    uint64_t idleTsk2AccRunTime;
    #if defined(SMP_QUADCORE_FREERTOS)
    uint32_t idleTsk3LastRunTime;
    uint64_t idleTsk3AccRunTime;
    uint32_t idleTsk4LastRunTime;
    uint64_t idleTsk4AccRunTime;
    #endif

} TaskP_Ctrl;
#else
typedef struct {

    TaskP_Struct *taskRegistry[TaskP_REGISTRY_MAX_ENTRIES];
    uint32_t lastTotalTime;
    uint64_t accTotalTime;
    uint32_t idleTskLastRunTime;
    uint64_t idleTskAccRunTime;

} TaskP_Ctrl;
#endif


TaskP_Ctrl gTaskP_ctrl;

static void TaskP_addToRegistry(TaskP_Struct *task)
{
    uint32_t i;
    BaseType_t schedularState;

    schedularState = xTaskGetSchedulerState();
    if(schedularState != taskSCHEDULER_NOT_STARTED)
    {
        vTaskSuspendAll();
    }

    for(i=0; i<TaskP_REGISTRY_MAX_ENTRIES; i++)
    {
        if(gTaskP_ctrl.taskRegistry[i]==NULL)
        {
            gTaskP_ctrl.taskRegistry[i] = task;
            break;
        }
    }

    if(schedularState != taskSCHEDULER_NOT_STARTED)
    {
        xTaskResumeAll();
    }
}

static void TaskP_removeFromRegistry(TaskP_Struct *task)
{
    uint32_t i;
    BaseType_t schedularState;

    schedularState = xTaskGetSchedulerState();
    if(schedularState != taskSCHEDULER_NOT_STARTED)
    {
        vTaskSuspendAll();
    }

    for(i=0; i<TaskP_REGISTRY_MAX_ENTRIES; i++)
    {
        if(gTaskP_ctrl.taskRegistry[i]==task)
        {
            gTaskP_ctrl.taskRegistry[i] = NULL;
            break;
        }
    }

    if(schedularState != taskSCHEDULER_NOT_STARTED)
    {
        xTaskResumeAll();
    }
}

static uint32_t TaskP_calcCounterDiff(uint32_t cur, uint32_t last)
{
    uint32_t delta;

    if(cur >= last)
    {
        delta = cur - last;
    }
    else
    {
        delta = ( 0xFFFFFFFFU - last ) + cur;
    }
    return delta;
}

static uint32_t TaskP_calcCpuLoad(uint64_t taskTime, uint64_t totalTime)
{
    uint32_t cpuLoad;

    cpuLoad = (uint32_t)((taskTime * TaskP_LOAD_CPU_LOAD_SCALE) / totalTime);
    if( cpuLoad > TaskP_LOAD_CPU_LOAD_SCALE)
    {
        cpuLoad = TaskP_LOAD_CPU_LOAD_SCALE;
    }
    return cpuLoad;
}


void TaskP_Params_init(TaskP_Params *params)
{
    params->name = "Task (DPL)";
    params->stackSize = 0;
    params->stack = NULL;
    params->priority = (TaskP_PRIORITY_HIGHEST - TaskP_PRIORITY_LOWEST) / 2U;
    params->args = NULL;
    params->taskMain = 0;
#ifdef SMP_FREERTOS
    params->coreAffinity = (uintptr_t)(~0);
#endif
}

int32_t TaskP_construct(TaskP_Object *obj, TaskP_Params *params)
{
    int32_t status = SystemP_SUCCESS;
    TaskP_Struct *taskObj = (TaskP_Struct *)obj;

    DebugP_assert(sizeof(TaskP_Struct) <= sizeof(TaskP_Object));
    DebugP_assert(params != NULL);
    DebugP_assert(taskObj != NULL);
    DebugP_assert(params->stackSize >= TaskP_STACK_SIZE_MIN);
    DebugP_assert( (params->stackSize & (sizeof(configSTACK_DEPTH_TYPE) - 1U)) == 0U);
    DebugP_assert(params->stack != NULL );
    DebugP_assert( ((uintptr_t)params->stack & (sizeof(configSTACK_DEPTH_TYPE) - 1U)) == 0U);
    DebugP_assert(params->taskMain != NULL );

    /* if prority is out of range, adjust to bring it in range */
    if(params->priority > TaskP_PRIORITY_HIGHEST)
    {
        params->priority = TaskP_PRIORITY_HIGHEST;
    }
    if(params->priority <= TaskP_PRIORITY_LOWEST)
    {
        params->priority = TaskP_PRIORITY_LOWEST;
    }

    taskObj->lastRunTime = 0;
    taskObj->accRunTime = 0;

    TaskP_addToRegistry(taskObj);
#ifdef SMP_FREERTOS

    taskObj->taskHndl = xTaskCreateStaticAffinitySet( params->taskMain, /* Pointer to the function that implements the task. */
                                  params->name,              /* Text name for the task.  This is to facilitate debugging only. */
                                  params->stackSize/(sizeof(configSTACK_DEPTH_TYPE)),  /* Stack depth in units of StackType_t typically uint32_t on 32b CPUs */
                                  params->args,       /* task specific args */
                                  params->priority,   /* task priority, 0 is lowest priority, configMAX_PRIORITIES-1 is highest */
                                  (StackType_t*)params->stack,      /* pointer to stack base */
                                  &taskObj->taskObj,    /* pointer to statically allocated task object memory */
                                  params->coreAffinity); /* A bitwise value that indicates the cores on which the task can run*/
#else
    taskObj->taskHndl = xTaskCreateStatic( params->taskMain, /* Pointer to the function that implements the task. */
                                  params->name,              /* Text name for the task.  This is to facilitate debugging only. */
                                  params->stackSize/(sizeof(configSTACK_DEPTH_TYPE)),  /* Stack depth in units of StackType_t typically uint32_t on 32b CPUs */
                                  params->args,       /* task specific args */
                                  params->priority,   /* task priority, 0 is lowest priority, configMAX_PRIORITIES-1 is highest */
                                  (StackType_t*)params->stack,      /* pointer to stack base */
                                  &taskObj->taskObj); /* pointer to statically allocated task object memory */
#endif
    if(taskObj->taskHndl == NULL)
    {
        status = SystemP_FAILURE;
    }



    return status;
}

void TaskP_destruct(TaskP_Object *obj)
{
    TaskP_Struct *taskObj = (TaskP_Struct *)obj;

    if(taskObj && taskObj->taskHndl)
    {
        vTaskDelete(taskObj->taskHndl);
        taskObj->taskHndl = NULL;
    }
    if(taskObj != NULL)
    {
        TaskP_removeFromRegistry(taskObj);
    }
}

void* TaskP_getHndl(TaskP_Object *obj)
{
    TaskP_Struct *taskObj = (TaskP_Struct *)obj;

    return (void*)taskObj->taskHndl;
}

void TaskP_yield()
{
    taskYIELD();
}

void TaskP_exit()
{
    vTaskDelete(NULL);
}

void TaskP_loadGet(TaskP_Object *obj, TaskP_Load *taskLoad)
{
    TaskP_Struct *taskObj = (TaskP_Struct *)obj;
    TaskStatus_t taskStatus;

    vTaskSuspendAll();

    vTaskGetInfo(taskObj->taskHndl, &taskStatus, pdFALSE, eReady);

    taskLoad->runTime = taskObj->accRunTime;
    taskLoad->totalTime  = gTaskP_ctrl.accTotalTime;
    taskLoad->cpuLoad = TaskP_calcCpuLoad(taskObj->accRunTime, gTaskP_ctrl.accTotalTime);
    taskLoad->name = taskStatus.pcTaskName;

    xTaskResumeAll();
}

uint32_t TaskP_loadGetTotalCpuLoad()
{
    uint32_t cpuLoad;

    /* This is need to get upto date IDLE task statistics, since the next update window could be some time away */
    TaskP_loadUpdateAll();

    vTaskSuspendAll();

    #ifdef SMP_FREERTOS
        #if defined(SMP_QUADCORE_FREERTOS)
        cpuLoad = TaskP_LOAD_CPU_LOAD_SCALE - TaskP_calcCpuLoad(gTaskP_ctrl.idleTsk1AccRunTime + gTaskP_ctrl.idleTsk2AccRunTime + gTaskP_ctrl.idleTsk3AccRunTime + gTaskP_ctrl.idleTsk4AccRunTime,
                                                                gTaskP_ctrl.accTotalTime);
        #else
        cpuLoad = TaskP_LOAD_CPU_LOAD_SCALE - TaskP_calcCpuLoad(gTaskP_ctrl.idleTsk1AccRunTime + gTaskP_ctrl.idleTsk2AccRunTime, gTaskP_ctrl.accTotalTime);
        #endif
    #else
    cpuLoad = TaskP_LOAD_CPU_LOAD_SCALE - TaskP_calcCpuLoad(gTaskP_ctrl.idleTskAccRunTime, gTaskP_ctrl.accTotalTime);
    #endif

    xTaskResumeAll();

    return cpuLoad;
}

void TaskP_loadResetAll()
{
    TaskP_Struct *taskObj;
    uint32_t i;

    vTaskSuspendAll();

    for(i=0; i<TaskP_REGISTRY_MAX_ENTRIES; i++)
    {
        if(gTaskP_ctrl.taskRegistry[i]!=NULL)
        {
            taskObj = gTaskP_ctrl.taskRegistry[i];

            taskObj->accRunTime = 0;
        }
    }

#ifdef SMP_FREERTOS
    gTaskP_ctrl.idleTsk1AccRunTime = 0;
    gTaskP_ctrl.idleTsk2AccRunTime = 0;
    #if defined(SMP_QUADCORE_FREERTOS)
    gTaskP_ctrl.idleTsk3AccRunTime = 0;
    gTaskP_ctrl.idleTsk4AccRunTime = 0;
    #endif
    gTaskP_ctrl.accTotalTime = 0;
#else
    gTaskP_ctrl.idleTskAccRunTime = 0;
    gTaskP_ctrl.accTotalTime = 0;
#endif

    xTaskResumeAll();
}

void TaskP_loadUpdateAll()
{
    TaskP_Struct *taskObj;
    TaskStatus_t taskStatus;
    uint32_t i, delta, curTotalTime;
#ifdef SMP_FREERTOS
    TaskHandle_t idleTskHndl[configNUMBER_OF_CORES];
#else
    TaskHandle_t idleTskHndl;
#endif

    vTaskSuspendAll();

    for(i=0; i<TaskP_REGISTRY_MAX_ENTRIES; i++)
    {
        if(gTaskP_ctrl.taskRegistry[i]!=NULL)
        {
            taskObj = gTaskP_ctrl.taskRegistry[i];

            vTaskGetInfo(taskObj->taskHndl, &taskStatus, pdFALSE, eReady);

            delta = TaskP_calcCounterDiff(taskStatus.ulRunTimeCounter, taskObj->lastRunTime);

            taskObj->accRunTime += delta;
            taskObj->lastRunTime = taskStatus.ulRunTimeCounter;
        }
    }


#ifdef SMP_FREERTOS
    for(uint8_t i = 0; i < configNUMBER_OF_CORES; i++){
        idleTskHndl[i] = xTaskGetIdleTaskHandleForCore(i);
    }

    if(idleTskHndl[0] != NULL)
    {
        vTaskGetInfo(idleTskHndl[0], &taskStatus, pdFALSE, eReady);

        delta = TaskP_calcCounterDiff(taskStatus.ulRunTimeCounter, gTaskP_ctrl.idleTsk1LastRunTime);

        gTaskP_ctrl.idleTsk1AccRunTime += delta;
        gTaskP_ctrl.idleTsk1LastRunTime = taskStatus.ulRunTimeCounter;

        vTaskGetInfo(idleTskHndl[1], &taskStatus, pdFALSE, eReady);

        delta += TaskP_calcCounterDiff(taskStatus.ulRunTimeCounter, gTaskP_ctrl.idleTsk2LastRunTime);

        gTaskP_ctrl.idleTsk2AccRunTime += delta;
        gTaskP_ctrl.idleTsk2LastRunTime = taskStatus.ulRunTimeCounter;
        #if defined(SMP_QUADCORE_FREERTOS)
        vTaskGetInfo(idleTskHndl[2], &taskStatus, pdFALSE, eReady);

        delta += TaskP_calcCounterDiff(taskStatus.ulRunTimeCounter, gTaskP_ctrl.idleTsk3LastRunTime);

        gTaskP_ctrl.idleTsk3AccRunTime += delta;
        gTaskP_ctrl.idleTsk3LastRunTime = taskStatus.ulRunTimeCounter;

        vTaskGetInfo(idleTskHndl[3], &taskStatus, pdFALSE, eReady);

        delta += TaskP_calcCounterDiff(taskStatus.ulRunTimeCounter, gTaskP_ctrl.idleTsk4LastRunTime);

        gTaskP_ctrl.idleTsk4AccRunTime += delta;
        gTaskP_ctrl.idleTsk4LastRunTime = taskStatus.ulRunTimeCounter;
        #endif
    }
#else
    idleTskHndl = xTaskGetIdleTaskHandle();
    if(idleTskHndl != NULL)
    {
        vTaskGetInfo(idleTskHndl, &taskStatus, pdFALSE, eReady);

        delta = TaskP_calcCounterDiff(taskStatus.ulRunTimeCounter, gTaskP_ctrl.idleTskLastRunTime);

        gTaskP_ctrl.idleTskAccRunTime += delta;
        gTaskP_ctrl.idleTskLastRunTime = taskStatus.ulRunTimeCounter;
    }
#endif


    curTotalTime = portGET_RUN_TIME_COUNTER_VALUE();

    delta = TaskP_calcCounterDiff(curTotalTime, gTaskP_ctrl.lastTotalTime);

    gTaskP_ctrl.accTotalTime += delta;
    gTaskP_ctrl.lastTotalTime = curTotalTime;

    xTaskResumeAll();
}

void vApplicationLoadHook()
{
    static uint64_t lastUpdateTime = 0;
    uint64_t curUpdateTime = ClockP_getTimeUsec();

    if( (curUpdateTime > lastUpdateTime) && (curUpdateTime - lastUpdateTime) > TaskP_LOAD_UPDATE_WINDOW_MSEC*1000u )
    {
        TaskP_loadUpdateAll();
        lastUpdateTime = curUpdateTime;
    }
}

uint32_t TaskP_disable(void)
{
    vTaskSuspendAll();
    return (uint32_t)0;
}

void TaskP_restore(uint32_t key)
{
    xTaskResumeAll();
    return;
}

void TaskP_endScheduler()
{
    extern void vTaskEndScheduler( void );
    vTaskEndScheduler();
}

