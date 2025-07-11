/*
 * FreeRTOS Kernel V11.1.0
 * Copyright (C) 2020 Amazon.com, Inc. or its affiliates.  All Rights Reserved.
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy of
 * this software and associated documentation files (the "Software"), to deal in
 * the Software without restriction, including without limitation the rights to
 * use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of
 * the Software, and to permit persons to whom the Software is furnished to do so,
 * subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS
 * FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR
 * COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER
 * IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
 * CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 *
 * https://www.FreeRTOS.org
 * https://github.com/FreeRTOS
 *
 * 1 tab == 4 spaces!
 */
/*
 *  Copyright (C) 2018-2025 Texas Instruments Incorporated
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

/* ========================================================================== */
/*                             Include Files                                  */
/* ========================================================================== */

#include <stdint.h>
#include <FreeRTOS.h>
#include <task.h>
#include <kernel/dpl/HwiP.h>
#include <kernel/dpl/ClockP.h>
#include <kernel/dpl/DebugP.h>

/* ========================================================================== */
/*                           Macros & Typedefs                                */
/* ========================================================================== */

/* Let the user override the pre-loading of the initial LR with the address of
 * prvTaskExitError() in case is messes up unwinding of the stack in the
 * debugger. */
#ifdef configTASK_RETURN_ADDRESS
    #define portTASK_RETURN_ADDRESS    configTASK_RETURN_ADDRESS
#else
    #define portTASK_RETURN_ADDRESS    prvTaskExitError
#endif

/* Constants required to setup the initial task context. */
#define portINITIAL_SPSR                 ( ( StackType_t ) 0x1f ) /* System mode, ARM mode, IRQ enabled FIQ enabled. */
#define portTHUMB_MODE_BIT               ( ( StackType_t ) 0x20 )
#define portINTERRUPT_ENABLE_BIT         ( 0x80UL )
#define portTHUMB_MODE_ADDRESS           ( 0x01UL )
#define portINITIAL_FPSR                 ( ( StackType_t ) 0x0 )
#define portNUM_FPU_REGS                 ( 32U )

/* A critical section is exited when the critical section nesting count reaches
 * this value. */
#define portNO_CRITICAL_NESTING          ( ( uint32_t ) 0 )

/* Tasks are not created with a floating point context, but can be given a
 * floating point context after they have been created.  A variable is stored as
 * part of the tasks context that holds portNO_FLOATING_POINT_CONTEXT if the task
 * does not have an FPU context, or any other value if the task does have an FPU
 * context. */
#define portNO_FLOATING_POINT_CONTEXT    ( ( StackType_t ) 0 )

/* Tasks are created with a floating point context. A variable is stored as
 * part of the tasks context that holds portFLOATING_POINT_CONTEXT if the task
 * have an FPU context. */
#define portFLOATING_POINT_CONTEXT       ( ( StackType_t ) 1 )

/* Refer arm_acle spec for the defines */
#define SY       (15U)       /*   Full system Any-Any                    */
#define ST       (14U)       /*   Full system Store-Store                */
#define LD       (13U)       /*   Full system Load-Load, Load-Store      */
#define ISH      (11U)       /*   Inner shareable Any-Any                */
#define ISHST    (10U)       /*   Inner shareable Store-Store            */
#define ISHLD    (9U)        /*   Inner shareable Load-Load, Load-Store  */
#define NSH      (7U)        /*   Non-shareable Any-Any                  */
#define NSHST    (6U)        /*   Non-shareable Store-Store              */
#define NSHLD    (5U)        /*   Non-shareable Load-Load, Load-Store    */
#define OSH      (3U)        /*   Outer shareable Any-Any                */
#define OSHST    (2U)        /*   Outer shareable Store-Store            */
#define OSHLD    (1U)        /*   Outer shareable Load-Load, Load-Store  */

/* ========================================================================== */
/*                         Structure Declarations                             */
/* ========================================================================== */

/* None */

/* ========================================================================== */
/*                          Function Declarations                             */
/* ========================================================================== */

/*
 * Starts the first task executing.  This function is necessarily written in
 * assembly code so is implemented in portASM.s.
 */
extern void vPortRestoreTaskContext( void );

/*
 * Functions to get the Data and Instruction Fault Status and Address into
 * DFAR, DFSR, IFAR, IFSR, and CPSR.
 */
void ulGetDataFaultStatusRegister( void );
void ulGetDataFaultAddressRegister( void );
void ulGetInstructionFaultStatusRegister( void );
void ulGetInstructionFaultAddressRegister( void );
void ulGetCPSR( void );

