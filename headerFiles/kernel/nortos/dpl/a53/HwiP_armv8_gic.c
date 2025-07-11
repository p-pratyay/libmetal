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


#include "HwiP_armv8_gic.h"
#include "SpinlockP_armv8.h"

typedef struct HwiP_Struct_s {

    uint32_t intNum;

} HwiP_Struct;


/* ========================================================================== */
/*                            Global Variables                                */
/* ========================================================================== */
HwiP_Ctrl gHwiCtrl;


/* Function prototypes */
static void HwiP_intrPrioritySet(uint32_t intrNum, uint32_t priority,
                         uint8_t coreId);
static int32_t HwiP_setSgiPpiIntrPriority( CSL_gic500_gicrRegs *pGic500GicrRegs, uint32_t coreId, uint32_t intrNum, uint32_t value );
static int32_t HwiP_setSpiIntrPriority( CSL_gic500_gicdRegs *pGic500GicdRegs, uint32_t intrNum, uint32_t value );
static int32_t HwiP_setPendingSgiPpiIntr( CSL_gic500_gicrRegs *pGic500GicrRegs, uint32_t coreId, uint32_t intNum );
static int32_t HwiP_setPendingSpiIntr( CSL_gic500_gicdRegs *pGic500GicdRegs, uint32_t intNum );
static int32_t HwiP_setLevelSpiIntrType(uint16_t intrNum);
static int32_t HwiP_setEdgeSpiIntrType(uint16_t intrNum);


void HwiP_Params_init(HwiP_Params *params)
{
    params->intNum = 0;
    params->callback = NULL;
    params->args = NULL;
    params->eventId = 0;
    params->priority = HWIP_GIC_DEFAULT_PRIORITY;
    params->isFIQ = 0;
    params->isPulse = 0;
}


int32_t HwiP_construct(HwiP_Object *handle, HwiP_Params *params)
{
    HwiP_Struct *obj = (HwiP_Struct *)handle;
    uint8_t coreId;

    DebugP_assertNoLog( sizeof(HwiP_Struct) <= sizeof(HwiP_Object) );
    DebugP_assertNoLog( params->callback != NULL );
    DebugP_assertNoLog( params->intNum < HwiP_MAX_INTERRUPTS );

    coreId = (uint8_t)Armv8_getCoreId();

    gHwiCtrl.isr[params->intNum] = params->callback;
    gHwiCtrl.isrArgs[params->intNum] = params->args;

    /* Set interrupt priority */
    HwiP_intrPrioritySet(params->intNum, (uint32_t)params->priority, coreId);

    #if defined(AMP_FREERTOS_A53)
    /* Route the spi interrupt */
    if(params->intNum >= HWIP_GICD_SGI_PPI_INTR_ID_MAX)
    {
        HwIP_routeGICSharedPeripheralInterrupt(params->intNum, coreId);
    }
    #endif

    if (params->isPulse == 0U)
    {
        (void)HwiP_setLevelSpiIntrType((uint16_t)params->intNum);
    }
    else
    {
        (void)HwiP_setEdgeSpiIntrType((uint16_t)params->intNum);
    }

    obj->intNum = params->intNum;

    HwiP_enableInt(params->intNum);

    return SystemP_SUCCESS;
}


int32_t HwiP_setArgs(HwiP_Object *handle, void *args)
{
    HwiP_Struct *obj = (HwiP_Struct *)handle;

    DebugP_assertNoLog( obj->intNum < HwiP_MAX_INTERRUPTS );

    gHwiCtrl.isrArgs[obj->intNum] = args;

    return SystemP_SUCCESS;
}


void HwiP_destruct(HwiP_Object *handle)
{
    HwiP_Struct *obj = (HwiP_Struct *)handle;

    DebugP_assertNoLog( obj->intNum < HwiP_MAX_INTERRUPTS );

    HwiP_disableInt(obj->intNum);

    /* Assign default ISR */
    gHwiCtrl.isr[obj->intNum] = NULL;
    gHwiCtrl.isrArgs[obj->intNum] = NULL;

    return;
}


