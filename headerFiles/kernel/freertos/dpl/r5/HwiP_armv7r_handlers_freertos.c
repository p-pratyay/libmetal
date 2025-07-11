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

#include <kernel/dpl/HwiP.h>
#include <kernel/nortos/dpl/r5/HwiP_armv7r_vim.h>
#include <drivers/hw_include/csl_types.h>
#include <stdbool.h>

static volatile uint32_t gdummy;

void __attribute__((section(".text.hwi"))) HwiP_irq_handler_c(void);
void __attribute__((interrupt("UNDEF"), section(".text.hwi"))) HwiP_reserved_handler(void);
void __attribute__((interrupt("UNDEF"), section(".text.hwi"))) HwiP_undefined_handler(void);
void __attribute__((interrupt("ABORT"), section(".text.hwi"))) HwiP_prefetch_abort_handler(void);
void __attribute__((interrupt("ABORT"), section(".text.hwi"))) HwiP_data_abort_handler(void);
extern void vPortDumpExceptionState( void );

/* IRQ handler starts execution in HwiP_irq_handler, defined in portASM.S
 * After some initial assembly logic it then branches to this function.
 * After exiting this function it does some more assembly including
 * doing task switch if requested.
 */
void __attribute__((section(".text.hwi"))) HwiP_irq_handler_c(void)
{
    printf("IRQ Handler triggered\n");
    int32_t status;
    uint32_t intNum;

    #ifndef HWIP_VIM_VIC_ENABLE
    volatile uint32_t gdummy;

    /* Read to force prioritization logic to take effect */
    gdummy = HwiP_getIRQVecAddr();
    #endif

    status = HwiP_getIRQ(&intNum);
    if(status == SystemP_SUCCESS)
    {
        bool isPulse = (bool)(HwiP_isPulse(intNum));
        HwiP_FxnCallback isr;
        void *args;

        if(isPulse == true)
        {
            HwiP_clearInt(intNum);
        }

        isr = gHwiCtrl.isr[intNum];
        args = gHwiCtrl.isrArgs[intNum];

        #ifdef HWIP_NESTED_INTERRUPTS_IRQ_ENABLE
        /* allow nesting of interrupts */
        HwiP_enable();
        #endif

        if(isr != NULL)
        {
            isr(args);
        }

        /* disallow nesting of interrupts */
        HwiP_disable();

        if(!isPulse)
        {
            HwiP_clearInt(intNum);
        }
        HwiP_ackIRQ(intNum);
    }
    else
    {
        /* spurious interrupt */
        gHwiCtrl.spuriousIRQCount++;
        HwiP_ackIRQ(0);
    }
}

void __attribute__((interrupt("FIQ"), section(".text.hwi"))) HwiP_fiq_handler(void)
{
    printf("FIQ Handler triggered\n");
    int32_t status;
    uint32_t intNum;

    /* Read to force prioritization logic to take effect */
    gdummy = HwiP_getFIQVecAddr();

    status = HwiP_getFIQ(&intNum);
    if(status == SystemP_SUCCESS)
    {
        bool isPulse = (bool)(HwiP_isPulse(intNum));
        HwiP_FxnCallback isr;
        void *args;

        if(isPulse == true)
        {
            HwiP_clearInt(intNum);
        }

        isr = gHwiCtrl.isr[intNum];
        args = gHwiCtrl.isrArgs[intNum];

        #if 0   /* FIQ interrupt nesting not supported */
        /* allow nesting of interrupts */
        HwiP_enableFIQ();
        #endif

        if(isr != NULL)
        {
            isr(args);
        }

        /* disallow nesting of interrupts */
        HwiP_disableFIQ();

        if(!isPulse)
        {
            HwiP_clearInt(intNum);
        }
        HwiP_ackFIQ(intNum);
    }
    else
    {
        /* spurious interrupt */
        gHwiCtrl.spuriousFIQCount++;
        HwiP_ackFIQ(0);
    }
}

void __attribute__((interrupt("UNDEF"), section(".text.hwi"))) HwiP_reserved_handler(void)
{
    printf("Reserved Handler triggered\n");
    volatile uint32_t loop = 1;
    while(loop)
    {
      /* Do Nothing */
    }
}

void __attribute__((interrupt("UNDEF"), section(".text.hwi"))) HwiP_undefined_handler(void)
{
    unsigned int pc,  lr, sp;
    __asm__ volatile ("mov %0, pc" : "=r"(pc));
    __asm__ volatile ("mov %0, lr" : "=r"(lr));
    __asm__ volatile ("mov %0, sp" : "=r"(sp));
    printf("Undefined Handler: PC=0x%08X, LR=0x%08X, SP=0x%08X\n", pc, lr, sp);
    // Optionally print stack contents
    // uint32_t *stack = (uint32_t *)sp;
    // for (int i = 0; i < 16; i++) {
    //     printf("stack[%d]=0x%08X\n", i, stack[i]);
    // }
    volatile uint32_t loop = 1;
    while(loop)
    {
      /* Do Nothing */
    }
}

void __attribute__((interrupt("ABORT"), section(".text.hwi"))) HwiP_prefetch_abort_handler(void)
{
     printf("Prefetch Abort Handler triggered\n");
    volatile uint32_t loop = 1;
    while(loop)
    {
      /* Do Nothing */
    }
}

void __attribute__((interrupt("ABORT"), section(".text.hwi"))) HwiP_data_abort_handler(void)
{
     printf("Data Abort Handler triggered\n");
    volatile uint32_t loop = 1;
    vPortDumpExceptionState();
    while(loop)
    {
      /* Do Nothing */
    }
}