/* Function prototype to dump the exception state */
void vPortDumpExceptionState( void );

/* ========================================================================== */
/*                            Global Variables                                */
/* ========================================================================== */

/* A variable is used to keep track of the critical section nesting.  This
 * variable has to be stored as part of the task context and must be initialised to
 * a non zero value to ensure interrupts don't inadvertently become unmasked before
 * the scheduler starts.  As it is stored as part of the task context it will
 * automatically be set to 0 when the first task is started. */
volatile uint32_t ulCriticalNesting = 9999UL;

/* Saved as part of the task context.  If ulPortTaskHasFPUContext is non-zero then
 * a floating point context must be saved and restored for the task. */
uint32_t ulPortTaskHasFPUContext = pdFALSE;

/* Set to 1 to pend a context switch from an ISR. */
uint32_t ulPortYieldRequired = pdFALSE;

/* Counts the interrupt nesting depth.  A context switch is only performed if
 * if the nesting depth is 0. */
uint32_t ulPortInterruptNesting = 0UL;

/* Data Fault Address and Staus Registers content */
uint32_t DFAR, DFSR;

/* Instruction Fault Address and Staus Registers content */
uint32_t IFAR, IFSR;

/* Variable to store the value of the Current Program Status Register (CPSR) */
volatile uint32_t CPSR;

/* set to true when schedular gets enabled in xPortStartScheduler */
uint32_t ulPortSchedularRunning = pdFALSE;

/* Saves Faulty Stack Pointer at Data Abort. */
uint32_t ulFaultySP;

/* Saves Faulty Link Register at Data Abort. */
uint32_t ulFaultyLR;

/* ========================================================================== */
/*                          Function Definitions                              */
/* ========================================================================== */

static void prvTaskExitError( void )
{
    /* A function that implements a task must not exit or attempt to return to
     * its caller as there is nothing to return to.  If a task wants to exit it
     * should instead call vTaskDelete( NULL ).
     *
     * Force an assert() to be triggered if configASSERT() is
     * defined, then stop here so application writers can catch the error. */
    DebugP_assertNoLog(0);
}

StackType_t * pxPortInitialiseStack( StackType_t * pxTopOfStack,
                                     TaskFunction_t pxCode,
                                     void * pvParameters )
{
    /* Setup the initial stack of the task.  The stack is set exactly as
     * expected by the portRESTORE_CONTEXT() macro.
     *
     * The fist real value on the stack is the status register, which is set for
     * system mode, with interrupts enabled.  A few NULLs are added first to ensure
     * GDB does not try decoding a non-existent return address. */
    *pxTopOfStack = ( StackType_t ) NULL;
    pxTopOfStack--;
    *pxTopOfStack = ( StackType_t ) NULL;
    pxTopOfStack--;
    *pxTopOfStack = ( StackType_t ) NULL;
    pxTopOfStack--;
    *pxTopOfStack = ( StackType_t ) portINITIAL_SPSR;

    if( ( ( uint32_t ) pxCode & portTHUMB_MODE_ADDRESS ) != 0x00UL )
    {
        /* The task will start in THUMB mode. */
        *pxTopOfStack |= portTHUMB_MODE_BIT;
    }

    pxTopOfStack--;

    /* Next the return address, which in this case is the start of the task. */
    *pxTopOfStack = ( StackType_t ) pxCode;
    pxTopOfStack--;

    /* Next all the registers other than the stack pointer. */
    *pxTopOfStack = ( StackType_t ) portTASK_RETURN_ADDRESS; /* R14 */
    pxTopOfStack--;
    *pxTopOfStack = ( StackType_t ) 0x12121212;              /* R12 */
    pxTopOfStack--;
    *pxTopOfStack = ( StackType_t ) 0x11111111;              /* R11 */
    pxTopOfStack--;
    *pxTopOfStack = ( StackType_t ) 0x10101010;              /* R10 */
    pxTopOfStack--;
    *pxTopOfStack = ( StackType_t ) 0x09090909;              /* R9 */
    pxTopOfStack--;
    *pxTopOfStack = ( StackType_t ) 0x08080808;              /* R8 */
    pxTopOfStack--;
    *pxTopOfStack = ( StackType_t ) 0x07070707;              /* R7 */
    pxTopOfStack--;
    *pxTopOfStack = ( StackType_t ) 0x06060606;              /* R6 */
    pxTopOfStack--;
    *pxTopOfStack = ( StackType_t ) 0x05050505;              /* R5 */
    pxTopOfStack--;
    *pxTopOfStack = ( StackType_t ) 0x04040404;              /* R4 */
    pxTopOfStack--;
    *pxTopOfStack = ( StackType_t ) 0x03030303;              /* R3 */
    pxTopOfStack--;
    *pxTopOfStack = ( StackType_t ) 0x02020202;              /* R2 */
    pxTopOfStack--;
    *pxTopOfStack = ( StackType_t ) 0x01010101;              /* R1 */
    pxTopOfStack--;
    *pxTopOfStack = ( StackType_t ) pvParameters;            /* R0 */
    pxTopOfStack--;

    /* The task will start with a critical nesting count of 0 as interrupts are
     * enabled. */
    *pxTopOfStack = portNO_CRITICAL_NESTING;
    pxTopOfStack--;

#if (configFLOATING_POINT_CONTEXT==0)
    /* The task will start without a floating point context.  A task that uses
     * the floating point hardware must call vPortTaskUsesFPU() before executing
     * any floating point instructions. */
    *pxTopOfStack = portNO_FLOATING_POINT_CONTEXT;
#else
    /* The task will start with a floating point context. */

    *pxTopOfStack = portINITIAL_FPSR;
    pxTopOfStack--;

    /* Next all the FPU bank registers S0 to S31 */
    uint32_t ulNumFpuReg = portNUM_FPU_REGS;
    while ( ulNumFpuReg -- )
    {
        *pxTopOfStack = ( StackType_t ) 0x00000000;     /* S0 to S31 */
        pxTopOfStack--;
    }

    *pxTopOfStack = portFLOATING_POINT_CONTEXT;
#endif

    return pxTopOfStack;
}