void HwiP_enableInt(uint32_t intrNum)
{
    uint32_t index, mask;
    CSL_gic500_gicdRegs *gicdRegs = (CSL_gic500_gicdRegs *)(HWIP_GIC_BASE_ADDR);
    CSL_gic500_gicrRegs *gicrRegs = (CSL_gic500_gicrRegs *) ( HWIP_GIC_BASE_ADDR + CSL_GIC500_GICR_CORE_CONTROL_CTLR(0U));

    uint8_t coreId = Armv8_getCoreId();

    if ((gicrRegs != NULL) && (intrNum < HWIP_GICD_SGI_PPI_INTR_ID_MAX))
    {
        mask = 1U << (intrNum);

        gicrRegs->CORE[coreId].SGI_PPI.ISENABLER0 = mask;
    }
    else if ((gicdRegs != NULL) && (intrNum < HWIP_GICD_SPI_INTR_ID_MAX ))
    {
        /* Transform Interrupt ID */
        intrNum -= HWIP_GICD_SGI_PPI_INTR_ID_MAX;

        index = (intrNum / 32U);
        mask = 1U << (intrNum & 0x1fU);

        gicdRegs->ISENABLER_SPI[index] = mask;
    }
    else
    {

    }

    return;
}


uint32_t HwiP_disableInt(uint32_t intrNum)
{
    uint32_t index, mask;
    uint32_t oldEnableState=0U;

    CSL_gic500_gicdRegs *gicdRegs = (CSL_gic500_gicdRegs *)(HWIP_GIC_BASE_ADDR);
    CSL_gic500_gicrRegs *gicrRegs = (CSL_gic500_gicrRegs *) ( HWIP_GIC_BASE_ADDR + CSL_GIC500_GICR_CORE_CONTROL_CTLR(0U));

    uint8_t coreId = Armv8_getCoreId();

    if ((gicrRegs != NULL) && (intrNum < HWIP_GICD_SGI_PPI_INTR_ID_MAX))
    {
        mask = 1U << (intrNum);

        oldEnableState = gicrRegs->CORE[coreId].SGI_PPI.ISENABLER0 & mask;
        gicrRegs->CORE[coreId].SGI_PPI.ICENABLER0 = mask;
    }
    else if ((gicdRegs != NULL) && (intrNum < HWIP_GICD_SPI_INTR_ID_MAX ))
    {
        /* Transform Interrupt ID */
        intrNum -= HWIP_GICD_SGI_PPI_INTR_ID_MAX;

        index = (intrNum / 32U);
        mask = 1U << (intrNum & 0x1fU);

        oldEnableState = gicdRegs->ISENABLER_SPI[index] & mask;
        gicdRegs->ICENABLER_SPI[index] = mask;
    }
    else
    {

    }

    if (oldEnableState != 0U)
    {
        oldEnableState = 1U;
    }

    return oldEnableState;
}


void HwiP_restoreInt(uint32_t intrNum, uint32_t oldIntrState)
{
    if(oldIntrState)
    {
        HwiP_enableInt(intrNum);
    }
    else
    {
        HwiP_disableInt(intrNum);
    }
}


void HwiP_clearInt(uint32_t intrNum)
{
    CSL_gic500_gicdRegs *gicdRegs = (CSL_gic500_gicdRegs *)(HWIP_GIC_BASE_ADDR);
    CSL_gic500_gicrRegs *gicrRegs = (CSL_gic500_gicrRegs *) ( HWIP_GIC_BASE_ADDR + CSL_GIC500_GICR_CORE_CONTROL_CTLR(0U));

    uint8_t coreId = Armv8_getCoreId();
    uint32_t index, mask;

    if ((gicrRegs != NULL) && (intrNum < HWIP_GICD_SGI_PPI_INTR_ID_MAX))
    {
        mask = 1U << (intrNum);

        gicrRegs->CORE[coreId].SGI_PPI.ICPENDR0 = mask;
    }
    else if ((gicdRegs != NULL) && (intrNum < HWIP_GICD_SPI_INTR_ID_MAX))
    {
        /* Transform Interrupt ID */
        intrNum -= HWIP_GICD_SGI_PPI_INTR_ID_MAX;

        index = (intrNum / 32U);
        mask = 1U << (intrNum & 0x1fU);

        gicdRegs->ICPENDR_SPI[index] = mask;
    }
    else
    {

    }

    return;
}