BaseType_t xPortStartScheduler(void)
{
    /* Interrupts are turned off in the CPU itself to ensure tick does
     * not execute	while the scheduler is being started.  Interrupts are
     * automatically turned back on in the CPU when the first task starts
     * executing.
     */
    portDISABLE_INTERRUPTS();

    /* Start the ISR handling of the timer that generates the tick ISR. */
    ulPortSchedularRunning = pdTRUE;

    /* Start the first task executing. */
    vPortRestoreTaskContext();

    /* Will only get here if vTaskStartScheduler() was called with the CPU in
     * a non-privileged mode or the binary point register was not set to its lowest
     * possible value.  prvTaskExitError() is referenced to prevent a compiler
     * warning about it being defined but not referenced in the case that the user
     * defines their own exit address. */
    ( void ) prvTaskExitError;

    return pdTRUE;
}

void vPortYeildFromISR( uint32_t xSwitchRequired )
{
    if( xSwitchRequired != pdFALSE )
    {
        ulPortYieldRequired = pdTRUE;
    }
}

void vPortTimerTickHandler( void )
{
    if( ulPortSchedularRunning == pdTRUE )
    {
        /* Increment the RTOS tick. */
        if( xTaskIncrementTick() != pdFALSE )
        {
            ulPortYieldRequired = pdTRUE;
        }
    }
}

void vPortTaskUsesFPU( void )
{
#if (configFLOATING_POINT_CONTEXT==0)
    uint32_t ulInitialFPSCR = 0;

    /* A task is registering the fact that it needs an FPU context.  Set the
     * FPU flag (which is saved as part of the task context). */
    ulPortTaskHasFPUContext = pdTRUE;

    /* Initialise the floating point status register. */
    __asm__ volatile ( "FMXR 	FPSCR, %0" ::"r" ( ulInitialFPSCR ) : "memory" );
#endif
}

void vPortEnterCritical( void )
{
    /* Mask interrupts up to the max syscall interrupt priority. */
    __asm__ __volatile__ ("dsb  sy"   "\n\t": : : "memory");
    __asm__ __volatile__ ("isb  sy" "\n\t": : : "memory");
    __asm__ volatile ( "CPSID	i" ::: "cc" );
    __asm__ __volatile__ ("dsb sy"   "\n\t": : : "memory");
    __asm__ __volatile__ ("isb sy" "\n\t": : : "memory");

    /* Now interrupts are disabled ulCriticalNesting can be accessed
     * directly.  Increment ulCriticalNesting to keep a count of how many times
     * portENTER_CRITICAL() has been called. */
    ulCriticalNesting++;
    __asm__ __volatile__ ("dsb sy"   "\n\t": : : "memory");
    __asm__ __volatile__ ("isb sy" "\n\t": : : "memory");

    #if (configOPTIMIZE_FOR_LATENCY==0)
    /* This API should NOT be called from within ISR context. Below logic checks for this.
     * Commenting this reduces task switch latency a bit, however if this API is by mistale
     * called in a ISR by user, it could have unexpected side effects.
     */
    /* This is not the interrupt safe version of the enter critical function so
     * assert() if it is being called from an interrupt context.  Only API
     * functions that end in "FromISR" can be used in an interrupt.  Only assert if
     * the critical nesting count is 1 to protect against recursive calls if the
     * assert function also uses a critical section. */
    if( ulCriticalNesting == 1 )
    {
        DebugP_assertNoLog( ulPortInterruptNesting == 0);
    }
    #endif
}