void HwiP_post(uint32_t intrNum)
{
    CSL_gic500_gicdRegs *gicdRegs = (CSL_gic500_gicdRegs *)(HWIP_GIC_BASE_ADDR);
    CSL_gic500_gicrRegs *gicrRegs = (CSL_gic500_gicrRegs *) ( HWIP_GIC_BASE_ADDR + CSL_GIC500_GICR_CORE_CONTROL_CTLR(0U));

    uint8_t coreId = Armv8_getCoreId();

    if (intrNum < HWIP_GICD_SGI_PPI_INTR_ID_MAX)
    {
        (void)HwiP_setPendingSgiPpiIntr(gicrRegs, coreId, intrNum);
    }
    else if (intrNum < HWIP_GICD_SPI_INTR_ID_MAX)
    {
        (void)HwiP_setPendingSpiIntr(gicdRegs, intrNum);
    }
    else
    {
        /* No action */
    }

    Armv8_dsbSy();

    return;

}

uint32_t HwiP_inISR(void)
{
#ifdef SMP_FREERTOS
    if(Armv8_getCoreId() == 0U)
    {
        return (gHwiInIsrFlagCore0 == 0U ? 0U : 1U);
    }
    else if(Armv8_getCoreId() == 1U)
    {
        return (gHwiInIsrFlagCore1 == 0U ? 0U : 1U);
    }

    #if defined(SMP_QUADCORE_FREERTOS)
    else if(Armv8_getCoreId() == 2U)
    {
        return (gHwiInIsrFlagCore2 == 0U ? 0U : 1U);
    }
    else if(Armv8_getCoreId() == 3U)
    {
        return (gHwiInIsrFlagCore3 == 0U ? 0U : 1U);
    }
    #endif

    else
    {
        return 0;
    }
#else
    return (gHwiInIsrFlag == 0U ? 0U : 1U);
#endif
}

void taskSwitchContextISR(void *arg)
{

    uint32_t coreId;

    coreId = (uint32_t)Armv8_getCoreId();

    extern uint64_t ullPortYieldRequired[];
    ullPortYieldRequired[coreId] = 1;

    return;
}

void HwiP_init()
{
    uint8_t  coreId;

    coreId = (uint8_t)Armv8_getCoreId();

    uint32_t i = 0, j = 0, intrActiveReg = 0;
    #if !defined(AMP_FREERTOS_A53)
    uint32_t routingMode = 0;
    uint8_t  aff0 = 0, aff1 = 0;
    #endif

    /* Initialize the GIC V3 */
    CSL_gic500_gicdRegs *gicdRegs = (CSL_gic500_gicdRegs *)(HWIP_GIC_BASE_ADDR);
    CSL_gic500_gicrRegs_core_control *gicrRegs;
    CSL_gic500_gicrRegs_core_sgi_ppi *gicsRegs;

    /* update the redistributor and sgi_ppi address */
    gicrRegs = (CSL_gic500_gicrRegs_core_control *) (HWIP_GIC_BASE_ADDR + CSL_GIC500_GICR_CORE_CONTROL_CTLR(coreId));
    gicsRegs = (CSL_gic500_gicrRegs_core_sgi_ppi *) ((uintptr_t)gicrRegs + (uintptr_t) 0x10000U);

    /* Initialize the Interrupt controller */
    {
        /* Disable IRQs */
        (void)HwiP_disable();
        /* Enable interrupt controller system register access at EL1 */
        HwiP_writeSystemReg(s3_0_c12_c12_5, 0x7); /* icc_sre_el1 */

        if((gicdRegs != NULL) && (0U == coreId))
        {
            /*
            * Disable forwarding of interrupts in GIC Distributer and CPU interface
            * Controller.
            */
            gicdRegs->CTLR = 0x0U;
            HwiP_writeSystemReg(s3_0_c12_c12_7, 0x0); /* icc_igrpen1_el1 */
        }
        /*
         * Disable all interrupts at startup
         */
        gicsRegs->ICENABLER0 = CSL_GIC500_GICR_CORE_SGI_PPI_ICENABLER0_ENABLE_MASK;
    #if !defined(AMP_FREERTOS_A53)
        if((gicdRegs != NULL) && (0U == coreId))
        {
            for (i = 0U; i < HWIP_GICD_SPI_INTR_COUNT_MAX/32U; i++)
            {
                gicdRegs->ICENABLER_SPI[i] = CSL_GIC500_GICD_ICENABLER_SPI_ENABLE_MASK;
            }
        }
    #endif

    #if defined(AMP_FREERTOS_A53)
        /*
            * Enable forwarding of interrupts in GIC Distributor and CPU interface
            * Controller.
        */
        /* Spin till lock is acquired */
        while(SpinlockP_swLock( &gSwSpinLockBuff[SW_SPIN_LOCK_2] ) == SW_SPINLOCK_IN_USE);
        if(!(gicdRegs->CTLR & 0x02U))
        {
            gicdRegs->CTLR = 0x2U;
        }
        SpinlockP_swUnlock(&gSwSpinLockBuff[SW_SPIN_LOCK_2]);
        HwiP_writeSystemReg(s3_0_c12_c12_7, 0x1);   /* icc_igrpen1_el1 */

    #else
        if(0U == coreId)
        {
            /*
            * Enable forwarding of interrupts in GIC Distributor and CPU interface
            * Controller.
            */
            gicdRegs->CTLR = 0x2U;
            HwiP_writeSystemReg(s3_0_c12_c12_7, 0x1);   /* icc_igrpen1_el1 */
        }
    #endif

        /* Search for any previously active interrupts and acknowledge them */
        intrActiveReg = gicsRegs->ICACTIVER0;
        if (intrActiveReg)
        {
            for (j = 0; j < HWIP_GICD_SGI_PPI_INTR_ID_MAX; j++)
            {
                if (intrActiveReg & 0x1U)
                {
                    HwiP_writeSystemReg(s3_0_c12_c12_1, j);   /* icc_eoir1_el1 */
                }
                intrActiveReg = intrActiveReg >> 1;
            }
        }
    #if defined(AMP_FREERTOS_A53)
        while(SpinlockP_swLock( &gSwSpinLockBuff[SW_SPIN_LOCK_2] ) == SW_SPINLOCK_IN_USE);
        if(gicdRegs != NULL)
        {
            for (i = 0; i < HWIP_GICD_SPI_INTR_COUNT_MAX/32U; i++)
            {
                intrActiveReg = gicdRegs->ISACTIVER_SPI[i];
                if (intrActiveReg)
                {
                    for (j = 0U; j < 32U; j++)
                    {
                        if (intrActiveReg & 0x1U)
                        {
                            HwiP_writeSystemReg(s3_0_c12_c12_1, (i * 32U) + j);
                                                        /* icc_eoir1_el1 */
                        }
                        intrActiveReg = intrActiveReg >> 1;
                    }
                }
            }
        }
        SpinlockP_swUnlock(&gSwSpinLockBuff[SW_SPIN_LOCK_2]);
    #else
        if((gicdRegs != NULL) && (0U == coreId))
        {
            for (i = 0; i < HWIP_GICD_SPI_INTR_COUNT_MAX/32U; i++)
            {
                intrActiveReg = gicdRegs->ISACTIVER_SPI[i];
                if (intrActiveReg)
                {
                    for (j = 0U; j < 32U; j++)
                    {
                        if (intrActiveReg & 0x1U)
                        {
                            HwiP_writeSystemReg(s3_0_c12_c12_1, (i * 32U) + j);
                                                    /* icc_eoir1_el1 */
                        }
                        intrActiveReg = intrActiveReg >> 1;
                    }
                }
            }
        }
    #endif

        /*
         * Clear any currently pending enabled interrupts
         */
        gicsRegs->ICPENDR0  = CSL_GIC500_GICR_CORE_SGI_PPI_ICPENDR0_PEND_MASK;
    #if defined(AMP_FREERTOS_A53)
        while(SpinlockP_swLock( &gSwSpinLockBuff[SW_SPIN_LOCK_2] ) == SW_SPINLOCK_IN_USE);
        if(gicdRegs != NULL)
        {
            for (i = 0; i < HWIP_GICD_SPI_INTR_COUNT_MAX/32U; i++)
            {
                if(gicdRegs->ICPENDR_SPI[i] != CSL_GIC500_GICD_ICPENDR_SPI_PEND_MASK)
                {
                    gicdRegs->ICPENDR_SPI[i] = CSL_GIC500_GICD_ICPENDR_SPI_PEND_MASK;
                }
            }
        }
        SpinlockP_swUnlock(&gSwSpinLockBuff[SW_SPIN_LOCK_2]);
   #else
        if((gicdRegs != NULL) && (0U == coreId))
        {
            for (i = 0U; i < HWIP_GICD_SPI_INTR_COUNT_MAX/32U; i++)
            {
                gicdRegs->ICPENDR_SPI[i] = CSL_GIC500_GICD_ICPENDR_SPI_PEND_MASK;
            }
        }
    #endif
        /*
         * Clear all interrupt active status registers
         */
        gicsRegs->ICACTIVER0 = CSL_GIC500_GICR_CORE_SGI_PPI_ICACTIVER0_SET_MASK;
    #if defined(AMP_FREERTOS_A53)
        while(SpinlockP_swLock( &gSwSpinLockBuff[SW_SPIN_LOCK_2] ) == SW_SPINLOCK_IN_USE);
        if(gicdRegs != NULL)
        {
            for (i = 0U; i < HWIP_GICD_SPI_INTR_COUNT_MAX/32U; i++)
            {
                if(gicdRegs->ICACTIVER_SPI[i] != CSL_GIC500_GICD_ICACTIVER_SPI_SET_MASK)
                {
                    gicdRegs->ICACTIVER_SPI[i] = CSL_GIC500_GICD_ICACTIVER_SPI_SET_MASK;
                }
            }
        }
        SpinlockP_swUnlock(&gSwSpinLockBuff[SW_SPIN_LOCK_2]);
    #else
        if((gicdRegs != NULL) && (0U == coreId))
        {
            for (i = 0U; i < HWIP_GICD_SPI_INTR_COUNT_MAX/32U; i++)
            {
                gicdRegs->ICACTIVER_SPI[i] = CSL_GIC500_GICD_ICACTIVER_SPI_SET_MASK;
            }
        }
    #endif

    #if !defined(AMP_FREERTOS_A53)
        if(0U == coreId)
        {
            /*
            * Initialize the interrupt routing registers for all
            * interrupts (32-1019).
            *
            * Interrupt number lower than 32 are not routed using these registers.
            */
            for (i = 0; i < HWIP_GICD_SPI_INTR_COUNT_MAX; i++)
            {
                gicdRegs->IROUTER[i].LOWER =  (aff1 << 8 | aff0);
                gicdRegs->IROUTER[i].UPPER = routingMode;
            }
        }
    #endif

        /*
         * Initialize Binary Point Register
         */
        HwiP_writeSystemReg(s3_0_c12_c12_3, 3);   /* icc_bpr1_el1 */

        /*
         * Set preconfigured interrupt Sense and Priority for each
         * interrupt
         */
        for (i = 0U; i < HWIP_GICD_SGI_PPI_INTR_ID_MAX/4U; i++)
        {
           gicsRegs->IPRIORITYR[i]=0x20202020;
        }
    #if defined(AMP_FREERTOS_A53)
        while(SpinlockP_swLock( &gSwSpinLockBuff[SW_SPIN_LOCK_2] ) == SW_SPINLOCK_IN_USE);
        if(gicdRegs != NULL)
        {
            for (i = 0; i < HWIP_GICD_SPI_INTR_COUNT_MAX/4U; i++)
            {
                if(gicdRegs->IPRIORITYR_SPI[i] != 0x20202020U)
                {
                    gicdRegs->IPRIORITYR_SPI[i] = 0x20202020U;
                }

            }
        }
        SpinlockP_swUnlock(&gSwSpinLockBuff[SW_SPIN_LOCK_2]);
    #else
        if((gicdRegs != NULL) && (0U == coreId))
        {
            for (i = 0; i < HWIP_GICD_SPI_INTR_COUNT_MAX/4U; i++)
            {
                gicdRegs->IPRIORITYR_SPI[i]= 0x20202020U;
            }
        }
    #endif

        Armv8_dsbSy();

        /*
         * Initialize Interrupt Priority Mask Register
         *
         * Initialize PMR with the minimum possible interrupt priority.
         */
        HwiP_writeSystemReg(s3_0_c4_c6_0, 0xFF); /* icc_pmr_el1 */

        /*
         * Set trigger sensitivity of interrupts
         *
         * On the Cortex-A15:
         *  -   For SGI:
         *          The ICFGR bits are read-only and a bit-pair always reads as
         *          b10 because SGIs are edge-triggered.
         *  -   For PPI and SPI :
         *          The LSB of the bit-pair is read only and is always 0. MSB can
         *          be altered to change trigger sensitivity.
         *          b00    Interrupt is active-High level-sensitive
         *          b10    Interrupt is rising edge-sensitive
         */
        gicsRegs->ICFGR1 = 0;
    #if defined(AMP_FREERTOS_A53)
        if(0U == coreId)
        {
            if(gicdRegs != NULL)
            {
                for (i = 0; i < HWIP_GICD_SPI_INTR_COUNT_MAX/16U; i++)
                {
                    if(gicdRegs->ICFGR_SPI[i] != 0U)
                    {
                        gicdRegs->ICFGR_SPI[i] = 0U;
                    }
                }
            }
            /* Signal the other cores for the process completion by locking mechanism. */
            (void)SpinlockP_swLock( &gSwSpinLockBuff[SW_SPIN_LOCK_3] );
        }
        else
        {
            /* Check for lock status to happen from core 0. This function returns 0 on completion. */
            while(SpinlockP_swLockStatus( &gSwSpinLockBuff[SW_SPIN_LOCK_3] ) == 1)
            {
                /* Do Nothing */
            };
        }
    #else
        if((gicdRegs != NULL) && (0U == coreId))
        {
            for (i = 0; i < HWIP_GICD_SPI_INTR_COUNT_MAX/16U; i++)
            {
                gicdRegs->ICFGR_SPI[i] = 0;
            }
        }
    #endif

    }

    /* Register the default handlers */
    #if defined(AMP_FREERTOS_A53)
        for(i = 0; i < HwiP_MAX_INTERRUPTS; i++)
        {
            /* Assign default ISR */
            gHwiCtrl.isr[i] = NULL;
            gHwiCtrl.isrArgs[i] = NULL;

        }
    #else
    if(coreId == 0U)
    {
        for(i = 0; i < HwiP_MAX_INTERRUPTS; i++)
        {
            /* Assign default ISR */
            gHwiCtrl.isr[i] = NULL;
            gHwiCtrl.isrArgs[i] = NULL;

        }
    }
    #endif

    #if SMP_FREERTOS
    HwiP_Params hwiParams;
    HwiP_Object hwiObj;

    HwiP_Params_init(&hwiParams);
    hwiParams.intNum = 0;
    hwiParams.callback = taskSwitchContextISR;

    HwiP_construct(&hwiObj, &hwiParams);
    #endif

    return;
}