void vPortExitCritical( void )
{
    /* if( ulCriticalNesting > portNO_CRITICAL_NESTING ) */
    {
        /* Decrement the nesting count as the critical section is being
         * exited. */
        __asm__ __volatile__ (" dsb sy"   "\n\t": : : "memory");
        __asm__ __volatile__ (" isb sy"   "\n\t": : : "memory");
        ulCriticalNesting--;
        __asm__ __volatile__ (" dsb sy"   "\n\t": : : "memory");
        __asm__ __volatile__ (" isb sy"   "\n\t": : : "memory");

        /* If the nesting level has reached zero then all interrupt
         * priorities must be re-enabled. */
        if( ulCriticalNesting == portNO_CRITICAL_NESTING )
        {
            /* Critical nesting has reached zero so all interrupt priorities
             * should be unmasked. */
            __asm__ volatile ( "CPSIE	i" ::: "cc" );
            __asm__ __volatile__ (" dsb sy"   "\n\t": : : "memory");
            __asm__ __volatile__ (" isb sy"   "\n\t": : : "memory");
        }
    }
}

/* initialize high resolution timer for CPU and task load calculation */
void vPortConfigTimerForRunTimeStats( void )
{
    /* we assume clock is initialized before the schedular is started */
}

/* return current counter value of high speed counter in units of usecs */
uint32_t uiPortGetRunTimeCounterValue( void )
{
    uint64_t timeInUsecs = ClockP_getTimeUsec();

    /* note, there is no overflow protection for this 32b value in FreeRTOS
     *
     * This value will overflow in
     * ((0xFFFFFFFF)/(1000000*60)) minutes ~ 71 minutes
     *
     * We call vApplicationLoadHook() in idle loop to accumlate the task load into a 64b value.
     * The implementation of vApplicationLoadHook() is in source\kernel\freertos\dpl\common\TaskP_freertos.c
     */
    return (uint32_t)(timeInUsecs);
}

/* This is used to make sure we are using the FreeRTOS API from within a valid interrupt priority level
 * In our R%F port this means IRQ.
 * i.e FreeRTOS API should not be called from FIQ, however right now we dont enforce it by checking
 * if we are in FIQ when this API is called.
 */
void vPortValidateInterruptPriority( void )
{
}

/* This is called as part of vTaskEndScheduler(), in our port, there is nothing to do here.
 * interrupt are disabled by FreeRTOS before calling this.
 */
void vPortEndScheduler( void )
{
    /* nothing to do */
}

/* configCHECK_FOR_STACK_OVERFLOW is set to 1, so the application must provide an
 * implementation of vApplicationStackOverflowHook()
 */
void vApplicationStackOverflowHook( TaskHandle_t xTask,
                                    char * pcTaskName )
{
    DebugP_logError("[FreeRTOS] Stack overflow detected for task [%s]", pcTaskName);
    DebugP_assertNoLog(0);
}

static StaticTask_t xIdleTaskTCB;
static StackType_t uxIdleTaskStack[ configMINIMAL_STACK_SIZE ];
/* configSUPPORT_STATIC_ALLOCATION is set to 1, so the application must provide an
 * implementation of vApplicationGetIdleTaskMemory() to provide the memory that is
 * used by the Idle task.
 */
void vApplicationGetIdleTaskMemory( StaticTask_t ** ppxIdleTaskTCBBuffer,
                                    StackType_t ** ppxIdleTaskStackBuffer,
                                    configSTACK_DEPTH_TYPE * pulIdleTaskStackSize )
{
    /* Pass out a pointer to the StaticTask_t structure in which the Idle task’s
     * state will be stored.
     */
    *ppxIdleTaskTCBBuffer = &xIdleTaskTCB;

    /* Pass out the array that will be used as the Idle task’s stack. */
    *ppxIdleTaskStackBuffer = uxIdleTaskStack;

    /* Pass out the size of the array pointed to by *ppxIdleTaskStackBuffer.
     * Note that, as the array is necessarily of type StackType_t,
     * configMINIMAL_STACK_SIZE is specified in words, not bytes.
     */
    *pulIdleTaskStackSize = configMINIMAL_STACK_SIZE;
}