static void HwiP_intrPrioritySet(uint32_t intrNum, uint32_t priority,
                         uint8_t coreId)
{
    CSL_gic500_gicdRegs *gicdRegs = (CSL_gic500_gicdRegs *)(HWIP_GIC_BASE_ADDR);
    CSL_gic500_gicrRegs *gicrRegs;

    /* update the redistributor and sgi_ppi address */
    gicrRegs = (CSL_gic500_gicrRegs *) (HWIP_GIC_BASE_ADDR + CSL_GIC500_GICR_CORE_CONTROL_CTLR(0U));

    if (intrNum < HWIP_GICD_SGI_PPI_INTR_ID_MAX)
    {
        HwiP_setSgiPpiIntrPriority(gicrRegs, (uint32_t) coreId, (uint32_t) intrNum, (uint32_t)priority);
    }
    else if (intrNum < HWIP_GICD_SPI_INTR_ID_MAX)
    {
        HwiP_setSpiIntrPriority(gicdRegs, (uint32_t)intrNum, (uint32_t)priority);
    }
    else
    {
      /* No action */
    }
    Armv8_dsbSy();

    return;

}


static int32_t HwiP_setSgiPpiIntrPriority( CSL_gic500_gicrRegs *pGic500GicrRegs, uint32_t coreId, uint32_t intrNum, uint32_t value )
{
	if ( coreId < HWIP_GIC_NUM_GICR_CORE )
	{
		if ( intrNum < HWIP_GICD_SGI_PPI_INTR_ID_MAX )
		{
            uint32_t index, mask, subindex;
            uint32_t m;

            index = intrNum/4U;
            subindex = intrNum % 4U;
            mask = 0xFU << ((subindex*8U) + 4U);

            m = pGic500GicrRegs->CORE[coreId].SGI_PPI.IPRIORITYR[index] & (~mask);
            value &= 0xFU;
            pGic500GicrRegs->CORE[coreId].SGI_PPI.IPRIORITYR[index] = (value << ((subindex*8U) + 4U)) | m;
		}
	}
	return SystemP_SUCCESS;
}


static int32_t HwiP_setSpiIntrPriority( CSL_gic500_gicdRegs *pGic500GicdRegs, uint32_t intrNum, uint32_t value )
{

	if ( intrNum < HWIP_GICD_SPI_INTR_ID_MAX )
	{
        /* Transform Interrupt ID */
        intrNum -= HWIP_GICD_SGI_PPI_INTR_ID_MAX;

        uint32_t index, mask, subindex;
        uint32_t m;

        index = intrNum/4U;
        subindex = intrNum % 4U;
        mask = 0xFU << ((subindex*8U) + 4U);

        m = pGic500GicdRegs->IPRIORITYR_SPI[index] & (~mask);
        value &= 0xFU;
        pGic500GicdRegs->IPRIORITYR_SPI[index] = (value << ((subindex*8U) + 4U)) | m;
	}
	return SystemP_SUCCESS;
}


static int32_t HwiP_setPendingSgiPpiIntr( CSL_gic500_gicrRegs *pGic500GicrRegs, uint32_t coreId, uint32_t intrNum )
{
	if ( coreId < HWIP_GIC_NUM_GICR_CORE )
	{
		if ( intrNum < HWIP_GICD_SGI_PPI_INTR_ID_MAX )
		{
            uint32_t mask = 1U << intrNum;
            pGic500GicrRegs->CORE[coreId].SGI_PPI.ISPENDR0 = mask;
		}
	}
	return SystemP_SUCCESS;
}