static StaticTask_t xTimerTaskTCB;
static StackType_t uxTimerTaskStack[ configTIMER_TASK_STACK_DEPTH ];
/* configSUPPORT_STATIC_ALLOCATION and configUSE_TIMERS are both set to 1, so the
 * application must provide an implementation of vApplicationGetTimerTaskMemory()
 * to provide the memory that is used by the Timer service task.
 */
void vApplicationGetTimerTaskMemory( StaticTask_t ** ppxTimerTaskTCBBuffer,
                                     StackType_t ** ppxTimerTaskStackBuffer,
                                     configSTACK_DEPTH_TYPE * pulTimerTaskStackSize )
{
    /* Pass out a pointer to the StaticTask_t structure in which the Timer
     * task’s state will be stored.
     */
    *ppxTimerTaskTCBBuffer = &xTimerTaskTCB;

    /* Pass out the array that will be used as the Timer task’s stack. */
    *ppxTimerTaskStackBuffer = uxTimerTaskStack;

    /* Pass out the size of the array pointed to by *ppxTimerTaskStackBuffer.
     * Note that, as the array is necessarily of type StackType_t,
     * configTIMER_TASK_STACK_DEPTH is specified in words, not bytes.
     */
    *pulTimerTaskStackSize = configTIMER_TASK_STACK_DEPTH;
}

/* This function is called when configUSE_IDLE_HOOK is 1 in FreeRTOSConfig.h */
__attribute__((weak)) void vApplicationIdleHook( void )
{
    void vApplicationLoadHook();

    vApplicationLoadHook();

    __asm__ __volatile__ ("wfi"   "\n\t": : : "memory");
}

/*
 * Retrieve the data fault status. The DFSR holds status information
 * about the last data fault (synchronous or asynchronous) or a
 * watchpoint hit that was not taken to Hyp mode.
 */
void ulGetDataFaultStatusRegister( void )
{
    __asm__ __volatile__ ("MRC     p15, #0, r0, c5, c0, #0\n");
    __asm__ __volatile__ ( "MOV %0, r0" : "=r" (DFSR) );
}

/*
 * Retrieve the data fault address. The DFAR holds the faulting address
 * that caused the last data fault (synchronous only) or a watchpoint hit
 * that was not taken to Hyp mode.
 */
void ulGetDataFaultAddressRegister( void )
{
    __asm__ __volatile__ ("MRC     p15, #0, r0, c6, c0, #0\n");
    __asm__ __volatile__ ( "MOV %0, r0" : "=r" (DFAR) );
}

/* Fetch the instruction fault status */
void ulGetInstructionFaultStatusRegister( void )
{
    __asm__ __volatile__ ("MRC     p15, #0, r0, c5, c0, #1\n");
    __asm__ __volatile__ ( "MOV %0, r0" : "=r" (IFSR) );
}

/* Fetch the instruction fault address */
void ulGetInstructionFaultAddressRegister( void )
{
    __asm__ __volatile__ ("MRC     p15, #0, r0, c6, c0, #2\n");
    __asm__ __volatile__ ( "MOV %0, r0" : "=r" (IFAR) );
}

/* Fetch the Current Processor Status Register (CPSR) */
void ulGetCPSR( void )
{
    __asm__ __volatile__ ( "MRS %0, CPSR" : "=r" (CPSR) );
}

void vPortDumpExceptionState( void )
{
    /* Load the faulting registers into DFAR, DFSR, IFAR, IFSR, and CPSR. */
    ulGetDataFaultStatusRegister();
    ulGetDataFaultAddressRegister();
    ulGetInstructionFaultStatusRegister();
    ulGetInstructionFaultAddressRegister();
    ulGetCPSR();

    /* Register Dump of the fault. */
    DebugP_logError("[FATAL]: Core has Aborted!!!\nDFAR = 0x%x DFSR = 0x%x\n", (uintptr_t)DFAR, (uintptr_t)DFSR);
    DebugP_logError("IFAR = 0x%x IFSR = 0x%x\n", (uintptr_t)IFAR, (uintptr_t)IFSR);
    DebugP_logError("CPSR = 0x%x SP = 0x%x\n", (uintptr_t)CPSR, (uintptr_t)ulFaultySP);
    DebugP_logError("LR = 0x%x\n", (uintptr_t)ulFaultyLR);
}