static int32_t HwiP_setPendingSpiIntr( CSL_gic500_gicdRegs *pGic500GicdRegs, uint32_t intrNum )
{
	if ( intrNum < HWIP_GICD_SPI_INTR_ID_MAX )
	{
        /* Transform Interrupt ID */
        intrNum -= HWIP_GICD_SGI_PPI_INTR_ID_MAX;

        uint32_t index, mask;

        index = intrNum/32u;
        mask = 1U << (intrNum % 32u);

        pGic500GicdRegs->ISPENDR_SPI[index] = mask;
	}
	return SystemP_SUCCESS;
}


static int32_t HwiP_setLevelSpiIntrType(uint16_t intrNum)
{
    CSL_gic500_gicdRegs *gicdRegs = (CSL_gic500_gicdRegs *)(HWIP_GIC_BASE_ADDR);
    CSL_gic500_gicrRegs *gicrRegs = (CSL_gic500_gicrRegs *) ( HWIP_GIC_BASE_ADDR + CSL_GIC500_GICR_CORE_CONTROL_CTLR(0U));

    uint8_t coreId = Armv8_getCoreId();
    uint32_t index, mask;

    if ((gicrRegs != NULL) && (intrNum < HWIP_GICD_SGI_PPI_INTR_ID_MAX))
    {
        index = intrNum / 16U;
        mask = 0b11 << ((intrNum % 16U) * 2U);

        if (index == 0U)
        {
            gicrRegs->CORE[coreId].SGI_PPI.ICFGR0 = gicrRegs->CORE[coreId].SGI_PPI.ICFGR0 & (~mask);
        }
        else
        {
            gicrRegs->CORE[coreId].SGI_PPI.ICFGR1 = gicrRegs->CORE[coreId].SGI_PPI.ICFGR1 & (~mask);
        }

    }
	else if ((gicdRegs != NULL) && (intrNum < HWIP_GICD_SPI_INTR_ID_MAX ))
	{
        /* Transform Interrupt ID */
        intrNum -= HWIP_GICD_SGI_PPI_INTR_ID_MAX;

        index = intrNum / 16u;
        mask = 0b11 << ((intrNum % 16u) * 2u);

        gicdRegs->ICFGR_SPI[index] = gicdRegs->ICFGR_SPI[index] & (~mask);
	}
    else
    {

    }

	return SystemP_SUCCESS;
}


static int32_t HwiP_setEdgeSpiIntrType(uint16_t intrNum)
{
    CSL_gic500_gicdRegs *gicdRegs = (CSL_gic500_gicdRegs *)(HWIP_GIC_BASE_ADDR);
    CSL_gic500_gicrRegs *gicrRegs = (CSL_gic500_gicrRegs *) ( HWIP_GIC_BASE_ADDR + CSL_GIC500_GICR_CORE_CONTROL_CTLR(0U));

    uint8_t coreId = Armv8_getCoreId();
    uint32_t index, mask;

    if ((gicrRegs != NULL) && (intrNum < HWIP_GICD_SGI_PPI_INTR_ID_MAX ))
    {
        index = intrNum / 16u;
        mask = 0b11 << ((intrNum % 16u) * 2u);

        if (index == 0U)
        {
            gicrRegs->CORE[coreId].SGI_PPI.ICFGR0 = gicrRegs->CORE[coreId].SGI_PPI.ICFGR0 | mask;
        }
        else
        {
            gicrRegs->CORE[coreId].SGI_PPI.ICFGR1 = gicrRegs->CORE[coreId].SGI_PPI.ICFGR1 | mask;
        }
    }
	else if ((gicdRegs != NULL) && (intrNum < HWIP_GICD_SPI_INTR_ID_MAX ))
	{
        /* Transform Interrupt ID */
        intrNum -= HWIP_GICD_SGI_PPI_INTR_ID_MAX;

        index = intrNum / 16u;
        mask = 0b11 << ((intrNum % 16u) * 2u);

        gicdRegs->ICFGR_SPI[index] = gicdRegs->ICFGR_SPI[index] | mask;
	}
    else
    {

    }

	return SystemP_SUCCESS;
}

void HwIP_routeGICSharedPeripheralInterrupt(uint32_t intrNum, uint8_t coreId)
{
    CSL_gic500_gicdRegs *gicdRegs = (CSL_gic500_gicdRegs *)(HWIP_GIC_BASE_ADDR);
    uint8_t aff1 = 0;
    uint32_t routingMode = 0;
    if(intrNum >= HWIP_GICD_SGI_PPI_INTR_ID_MAX && intrNum < HWIP_GICD_SPI_INTR_ID_MAX)
    {
        intrNum -= HWIP_GICD_SGI_PPI_INTR_ID_MAX;

        gicdRegs->IROUTER[intrNum].LOWER =  (aff1 << 8 | coreId);
        gicdRegs->IROUTER[intrNum].UPPER = routingMode;
    }
    else
    {
      /* No action */
    }
}