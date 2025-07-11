/*
 *  Copyright (C) 2021-24 Texas Instruments Incorporated
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
 *  \file mmcsd_v0.c
 *
 *  \brief File containing MMCSD Driver APIs implementation for version V0.
 *
 */

/* ========================================================================== */
/*                             Include Files                                  */
/* ========================================================================== */

#include <string.h>
#include <drivers/mmcsd.h>
#include <kernel/dpl/SemaphoreP.h>
#include <kernel/dpl/HwiP.h>
#include <kernel/dpl/CacheP.h>
#include <kernel/dpl/ClockP.h>
#include <drivers/hw_include/cslr.h>
#include <drivers/mmcsd/mmcsd_priv.h>

/* ========================================================================== */
/*                           Macros & Typedefs                                */
/* ========================================================================== */
/** \brief This is the timeout value for sending CMD13 to the card.
 * After every write, the CMD13 is sent this many times and wait for
 * the card to go to transfer state
 * */
#define MMCSD_MEDIA_STATE_THRESHOLD  (10000U)

/* Card status value (Bits 9-12) as defined in physical layer
 * specification section 4.10.1.
 */
#define MMCSD_MEDIA_STATE_TRANSFER  (4U)

/* Interrupt Masks */
#define MMCSD_INTERRUPT_ALL_NORMAL  (0xFFFFU)
#define MMCSD_INTERRUPT_ALL_ERROR   (0x17FFU)

/* Some macros related to setting operating voltage in SD devices */
#define MMCSD_SD_VOLT_2P7_3P6        (0x100U)
#define MMCSD_SD_VOLT_LOW_RANGE      (0x200U)

/* Some Macros related to setting ECSD register in eMMC devices */
#define MMCSD_EMMC_ECSD_DEVICE_TYPE_INDEX             (196)
#define MMCSD_EMMC_ECSD_DEVICE_TYPE_DS                (0x0U)
#define MMCSD_EMMC_ECSD_DEVICE_TYPE_HS_26MHZ          (0x1U)
#define MMCSD_EMMC_ECSD_DEVICE_TYPE_HS_52MHZ          (0x2U)
#define MMCSD_EMMC_ECSD_DEVICE_TYPE_HS_DDR_52MHZ_1P8V (0x4U)
#define MMCSD_EMMC_ECSD_DEVICE_TYPE_HS_DDR_52MHZ_1P2V (0x8U)

#define MMCSD_EMMC_ECSD_DEVICE_TYPE_HS200_200MHZ_1P8V (0x10U)
#define MMCSD_EMMC_ECSD_DEVICE_TYPE_HS200_200MHZ_1P2V (0x20U)
#define MMCSD_EMMC_ECSD_DEVICE_TYPE_HS400_200MHZ_1P8V (0x40U)
#define MMCSD_EMMC_ECSD_DEVICE_TYPE_HS400_200MHZ_1P2V (0x80U)

#define MMCSD_ECSD_BUS_WIDTH_INDEX      (183U)
#define MMCSD_ECSD_BUS_WIDTH_1BIT       (0U)
#define MMCSD_ECSD_BUS_WIDTH_4BIT       (1U)
#define MMCSD_ECSD_BUS_WIDTH_8BIT       (2U)
#define MMCSD_ECSD_BUS_WIDTH_4BIT_DDR   (5U)
#define MMCSD_ECSD_BUS_WIDTH_8BIT_DDR   (6U)

#define MMCSD_ECSD_BUS_WIDTH_BUSWIDTH_MASK        (0x0FU)
#define MMCSD_ECSD_BUS_WIDTH_BUSWIDTH_SHIFT       (0U)

#define MMCSD_ECSD_BUS_WIDTH_ES_ENABLE            (0x80U)

#define MMCSD_ECSD_BUS_WIDTH_ES_MASK              (0x80U)
#define MMCSD_ECSD_BUS_WIDTH_ES_SHIFT             (0x07U)

#define MMCSD_ECSD_RST_N_INDEX                    (162U)
#define MMCSD_ECSD_RST_N_TEMPORARILY_DISABLE      (0U)
#define MMCSD_ECSD_RST_N_PERMANENTLY_ENABLE       (1U)
#define MMCSD_ECSD_RST_N_PERMANENTLY_DISABLE      (2U)

#define MMCSD_ECSD_RST_N_MASK                     (0xFCU)
#define MMCSD_ECSD_RST_N_SHIFT                    (0U)

#define MMCSD_ECSD_HS_TIMING_INDEX                (185U)
#define MMCSD_ECSD_HS_TIMING_BACKWARD_COMPATIBLE  (0U)
#define MMCSD_ECSD_HS_TIMING_HIGH_SPEED           (1U)
#define MMCSD_ECSD_HS_TIMING_HS200                (2U)
#define MMCSD_ECSD_HS_TIMING_HS400                (3U)

#define MMCSD_ECSD_STROBE_SUPPORT_INDEX           (184U)
#define MMCSD_ECSD_STROBE_SUPPORT_ENHANCED_DIS    (0U)
#define MMCSD_ECSD_STROBE_SUPPORT_ENHANCED_EN     (1U)

#define MMCSD_ECSD_ACCESS_MODE                    (0x03U)

#define MMCSD_REFERENCE_CLOCK_200M                (200*1000000U)
#define MMCSD_REFERENCE_CLOCK_52M                 (52*1000000U)

#define MMCSD_DEFAULT_CMD6_TIMEOUT_MS             (500U)
#define MMCSD_GENERIC_CMD6_TIME_INDEX             (248U)

/* Number of delay ratio elements (related to sw tuning) */
#define MMCSD_ITAPDLY_LENGTH                      (uint8_t)(32U)
#define MMCSD_ITAPDLY_LAST_INDEX                  (uint8_t)(31U)
#define MMCSD_RETRY_TUNING_MAX                    (uint8_t)(10U)

/* Number of retries if a transaction fails */
#define MMCSD_TRANS_RETRIES                       (3U)

/* ========================================================================== */
/*                         Structure Declarations                             */
/* ========================================================================== */

typedef struct
{
    void *openLock;
    /**<  Lock to protect MMCSD open*/
    SemaphoreP_Object lockObj;
    /**< Lock object */
} MMCSD_DrvObj;

typedef struct
{
    uint32_t dmaParams;
    uint32_t addrLo;
    uint32_t addrHi;

} MMCSD_ADMA2Descriptor;

/* ========================================================================== */
/*                          Function Declarations                             */
/* ========================================================================== */
static int32_t MMCSD_initSD(MMCSD_Handle handle);
static int32_t MMCSD_initEMMC(MMCSD_Handle handle);
static int32_t MMCSD_transfer(MMCSD_Handle, MMCSD_Transaction *trans);
static int32_t MMCSD_isReadyForTransfer(MMCSD_Handle handle);
static int32_t MMCSD_sendStopCmd(MMCSD_Handle handle);
static int32_t MMCSD_sendCmd23(MMCSD_Handle handle, uint32_t numBlks);
static int32_t MMCSD_setupADMA2(MMCSD_Handle handle, MMCSD_ADMA2Descriptor *desc, uint64_t bufAddr, uint32_t dataSize);
static int32_t MMCSD_switchEmmcMode(MMCSD_Handle handle, uint32_t mode);
static uint32_t MMCSD_getModeEmmc(MMCSD_Handle handle);
static uint32_t MMCSD_getXferSpeedFromModeEmmc(uint32_t mode);
static void MMCSD_initTransaction(MMCSD_Transaction *trans);
static void MMCSD_cmdStatusPollingFxn(MMCSD_Handle handle);
static void MMCSD_xferStatusPollingFxn(MMCSD_Handle handle);
static void MMCSD_xferStatusPollingFxnCMD19(MMCSD_Handle handle);
static int32_t MMCSD_errorRecovery(MMCSD_Handle handle, MMCSD_Transaction *trans);
static int32_t MMCSD_retune(MMCSD_Handle handle);

/* PHY related functions */
static int32_t MMCSD_phyInit(uint32_t ssBaseAddr, uint32_t phyType);
static inline void MMCSD_phyDisableDLL(uint32_t ssBaseAddr);
static int32_t MMCSD_phyConfigure(uint32_t ssBaseAddr, uint32_t phyMode, uint32_t phyClkFreq, uint32_t driverImpedance, uint8_t tunedItap);
static int32_t MMCSD_phyTuneManual(MMCSD_Handle handle, uint8_t *tunedItap, uint8_t tuningCount);
static int32_t MMCSD_phyTuneManualEMMC(MMCSD_Handle handle, uint8_t *tunedItap);
static int32_t MMCSD_phyTuneAuto(MMCSD_Handle handle);

/* CSL like functions */
static int32_t MMCSD_halSoftReset(uint32_t ctrlBaseAddr);
static int32_t MMCSD_halLinesResetCmd(uint32_t ctrlBaseAddr);
static int32_t MMCSD_halLinesResetDat(uint32_t ctrlBaseAddr);
static int32_t MMCSD_halSetBusWidth(uint32_t ctrlBaseAddr, uint32_t busWidth);
static int32_t MMCSD_halSetBusVolt(uint32_t ctrlBaseAddr, uint32_t volt);
static inline int32_t MMCSD_halIsCardInserted(uint32_t ctrlBaseAddr);
static int32_t MMCSD_halBusPower(uint32_t ctrlBaseAddr, uint32_t pwr);
static int32_t MMCSD_halIsClockStable(uint32_t ctrlBaseAddr, uint32_t timeout);
static int32_t MMCSD_halEnableInternalClock(uint32_t ctrlBaseAddr, uint32_t clkState);
static int32_t MMCSD_halSetBusFreq(uint32_t ctrlBaseAddr, uint32_t inClk, uint32_t outClk, uint32_t bypass);
static int32_t MMCSD_halSendCommand(uint32_t ctrlBaseAddr, MMCSD_Transaction *trans);
static int32_t MMCSD_halCmdResponseGet(uint32_t ctrlBaseAddr, uint32_t *rsp);
static int32_t MMCSD_halSetUHSMode(uint32_t ctrlBaseAddr, uint32_t uhsMode);

/* Interrupt related functions */
static uint32_t MMCSD_halNormalIntrStatusGet(uint32_t ctrlBaseAddr, uint16_t intrFlag);
static inline void MMCSD_halNormalIntrStatusClear(uint32_t ctrlBaseAddr, uint16_t intrFlag);
static uint32_t MMCSD_halErrorIntrStatusGet(uint32_t ctrlBaseAddr, uint16_t intrFlag);
static inline void MMCSD_halErrorIntrStatusClear(uint32_t ctrlBaseAddr, uint16_t intrFlag);
static inline void MMCSD_halNormalIntrStatusEnable(uint32_t ctrlBaseAddr, uint16_t intrFlag);
static inline void MMCSD_halNormalIntrStatusDisable(uint32_t ctrlBaseAddr, uint16_t intrFlag);
static inline void MMCSD_halErrorIntrStatusEnable(uint32_t ctrlBaseAddr, uint16_t intrFlag);
static inline void MMCSD_halErrorIntrStatusDisable(uint32_t ctrlBaseAddr, uint16_t intrFlag);
static inline void MMCSD_halNormalSigIntrDisable(uint32_t ctrlBaseAddr, uint16_t intrFlag);
static inline void MMCSD_halNormalSigIntrEnable(uint32_t ctrlBaseAddr, uint16_t intrFlag);
static inline void MMCSD_halErrorSigIntrDisable(uint32_t ctrlBaseAddr, uint16_t intrFlag);
static inline void MMCSD_halErrorSigIntrEnable(uint32_t ctrlBaseAddr, uint16_t intrFlag);

static void MMCSD_isr(void *arg);

/* ========================================================================== */
/*                            Global Variables                                */
/* ========================================================================== */

/** \brief Driver object */
static MMCSD_DrvObj gMmcsdDrvObj =
{
    .openLock      = NULL,
};

/** \brief Global ADMA2 Descriptor */
MMCSD_ADMA2Descriptor gADMA2Desc;

uint8_t gTuningPattern8Bit[] __attribute__((aligned(128U))) = {
    0xff, 0xff, 0x00, 0xff, 0xff, 0xff, 0x00, 0x00,
    0xff, 0xff, 0xcc, 0xcc, 0xcc, 0x33, 0xcc, 0xcc,
    0xcc, 0x33, 0x33, 0xcc, 0xcc, 0xcc, 0xff, 0xff,
    0xff, 0xee, 0xff, 0xff, 0xff, 0xee, 0xee, 0xff,
    0xff, 0xff, 0xdd, 0xff, 0xff, 0xff, 0xdd, 0xdd,
    0xff, 0xff, 0xff, 0xbb, 0xff, 0xff, 0xff, 0xbb,
    0xbb, 0xff, 0xff, 0xff, 0x77, 0xff, 0xff, 0xff,
    0x77, 0x77, 0xff, 0x77, 0xbb, 0xdd, 0xee, 0xff,
    0xff, 0xff, 0xff, 0x00, 0xff, 0xff, 0xff, 0x00,
    0x00, 0xff, 0xff, 0xcc, 0xcc, 0xcc, 0x33, 0xcc,
    0xcc, 0xcc, 0x33, 0x33, 0xcc, 0xcc, 0xcc, 0xff,
    0xff, 0xff, 0xee, 0xff, 0xff, 0xff, 0xee, 0xee,
    0xff, 0xff, 0xff, 0xdd, 0xff, 0xff, 0xff, 0xdd,
    0xdd, 0xff, 0xff, 0xff, 0xbb, 0xff, 0xff, 0xff,
    0xbb, 0xbb, 0xff, 0xff, 0xff, 0x77, 0xff, 0xff,
    0xff, 0x77, 0x77, 0xff, 0x77, 0xbb, 0xdd, 0xee
};

/* ========================================================================== */
/*                          Function Definitions                              */
/* ========================================================================== */

void MMCSD_init(void)
{
    int32_t status;
    uint32_t count;
    MMCSD_Object *obj;

    /* Init each driver instance object */
    for(count = 0U; count < gMmcsdConfigNum; count++)
    {
        /* Init object variables */
        obj = gMmcsdConfig[count].object;
        DebugP_assert(NULL != obj);
        memset(obj, 0, sizeof(MMCSD_Object));
    }

    /* Create the driver lock */
    status = SemaphoreP_constructMutex(&gMmcsdDrvObj.lockObj);
    if(SystemP_SUCCESS == status)
    {
        gMmcsdDrvObj.openLock = &gMmcsdDrvObj.lockObj;
    }

    return;
}

void MMCSD_deinit(void)
{
    /* Delete driver lock */
    if(NULL != gMmcsdDrvObj.openLock)
    {
        SemaphoreP_destruct(&gMmcsdDrvObj.lockObj);
        gMmcsdDrvObj.openLock = NULL;
    }
    return;
}

void MMCSD_Params_init(MMCSD_Params *mmcsdParams)
{
    if(mmcsdParams != NULL)
    {
        /* NULL init deviceData */
        mmcsdParams->deviceData = NULL;
    }
}

MMCSD_Handle MMCSD_open(uint32_t index, const MMCSD_Params *openParams)
{
    int32_t status = SystemP_SUCCESS;
    MMCSD_Handle handle = NULL;
    MMCSD_Config *config = NULL;
    MMCSD_Object *obj = NULL;
    HwiP_Params hwiPrms;
    const MMCSD_Attrs *attrs;

    /* Check for valid index */
    if(index >= gMmcsdConfigNum)
    {
        status = SystemP_FAILURE;
    }
    else
    {
        config = &gMmcsdConfig[index];
    }

    /* Protect this region from a concurrent OSPI_Open */
    DebugP_assert(NULL != gMmcsdDrvObj.openLock);
    SemaphoreP_pend(&gMmcsdDrvObj.lockObj, SystemP_WAIT_FOREVER);

    if(SystemP_SUCCESS == status)
    {
        obj = config->object;
        DebugP_assert(NULL != obj);
        DebugP_assert(NULL != config->attrs);
        attrs = config->attrs;
        if(TRUE == obj->isOpen)
        {
            /* Handle already opened */
            status = SystemP_FAILURE;
        }
    }

    if(SystemP_SUCCESS == status)
    {
        obj->handle = (MMCSD_Handle)config;

        /* Register interrupt */
        if(TRUE == attrs->intrEnable)
        {
            HwiP_Params_init(&hwiPrms);
            hwiPrms.intNum      = attrs->intrNum;
            hwiPrms.callback    = &MMCSD_isr;
            hwiPrms.args        = (void *)config;
            status += HwiP_construct(&obj->hwiObj, &hwiPrms);
        }

        /* Create semaphores for transfer completion */
        status += SemaphoreP_constructMutex(&obj->cmdMutex);
        status += SemaphoreP_constructMutex(&obj->xferMutex);
        status += SemaphoreP_constructBinary(&obj->cmdCompleteSemObj, 0);
        status += SemaphoreP_constructBinary(&obj->dataCopyCompleteSemObj, 0);
        status += SemaphoreP_constructBinary(&obj->xferCompleteSemObj, 0);

        /* Program MMCSD instance according the user config */
        obj->cardType = attrs->cardType;
        obj->enableDma = attrs->enableDma;
        obj->intrEnable = attrs->intrEnable;
        obj->tempDataBuf = openParams->dataBuf;

        if(MMCSD_CARD_TYPE_SD == obj->cardType)
        {
            obj->sdData = (MMCSD_SdDeviceData *) openParams->deviceData;
            status = MMCSD_initSD(config);
        }
        else if(MMCSD_CARD_TYPE_EMMC == obj->cardType)
        {
            obj->emmcData = (MMCSD_EmmcDeviceData *) openParams->deviceData;
            status = MMCSD_initEMMC(config);
        }
        else if(MMCSD_CARD_TYPE_NO_DEVICE == obj->cardType)
        {
            /* Nothing to be initialized */
            status = SystemP_SUCCESS;
        }
    }

    if(SystemP_SUCCESS == status)
    {
        obj->isOpen = 1;
        handle = (MMCSD_Handle) config;
    }

    SemaphoreP_post(&gMmcsdDrvObj.lockObj);

    /* Free up resources in case of error */
    if(SystemP_SUCCESS != status)
    {
        if(NULL != config)
        {
            MMCSD_close((MMCSD_Handle) config);
        }
    }

    return handle;
}

void MMCSD_close(MMCSD_Handle handle)
{
    int32_t status = SystemP_SUCCESS;
    MMCSD_Object *obj = ((MMCSD_Config *)handle)->object;
    MMCSD_Attrs const *attrs = ((MMCSD_Config *)handle)->attrs;
    const CSL_mmc_ctlcfgRegs *pReg = (const CSL_mmc_ctlcfgRegs *)attrs->ctrlBaseAddr;

    MMCSD_Transaction trans;

    if(obj->intrEnable == TRUE)
    {
        HwiP_destruct(&obj->hwiObj);
    }

    if(obj->cardType == MMCSD_CARD_TYPE_EMMC)
    {
        MMCSD_initTransaction(&trans);
        trans.cmd = MMCSD_MMC_CMD(6);
        trans.arg = (MMCSD_ECSD_ACCESS_MODE << 24U) | (MMCSD_ECSD_BUS_WIDTH_INDEX << 16U) | (((0 << MMCSD_ECSD_BUS_WIDTH_ES_SHIFT) | MMCSD_ECSD_BUS_WIDTH_1BIT) << 8U);
        trans.retries = MMCSD_TRANS_RETRIES;
        status = MMCSD_transfer(handle, &trans);

        obj->busWidth = MMCSD_BUS_WIDTH_1BIT;

        if(SystemP_SUCCESS == status)
        {
            MMCSD_halSetBusWidth(attrs->ctrlBaseAddr, MMCSD_BUS_WIDTH_1BIT);

            status = MMCSD_isReadyForTransfer(handle);
        }

        if(SystemP_SUCCESS == status)
        {
            MMCSD_initTransaction(&trans);
            trans.cmd = MMCSD_MMC_CMD(6);
            trans.arg = (MMCSD_ECSD_ACCESS_MODE << 24U) | (MMCSD_ECSD_HS_TIMING_INDEX << 16U) | ((((obj->emmcData->driveStrength) << 4U) | 1U) << 8U);
            trans.retries = MMCSD_TRANS_RETRIES;
            status = MMCSD_transfer(handle, &trans);

            if(SystemP_SUCCESS == status)
            {
                while(CSL_REG32_FEXT(&pReg->PRESENTSTATE, MMC_CTLCFG_PRESENTSTATE_SDIF_DAT0IN) != 1U);
            }

            status = MMCSD_halSetUHSMode(attrs->ctrlBaseAddr, MMCSD_UHS_MODE_SDR50);

            MMCSD_phyDisableDLL(attrs->ssBaseAddr);

            status |= MMCSD_halSetBusFreq(attrs->ctrlBaseAddr, attrs->inputClkFreq, MMCSD_REFERENCE_CLOCK_52M, 0U);

            status |= MMCSD_phyConfigure(attrs->ssBaseAddr, MMCSD_PHY_MODE_HSSDR50, MMCSD_REFERENCE_CLOCK_52M, 0U, 0U);

        }

        if(SystemP_SUCCESS == status)
        {
            status = MMCSD_halSoftReset(attrs->ctrlBaseAddr);

            status |= MMCSD_halSetBusVolt(attrs->ctrlBaseAddr, MMCSD_BUS_VOLT_1_8V);

            MMCSD_phyInit(attrs->ssBaseAddr, attrs->phyType);

            status |= MMCSD_halBusPower(attrs->ctrlBaseAddr, CSL_MMC_CTLCFG_POWER_CONTROL_SD_BUS_POWER_VAL_PWR_ON);

            status |= MMCSD_halSetBusFreq(attrs->ctrlBaseAddr, attrs->inputClkFreq, 400000, FALSE);
        }

        if(SystemP_SUCCESS == status)
        {
            MMCSD_initTransaction(&trans);
            trans.cmd   = MMCSD_MMC_CMD(0);
            trans.arg   = 0U;
            status = MMCSD_transfer(handle, &trans);
        }

        ClockP_usleep(5000);
    }

    SemaphoreP_destruct(&obj->cmdMutex);
    SemaphoreP_destruct(&obj->xferMutex);
    SemaphoreP_destruct(&obj->cmdCompleteSemObj);
    SemaphoreP_destruct(&obj->dataCopyCompleteSemObj);
    SemaphoreP_destruct(&obj->xferCompleteSemObj);

    memset(obj, 0, sizeof(MMCSD_Object));
}

MMCSD_Handle MMCSD_getHandle(uint32_t driverInstanceIndex)
{
    MMCSD_Handle         handle = NULL;
    /* Check index */
    if(driverInstanceIndex < gMmcsdConfigNum)
    {
        MMCSD_Object *obj;
        obj = gMmcsdConfig[driverInstanceIndex].object;

        if(obj && (TRUE == obj->isOpen))
        {
            /* valid handle */
            handle = obj->handle;
        }
    }
    return handle;
}

int32_t MMCSD_read(MMCSD_Handle handle, uint8_t *buf, uint32_t startBlk, uint32_t numBlks)
{
    int32_t status = SystemP_SUCCESS;
    MMCSD_Object *obj = ((MMCSD_Config *)handle)->object;
    MMCSD_Attrs const *attrs = ((MMCSD_Config *)handle)->attrs;
    MMCSD_Transaction trans;
    uint32_t addr = 0U;
    uint32_t cmd = 0U;
    uint32_t blockSize = MMCSD_getBlockSize(handle);
    if(((obj->emmcData->supportedModes & MMCSD_EMMC_ECSD_DEVICE_TYPE_HS200_200MHZ_1P8V) &&
       (attrs->supportedModes & MMCSD_SUPPORT_MMC_HS200)) ||
       ((obj->emmcData->supportedModes & MMCSD_EMMC_ECSD_DEVICE_TYPE_HS400_200MHZ_1P8V) &&
       (attrs->supportedModes & MMCSD_SUPPORT_MMC_HS400)))
    {
        obj->xferHighSpeedEn = 1;
    }

    obj->readBufIdx = buf;
    obj->readBlockCount = numBlks;

    if(SystemP_SUCCESS == status)
    {
        if(obj->isHC == TRUE)
        {
            addr = startBlk;
        }
        else
        {
            addr = startBlk * blockSize;
        }

        if(numBlks >  1U)
        {
            cmd = MMCSD_SD_CMD(18);
        }
        else
        {
            cmd = MMCSD_SD_CMD(17);
        }

        if(numBlks > CSL_MMC_CTLCFG_BLOCK_COUNT_XFER_BLK_CNT_MAX)
        {
            MMCSD_initTransaction(&trans);
            trans.dir = MMCSD_CMD_XFER_TYPE_READ;
            trans.arg = addr;
            trans.blockSize = blockSize;
            trans.dataBuf = (void *)buf;
            trans.cmd = cmd;

            uint32_t currNumBlks = numBlks;
            while(status == SystemP_SUCCESS && currNumBlks > CSL_MMC_CTLCFG_BLOCK_COUNT_XFER_BLK_CNT_MAX)
            {
                trans.blockCount = CSL_MMC_CTLCFG_BLOCK_COUNT_XFER_BLK_CNT_MAX;
                trans.retries = MMCSD_TRANS_RETRIES;

                status = MMCSD_isReadyForTransfer(handle);

                if(SystemP_SUCCESS == status)
                {
                    if(obj->isCmd23 != 0U)
                    {
                        MMCSD_sendCmd23(handle, trans.blockCount);
                    }
                }

                if(SystemP_SUCCESS == status)
                {
                    status = MMCSD_transfer(handle, &trans);
                }

                if(SystemP_SUCCESS == status)
                {
                    if(obj->isCmd23 == 0U)
                    {
                        MMCSD_sendStopCmd(handle);
                    }
                }

                currNumBlks = currNumBlks - CSL_MMC_CTLCFG_BLOCK_COUNT_XFER_BLK_CNT_MAX;
                if(obj->isHC == TRUE)
                {
                    addr = addr + CSL_MMC_CTLCFG_BLOCK_COUNT_XFER_BLK_CNT_MAX;
                }
                else
                {
                    addr = addr + CSL_MMC_CTLCFG_BLOCK_COUNT_XFER_BLK_CNT_MAX * blockSize;
                }
                trans.arg = addr;
                trans.dataBuf = trans.dataBuf + CSL_MMC_CTLCFG_BLOCK_COUNT_XFER_BLK_CNT_MAX * blockSize;
            }
            if(currNumBlks > 0U)
            {
                trans.blockCount = currNumBlks;
                trans.retries = MMCSD_TRANS_RETRIES;
                if(currNumBlks > 1U)
                {
                    trans.cmd = MMCSD_MMC_CMD(18);
                }
                else
                {
                    trans.cmd = MMCSD_MMC_CMD(17);
                }

                status = MMCSD_isReadyForTransfer(handle);

                if(SystemP_SUCCESS == status)
                {
                    if(obj->isCmd23 != 0U)
                    {
                        MMCSD_sendCmd23(handle, trans.blockCount);
                    }
                }

                if(SystemP_SUCCESS == status)
                {
                    status = MMCSD_transfer(handle, &trans);
                }
            }
        }
        else
        {
            status = MMCSD_isReadyForTransfer(handle);

            if(SystemP_SUCCESS == status)
            {
                if(obj->isCmd23 != 0U)
                {
                    MMCSD_sendCmd23(handle, numBlks);
                }
            }

            MMCSD_initTransaction(&trans);
            trans.dir = MMCSD_CMD_XFER_TYPE_READ;
            trans.arg = addr;
            trans.blockCount = numBlks;
            trans.blockSize = blockSize;
            trans.dataBuf = (void *)buf;
            trans.cmd = cmd;
            trans.retries = MMCSD_TRANS_RETRIES;
            status = MMCSD_transfer(handle, &trans);
        }
    }

    if((SystemP_SUCCESS == status) && (obj->isCmd23 != TRUE))
    {
        if(trans.blockCount > 1U)
        {
            MMCSD_sendStopCmd(handle);
        }
    }

    return status;
}

int32_t MMCSD_write(MMCSD_Handle handle, uint8_t *buf, uint32_t startBlk, uint32_t numBlks)
{
    int32_t status = SystemP_SUCCESS;
    MMCSD_Object *obj = ((MMCSD_Config *)handle)->object;
    MMCSD_Attrs const *attrs = ((MMCSD_Config *)handle)->attrs;
    MMCSD_Transaction trans;
    uint32_t addr = 0U;
    uint32_t cmd = 0U;
    uint32_t blockSize = MMCSD_getBlockSize(handle);
    if(((obj->emmcData->supportedModes & MMCSD_EMMC_ECSD_DEVICE_TYPE_HS200_200MHZ_1P8V) &&
       (attrs->supportedModes & MMCSD_SUPPORT_MMC_HS200)) ||
       ((obj->emmcData->supportedModes & MMCSD_EMMC_ECSD_DEVICE_TYPE_HS400_200MHZ_1P8V) &&
       (attrs->supportedModes & MMCSD_SUPPORT_MMC_HS400)))
    {
        obj->xferHighSpeedEn = 1;
    }

    obj->writeBufIdx = buf;
    obj->writeBlockCount = numBlks;

    if(SystemP_SUCCESS == status)
    {
        if(obj->isHC == TRUE)
        {
            addr = startBlk;
        }
        else
        {
            addr = startBlk * blockSize;
        }
        if(numBlks > 1U)
        {
            cmd = MMCSD_MMC_CMD(25);
        }
        else
        {
            cmd = MMCSD_MMC_CMD(24);
        }

        if(numBlks > CSL_MMC_CTLCFG_BLOCK_COUNT_XFER_BLK_CNT_MAX)
        {
            MMCSD_initTransaction(&trans);
            trans.dir = MMCSD_CMD_XFER_TYPE_WRITE;
            trans.arg = addr;
            trans.blockCount = numBlks;
            trans.blockSize = blockSize;
            trans.dataBuf = buf;
            trans.cmd = cmd;

            uint32_t currNumBlks = numBlks;
            while(status == SystemP_SUCCESS && currNumBlks > CSL_MMC_CTLCFG_BLOCK_COUNT_XFER_BLK_CNT_MAX)
            {
                trans.blockCount = CSL_MMC_CTLCFG_BLOCK_COUNT_XFER_BLK_CNT_MAX;
                trans.retries = MMCSD_TRANS_RETRIES;

                status = MMCSD_isReadyForTransfer(handle);

                if(SystemP_SUCCESS == status)
                {
                    if(obj->isCmd23 != 0U)
                    {
                        MMCSD_sendCmd23(handle, trans.blockCount);
                    }
                }

                if(SystemP_SUCCESS == status)
                {
                    status = MMCSD_transfer(handle, &trans);
                }

                if(SystemP_SUCCESS == status)
                {
                    if(obj->isCmd23 == 0U)
                    {
                        MMCSD_sendStopCmd(handle);
                    }
                }

                currNumBlks = currNumBlks - CSL_MMC_CTLCFG_BLOCK_COUNT_XFER_BLK_CNT_MAX;
                if(obj->isHC == TRUE)
                {
                    addr = addr + CSL_MMC_CTLCFG_BLOCK_COUNT_XFER_BLK_CNT_MAX;
                }
                else
                {
                    addr = addr + CSL_MMC_CTLCFG_BLOCK_COUNT_XFER_BLK_CNT_MAX * blockSize;
                }
                trans.arg = addr;
                trans.dataBuf = trans.dataBuf + CSL_MMC_CTLCFG_BLOCK_COUNT_XFER_BLK_CNT_MAX * blockSize;
            }
            if(currNumBlks > 0U)
            {
                trans.blockCount = currNumBlks;
                trans.retries = MMCSD_TRANS_RETRIES;
                if(currNumBlks > 1U)
                {
                    trans.cmd = MMCSD_MMC_CMD(25);
                }
                else
                {
                    trans.cmd = MMCSD_MMC_CMD(24);
                }

                status = MMCSD_isReadyForTransfer(handle);

                if(SystemP_SUCCESS == status)
                {
                    if(obj->isCmd23 != 0U)
                    {
                        MMCSD_sendCmd23(handle, trans.blockCount);
                    }
                }

                if(SystemP_SUCCESS == status)
                {
                    status = MMCSD_transfer(handle, &trans);
                }
            }
        }
        else
        {
            status = MMCSD_isReadyForTransfer(handle);

            if(SystemP_SUCCESS == status)
            {
                if(obj->isCmd23 != 0U)
                {
                    MMCSD_sendCmd23(handle, numBlks);
                }
            }

            MMCSD_initTransaction(&trans);
            trans.dir = MMCSD_CMD_XFER_TYPE_WRITE;
            trans.arg = addr;
            trans.blockCount = numBlks;
            trans.blockSize = blockSize;
            trans.dataBuf = buf;
            trans.cmd = cmd;
            trans.retries = MMCSD_TRANS_RETRIES;
            status = MMCSD_transfer(handle, &trans);
        }
    }

    if((SystemP_SUCCESS == status) && (obj->isCmd23 != TRUE))
    {
        if(trans.blockCount > 1U)
        {
            MMCSD_sendStopCmd(handle);
        }
    }

    return status;
}

uint32_t MMCSD_getBlockSize(MMCSD_Handle handle)
{
    MMCSD_Object *obj = ((MMCSD_Config *)handle)->object;

    uint32_t blockSize = 0U;

    if(obj->cardType == MMCSD_CARD_TYPE_EMMC)
    {
        blockSize = obj->emmcData->maxWriteBlockLen;
    }
    else
    {
        blockSize = obj->sdData->maxWriteBlockLen;
    }

    return blockSize;
}

uint32_t MMCSD_getBlockCount(MMCSD_Handle handle)
{
    MMCSD_Object *obj = ((MMCSD_Config *)handle)->object;

    uint32_t blockSize = 0U;

    if(obj->cardType == MMCSD_CARD_TYPE_EMMC)
    {
        blockSize = obj->emmcData->blockCount;
    }
    else
    {
        blockSize = obj->sdData->blockCount;
    }

    return blockSize;
}

uint32_t MMCSD_isHC(MMCSD_Handle handle)
{
    MMCSD_Object *obj = ((MMCSD_Config *)handle)->object;

    return obj->isHC;
}

int32_t MMCSD_enableBootPartition(MMCSD_Handle handle, uint32_t partitionNum)
{
    int32_t status = SystemP_SUCCESS;

    MMCSD_Object *obj = ((MMCSD_Config *)handle)->object;

    if(obj->cardType == MMCSD_CARD_TYPE_EMMC)
    {
        /* Enable boot partition */
        if((partitionNum == 1) || (partitionNum == 2))
        {
            uint8_t bootAck = 1U; /* ROM Needs boot ack */
            uint8_t bootPartition = ((bootAck << 6U) | (partitionNum << 3) | partitionNum);
            uint8_t bootBusWidth = 0x02;
            uint32_t arg = (uint32_t)((bootPartition << 8) | (0xB3 << 16) | (0x03 << 24));

            MMCSD_Transaction trans;

            /* Configure the ECSD register using CMD6 */
            MMCSD_initTransaction(&trans);
            trans.cmd = MMCSD_MMC_CMD(6);
            trans.arg = arg;
            trans.retries = MMCSD_TRANS_RETRIES;
            status = MMCSD_transfer(handle, &trans);

            if(status == SystemP_SUCCESS)
            {
                status = MMCSD_isReadyForTransfer(handle);
            }

            if(status == SystemP_SUCCESS)
            {
                /* Set bus width now */
                arg = (uint32_t)((bootBusWidth << 8) | (0xB1 << 16) | (0x03 << 24));

                MMCSD_initTransaction(&trans);
                trans.cmd = MMCSD_MMC_CMD(6);
                trans.arg = arg;
                trans.retries = MMCSD_TRANS_RETRIES;
                status = MMCSD_transfer(handle, &trans);
            }

            if(status == SystemP_SUCCESS)
            {
                status = MMCSD_isReadyForTransfer(handle);
            }
        }
        else
        {
            status = SystemP_FAILURE;
        }

    }
    else
    {
        /* Do nothing */
    }

    return status;
}

int32_t MMCSD_disableBootPartition(MMCSD_Handle handle)
{
    int32_t status = SystemP_SUCCESS;

    MMCSD_Object *obj = ((MMCSD_Config *)handle)->object;

    if(obj->cardType == MMCSD_CARD_TYPE_EMMC)
    {
        /* Disable boot partition */
        uint8_t bootPartition = 0U;
        uint32_t arg = (uint32_t)((bootPartition << 8) | (0xB3 << 16) | (0x03 << 24));

        MMCSD_Transaction trans;

        /* Configure the ECSD register using CMD6 */
        MMCSD_initTransaction(&trans);
        trans.cmd = MMCSD_MMC_CMD(6);
        trans.arg = arg;
        trans.retries = MMCSD_TRANS_RETRIES;
        status = MMCSD_transfer(handle, &trans);

        if(status == SystemP_SUCCESS)
        {
            status = MMCSD_isReadyForTransfer(handle);
        }
    }
    else
    {
        /* Do nothing */
    }

    return status;
}

uint32_t MMCSD_getInputClk(MMCSD_Handle handle)
{
    uint32_t retVal = 0U;
    if(handle != NULL)
    {
        const MMCSD_Attrs* attrs = ((MMCSD_Config *)handle)->attrs;
        retVal = attrs->inputClkFreq;
    }
    return retVal;
}


/* ========================================================================== */
/*                     Internal function definitions                          */
/* ========================================================================== */

static int32_t MMCSD_initSD(MMCSD_Handle handle)
{
    int32_t status = SystemP_SUCCESS;
    MMCSD_Object *obj = ((MMCSD_Config *)handle)->object;
    MMCSD_Attrs const *attrs = ((MMCSD_Config *)handle)->attrs;
    const CSL_mmc_sscfgRegs *pSSReg = (const CSL_mmc_sscfgRegs *)attrs->ssBaseAddr;
    const CSL_mmc_ctlcfgRegs *pReg = (const CSL_mmc_ctlcfgRegs *)attrs->ctrlBaseAddr;

    MMCSD_Transaction trans;

    /* Initialize PHY */
    status = MMCSD_phyInit(attrs->ssBaseAddr, attrs->phyType);

    if(SystemP_SUCCESS == status)
    {
        status = MMCSD_halSoftReset(attrs->ctrlBaseAddr);
    }

    if(SystemP_SUCCESS == status)
    {
        /* Set the bus width */
        status = MMCSD_halSetBusWidth(attrs->ctrlBaseAddr, MMCSD_BUS_WIDTH_1BIT);
    }

    /* Set 3.3 V bus volt */
    if(SystemP_SUCCESS == status)
    {
        /* Set the bus voltage */
        status = MMCSD_halSetBusVolt(attrs->ctrlBaseAddr, MMCSD_BUS_VOLT_3_3V);
    }

    if(SystemP_SUCCESS == status)
    {
        /* Set the slot type to removable */
        CSL_REG32_FINS(&pSSReg->CTL_CFG_2_REG,
            MMC_SSCFG_CTL_CFG_2_REG_SLOTTYPE,
            CSL_MMC_CTLCFG_CAPABILITIES_SLOT_TYPE_VAL_REMOVABLE);

        /* Wait for card detect */
        while(!MMCSD_halIsCardInserted(attrs->ctrlBaseAddr));

        /* Switch on Bus Power */
        status = MMCSD_halBusPower(attrs->ctrlBaseAddr, CSL_MMC_CTLCFG_POWER_CONTROL_SD_BUS_POWER_VAL_PWR_ON);
    }

    if(SystemP_SUCCESS == status)
    {
        /* Set initialization frequency */
        status = MMCSD_halSetBusFreq(attrs->ctrlBaseAddr, attrs->inputClkFreq, 25*1000000, 0);
    }

    /* Card initialization */
    if(SystemP_SUCCESS == status)
    {
        /* Send CMD 0 to reset */
        MMCSD_initTransaction(&trans);
        trans.cmd = MMCSD_SD_CMD(0);
        status = MMCSD_transfer(handle, &trans);
    }

    /* Send operating voltage */
    if(SystemP_SUCCESS == status)
    {
        MMCSD_initTransaction(&trans);
        trans.cmd = MMCSD_SD_CMD(8);
        trans.arg = 0xAA | MMCSD_SD_VOLT_2P7_3P6;
        trans.retries = MMCSD_TRANS_RETRIES;
        status = MMCSD_transfer(handle, &trans);
    }
    /* Send OCR - ACMD*/
    if(SystemP_SUCCESS == status)
    {
        uint32_t ocrb31 = 0U;
        uint32_t retry = 0xFFFFU;

        while((ocrb31 == 0U) && (retry != 0U))
        {
            MMCSD_initTransaction(&trans);
            trans.cmd = MMCSD_SD_CMD(55);
            trans.retries = MMCSD_TRANS_RETRIES;
            status = MMCSD_transfer(handle, &trans);

            if(SystemP_SUCCESS == status)
            {
                /* Send ACMD41 */
                MMCSD_initTransaction(&trans);
                trans.cmd = MMCSD_SD_ACMD(41);
                trans.arg = (1 << 30) | (0x01FFU << 15); /* 30th bit - High Cap, 24:15 bits - VDD wild card */
                trans.retries = MMCSD_TRANS_RETRIES;
                status = MMCSD_transfer(handle, &trans);
            }

            ocrb31 = (trans.response[0] & (1 << 31));
            retry--;
        }
        
        if(retry == 0U)
        {
            status = SystemP_FAILURE;
            return status;
        }
    }

    obj->isUHS = FALSE;

    /* Parse OCR and change voltage if needed - TODO: do later */
    if(SystemP_SUCCESS == status)
    {
        obj->sdData->ocr  = trans.response[0];
        obj->isHC = ((obj->sdData->ocr >> 30) & 0x01) ? TRUE : FALSE;
    }

    if(SystemP_SUCCESS == status)
    {
        MMCSD_initTransaction(&trans);
        trans.cmd = MMCSD_SD_CMD(2);
        trans.retries = MMCSD_TRANS_RETRIES;
        status = MMCSD_transfer(handle, &trans);
        MMCSD_parseCIDSd(obj->sdData, trans.response);
    }

    /* Set RCA */
    if(SystemP_SUCCESS == status)
    {
        MMCSD_initTransaction(&trans);
        trans.cmd = MMCSD_SD_CMD(3);
        trans.retries = MMCSD_TRANS_RETRIES;
        status = MMCSD_transfer(handle, &trans);

        /* RCA is the most significant 16 bits */
        obj->sdData->rca = ((trans.response[0] >> 16) & 0xFFFFU);
    }

    if(SystemP_SUCCESS == status)
    {
        MMCSD_initTransaction(&trans);
        trans.cmd = MMCSD_SD_CMD(9);
        trans.arg = (obj->sdData->rca << 16U);
        trans.retries = MMCSD_TRANS_RETRIES;
        status = MMCSD_transfer(handle, &trans);
        MMCSD_parseCSDSd(obj->sdData, trans.response);
    }

    /* Select Card */
    if(SystemP_SUCCESS == status)
    {
        MMCSD_initTransaction(&trans);
        trans.cmd = MMCSD_SD_CMD(7);
        trans.arg = (obj->sdData->rca << 16U);
        trans.retries = MMCSD_TRANS_RETRIES;

        status = MMCSD_transfer(handle, &trans);
    }

    /* Wait for DAT0 */
    if(SystemP_SUCCESS == status)
    {
        while(CSL_REG32_FEXT(&pReg->PRESENTSTATE, MMC_CTLCFG_PRESENTSTATE_SDIF_DAT0IN) != 1U);
    }

    /* Set block length to 512B if not a High Capacity card */
    if((SystemP_SUCCESS == status) && (obj->isHC == FALSE))
    {
        MMCSD_initTransaction(&trans);
        trans.cmd = MMCSD_SD_CMD(16);
        trans.arg = 512U;
        trans.retries = MMCSD_TRANS_RETRIES;
        status = MMCSD_transfer(handle, &trans);
    }

    /* Get the SCR register */
    if(SystemP_SUCCESS == status)
    {
        MMCSD_initTransaction(&trans);
        trans.cmd = MMCSD_SD_CMD(55);
        trans.arg = (obj->sdData->rca << 16U);
        trans.retries = MMCSD_TRANS_RETRIES;
        status = MMCSD_transfer(handle, &trans);

        if(SystemP_SUCCESS == status)
        {
            MMCSD_initTransaction(&trans);
            trans.cmd = MMCSD_SD_ACMD(51);
            trans.dir = MMCSD_CMD_XFER_TYPE_READ;
            trans.blockCount = 1U;
            trans.blockSize = 8U;
            trans.dataBuf = obj->tempDataBuf;
            trans.retries = MMCSD_TRANS_RETRIES;
            status = MMCSD_transfer(handle, &trans);
        }
    }

    if(SystemP_SUCCESS == status)
    {
        MMCSD_parseSCRSd(obj->sdData, obj->tempDataBuf);

        obj->isCmd23 = obj->sdData->isCmd23;

        MMCSD_initTransaction(&trans);
        trans.cmd = MMCSD_SD_CMD(55);
        trans.arg = (obj->sdData->rca << 16U);
        trans.retries = MMCSD_TRANS_RETRIES;
        status = MMCSD_transfer(handle, &trans);

        if(SystemP_SUCCESS == status)
        {
            MMCSD_initTransaction(&trans);
            trans.cmd = MMCSD_SD_ACMD(6);
            uint32_t width = 0U;
            if(attrs->busWidth & MMCSD_BUS_WIDTH_4BIT)
            {
                width = MMCSD_BUS_WIDTH_4BIT;
            }
            else
            {
                width = MMCSD_BUS_WIDTH_1BIT;
            }
            trans.arg = width >> 1U;
            trans.retries = MMCSD_TRANS_RETRIES;
            status = MMCSD_transfer(handle, &trans);

            if(SystemP_SUCCESS == status)
            {
                MMCSD_halSetBusWidth(attrs->ctrlBaseAddr, width);
            }
        }
    }

    return status;
}

static int32_t MMCSD_initEMMC(MMCSD_Handle handle)
{
    int32_t status = SystemP_SUCCESS;
    uint64_t curTime, timeoutMilliSec;
    MMCSD_Object *obj = ((MMCSD_Config *)handle)->object;
    MMCSD_Attrs const *attrs = ((MMCSD_Config *)handle)->attrs;
    const CSL_mmc_sscfgRegs *pSSReg = (const CSL_mmc_sscfgRegs *)attrs->ssBaseAddr;
    const CSL_mmc_ctlcfgRegs *pReg = (const CSL_mmc_ctlcfgRegs *)attrs->ctrlBaseAddr;

    MMCSD_Transaction trans;

    if(SystemP_SUCCESS == status)
    {
        status = MMCSD_halSoftReset(attrs->ctrlBaseAddr);
    }

    if(SystemP_SUCCESS == status)
    {
        /* Set the bus width */
        status = MMCSD_halSetBusWidth(attrs->ctrlBaseAddr, MMCSD_BUS_WIDTH_1BIT);
    }

    if(SystemP_SUCCESS == status)
    {
        /* Set the bus voltage */
        status = MMCSD_halSetBusVolt(attrs->ctrlBaseAddr, MMCSD_BUS_VOLT_1_8V);
    }

    if(SystemP_SUCCESS == status)
    {
        /* Set the slot type to embedded */
        CSL_REG32_FINS(&pSSReg->CTL_CFG_2_REG,
            MMC_SSCFG_CTL_CFG_2_REG_SLOTTYPE,
            CSL_MMC_CTLCFG_CAPABILITIES_SLOT_TYPE_VAL_EMBEDDED);

        /* Wait for card detect */
        while(!MMCSD_halIsCardInserted(attrs->ctrlBaseAddr));

        /* Initialize the PHY */
        MMCSD_phyInit(attrs->ssBaseAddr, attrs->phyType);

        /* Switch on Bus Power */
        status = MMCSD_halBusPower(attrs->ctrlBaseAddr, CSL_MMC_CTLCFG_POWER_CONTROL_SD_BUS_POWER_VAL_PWR_ON);
    }

    if(SystemP_SUCCESS == status)
    {
        /* Set initialization frequency */
        status = MMCSD_halSetBusFreq(attrs->ctrlBaseAddr, attrs->inputClkFreq, 400000, FALSE);
    }

    /* Controller initialization done, moving to card init */
    if(SystemP_SUCCESS == status)
    {
        /* CMD0 - reset card */
        MMCSD_initTransaction(&trans);
        trans.cmd   = MMCSD_MMC_CMD(0);
        trans.arg   = 0U;
        status = MMCSD_transfer(handle, &trans);
    }

    /* Sleep for 5ms for input clock frequency, as mentioned
     * in JEDEC standard JESD84-B51 section 10.1
     */
    ClockP_usleep(5000);

    if(SystemP_SUCCESS == status)
    {
        /* OCR query */
        uint32_t hostOCR = 0U;

        /* Sector mode */
        hostOCR |= (2U << 29U);

        /* 1.7-1.95 VDD support */
        hostOCR |= (1U << 7U);

        /* Busy bit set */
        hostOCR |= (1U << 31U);

        if(CSL_REG64_FEXT(&pReg->CAPABILITIES, MMC_CTLCFG_CAPABILITIES_VOLT_3P3_SUPPORT) == TRUE)
        {
            /* 2.7-3.6 VDD support */
            hostOCR |= (0x1FFU << 15U);
        }

        /* High Speed support bit set */
        if(CSL_REG64_FEXT(&pReg->CAPABILITIES, MMC_SSCFG_CTL_CFG_2_REG_HIGHSPEEDSUPPORT) == TRUE)
        {
            CSL_REG32_FINS(&pSSReg->CTL_CFG_2_REG,MMC_SSCFG_CTL_CFG_2_REG_HIGHSPEEDSUPPORT, 1);
        }

        if(CSL_REG64_FEXT(&pReg->CAPABILITIES, MMC_CTLCFG_CAPABILITIES_VOLT_1P8_SUPPORT) == TRUE)
        {
            CSL_REG32_FINS(&pSSReg->CTL_CFG_2_REG, MMC_SSCFG_CTL_CFG_2_REG_SUPPORT1P8VOLT, 1);
        }
        if(CSL_REG64_FEXT(&pReg->CAPABILITIES, MMC_SSCFG_CTL_CFG_3_REG_HS400SUPPORT) == TRUE)
        {
            CSL_REG32_FINS(&pSSReg->CTL_CFG_3_REG, MMC_SSCFG_CTL_CFG_3_REG_HS400SUPPORT, 1);
        }
        if(CSL_REG64_FEXT(&pReg->CAPABILITIES, MMC_SSCFG_CTL_CFG_3_REG_DDR50SUPPORT) == TRUE)
        {
            CSL_REG32_FINS(&pSSReg->CTL_CFG_3_REG, MMC_SSCFG_CTL_CFG_3_REG_DDR50SUPPORT, 1);
        }

        /* Poll until card status bit is powered up */
        uint32_t retry = 0xFFFFU;

        MMCSD_initTransaction(&trans);
        trans.cmd = MMCSD_MMC_CMD(1);
        trans.arg = hostOCR;
        trans.retries = MMCSD_TRANS_RETRIES;

        status = MMCSD_transfer(handle, &trans);
        if(SystemP_SUCCESS == status)
        {
            while(((trans.response[0] >> 31)==0) && (retry != 0))
            {
                status = MMCSD_transfer(handle, &trans);
                retry--;
            }

            if(retry == 0U)
            {
                status = SystemP_FAILURE;
            }
        }
    }

    /* MMC should always support CMD23 */
    obj->isCmd23 = TRUE;

    if(SystemP_SUCCESS == status)
    {
        obj->emmcData->ocr = trans.response[0];
        obj->isHC = (obj->emmcData->ocr >> 30) & 0x01U;

        /* Get card identification register CID */
        MMCSD_initTransaction(&trans);
        trans.cmd = MMCSD_MMC_CMD(2);
        trans.arg = 0U;
        trans.retries = MMCSD_TRANS_RETRIES;

        status = MMCSD_transfer(handle, &trans);
    }

    if(SystemP_SUCCESS == status)
    {
        MMCSD_parseCIDEmmc(obj->emmcData, trans.response);

        /* Get RCA */
        MMCSD_initTransaction(&trans);
        obj->emmcData->rca = 2U;
        trans.cmd = MMCSD_MMC_CMD(3);
        trans.arg = ((obj->emmcData->rca) << 16U); /* RCA */
        trans.retries = MMCSD_TRANS_RETRIES;
        status = MMCSD_transfer(handle, &trans);
    }

    if(SystemP_SUCCESS == status)
    {
        /* Get CSD */
        MMCSD_initTransaction(&trans);
        trans.cmd = MMCSD_MMC_CMD(9);
        trans.arg = ((obj->emmcData->rca) << 16U);
        trans.retries = MMCSD_TRANS_RETRIES;
        status = MMCSD_transfer(handle, &trans);
    }

    if(SystemP_SUCCESS == status)
    {
        /* Simplify */
        MMCSD_parseCSDEmmc(obj->emmcData, trans.response);

        /* Check for spec version */
        if(obj->emmcData->specVersion != 0x04U)
        {
            status = SystemP_FAILURE;
        }
    }

    /* The 16-bit driver stage register (DSR). It can be optionally used to improve
     * the bus performance for extended operating conditions (depending on parameters
     * like bus length, transfer rate or number of Devices). The CSD register carries
     * the information about the DSR register usage. The default value of the DSR
     * register is 0x404.
     */
    if(SystemP_SUCCESS == status && obj->emmcData->impDsr)
    {
        MMCSD_initTransaction(&trans);
        trans.cmd = MMCSD_MMC_CMD(4);
        trans.arg = (obj->emmcData->dsr & 0xffff) << 16U;
        trans.retries = MMCSD_TRANS_RETRIES;
        status = MMCSD_transfer(handle, &trans);
    }

    if(status == SystemP_SUCCESS)
    {
        /* Select card */
        MMCSD_initTransaction(&trans);
        trans.cmd = MMCSD_MMC_CMD(7);
        trans.arg = (obj->emmcData->rca << 16U);
        trans.retries = MMCSD_TRANS_RETRIES;
        status = MMCSD_transfer(handle, &trans);
    }

    if(status == SystemP_SUCCESS)
    {
        /* Read ECSD register as data block */
        MMCSD_initTransaction(&trans);
        trans.cmd = MMCSD_MMC_CMD(8);
        trans.dir = MMCSD_CMD_XFER_TYPE_READ;
        trans.arg = (obj->emmcData->rca << 16U);
        trans.blockCount = 1U;
        trans.blockSize = 512U;
        trans.dataBuf = obj->tempDataBuf;
        trans.retries = MMCSD_TRANS_RETRIES;
        status = MMCSD_transfer(handle, &trans);
    }

    if(status == SystemP_SUCCESS)
    {
        MMCSD_parseECSDEmmc(obj->emmcData, obj->tempDataBuf);
        if(obj->tempDataBuf[MMCSD_ECSD_RST_N_INDEX] == MMCSD_ECSD_RST_N_TEMPORARILY_DISABLE)
        {
            timeoutMilliSec = MMCSD_DEFAULT_CMD6_TIMEOUT_MS;
            if(obj->tempDataBuf[MMCSD_GENERIC_CMD6_TIME_INDEX])
            {
                timeoutMilliSec = (obj->tempDataBuf[MMCSD_GENERIC_CMD6_TIME_INDEX]) * 10;
            }

            MMCSD_initTransaction(&trans);
            trans.cmd = MMCSD_MMC_CMD(6);
            trans.arg = (MMCSD_ECSD_ACCESS_MODE << 24U) | (MMCSD_ECSD_RST_N_INDEX << 16) | ((MMCSD_ECSD_RST_N_PERMANENTLY_ENABLE) << 8);
            trans.retries = MMCSD_TRANS_RETRIES;
            status = MMCSD_transfer(handle, &trans);

            if(SystemP_SUCCESS == status)
            {
                curTime = ClockP_getTimeUsec();
                while((CSL_REG32_FEXT(&pReg->PRESENTSTATE, MMC_CTLCFG_PRESENTSTATE_SDIF_DAT0IN) != 1U) &&
                ((ClockP_getTimeUsec() - curTime) < timeoutMilliSec * 1000))
                {
                    /* Do nothing. */
                }

                /* Read ECSD register as data block */
                MMCSD_initTransaction(&trans);
                trans.cmd = MMCSD_MMC_CMD(8);
                trans.dir = MMCSD_CMD_XFER_TYPE_READ;
                trans.arg = (obj->emmcData->rca << 16U);
                trans.blockCount = 1U;
                trans.blockSize = 512U;
                trans.dataBuf = obj->tempDataBuf;
                trans.retries = MMCSD_TRANS_RETRIES;
                status = MMCSD_transfer(handle, &trans);

                if(obj->tempDataBuf[MMCSD_ECSD_RST_N_INDEX] != MMCSD_ECSD_RST_N_PERMANENTLY_ENABLE)
                {
                    DebugP_logWarn("Unable to set RST_n_FUNC in ECSD due to timeout. \r\n");
                }
            }
        }

    }

    /* Set bus width in controller and device */
    uint32_t controllerBusWidth, ecsdBusWidth;

    if(attrs->busWidth & MMCSD_BUS_WIDTH_8BIT)
    {
        controllerBusWidth = MMCSD_BUS_WIDTH_8BIT;
        ecsdBusWidth = MMCSD_ECSD_BUS_WIDTH_8BIT;
    }
    else if(attrs->busWidth & MMCSD_BUS_WIDTH_4BIT)
    {
        controllerBusWidth = MMCSD_BUS_WIDTH_4BIT;
        ecsdBusWidth = MMCSD_ECSD_BUS_WIDTH_4BIT;
    }
    else if(attrs->busWidth & MMCSD_BUS_WIDTH_1BIT)
    {
        controllerBusWidth = MMCSD_BUS_WIDTH_1BIT;
        ecsdBusWidth = MMCSD_ECSD_BUS_WIDTH_1BIT;
    }
    else
    {
        controllerBusWidth = MMCSD_BUS_WIDTH_1BIT;
        ecsdBusWidth = MMCSD_ECSD_BUS_WIDTH_1BIT;
    }

    if(SystemP_SUCCESS == status)
    {
        MMCSD_initTransaction(&trans);
        trans.cmd = MMCSD_MMC_CMD(6);
        trans.arg = (MMCSD_ECSD_ACCESS_MODE << 24U) | (MMCSD_ECSD_BUS_WIDTH_INDEX << 16) | (((0 << MMCSD_ECSD_BUS_WIDTH_ES_SHIFT) | ecsdBusWidth) << 8);
        trans.retries = MMCSD_TRANS_RETRIES;
        status = MMCSD_transfer(handle, &trans);
    }

    obj->busWidth = controllerBusWidth;

    if(SystemP_SUCCESS == status)
    {
        MMCSD_halSetBusWidth(attrs->ctrlBaseAddr, controllerBusWidth);
    }

    status = MMCSD_isReadyForTransfer(handle);

    /* Find the highest mode supported by device and the controller */
    uint32_t mode = MMCSD_getModeEmmc(handle);

    if(mode != 0U)
    {
        status = MMCSD_switchEmmcMode(handle, mode);
    }

    if(status == SystemP_SUCCESS)
    {
        obj->transferSpeed = MMCSD_getXferSpeedFromModeEmmc(mode);
    }

    if(SystemP_SUCCESS != status)
    {
        MMCSD_close(handle);
    }

    return status;
}

static int32_t MMCSD_transfer(MMCSD_Handle handle, MMCSD_Transaction *trans)
{
    int32_t status = SystemP_SUCCESS;
    MMCSD_Object *obj = NULL;
    const MMCSD_Attrs *attrs = NULL;
    const CSL_mmc_ctlcfgRegs *pReg = NULL;

    if((handle != NULL) && (trans != NULL))
    {
        obj = ((MMCSD_Config *)handle)->object;
        attrs = ((MMCSD_Config *)handle)->attrs;
        pReg = (const CSL_mmc_ctlcfgRegs *)(attrs->ctrlBaseAddr);

        if(obj->intrEnable == TRUE)
        {
            SemaphoreP_pend(&obj->cmdCompleteSemObj, SystemP_WAIT_FOREVER);
        }
    }
    else
    {
        status = SystemP_FAILURE;
    }

    if(SystemP_SUCCESS == status)
    {
        /* Check for interrupt enable */
        if(obj->intrEnable == TRUE)
        {
            MMCSD_halNormalSigIntrDisable(attrs->ctrlBaseAddr, CSL_MMC_CTLCFG_NORMAL_INTR_STS_ENA_BUF_WR_READY_MASK);
            MMCSD_halNormalSigIntrDisable(attrs->ctrlBaseAddr, CSL_MMC_CTLCFG_NORMAL_INTR_STS_ENA_BUF_RD_READY_MASK);
            MMCSD_halNormalSigIntrDisable(attrs->ctrlBaseAddr, CSL_MMC_CTLCFG_NORMAL_INTR_STS_ENA_XFER_COMPLETE_MASK);
        }
        else
        {
            MMCSD_halNormalSigIntrDisable(attrs->ctrlBaseAddr, MMCSD_INTERRUPT_ALL_NORMAL);
        }

        obj->cmdComp = 0;
        obj->cmdTimeout = 0;
        obj->cmdCRCError = 0;
        obj->cmdIndexError = 0;
        obj->cmdEBError = 0;
        obj->dataTimeoutError = 0;
        obj->dataCRCError = 0;
        obj->dataEBError = 0;
        obj->cmdError = 0;
        obj->xferInProgress = 0;
        obj->xferComp = 0;
        obj->xferTimeout = 0;

        if(trans->cmd & CSL_MMC_CTLCFG_COMMAND_DATA_PRESENT_MASK)
        {
            SemaphoreP_pend(&(obj->cmdMutex), SystemP_WAIT_FOREVER);
            SemaphoreP_pend(&(obj->xferMutex), SystemP_WAIT_FOREVER);

            /* Clear all interrupt status flags */
            MMCSD_halNormalIntrStatusClear(attrs->ctrlBaseAddr, MMCSD_INTERRUPT_ALL_NORMAL);
            MMCSD_halErrorIntrStatusClear(attrs->ctrlBaseAddr, MMCSD_INTERRUPT_ALL_ERROR);

            obj->dataBufIdx = (uint8_t *)trans->dataBuf;
            obj->dataBlockCount = trans->blockCount;
            obj->dataBlockSize = trans->blockSize;

            if(trans->dir == MMCSD_CMD_XFER_TYPE_READ)
            {
                MMCSD_halNormalIntrStatusEnable(attrs->ctrlBaseAddr, CSL_MMC_CTLCFG_NORMAL_INTR_STS_ENA_BUF_RD_READY_MASK);
                MMCSD_halNormalIntrStatusDisable(attrs->ctrlBaseAddr, CSL_MMC_CTLCFG_NORMAL_INTR_STS_ENA_BUF_WR_READY_MASK);
                obj->readBlockCount = obj->dataBlockCount;
            }
            else
            {
                MMCSD_halNormalIntrStatusEnable(attrs->ctrlBaseAddr, CSL_MMC_CTLCFG_NORMAL_INTR_STS_ENA_BUF_WR_READY_MASK);
                MMCSD_halNormalIntrStatusDisable(attrs->ctrlBaseAddr, CSL_MMC_CTLCFG_NORMAL_INTR_STS_ENA_BUF_RD_READY_MASK);

                if(obj->intrEnable == TRUE)
                {
                    MMCSD_halNormalSigIntrDisable(attrs->ctrlBaseAddr, CSL_MMC_CTLCFG_NORMAL_INTR_SIG_ENA_BUF_WR_READY_MASK);
                    MMCSD_halNormalSigIntrDisable(attrs->ctrlBaseAddr, CSL_MMC_CTLCFG_NORMAL_INTR_SIG_ENA_BUF_RD_READY_MASK);
                    MMCSD_halNormalSigIntrDisable(attrs->ctrlBaseAddr, CSL_MMC_CTLCFG_NORMAL_INTR_SIG_ENA_XFER_COMPLETE_MASK);
                }
                obj->writeBlockCount = obj->dataBlockCount;
            }

            /* Set block length */
            CSL_REG16_FINS(&pReg->BLOCK_SIZE, MMC_CTLCFG_BLOCK_SIZE_XFER_BLK_SIZE, trans->blockSize);

            /* Set block count */
            CSL_REG16_WR(&pReg->BLOCK_COUNT, trans->blockCount);

            MMCSD_halNormalIntrStatusEnable(attrs->ctrlBaseAddr,
                CSL_MMC_CTLCFG_NORMAL_INTR_STS_ENA_CMD_COMPLETE_MASK | CSL_MMC_CTLCFG_NORMAL_INTR_STS_ENA_XFER_COMPLETE_MASK);
            MMCSD_halErrorIntrStatusEnable(attrs->ctrlBaseAddr, MMCSD_INTERRUPT_ALL_ERROR);

            if(obj->intrEnable == TRUE)
            {
                MMCSD_halNormalSigIntrEnable(attrs->ctrlBaseAddr,
                    CSL_MMC_CTLCFG_NORMAL_INTR_SIG_ENA_CMD_COMPLETE_MASK);
                MMCSD_halErrorSigIntrEnable(attrs->ctrlBaseAddr,
                    CSL_MMC_CTLCFG_ERROR_INTR_SIG_ENA_CMD_TIMEOUT_MASK | CSL_MMC_CTLCFG_ERROR_INTR_SIG_ENA_DATA_TIMEOUT_MASK);
            }

            CacheP_wbInv(obj->dataBufIdx, (trans->blockSize * trans->blockCount), CacheP_TYPE_ALL);

            if(obj->enableDma == TRUE)
            {
                /* Setup ADMA2 descriptor */
                uint32_t dataSize = trans->blockCount * trans->blockSize;

                status = MMCSD_setupADMA2(handle, &gADMA2Desc, (uint64_t)trans->dataBuf, dataSize);

                if(status == SystemP_SUCCESS)
                {
                    trans->enableDma = 1U;
                }
                else
                {
                    trans->enableDma = 0U;
                }
            }
            else
            {
                trans->enableDma = 0U;
            }
            /* Wait for CMD and DATA inhibit to go low */
            while(CSL_REG32_FEXT(&pReg->PRESENTSTATE, MMC_CTLCFG_PRESENTSTATE_INHIBIT_CMD) != 0U);
            while(CSL_REG32_FEXT(&pReg->PRESENTSTATE, MMC_CTLCFG_PRESENTSTATE_INHIBIT_DAT) != 0U);

            if(obj->intrEnable == TRUE)
            {
                if(trans->dir == MMCSD_CMD_XFER_TYPE_READ)
                {
                    MMCSD_halNormalSigIntrEnable(attrs->ctrlBaseAddr, CSL_MMC_CTLCFG_NORMAL_INTR_SIG_ENA_BUF_RD_READY_MASK);
                }
                else
                {
                    MMCSD_halNormalSigIntrEnable(attrs->ctrlBaseAddr, CSL_MMC_CTLCFG_NORMAL_INTR_SIG_ENA_BUF_WR_READY_MASK);
                }
                MMCSD_halNormalSigIntrEnable(attrs->ctrlBaseAddr, CSL_MMC_CTLCFG_NORMAL_INTR_SIG_ENA_XFER_COMPLETE_MASK);
            }

            MMCSD_halSendCommand(attrs->ctrlBaseAddr, trans);

            /* Wait for transfer to complete */
            if((!obj->isManualTuning) && (trans->isTuning == TRUE))
            {
                obj->cmdComp = TRUE;
            }
            else
            {
                if(obj->intrEnable == TRUE)
                {
                    SemaphoreP_pend(&obj->cmdCompleteSemObj, SystemP_WAIT_FOREVER);
                }
                else
                {
                    while((obj->cmdComp == FALSE) && (obj->cmdError == FALSE))
                    {
                        MMCSD_cmdStatusPollingFxn(handle);
                    }
                }
            }

            if((obj->isManualTuning == TRUE) && (obj->cmdError) && (trans->isTuning == TRUE))
            {
                status = MMCSD_halLinesResetCmd(attrs->ctrlBaseAddr);
                status = MMCSD_halLinesResetDat(attrs->ctrlBaseAddr);

                /* Tuning failed */
                status = SystemP_FAILURE;
            }
            else
            {
                /* Check for CMD errors */
                if(obj->cmdTimeout == TRUE)
                {
                    status = SystemP_FAILURE;
                    obj->cmdTimeout = FALSE;
                }

                if(obj->cmdCRCError == TRUE)
                {
                    status = SystemP_FAILURE;
                    obj->cmdCRCError = FALSE;
                }
            }

            /* Check for command execution */
            if(obj->cmdComp == TRUE)
            {
                status = SystemP_SUCCESS;
                obj->cmdComp = FALSE;

                if(obj->intrEnable == FALSE)
                {
                    obj->xferInProgress = TRUE;
                }
                else
                {
                    SemaphoreP_pend(&obj->dataCopyCompleteSemObj, SystemP_WAIT_FOREVER);
                }

                /* Get command response and update book keeping */
                MMCSD_halCmdResponseGet(attrs->ctrlBaseAddr, trans->response);
            }

            SemaphoreP_post(&obj->cmdMutex);

            if(SystemP_SUCCESS == status)
            {
                if(obj->intrEnable == TRUE)
                {
                    SemaphoreP_pend(&obj->xferCompleteSemObj, SystemP_WAIT_FOREVER);
                }
                else
                {
                    if((obj->isManualTuning == FALSE) && (trans->isTuning == TRUE))
                    {
                        while((obj->xferComp == FALSE) && (obj->xferTimeout == FALSE))
                        {
                            MMCSD_xferStatusPollingFxnCMD19(handle);
                        }
                    }
                    else
                    {
                        while((obj->cmdError == FALSE) &&
                              (obj->xferComp == FALSE) &&
                              (obj->xferTimeout == FALSE) &&
                              (obj->dataCRCError == FALSE) &&
                              (obj->dataEBError == FALSE))
                        {
                            MMCSD_xferStatusPollingFxn(handle);
                        }
                    }
                }
            }

            if((obj->isManualTuning == TRUE) && (obj->cmdError) && (trans->isTuning == TRUE))
            {
                MMCSD_halLinesResetCmd(attrs->ctrlBaseAddr);
                MMCSD_halLinesResetDat(attrs->ctrlBaseAddr);
                status = SystemP_FAILURE;
            }
            else
            {
                /* Check for data transfer */
                if(obj->xferTimeout == TRUE)
                {
                    status = SystemP_FAILURE;
                    obj->xferTimeout = FALSE;
                }

                if(obj->xferComp == TRUE)
                {
                    status = SystemP_SUCCESS;
                    obj->xferComp = FALSE;
                }
            }

            SemaphoreP_post(&obj->xferMutex);
        }
        else
        {
            /* Data not enabled, only command */
            SemaphoreP_pend(&obj->cmdMutex, SystemP_WAIT_FOREVER);

            /* Clear all interrupt status flags */
            MMCSD_halNormalIntrStatusClear(attrs->ctrlBaseAddr, MMCSD_INTERRUPT_ALL_NORMAL);
            MMCSD_halErrorIntrStatusClear(attrs->ctrlBaseAddr, MMCSD_INTERRUPT_ALL_ERROR);

            obj->cmdComp = FALSE;
            obj->cmdTimeout = FALSE;
            obj->cmdError = FALSE;
            obj->cmdCRCError = FALSE;

            MMCSD_halNormalIntrStatusEnable(attrs->ctrlBaseAddr, CSL_MMC_CTLCFG_NORMAL_INTR_STS_CMD_COMPLETE_MASK);
            MMCSD_halErrorIntrStatusEnable(attrs->ctrlBaseAddr, CSL_MMC_CTLCFG_ERROR_INTR_STS_CMD_TIMEOUT_MASK);

            if(obj->intrEnable == TRUE)
            {
                MMCSD_halNormalSigIntrEnable(attrs->ctrlBaseAddr, CSL_MMC_CTLCFG_NORMAL_INTR_SIG_ENA_CMD_COMPLETE_MASK);
                MMCSD_halErrorSigIntrEnable(attrs->ctrlBaseAddr, CSL_MMC_CTLCFG_ERROR_INTR_SIG_ENA_CMD_TIMEOUT_MASK);
            }
            else
            {
                MMCSD_halNormalSigIntrDisable(attrs->ctrlBaseAddr, MMCSD_INTERRUPT_ALL_NORMAL);
                MMCSD_halErrorSigIntrDisable(attrs->ctrlBaseAddr, MMCSD_INTERRUPT_ALL_ERROR);
            }

            /* Wait for command inhibit to go low */
            while(CSL_REG32_FEXT(&pReg->PRESENTSTATE, MMC_CTLCFG_PRESENTSTATE_INHIBIT_CMD) != 0U);

            MMCSD_halSendCommand(attrs->ctrlBaseAddr, trans);

            /* Wait for transfer to complete */
            if(obj->intrEnable == TRUE)
            {
                SemaphoreP_pend(&obj->cmdCompleteSemObj, SystemP_WAIT_FOREVER);
            }
            else
            {
                while((obj->cmdComp == FALSE) && (obj->cmdTimeout == FALSE))
                {
                    MMCSD_cmdStatusPollingFxn(handle);
                }
            }

            if(obj->cmdComp == TRUE)
            {
                status = SystemP_SUCCESS;
                obj->cmdComp = FALSE;
            }

            /* Get response for command */
            MMCSD_halCmdResponseGet(attrs->ctrlBaseAddr, trans->response);

            SemaphoreP_post(&obj->cmdMutex);
        }
    }
    
    if(status == SystemP_SUCCESS)
    {
        status = MMCSD_errorRecovery(handle, trans);                    
    }

    return status;
}

static uint32_t MMCSD_getModeEmmc(MMCSD_Handle handle)
{
    uint32_t mode = 0U;
    MMCSD_Object *obj = ((MMCSD_Config *)handle)->object;
    MMCSD_Attrs const *attrs = ((MMCSD_Config *)handle)->attrs;

    uint32_t deviceModes = obj->emmcData->supportedModes;
    uint32_t controllerModes = attrs->supportedModes;

    if((deviceModes & MMCSD_EMMC_ECSD_DEVICE_TYPE_HS400_200MHZ_1P8V) &&
       (controllerModes & MMCSD_SUPPORT_MMC_HS400))
    {
        mode = MMCSD_SUPPORT_MMC_HS400;
    }
    else if((deviceModes & MMCSD_EMMC_ECSD_DEVICE_TYPE_HS200_200MHZ_1P8V) &&
       (controllerModes & MMCSD_SUPPORT_MMC_HS200))
    {
        mode = MMCSD_SUPPORT_MMC_HS200;
    }
    else if((deviceModes & MMCSD_EMMC_ECSD_DEVICE_TYPE_HS_DDR_52MHZ_1P8V) &&
      (controllerModes & MMCSD_SUPPORT_MMC_HS_DDR))
    {
        mode = MMCSD_SUPPORT_MMC_HS_DDR;
    }
    else if((deviceModes & MMCSD_EMMC_ECSD_DEVICE_TYPE_HS_52MHZ) &&
       (controllerModes & MMCSD_SUPPORT_MMC_HS_SDR))
    {
        mode = MMCSD_SUPPORT_MMC_HS_SDR;
    }
    else if(controllerModes & MMCSD_SUPPORT_MMC_DS)
    {
        mode = MMCSD_SUPPORT_MMC_DS;
    }
    else
    {
        mode = 0U;
    }

    return mode;
}

static uint32_t MMCSD_getXferSpeedFromModeEmmc(uint32_t mode)
{
    uint32_t speed = 0U;

    switch(mode)
    {
        case MMCSD_SUPPORT_MMC_DS:
            speed = MMCSD_TRANSPEED_SDR25;
            break;
        case MMCSD_SUPPORT_MMC_HS_SDR:
            speed = MMCSD_TRANSPEED_SDR50;
            break;
        case MMCSD_SUPPORT_MMC_HS_DDR:
            speed = MMCSD_TRANSPEED_DDR50;
            break;
        case MMCSD_SUPPORT_MMC_HS200:
            speed = MMCSD_TRANSPEED_HS200;
            break;
        case MMCSD_SUPPORT_MMC_HS400:
            speed = MMCSD_TRANSPEED_HS400;
            break;
        default:
            speed = 0U;
            break;
    }

    return speed;
}

static int32_t MMCSD_isReadyForTransfer(MMCSD_Handle handle)
{
    int32_t status = SystemP_SUCCESS;
    uint32_t readyCheckTryCount = 0U;
    MMCSD_Object *obj = ((MMCSD_Config *)handle)->object;
    MMCSD_Transaction trans;

    uint32_t mediaCurrentState = 0U;

    while((mediaCurrentState != MMCSD_MEDIA_STATE_TRANSFER) && (readyCheckTryCount < MMCSD_MEDIA_STATE_THRESHOLD))
    {
        MMCSD_initTransaction(&trans);
        if(obj->cardType == MMCSD_CARD_TYPE_EMMC)
        {
            trans.cmd = MMCSD_MMC_CMD(13);
            trans.arg = (obj->emmcData->rca << 16U);
            trans.retries = MMCSD_TRANS_RETRIES;
        }
        else
        {
            trans.cmd = MMCSD_SD_CMD(13);
            trans.arg = (obj->sdData->rca << 16U);
            trans.retries = MMCSD_TRANS_RETRIES;
        }

        status = MMCSD_transfer(handle, &trans);
        readyCheckTryCount++;
        mediaCurrentState = ((trans.response[0] >> 9U) & 0x0FU);
    }

    if(readyCheckTryCount == MMCSD_MEDIA_STATE_THRESHOLD)
    {
        status = SystemP_TIMEOUT;
    }

    return status;
}

static int32_t MMCSD_setupADMA2(MMCSD_Handle handle, MMCSD_ADMA2Descriptor *desc, uint64_t bufAddr, uint32_t dataSize)
{
    int32_t status = SystemP_SUCCESS;
    MMCSD_Object *obj = NULL;
    const MMCSD_Attrs *attrs = NULL;
    const CSL_mmc_ctlcfgRegs *pReg = NULL;
    uint32_t dmaParams = 0U;
    uint16_t regVal = 0U;

    if((desc == NULL) || (handle == NULL))
    {
        status = SystemP_FAILURE;
    }
    if (SystemP_SUCCESS == status)
    {
        attrs = ((MMCSD_Config *)handle)->attrs;
        obj = ((MMCSD_Config *)handle)->object;

        if ((attrs == NULL) || (obj == NULL))
        {
            status = SystemP_FAILURE;
        }
        else
        {
            pReg = (const CSL_mmc_ctlcfgRegs *)(attrs->ctrlBaseAddr);
        }
        if (pReg == NULL)
        {
            status = SystemP_FAILURE;
        }
    }
    if (SystemP_SUCCESS == status)
    {

        dmaParams = dataSize << 16U;
        dmaParams |= (((dataSize >> 16U) << 6U) | 0x0023U);

        /* Enable version 4 for 26 bit sizes */
        CSL_REG16_FINS(&pReg->HOST_CONTROL2, MMC_CTLCFG_HOST_CONTROL2_HOST_VER40_ENA, 1U);
        CSL_REG16_FINS(&pReg->HOST_CONTROL2, MMC_CTLCFG_HOST_CONTROL2_ADMA2_LEN_MODE, 1U);
        if(obj->xferHighSpeedEn == 1)
        {
        regVal = CSL_FMK(MMC_CTLCFG_HOST_CONTROL2_UHS_MODE_SELECT, obj->uhsmode) |
                 CSL_FMK(MMC_CTLCFG_HOST_CONTROL2_ADMA2_LEN_MODE, 1)  ;
        CSL_REG16_WR(&pReg->HOST_CONTROL2, regVal);
        }

        /* Setup ADMA2 descriptor */
        desc->dmaParams = dmaParams;
        desc->addrLo    = (uint64_t)bufAddr;
        desc->addrHi    = ((uint64_t)bufAddr >> 32) & 0xFFFFU;

        /* Set 32 bit ADMA2 */
        CSL_REG8_FINS(&pReg->HOST_CONTROL1, MMC_CTLCFG_HOST_CONTROL1_DMA_SELECT, 2U);
        if(obj->xferHighSpeedEn == 1)
        {
            CSL_REG8_WR(&pReg->HOST_CONTROL1, ((1 << CSL_MMC_CTLCFG_HOST_CONTROL1_EXT_DATA_WIDTH_SHIFT) |
                       (2 << CSL_MMC_CTLCFG_HOST_CONTROL1_DMA_SELECT_SHIFT)));
            obj->xferHighSpeedEn = 0;
        }

        /* Write the descriptor address to ADMA2 Address register */
        CSL_REG64_WR(&pReg->ADMA_SYS_ADDRESS, (uint64_t)desc);

        CacheP_wbInv(desc, sizeof(MMCSD_ADMA2Descriptor), CacheP_TYPE_ALL);
    }
    else
    {
        status = SystemP_FAILURE;
    }

    return status;
}

static int32_t MMCSD_sendStopCmd(MMCSD_Handle handle)
{
    int32_t status = SystemP_SUCCESS;
    MMCSD_Object *obj = ((MMCSD_Config *)handle)->object;
    MMCSD_Transaction trans;
    uint32_t stopCmd = 0U;

    if(obj->cardType == MMCSD_CARD_TYPE_EMMC)
    {
        stopCmd = MMCSD_MMC_CMD(12);
    }
    else
    {
        stopCmd = MMCSD_SD_CMD(12);
    }

    MMCSD_initTransaction(&trans);
    trans.cmd = stopCmd;
    trans.arg = 0;
    trans.retries = MMCSD_TRANS_RETRIES;
    status = MMCSD_transfer(handle, &trans);

    return status;
}

static int32_t MMCSD_sendCmd23(MMCSD_Handle handle, uint32_t numBlks)
{
    int32_t status = SystemP_SUCCESS;
    MMCSD_Transaction trans;

    MMCSD_initTransaction(&trans);
    trans.cmd = MMCSD_MMC_CMD(23);
    trans.arg = numBlks;
    trans.retries = MMCSD_TRANS_RETRIES;
    status = MMCSD_transfer(handle, &trans);

    return status;
}

static void MMCSD_initTransaction(MMCSD_Transaction *trans)
{
    if(trans != NULL)
    {
        memset(trans, 0, sizeof(MMCSD_Transaction));
        trans->blockSize = 512U;
        trans->blockCount = 1U;
        trans->retries = 0U;
    }
}

static int32_t MMCSD_switchEmmcMode(MMCSD_Handle handle, uint32_t mode)
{
    int32_t status = SystemP_SUCCESS;
    uint32_t hsTimingVal = 0U;
    uint32_t phyClkFreq = 26000000, clkFreq = 26000000;
    uint32_t uhsMode = MMCSD_UHS_MODE_SDR12;
    uint32_t phyDriverType = 0;
    uint32_t phyMode = MMCSD_PHY_MODE_DS;
    uint32_t tuningRequired = FALSE;
    uint8_t tunedItap = 0U;
    uint32_t ddrMode = FALSE;
    uint32_t es = 0U;
    MMCSD_Object *obj = ((MMCSD_Config *)handle)->object;
    const MMCSD_Attrs *attrs = ((MMCSD_Config *)handle)->attrs;
    const CSL_mmc_ctlcfgRegs *pReg = (const CSL_mmc_ctlcfgRegs *)(attrs->ctrlBaseAddr);
    MMCSD_Transaction trans;

    if((mode == MMCSD_SUPPORT_MMC_HS200) || (mode == MMCSD_SUPPORT_MMC_HS400))
    {
        hsTimingVal = MMCSD_ECSD_HS_TIMING_HS200;
        clkFreq = MMCSD_REFERENCE_CLOCK_200M;
        phyMode = MMCSD_PHY_MODE_HS200;
        tuningRequired = TRUE;
        uhsMode = MMCSD_UHS_MODE_SDR104;
        obj->uhsmode = MMCSD_UHS_MODE_SDR104;
    }
    else if((mode == MMCSD_SUPPORT_MMC_HS_SDR) || (mode == MMCSD_SUPPORT_MMC_HS_DDR))
    {
        hsTimingVal = MMCSD_ECSD_HS_TIMING_HIGH_SPEED;
        tuningRequired = FALSE;
        clkFreq = MMCSD_REFERENCE_CLOCK_52M;
        if(mode == MMCSD_SUPPORT_MMC_HS_DDR)
        {
            ddrMode = TRUE;
            uhsMode = MMCSD_UHS_MODE_DDR50;
            phyMode = MMCSD_PHY_MODE_HSDDR50;
            obj->uhsmode = MMCSD_UHS_MODE_DDR50;
        }
        else
        {
            ddrMode = FALSE;
            uhsMode = MMCSD_UHS_MODE_SDR50;
            phyMode = MMCSD_PHY_MODE_HSSDR50;
            obj->uhsmode = MMCSD_UHS_MODE_SDR50;
        }
    }
    else
    {
        phyMode = MMCSD_PHY_MODE_DS;
        hsTimingVal = MMCSD_ECSD_HS_TIMING_BACKWARD_COMPATIBLE;
        tuningRequired = FALSE;
        clkFreq = 26*1000000;
    }

    MMCSD_initTransaction(&trans);
    trans.cmd = MMCSD_MMC_CMD(6);
    trans.arg = (MMCSD_ECSD_ACCESS_MODE << 24U) | (MMCSD_ECSD_HS_TIMING_INDEX << 16U) | ((((obj->emmcData->driveStrength) << 4U) | hsTimingVal) << 8U);
    trans.retries = MMCSD_TRANS_RETRIES;
    status = MMCSD_transfer(handle, &trans);

    if(SystemP_SUCCESS == status)
    {
        while(CSL_REG32_FEXT(&pReg->PRESENTSTATE, MMC_CTLCFG_PRESENTSTATE_SDIF_DAT0IN) != 1U);
    }

    if(ddrMode == TRUE)
    {
        uint8_t ecsdBusWidth;

        if(obj->busWidth == MMCSD_BUS_WIDTH_8BIT)
        {
            ecsdBusWidth = MMCSD_ECSD_BUS_WIDTH_8BIT_DDR;
        }
        else
        {
            ecsdBusWidth = MMCSD_ECSD_BUS_WIDTH_4BIT_DDR;
        }
        MMCSD_initTransaction(&trans);
        trans.cmd = MMCSD_MMC_CMD(6);
        trans.arg = (MMCSD_ECSD_ACCESS_MODE << 24U) | (MMCSD_ECSD_BUS_WIDTH_INDEX << 16) | (((es << MMCSD_ECSD_BUS_WIDTH_ES_SHIFT) | ecsdBusWidth) << 8);
        trans.retries = MMCSD_TRANS_RETRIES;
        status = MMCSD_transfer(handle, &trans);


        if(SystemP_SUCCESS == status)
        {
            while(CSL_REG32_FEXT(&pReg->PRESENTSTATE, MMC_CTLCFG_PRESENTSTATE_SDIF_DAT0IN) != 1U);
        }

        phyClkFreq = MMCSD_REFERENCE_CLOCK_52M;
    }
    else
    {
        phyClkFreq = clkFreq;
    }

    /* Configure the HC */
    MMCSD_halSetUHSMode(attrs->ctrlBaseAddr, uhsMode);

    /* Disable PHY DLL */
    MMCSD_phyDisableDLL(attrs->ssBaseAddr);

    status = MMCSD_halSetBusFreq(attrs->ctrlBaseAddr, attrs->inputClkFreq, clkFreq, 0U);

    if(SystemP_SUCCESS == status)
    {

        /* Enable DLL */
        MMCSD_phyConfigure(attrs->ssBaseAddr, phyMode, phyClkFreq, phyDriverType, tunedItap);

        /* Tune the PHY */
        if(attrs->tuningType == MMCSD_PHY_TUNING_TYPE_AUTO)
        {
            obj->isManualTuning = FALSE;
        }
        else
        {
            obj->isManualTuning = TRUE;
        }

        if(tuningRequired)
        {
            if(obj->isManualTuning == TRUE)
            {
                status = MMCSD_phyTuneManual(handle, &tunedItap, 0U);
            }
            else
            {
                status = MMCSD_phyTuneAuto(handle);
            }
        }
    }

    if(mode == MMCSD_SUPPORT_MMC_HS400)
    {
        phyMode = MMCSD_PHY_MODE_HS;
        MMCSD_phyConfigure(attrs->ssBaseAddr, phyMode, phyClkFreq, phyDriverType, tunedItap);

        hsTimingVal = MMCSD_ECSD_HS_TIMING_HIGH_SPEED;
        MMCSD_initTransaction(&trans);
        trans.cmd   = MMCSD_MMC_CMD(6);
        trans.arg   = (MMCSD_ECSD_ACCESS_MODE << 24U) | (MMCSD_ECSD_HS_TIMING_INDEX << 16U) | (((es << 4U) | hsTimingVal) << 8U);
        trans.retries = MMCSD_TRANS_RETRIES;
        status = MMCSD_transfer(handle, &trans);

        if(status == SystemP_SUCCESS)
        {
            /* Wait for DAT0 to go low */
            while(CSL_REG32_FEXT(&pReg->PRESENTSTATE, MMC_CTLCFG_PRESENTSTATE_SDIF_DAT0IN) != 1U);
        }

        /* Disable PHY DLL */
        MMCSD_phyDisableDLL(attrs->ssBaseAddr);

        status = MMCSD_halSetBusFreq(attrs->ctrlBaseAddr, attrs->inputClkFreq, MMCSD_REFERENCE_CLOCK_52M, 0U);

        if(SystemP_SUCCESS == status)
        {
            phyMode = MMCSD_PHY_MODE_DDR50;

            MMCSD_phyConfigure(attrs->ssBaseAddr, phyMode, phyClkFreq, phyDriverType, tunedItap);

            /* Set bus width to 0x06 to select DDR 8-bit bus mode */
            MMCSD_initTransaction(&trans);
            trans.cmd   = MMCSD_MMC_CMD(6);
            trans.arg   = (MMCSD_ECSD_ACCESS_MODE << 24U) | (MMCSD_ECSD_BUS_WIDTH_INDEX << 16U) | (((es << MMCSD_ECSD_BUS_WIDTH_ES_SHIFT) | MMCSD_ECSD_BUS_WIDTH_8BIT_DDR) << 8U);
            trans.retries = MMCSD_TRANS_RETRIES;
            status = MMCSD_transfer(handle, &trans);


            if(status == SystemP_SUCCESS)
            {
                /* Wait for DAT0 to go low */
                while(CSL_REG32_FEXT(&pReg->PRESENTSTATE, MMC_CTLCFG_PRESENTSTATE_SDIF_DAT0IN) != 1U);
            }

            if(status == SystemP_SUCCESS)
            {
                /* Change HS timing to set HS400 */
                MMCSD_initTransaction(&trans);
                trans.cmd = MMCSD_MMC_CMD(6);
                trans.arg = (MMCSD_ECSD_ACCESS_MODE << 24U) | (MMCSD_ECSD_HS_TIMING_INDEX << 16U) | (((es << 4U) | MMCSD_ECSD_HS_TIMING_HS400) << 8U);
                trans.retries = MMCSD_TRANS_RETRIES;
                status = MMCSD_transfer(handle, &trans);
            }

            if(status == SystemP_SUCCESS)
            {
                /* Wait for DAT0 to go low */
                while(CSL_REG32_FEXT(&pReg->PRESENTSTATE, MMC_CTLCFG_PRESENTSTATE_SDIF_DAT0IN) != 1U);
            }

            if(status == SystemP_SUCCESS)
            {
                obj->uhsmode = MMCSD_UHS_MODE_HS400;
                status = MMCSD_halSetUHSMode(attrs->ctrlBaseAddr, MMCSD_UHS_MODE_HS400);
            }

            MMCSD_phyDisableDLL(attrs->ssBaseAddr);

            /* Set O/P clock to 200 MHz. HC may set it to a value <= 200 MHz */
            status = MMCSD_halSetBusFreq(attrs->ctrlBaseAddr, attrs->inputClkFreq, MMCSD_REFERENCE_CLOCK_200M, 0U);

            phyMode = MMCSD_PHY_MODE_HS400;

            MMCSD_phyConfigure(attrs->ssBaseAddr, phyMode, MMCSD_REFERENCE_CLOCK_200M, phyDriverType, tunedItap);
        }
    }

    return status;
}

static int32_t MMCSD_errorRecovery(MMCSD_Handle handle, MMCSD_Transaction *trans)
{
    int32_t status = SystemP_SUCCESS;
    MMCSD_Object *obj = ((MMCSD_Config *)handle)->object;
    const MMCSD_Attrs *attrs = ((MMCSD_Config *)handle)->attrs;

    if(obj->dataCRCError || obj->cmdCRCError)
    {
        MMCSD_halLinesResetCmd(attrs->ctrlBaseAddr);
        MMCSD_halLinesResetDat(attrs->ctrlBaseAddr);

        while(status == SystemP_SUCCESS && trans->retries > 0U && (obj->dataCRCError || obj->cmdCRCError))
        {
            status = MMCSD_retune(handle);
            if(status == SystemP_SUCCESS)
            {
                trans->retries = trans->retries - 1U;
                status = MMCSD_transfer(handle, trans);
            }
        }

        if(trans->retries == 0U)
        {
            status = SystemP_FAILURE;
        }
    }
    else if(obj->cmdTimeout || obj->cmdIndexError || obj->cmdEBError ||
        obj->dataTimeoutError || obj->dataEBError)
    {
        MMCSD_halLinesResetCmd(attrs->ctrlBaseAddr);
        MMCSD_halLinesResetDat(attrs->ctrlBaseAddr);

        while(status == SystemP_SUCCESS && trans->retries > 0U && (obj->cmdTimeout || obj->cmdIndexError || 
        obj->cmdEBError || obj->dataTimeoutError || obj->dataEBError))
        {
            trans->retries = trans->retries - 1U;
            status = MMCSD_transfer(handle, trans);
        }
        
        if(trans->retries == 0U)
        {
            status = SystemP_FAILURE;
        }
    }

    return status;
}

static int32_t MMCSD_retune(MMCSD_Handle handle)
{
    int32_t status = SystemP_SUCCESS;
    uint32_t hsTimingVal = 0U;
    uint32_t phyClkFreq = 26000000, clkFreq = 26000000;
    uint32_t phyDriverType = 0;
    uint32_t phyMode = MMCSD_PHY_MODE_DS;
    uint8_t tunedItap = 0U;
    const MMCSD_Attrs *attrs = ((MMCSD_Config *)handle)->attrs;
    const CSL_mmc_ctlcfgRegs *pReg = (const CSL_mmc_ctlcfgRegs *)(attrs->ctrlBaseAddr);
    MMCSD_Transaction trans;
    uint32_t mode = MMCSD_getModeEmmc(handle);

    if(mode == MMCSD_SUPPORT_MMC_HS400)
    {
        /* Switch from HS400 mode to HS200 mode and 
         * then from HS200 mode to HS400 mode.
         */

        phyMode = MMCSD_PHY_MODE_HS;
        clkFreq = MMCSD_REFERENCE_CLOCK_52M;
        phyClkFreq = clkFreq;
        MMCSD_phyConfigure(attrs->ssBaseAddr, phyMode, phyClkFreq, phyDriverType, tunedItap);

        hsTimingVal = MMCSD_ECSD_HS_TIMING_HIGH_SPEED;
        MMCSD_initTransaction(&trans);
        trans.cmd   = MMCSD_MMC_CMD(6);
        trans.arg   = (MMCSD_ECSD_ACCESS_MODE << 24U) | (MMCSD_ECSD_HS_TIMING_INDEX << 16U) | (hsTimingVal << 8U);
        status = MMCSD_transfer(handle, &trans);

        if(status == SystemP_SUCCESS)
        {
            /* Wait for DAT0 to go low */
            while(CSL_REG32_FEXT(&pReg->PRESENTSTATE, MMC_CTLCFG_PRESENTSTATE_SDIF_DAT0IN) != 1U);
        }

        /* Disable PHY DLL */
        MMCSD_phyDisableDLL(attrs->ssBaseAddr);

        status = MMCSD_halSetBusFreq(attrs->ctrlBaseAddr, attrs->inputClkFreq, clkFreq, 0U);

        if(status == SystemP_SUCCESS)
        {
            /* Set bus width to 0x02 to select 8-bit bus mode */
            MMCSD_initTransaction(&trans);
            trans.cmd   = MMCSD_MMC_CMD(6);
            trans.arg   = (MMCSD_ECSD_ACCESS_MODE << 24U) | (MMCSD_ECSD_BUS_WIDTH_INDEX << 16U) | ((MMCSD_ECSD_BUS_WIDTH_8BIT) << 8U);
            status = MMCSD_transfer(handle, &trans);

            if(status == SystemP_SUCCESS)
            {
                /* Wait for DAT0 to go low */
                while(CSL_REG32_FEXT(&pReg->PRESENTSTATE, MMC_CTLCFG_PRESENTSTATE_SDIF_DAT0IN) != 1U);
            }

            status = MMCSD_switchEmmcMode(handle, MMCSD_SUPPORT_MMC_HS400);
        }
    }
    else if(mode == MMCSD_SUPPORT_MMC_HS200)
    {
        status = MMCSD_switchEmmcMode(handle, MMCSD_SUPPORT_MMC_HS200);            
    }

    return status;
}

static void MMCSD_cmdStatusPollingFxn(MMCSD_Handle handle)
{
    MMCSD_Object *obj = ((MMCSD_Config *)handle)->object;
    const MMCSD_Attrs *attrs = ((MMCSD_Config *)handle)->attrs;

    uint16_t normalIntrStatus = MMCSD_halNormalIntrStatusGet(attrs->ctrlBaseAddr, MMCSD_INTERRUPT_ALL_NORMAL);
    uint16_t errorIntrStatus = MMCSD_halErrorIntrStatusGet(attrs->ctrlBaseAddr, MMCSD_INTERRUPT_ALL_ERROR);

    /* Check for command completion */
    if(normalIntrStatus & CSL_MMC_CTLCFG_NORMAL_INTR_STS_CMD_COMPLETE_MASK)
    {
        MMCSD_halNormalIntrStatusClear(attrs->ctrlBaseAddr, CSL_MMC_CTLCFG_NORMAL_INTR_STS_CMD_COMPLETE_MASK);
        obj->cmdComp = TRUE;
    }

    /* Check for errors */
    if(errorIntrStatus)
    {
        obj->cmdError = TRUE;

        /* Find out which errors */
        if(errorIntrStatus & CSL_MMC_CTLCFG_ERROR_INTR_STS_CMD_TIMEOUT_MASK)
        {
            MMCSD_halErrorIntrStatusClear(attrs->ctrlBaseAddr, CSL_MMC_CTLCFG_ERROR_INTR_STS_CMD_TIMEOUT_MASK);
            obj->cmdTimeout = TRUE;
        }
        if(errorIntrStatus & CSL_MMC_CTLCFG_ERROR_INTR_STS_CMD_CRC_MASK)
        {
            MMCSD_halErrorIntrStatusClear(attrs->ctrlBaseAddr, CSL_MMC_CTLCFG_ERROR_INTR_STS_CMD_CRC_MASK);
            obj->cmdCRCError = TRUE;
        }
        if(errorIntrStatus & CSL_MMC_CTLCFG_ERROR_INTR_STS_CMD_INDEX_MASK)
        {
            MMCSD_halErrorIntrStatusClear(attrs->ctrlBaseAddr, CSL_MMC_CTLCFG_ERROR_INTR_STS_CMD_INDEX_MASK);
            obj->cmdIndexError = TRUE;
        }
        if(errorIntrStatus & CSL_MMC_CTLCFG_ERROR_INTR_STS_CMD_ENDBIT_MASK)
        {
            MMCSD_halErrorIntrStatusClear(attrs->ctrlBaseAddr, CSL_MMC_CTLCFG_ERROR_INTR_STS_CMD_ENDBIT_MASK);
            obj->cmdEBError = TRUE;
        }
    }
}

static void MMCSD_xferStatusPollingFxn(MMCSD_Handle handle)
{
    MMCSD_Object *obj = ((MMCSD_Config *)handle)->object;
    const MMCSD_Attrs *attrs = ((MMCSD_Config *)handle)->attrs;
    const CSL_mmc_ctlcfgRegs *pReg = (const CSL_mmc_ctlcfgRegs *)(attrs->ctrlBaseAddr);

    volatile uint32_t dataLength = 0U;
    uint32_t remainingBlocks = 0U, offset = 0U;
    uint32_t tempWord = 0xDEADBABE;
    uint8_t *pTempWord = NULL;
    uint32_t i;

    uint16_t normalIntrStatus = MMCSD_halNormalIntrStatusGet(attrs->ctrlBaseAddr, MMCSD_INTERRUPT_ALL_NORMAL);
    uint16_t errorIntrStatus = MMCSD_halErrorIntrStatusGet(attrs->ctrlBaseAddr, MMCSD_INTERRUPT_ALL_ERROR);

    /* Read data received from media */
    if(normalIntrStatus & CSL_MMC_CTLCFG_NORMAL_INTR_STS_BUF_RD_READY_MASK)
    {
        if(obj->xferInProgress == TRUE)
        {
            MMCSD_halNormalIntrStatusClear(attrs->ctrlBaseAddr, CSL_MMC_CTLCFG_NORMAL_INTR_STS_BUF_RD_READY_MASK);

            if((obj->dataBufIdx != NULL) && (obj->readBlockCount != 0))
            {
                dataLength = obj->dataBlockSize;
                remainingBlocks = obj->readBlockCount;
                offset = (obj->dataBlockCount - remainingBlocks) * (obj->dataBlockSize);

                for(i = 0; i < dataLength; i += 4U)
                {
                    tempWord = CSL_REG32_RD(&pReg->DATA_PORT);
                    pTempWord = (uint8_t *)&tempWord;
                    obj->dataBufIdx[offset + i] = *(pTempWord);
                    obj->dataBufIdx[offset + i + 1U] = *(pTempWord + 1U);
                    obj->dataBufIdx[offset + i + 2U] = *(pTempWord + 2U);
                    obj->dataBufIdx[offset + i + 3U] = *(pTempWord + 3U);
                }
                obj->readBlockCount--;
            }
        }
    }

    /* Write data to media */
    if(normalIntrStatus & CSL_MMC_CTLCFG_NORMAL_INTR_STS_BUF_WR_READY_MASK)
    {
        if(obj->xferInProgress == TRUE)
        {
            MMCSD_halNormalIntrStatusClear(attrs->ctrlBaseAddr, CSL_MMC_CTLCFG_NORMAL_INTR_STS_BUF_WR_READY_MASK);

            if(obj->dataBufIdx != NULL)
            {
                dataLength = obj->dataBlockSize;
                remainingBlocks = obj->writeBlockCount;
                offset = (obj->dataBlockCount - remainingBlocks) * (obj->dataBlockSize);

                for(i = 0; i < dataLength; i += 4U)
                {
                    pTempWord = (uint8_t *)&tempWord;
                    *(pTempWord)      = obj->dataBufIdx[offset + i];
                    *(pTempWord + 1U) = obj->dataBufIdx[offset + i + 1U];
                    *(pTempWord + 2U) = obj->dataBufIdx[offset + i + 2U];
                    *(pTempWord + 3U) = obj->dataBufIdx[offset + i + 3U];
                    CSL_REG32_WR(&pReg->DATA_PORT, tempWord);
                }
                obj->writeBlockCount--;
            }
        }
    }

    /* Transfer complete */
    if(normalIntrStatus & CSL_MMC_CTLCFG_NORMAL_INTR_STS_XFER_COMPLETE_MASK)
    {
        if(obj->xferInProgress == TRUE)
        {
            MMCSD_halNormalIntrStatusClear(attrs->ctrlBaseAddr, CSL_MMC_CTLCFG_NORMAL_INTR_STS_XFER_COMPLETE_MASK);
            obj->xferComp = TRUE;
            obj->xferInProgress = FALSE;
        }
    }

    /* Check for errors */
    if(errorIntrStatus)
    {
        obj->cmdError = TRUE;
    }

    if(errorIntrStatus & CSL_MMC_CTLCFG_ERROR_INTR_STS_DATA_CRC_MASK)
    {
        if(obj->xferInProgress == TRUE)
        {
            MMCSD_halNormalIntrStatusClear(attrs->ctrlBaseAddr, CSL_MMC_CTLCFG_ERROR_INTR_STS_DATA_CRC_MASK);
            obj->dataCRCError = TRUE;
            obj->xferInProgress = FALSE;
        }
    }

    if(errorIntrStatus & CSL_MMC_CTLCFG_ERROR_INTR_STS_DATA_ENDBIT_MASK)
    {
        if(obj->xferInProgress == TRUE)
        {
            MMCSD_halNormalIntrStatusClear(attrs->ctrlBaseAddr, CSL_MMC_CTLCFG_ERROR_INTR_STS_DATA_ENDBIT_MASK);
            obj->dataCRCError = TRUE;
            obj->xferInProgress = FALSE;
        }
    }

    if((errorIntrStatus & CSL_MMC_CTLCFG_ERROR_INTR_STS_DATA_TIMEOUT_MASK) &&
        !(normalIntrStatus & CSL_MMC_CTLCFG_NORMAL_INTR_STS_XFER_COMPLETE_MASK))
    {
        if(obj->xferInProgress == TRUE)
        {
            MMCSD_halErrorIntrStatusClear(attrs->ctrlBaseAddr, CSL_MMC_CTLCFG_ERROR_INTR_STS_DATA_TIMEOUT_MASK);
            obj->dataTimeoutError = TRUE;
            obj->xferInProgress = FALSE;
        }
    }
}

/* CMD19 is a bus test pattern command, used for manual tuning*/
static void MMCSD_xferStatusPollingFxnCMD19(MMCSD_Handle handle)
{
    MMCSD_Object *obj = ((MMCSD_Config *)handle)->object;
    const MMCSD_Attrs *attrs = ((MMCSD_Config *)handle)->attrs;
    const CSL_mmc_ctlcfgRegs *pReg = (const CSL_mmc_ctlcfgRegs *)(attrs->ctrlBaseAddr);

    uint16_t normalIntrStatus = MMCSD_halNormalIntrStatusGet(attrs->ctrlBaseAddr, MMCSD_INTERRUPT_ALL_NORMAL);
    uint16_t errorIntrStatus = MMCSD_halErrorIntrStatusGet(attrs->ctrlBaseAddr, MMCSD_INTERRUPT_ALL_ERROR);

    /* Read data received from media */
    if(normalIntrStatus & CSL_MMC_CTLCFG_NORMAL_INTR_STS_BUF_RD_READY_MASK)
    {
        if(obj->xferInProgress == TRUE)
        {
            MMCSD_halNormalIntrStatusClear(attrs->ctrlBaseAddr, CSL_MMC_CTLCFG_NORMAL_INTR_STS_BUF_RD_READY_MASK);

            if(obj->dataBufIdx != NULL)
            {
                uint32_t dataLength = obj->dataBlockCount * obj->dataBlockSize;
                uint32_t i, tempWord = 0xDEADBABE;

                for(i = 0; i < dataLength; i += 4U)
                {
                    tempWord = CSL_REG32_RD(&pReg->DATA_PORT);
                    uint8_t *pTempWord = (uint8_t *)&tempWord;
                    obj->dataBufIdx[i] = *(pTempWord);
                    obj->dataBufIdx[i + 1U] = *(pTempWord + 1U);
                    obj->dataBufIdx[i + 2U] = *(pTempWord + 2U);
                    obj->dataBufIdx[i + 3U] = *(pTempWord + 3U);
                }
            }

            /* Transfer is complete in case of CMD19 */
            MMCSD_halNormalIntrStatusClear(attrs->ctrlBaseAddr, CSL_MMC_CTLCFG_NORMAL_INTR_STS_XFER_COMPLETE_MASK);
            obj->xferComp = TRUE;
            obj->xferInProgress = FALSE;
        }
    }

    /* Check for errors */
    if((errorIntrStatus & CSL_MMC_CTLCFG_ERROR_INTR_STS_DATA_TIMEOUT_MASK) &&
        !(normalIntrStatus & CSL_MMC_CTLCFG_NORMAL_INTR_STS_XFER_COMPLETE_MASK))
    {
        if(obj->xferInProgress == TRUE)
        {
            MMCSD_halErrorIntrStatusClear(attrs->ctrlBaseAddr, CSL_MMC_CTLCFG_ERROR_INTR_STS_DATA_TIMEOUT_MASK);
            obj->dataTimeoutError = TRUE;
            obj->xferInProgress = FALSE;
        }
    }
}

static int32_t MMCSD_sendTuningDataEMMC(MMCSD_Handle handle)
{
    int32_t status = SystemP_SUCCESS;
    MMCSD_Object *obj = ((MMCSD_Config *)handle)->object;
    MMCSD_Transaction trans;

    uint32_t intrState = obj->intrEnable;
    uint32_t dmaState = obj->enableDma;

    /* Disable interrupts and DMA during tuning */
    obj->intrEnable = FALSE;
    obj->enableDma = FALSE;

    /* Send CMD 21 */
    MMCSD_initTransaction(&trans);

    trans.cmd = MMCSD_MMC_CMD(21);
    trans.arg = 0U;
    trans.dir = MMCSD_CMD_XFER_TYPE_READ;
    trans.blockCount = 1U;
    trans.blockSize = sizeof(gTuningPattern8Bit);
    trans.dataBuf = obj->tempDataBuf;
    trans.isTuning = TRUE;
    status = MMCSD_transfer(handle, &trans);

    /* TODO: Enable 4 bit tuning for eMMC */

    if(status == SystemP_SUCCESS)
    {
        status = memcmp(gTuningPattern8Bit, obj->tempDataBuf, sizeof(gTuningPattern8Bit));
    }

    /* Restore interrupts and DMA */
    obj->intrEnable = intrState;
    obj->enableDma = dmaState;

    return status;
}

static int32_t MMCSD_sendTuningDataSD(MMCSD_Handle handle)
{
    int32_t status = SystemP_SUCCESS;

    return status;
}
/* ========================================================================== */
/*                          PHY function definitions                          */
/* ========================================================================== */

static int32_t MMCSD_phyInit(uint32_t ssBaseAddr, uint32_t phyType)
{
    int32_t status = SystemP_SUCCESS;
    const CSL_mmc_sscfgRegs *ssReg = (const CSL_mmc_sscfgRegs *)ssBaseAddr;

    if(phyType == MMCSD_PHY_TYPE_HW_PHY)
    {
        /* Set DLL_TRIM_ICP value to 8 */
        CSL_REG32_FINS(&ssReg->PHY_CTRL_1_REG, MMC_SSCFG_PHY_CTRL_1_REG_DLL_TRM_ICP, 8U);

        /* Reset PHY CONTROL 2 REG */
        CSL_REG32_WR(&ssReg->PHY_CTRL_2_REG, 0U);

        /* Reset PHY CONTROL 3 REG */
        CSL_REG32_WR(&ssReg->PHY_CTRL_3_REG, 0x10FF30FF);

        /* Do the calibration */
        /* Set EN_RTRIM bit */
        CSL_REG32_FINS(&ssReg->PHY_CTRL_1_REG, MMC_SSCFG_PHY_CTRL_1_REG_EN_RTRIM, 1U);
        while(CSL_REG32_FEXT(&ssReg->PHY_CTRL_1_REG, MMC_SSCFG_PHY_CTRL_1_REG_EN_RTRIM) != 1U);

        /* Set PDB to trigger calibration */
        CSL_REG32_FINS(&ssReg->PHY_CTRL_1_REG, MMC_SSCFG_PHY_CTRL_1_REG_PDB, 1U);

        /* Wait for calibration to finish */
        while(CSL_REG32_FEXT(&ssReg->PHY_STAT_1_REG, MMC_SSCFG_PHY_STAT_1_REG_CALDONE) != 1U);
    }
    else if(phyType == MMCSD_PHY_TYPE_SW_PHY)
    {
        /* Enable Output Tap Delay and select the Output Tap Delay */
        CSL_REG32_FINS(&ssReg->PHY_CTRL_4_REG, MMC_SSCFG_PHY_CTRL_4_REG_OTAPDLYENA, 1U);
        CSL_REG32_FINS(&ssReg->PHY_CTRL_4_REG, MMC_SSCFG_PHY_CTRL_4_REG_OTAPDLYSEL, 2U);
    }
    else if(phyType == MMCSD_PHY_TYPE_NO_PHY)
    {
        /* do nothing */
    }
    else
    {
        status = SystemP_FAILURE;
    }

    return status;
}

static inline void MMCSD_phyDisableDLL(uint32_t ssBaseAddr)
{
    const CSL_mmc_sscfgRegs *ssReg = (const CSL_mmc_sscfgRegs *)ssBaseAddr;

    CSL_REG32_FINS(&ssReg->PHY_CTRL_1_REG, MMC_SSCFG_PHY_CTRL_1_REG_ENDLL, 0U);
}

static void MMCSD_phyGetOtapDelay(uint32_t *outputTapDelaySel, uint32_t *outputTapDelayVal,
    uint32_t *inputTapDelaySel, uint32_t *inputTapDelayVal, uint32_t phyMode, uint8_t tunedItap)
{
    switch(phyMode) {
        case MMCSD_PHY_MODE_SDR50:
            *outputTapDelaySel = 1U;
            *outputTapDelayVal = 8U;
            *inputTapDelaySel = 0U;
            *inputTapDelayVal = 0U;
            break;
        case MMCSD_PHY_MODE_HSSDR50:
            *outputTapDelaySel = 1U;
            *outputTapDelayVal = 8U;
            *inputTapDelaySel = 0U;
            *inputTapDelayVal = 0U;
            break;
        case MMCSD_PHY_MODE_HS200:
            *outputTapDelaySel = 1U;
            *outputTapDelayVal = 8U;
            *inputTapDelaySel = 1U;
            *inputTapDelayVal = tunedItap;
            break;
        case MMCSD_PHY_MODE_SDR104:
            *outputTapDelaySel = 1U;
            *outputTapDelayVal = 8U;
            *inputTapDelaySel = 1U;
            *inputTapDelayVal = 0U;
            break;
        case MMCSD_PHY_MODE_DDR50:
            *outputTapDelaySel = 1U;
            *outputTapDelayVal = 6U;
            *inputTapDelaySel = 1U;
            *inputTapDelayVal = 3U;
            break;
        case MMCSD_PHY_MODE_HS400:
            *outputTapDelaySel = 1U;
            *outputTapDelayVal = 5U;
            *inputTapDelaySel = 1U;
            *inputTapDelayVal = tunedItap;
            break;
        case MMCSD_PHY_MODE_DS:
        case MMCSD_PHY_MODE_HS:
            *outputTapDelaySel = 0U;
            *outputTapDelayVal = 0U;
            *inputTapDelaySel = 0U;
            *inputTapDelayVal = 0U;
            break;
        default:
            break;
    }

}

static int32_t MMCSD_phyConfigure(uint32_t ssBaseAddr, uint32_t phyMode, uint32_t phyClkFreq, uint32_t driverImpedance, uint8_t tunedItap)
{
    int32_t status = SystemP_SUCCESS;
    const CSL_mmc_sscfgRegs *ssReg = (const CSL_mmc_sscfgRegs *)ssBaseAddr;

    uint32_t freqSel = 0U, strobeSel = 0U;
    uint32_t outputTapDelaySel = 0U, outputTapDelayVal = 0U;
    uint32_t inputTapDelaySel = 0U, inputTapDelayVal = 0U;

    if(phyMode == MMCSD_PHY_MODE_HS400)
    {
        strobeSel = 0x55U;
    }

    CSL_REG32_FINS(&ssReg->PHY_CTRL_4_REG, MMC_SSCFG_PHY_CTRL_4_REG_STRBSEL, strobeSel);

    /* Enable internal pull-up */
    CSL_REG32_FINS(&ssReg->PHY_CTRL_3_REG, MMC_SSCFG_PHY_CTRL_3_REG_REN_STRB, 1U);

    /* Configure freqSel */
    if((phyClkFreq > 170*1000000) && (phyClkFreq <= 200*1000000))
    {
        freqSel = 0U;
    }
    else if((phyClkFreq > 140*1000000) && (phyClkFreq <= 170*1000000))
    {
        freqSel = 1U;
    }
    else if((phyClkFreq > 110*1000000) && (phyClkFreq <= 140*1000000))
    {
        freqSel = 2U;
    }
    else if((phyClkFreq > 80*1000000) && (phyClkFreq <= 110*1000000))
    {
        freqSel = 3U;
    }
    else if((phyClkFreq > 50*1000000) && (phyClkFreq <= 80*1000000))
    {
        freqSel = 4U;
    }
    else if((phyClkFreq > 250*1000000) && (phyClkFreq <= 275*1000000))
    {
        freqSel = 5U;
    }
    else if((phyClkFreq > 225*1000000) && (phyClkFreq <= 250*1000000))
    {
        freqSel = 6U;
    }
    else if((phyClkFreq > 200*1000000) && (phyClkFreq <= 225*1000000))
    {
        freqSel = 7U;
    }
    else
    {
        /* Default 50 MHz */
        freqSel = 4U;
    }

    if((phyMode == MMCSD_PHY_MODE_HSDDR50 || phyMode == MMCSD_PHY_MODE_HS200 || phyMode == MMCSD_PHY_MODE_HS400) && phyClkFreq >= 50*1000000)
    {
        CSL_REG32_FINS(&ssReg->PHY_CTRL_5_REG, MMC_SSCFG_PHY_CTRL_5_REG_SELDLYTXCLK, 0U);
        CSL_REG32_FINS(&ssReg->PHY_CTRL_5_REG, MMC_SSCFG_PHY_CTRL_5_REG_SELDLYRXCLK, 0U);

        /* Set FRQSEL */
        CSL_REG32_FINS(&ssReg->PHY_CTRL_5_REG, MMC_SSCFG_PHY_CTRL_5_REG_FRQSEL, freqSel);

        /* Set DLL TRIM ICP */
        CSL_REG32_FINS(&ssReg->PHY_CTRL_1_REG, MMC_SSCFG_PHY_CTRL_1_REG_DLL_TRM_ICP, 8U);

        /* Set driver impedance */
        CSL_REG32_FINS(&ssReg->PHY_CTRL_1_REG, MMC_SSCFG_PHY_CTRL_1_REG_DR_TY, driverImpedance);

        /* Enable DLL */
        CSL_REG32_FINS(&ssReg->PHY_CTRL_1_REG, MMC_SSCFG_PHY_CTRL_1_REG_ENDLL, 1U);

        /* Wait for DLL READY bit */
        while(CSL_REG32_FEXT(&ssReg->PHY_STAT_1_REG, MMC_SSCFG_PHY_STAT_1_REG_DLLRDY) != TRUE);
    }
    else
    {
        /* Disable PHY DLL */
        CSL_REG32_FINS(&ssReg->PHY_CTRL_1_REG, MMC_SSCFG_PHY_CTRL_1_REG_ENDLL, 0U);

        CSL_REG32_FINS(&ssReg->PHY_CTRL_5_REG, MMC_SSCFG_PHY_CTRL_5_REG_SELDLYTXCLK, 1U);
        CSL_REG32_FINS(&ssReg->PHY_CTRL_5_REG, MMC_SSCFG_PHY_CTRL_5_REG_SELDLYRXCLK, 1U);
    }

    /* Set CLKBUFSEL*/
    CSL_REG32_FINS(&ssReg->PHY_CTRL_5_REG, MMC_SSCFG_PHY_CTRL_5_REG_CLKBUFSEL, 7U);

    MMCSD_phyGetOtapDelay(&outputTapDelaySel, &outputTapDelayVal, &inputTapDelaySel, &inputTapDelayVal, phyMode, tunedItap);

    /* Disable tap window before modifying the receiver clock delay's, so as to not affect the configured delay's */
    if(outputTapDelaySel | inputTapDelaySel)
    {
        CSL_REG32_FINS(&ssReg->PHY_CTRL_4_REG, MMC_SSCFG_PHY_CTRL_4_REG_ITAPCHGWIN, 1U);

        CSL_REG32_FINS(&ssReg->PHY_CTRL_4_REG, MMC_SSCFG_PHY_CTRL_4_REG_OTAPDLYENA, outputTapDelaySel);
        CSL_REG32_FINS(&ssReg->PHY_CTRL_4_REG, MMC_SSCFG_PHY_CTRL_4_REG_OTAPDLYSEL, outputTapDelayVal);
        CSL_REG32_FINS(&ssReg->PHY_CTRL_4_REG, MMC_SSCFG_PHY_CTRL_4_REG_ITAPDLYENA, inputTapDelaySel);
        CSL_REG32_FINS(&ssReg->PHY_CTRL_4_REG, MMC_SSCFG_PHY_CTRL_4_REG_ITAPDLYSEL, inputTapDelayVal);

        CSL_REG32_FINS(&ssReg->PHY_CTRL_4_REG, MMC_SSCFG_PHY_CTRL_4_REG_ITAPCHGWIN, 0U);
    }

    return status;
}

static int32_t MMCSD_calculateItap(MMCSD_TuningPassOrFailWindow *failWindow, uint8_t numFails, uint8_t *tunedItap)
{
    int32_t status = SystemP_SUCCESS;
    MMCSD_TuningPassOrFailWindow passWindow = {0U, 0U, 0U};
    uint8_t firstFailStart = 0U, lastFailEnd = 0U, startFail = 0U, endFail = 0U, passLength = 0U, prevFailEnd = 0xFFU, count;

    if(!numFails)
    {
        status = SystemP_FAILURE;
        return status;
    }

    if(failWindow->length == MMCSD_ITAPDLY_LENGTH)
    {
        status = SystemP_FAILURE;
        return status;
    }

    firstFailStart = failWindow->start;
    lastFailEnd = failWindow[numFails - 1].end;

    for(count=0U; count < numFails; count++)
    {
        startFail = failWindow[count].start;
        endFail = failWindow[count].end;
        if(prevFailEnd == 0xFFU)
        {
            passLength = startFail;
        }
        else
        {
            passLength = startFail - (uint8_t)(prevFailEnd + 1U);
        }

        if(passLength > passWindow.length)
        {
            passWindow.start = (uint8_t)(prevFailEnd + 1U);
            passWindow.length = passLength;
        }
        prevFailEnd = endFail;
    }

    passLength =  MMCSD_ITAPDLY_LAST_INDEX - lastFailEnd + firstFailStart;

    if(passLength > passWindow.length)
    {
        passWindow.start = (uint8_t)(prevFailEnd + 1U);
        passWindow.length = passLength;
    }

    *tunedItap = (passWindow.start + (passWindow.length >> 1)) % MMCSD_ITAPDLY_LENGTH;

    return status;
}

static int32_t MMCSD_phyTuneManual(MMCSD_Handle handle, uint8_t *tunedItap, uint8_t tuningCount)
{
    int32_t status = SystemP_SUCCESS;
    const MMCSD_Attrs *attrs = ((MMCSD_Config *)handle)->attrs;
    const CSL_mmc_sscfgRegs *ssReg = (const CSL_mmc_sscfgRegs *)(attrs->ssBaseAddr);

    do
    {
        status = MMCSD_phyTuneManualEMMC(handle, tunedItap);
        if(status == SystemP_SUCCESS)
        {
            break;
        }

        tuningCount++;
    }
    while(tuningCount < MMCSD_RETRY_TUNING_MAX);
    
    if(status == SystemP_SUCCESS)
    {
        CSL_REG32_FINS(&ssReg->PHY_CTRL_4_REG, MMC_SSCFG_PHY_CTRL_4_REG_ITAPCHGWIN, 1U);

        CSL_REG32_FINS(&ssReg->PHY_CTRL_4_REG, MMC_SSCFG_PHY_CTRL_4_REG_ITAPDLYENA, 1U);
        CSL_REG32_FINS(&ssReg->PHY_CTRL_4_REG, MMC_SSCFG_PHY_CTRL_4_REG_ITAPDLYSEL, *tunedItap);

        CSL_REG32_FINS(&ssReg->PHY_CTRL_4_REG, MMC_SSCFG_PHY_CTRL_4_REG_ITAPCHGWIN, 0U);

        MMCSD_halLinesResetCmd(attrs->ctrlBaseAddr);
        MMCSD_halLinesResetDat(attrs->ctrlBaseAddr);
    }

    return status;
}

static int32_t MMCSD_phyTuneManualEMMC(MMCSD_Handle handle, uint8_t *tunedItap)
{
    int32_t status = SystemP_SUCCESS;
    const MMCSD_Attrs *attrs = ((MMCSD_Config *)handle)->attrs;
    const CSL_mmc_sscfgRegs *ssReg = (const CSL_mmc_sscfgRegs *)(attrs->ssBaseAddr);
    MMCSD_TuningPassOrFailWindow failWindow[MMCSD_ITAPDLY_LENGTH];
    uint8_t prevPass = 1U, currPass = 0U, failIndex = 0U, itap;

    memset(failWindow, 0U, sizeof(failWindow[0]) * MMCSD_ITAPDLY_LENGTH);

    for(itap = 0U; itap < MMCSD_ITAPDLY_LENGTH; itap++)
    {
        CSL_REG32_FINS(&ssReg->PHY_CTRL_4_REG, MMC_SSCFG_PHY_CTRL_4_REG_ITAPCHGWIN, 1U);

        CSL_REG32_FINS(&ssReg->PHY_CTRL_4_REG, MMC_SSCFG_PHY_CTRL_4_REG_ITAPDLYENA, 1U);
        CSL_REG32_FINS(&ssReg->PHY_CTRL_4_REG, MMC_SSCFG_PHY_CTRL_4_REG_ITAPDLYSEL, itap);

        CSL_REG32_FINS(&ssReg->PHY_CTRL_4_REG, MMC_SSCFG_PHY_CTRL_4_REG_ITAPCHGWIN, 0U);

        currPass = !MMCSD_sendTuningDataEMMC(handle);

        if(!currPass && prevPass)
        {
            failWindow[failIndex].start = itap;
        }

        if(!currPass)
        {
            failWindow[failIndex].end = itap;
            failWindow[failIndex].length++;
        }

        if(currPass && !prevPass)
        {
            failIndex++;
        }

        prevPass = currPass;
    }

    if(failWindow[failIndex].length != 0U)
    {
        failIndex++;
    }

    status = MMCSD_calculateItap(failWindow, failIndex, tunedItap);    
    
    return status;
}

static int32_t MMCSD_phyTuneAuto(MMCSD_Handle handle)
{
    int32_t status = SystemP_SUCCESS;
    MMCSD_Object *obj = ((MMCSD_Config *)handle)->object;
    const MMCSD_Attrs *attrs = ((MMCSD_Config *)handle)->attrs;
    const CSL_mmc_ctlcfgRegs *pReg = (const CSL_mmc_ctlcfgRegs *)(attrs->ctrlBaseAddr);
    uint32_t tuningSuccess = FALSE;
    uint32_t state = 0xFFU, samplingClock = 0U;

    /* Execute tuning */
    CSL_REG16_FINS(&pReg->HOST_CONTROL2, MMC_CTLCFG_HOST_CONTROL2_EXECUTE_TUNING, 1U);

    /* Wait for tuning to complete */
    while(CSL_REG16_FEXT(&pReg->HOST_CONTROL2, MMC_CTLCFG_HOST_CONTROL2_EXECUTE_TUNING) != 1U);

    /* Send tuning data multiple times */
    uint32_t i = 0U;
    for(i = 0U; i < 40U; i++)
    {
        if(obj->cardType == MMCSD_CARD_TYPE_EMMC)
        {
            MMCSD_sendTuningDataEMMC(handle);
        }
        else
        {
            MMCSD_sendTuningDataSD(handle);
        }

        state = CSL_REG16_FEXT(&pReg->HOST_CONTROL2, MMC_CTLCFG_HOST_CONTROL2_EXECUTE_TUNING);
        samplingClock = CSL_REG16_FEXT(&pReg->HOST_CONTROL2, MMC_CTLCFG_HOST_CONTROL2_SAMPLING_CLK_SELECT);

        if((state == 1U) && (samplingClock == 1U))
        {
            tuningSuccess = TRUE;
            break;
        }
    }

    if(tuningSuccess != TRUE)
    {
        CSL_REG16_FINS(&pReg->HOST_CONTROL2, MMC_CTLCFG_HOST_CONTROL2_EXECUTE_TUNING, 0U);
        CSL_REG16_FINS(&pReg->HOST_CONTROL2, MMC_CTLCFG_HOST_CONTROL2_SAMPLING_CLK_SELECT, 0U);
        status = SystemP_FAILURE;
    }

    MMCSD_halLinesResetCmd(attrs->ctrlBaseAddr);
    MMCSD_halLinesResetDat(attrs->ctrlBaseAddr);

    return status;
}


/* ========================================================================== */
/*                     HW Abstraction function definitions                    */
/* ========================================================================== */
static int32_t MMCSD_halSoftReset(uint32_t ctrlBaseAddr)
{
    int32_t status = SystemP_SUCCESS;
    volatile uint32_t timeout = 0xFFFFU;
    const CSL_mmc_ctlcfgRegs *pReg = (const CSL_mmc_ctlcfgRegs *)ctrlBaseAddr;

    CSL_REG8_FINS(&pReg->SOFTWARE_RESET,
        MMC_CTLCFG_SOFTWARE_RESET_SWRST_FOR_ALL,
        CSL_MMC_CTLCFG_SOFTWARE_RESET_SWRST_FOR_ALL_VAL_RESET);

    uint8_t resetStatus = CSL_REG8_FEXT(&pReg->SOFTWARE_RESET, MMC_CTLCFG_SOFTWARE_RESET_SWRST_FOR_ALL);

    while((resetStatus != CSL_MMC_CTLCFG_SOFTWARE_RESET_SWRST_FOR_CMD_VAL_WORK) && (timeout > 0))
    {
        timeout--;
        resetStatus = CSL_REG8_FEXT(&pReg->SOFTWARE_RESET, MMC_CTLCFG_SOFTWARE_RESET_SWRST_FOR_ALL);
    }

    if(0U == timeout)
    {
        status = SystemP_TIMEOUT;
    }

    return status;
}

static int32_t MMCSD_halLinesResetCmd(uint32_t ctrlBaseAddr)
{
    int32_t status = SystemP_SUCCESS;
    volatile uint32_t timeout = 0xFFFFU;
    const CSL_mmc_ctlcfgRegs *pReg = (const CSL_mmc_ctlcfgRegs *)ctrlBaseAddr;

    CSL_REG8_FINS(&pReg->SOFTWARE_RESET,
        MMC_CTLCFG_SOFTWARE_RESET_SWRST_FOR_CMD,
        CSL_MMC_CTLCFG_SOFTWARE_RESET_SWRST_FOR_ALL_VAL_RESET);

    uint8_t resetStatus = CSL_REG8_FEXT(&pReg->SOFTWARE_RESET, MMC_CTLCFG_SOFTWARE_RESET_SWRST_FOR_CMD);

    while((resetStatus != CSL_MMC_CTLCFG_SOFTWARE_RESET_SWRST_FOR_ALL_VAL_WORK) && (timeout > 0))
    {
        timeout--;
        resetStatus = CSL_REG8_FEXT(&pReg->SOFTWARE_RESET, MMC_CTLCFG_SOFTWARE_RESET_SWRST_FOR_CMD);
    }

    if(0U == timeout)
    {
        status = SystemP_TIMEOUT;
    }

    return status;
}

static int32_t MMCSD_halLinesResetDat(uint32_t ctrlBaseAddr)
{
    int32_t status = SystemP_SUCCESS;
    volatile uint32_t timeout = 0xFFFFU;
    const CSL_mmc_ctlcfgRegs *pReg = (const CSL_mmc_ctlcfgRegs *)ctrlBaseAddr;

    CSL_REG8_FINS(&pReg->SOFTWARE_RESET,
        MMC_CTLCFG_SOFTWARE_RESET_SWRST_FOR_DAT,
        CSL_MMC_CTLCFG_SOFTWARE_RESET_SWRST_FOR_ALL_VAL_RESET);

    uint8_t resetStatus = CSL_REG8_FEXT(&pReg->SOFTWARE_RESET, MMC_CTLCFG_SOFTWARE_RESET_SWRST_FOR_DAT);

    while((resetStatus != CSL_MMC_CTLCFG_SOFTWARE_RESET_SWRST_FOR_ALL_VAL_WORK) && (timeout > 0))
    {
        timeout--;
        resetStatus = CSL_REG8_FEXT(&pReg->SOFTWARE_RESET, MMC_CTLCFG_SOFTWARE_RESET_SWRST_FOR_DAT);
    }

    if(0U == timeout)
    {
        status = SystemP_TIMEOUT;
    }

    return status;
}

static int32_t MMCSD_halSetBusWidth(uint32_t ctrlBaseAddr, uint32_t busWidth)
{
    int32_t status = SystemP_SUCCESS;
    const CSL_mmc_ctlcfgRegs *pReg = (const CSL_mmc_ctlcfgRegs *)ctrlBaseAddr;
    switch(busWidth)
    {
        case MMCSD_BUS_WIDTH_8BIT:
            CSL_REG8_FINS(&pReg->HOST_CONTROL1,
                MMC_CTLCFG_HOST_CONTROL1_EXT_DATA_WIDTH,
                CSL_MMC_CTLCFG_HOST_CONTROL1_EXT_DATA_WIDTH_VAL_ENABLE);
            break;

        case MMCSD_BUS_WIDTH_4BIT:
            CSL_REG8_FINS(&pReg->HOST_CONTROL1,
                MMC_CTLCFG_HOST_CONTROL1_EXT_DATA_WIDTH,
                CSL_MMC_CTLCFG_HOST_CONTROL1_EXT_DATA_WIDTH_VAL_DISABLE);
            CSL_REG8_FINS(&pReg->HOST_CONTROL1,
                MMC_CTLCFG_HOST_CONTROL1_DATA_WIDTH,
                CSL_MMC_CTLCFG_HOST_CONTROL1_DATA_WIDTH_VAL_BIT4);
            break;

        case MMCSD_BUS_WIDTH_1BIT:
            CSL_REG8_FINS(&pReg->HOST_CONTROL1,
                MMC_CTLCFG_HOST_CONTROL1_EXT_DATA_WIDTH,
                CSL_MMC_CTLCFG_HOST_CONTROL1_EXT_DATA_WIDTH_VAL_DISABLE);
            CSL_REG8_FINS(&pReg->HOST_CONTROL1,
                MMC_CTLCFG_HOST_CONTROL1_DATA_WIDTH,
                CSL_MMC_CTLCFG_HOST_CONTROL1_DATA_WIDTH_VAL_BIT1);
            break;

        default:
            status = SystemP_FAILURE;
            break;
    }
    return status;
}

static int32_t MMCSD_halSetBusVolt(uint32_t ctrlBaseAddr, uint32_t volt)
{
    int32_t status = SystemP_SUCCESS;
    const CSL_mmc_ctlcfgRegs *pReg = (const CSL_mmc_ctlcfgRegs *)ctrlBaseAddr;

    if((volt == MMCSD_BUS_VOLT_1_8V) || (volt == MMCSD_BUS_VOLT_3_0V) || (volt == MMCSD_BUS_VOLT_3_3V))
    {
        CSL_REG8_FINS(&pReg->POWER_CONTROL,
            MMC_CTLCFG_POWER_CONTROL_SD_BUS_VOLTAGE,
            volt);
    }
    else
    {
        status = SystemP_FAILURE;
    }
    return status;
}

static inline int32_t MMCSD_halIsCardInserted(uint32_t ctrlBaseAddr)
{
    volatile int32_t retVal = 0;
    const CSL_mmc_ctlcfgRegs *pReg = (const CSL_mmc_ctlcfgRegs *)ctrlBaseAddr;

    retVal = CSL_REG32_FEXT(&pReg->PRESENTSTATE, MMC_CTLCFG_PRESENTSTATE_CARD_INSERTED);

    return retVal;
}

static int32_t MMCSD_halBusPower(uint32_t ctrlBaseAddr, uint32_t pwr)
{
    int32_t status = SystemP_SUCCESS;
    volatile uint32_t timeout = 0xFFFFFU;
    const CSL_mmc_ctlcfgRegs *pReg = (const CSL_mmc_ctlcfgRegs *)ctrlBaseAddr;

    if((pwr == CSL_MMC_CTLCFG_POWER_CONTROL_SD_BUS_POWER_VAL_PWR_ON) || (pwr == CSL_MMC_CTLCFG_POWER_CONTROL_SD_BUS_POWER_VAL_PWR_OFF))
    {
        CSL_REG8_FINS(&pReg->POWER_CONTROL,
            MMC_CTLCFG_POWER_CONTROL_SD_BUS_POWER,
            pwr);
    }

    if(pwr == CSL_MMC_CTLCFG_POWER_CONTROL_SD_BUS_POWER_VAL_PWR_ON)
    {
        while((CSL_REG8_FEXT(&pReg->POWER_CONTROL, MMC_CTLCFG_POWER_CONTROL_SD_BUS_POWER) != 1) && (timeout > 0U))
        {
            timeout--;
        }
    }

    if(timeout == 0U)
    {
        status = SystemP_TIMEOUT;
    }

    return status;
}

static int32_t MMCSD_halIsClockStable(uint32_t ctrlBaseAddr, uint32_t timeout)
{
    int32_t status = SystemP_SUCCESS;
    const CSL_mmc_ctlcfgRegs *pReg = (const CSL_mmc_ctlcfgRegs *)ctrlBaseAddr;

    while((CSL_REG16_FEXT(&pReg->CLOCK_CONTROL, MMC_CTLCFG_CLOCK_CONTROL_INT_CLK_STABLE) !=
        CSL_MMC_CTLCFG_CLOCK_CONTROL_INT_CLK_STABLE_VAL_READY) && (timeout > 0U))
    {
        timeout--;
    }

    if(timeout == 0U)
    {
        status = SystemP_TIMEOUT;
    }

    return status;
}

static int32_t MMCSD_halEnableInternalClock(uint32_t ctrlBaseAddr, uint32_t clkState)
{
    int32_t status = SystemP_SUCCESS;
    const CSL_mmc_ctlcfgRegs *pReg = (const CSL_mmc_ctlcfgRegs *)ctrlBaseAddr;

    if((clkState == CSL_MMC_CTLCFG_CLOCK_CONTROL_INT_CLK_ENA_VAL_OSCILLATE) ||
       (clkState == CSL_MMC_CTLCFG_CLOCK_CONTROL_INT_CLK_ENA_VAL_STOP))
    {
        CSL_REG16_FINS(&pReg->CLOCK_CONTROL, MMC_CTLCFG_CLOCK_CONTROL_INT_CLK_ENA, clkState);

        if(clkState == CSL_MMC_CTLCFG_CLOCK_CONTROL_INT_CLK_ENA_VAL_OSCILLATE)
        {
            status = MMCSD_halIsClockStable(ctrlBaseAddr, 0xFFFFU);
        }
    }
    else
    {
        status = SystemP_FAILURE;
    }

    return status;
}

static int32_t MMCSD_halSetBusFreq(uint32_t ctrlBaseAddr, uint32_t inClk, uint32_t outClk, uint32_t bypass)
{
    int32_t status = SystemP_SUCCESS;
    volatile uint32_t clkDiv = 0U;
    const CSL_mmc_ctlcfgRegs *pReg = (const CSL_mmc_ctlcfgRegs *)ctrlBaseAddr;

    /* Enable internal clocks */
    status = MMCSD_halEnableInternalClock(ctrlBaseAddr, CSL_MMC_CTLCFG_CLOCK_CONTROL_INT_CLK_ENA_VAL_OSCILLATE);
    CSL_REG16_FINS(&pReg->CLOCK_CONTROL, MMC_CTLCFG_CLOCK_CONTROL_SD_CLK_ENA, CSL_MMC_CTLCFG_CLOCK_CONTROL_SD_CLK_ENA_VAL_DISABLE);

    if((status == SystemP_SUCCESS) && (bypass == 0U))
    {
        clkDiv = inClk/(outClk * 2);
        if(clkDiv > 1023U)
        {
            clkDiv = 1023U;
        }
        while((inClk/clkDiv) > (2 * outClk))
        {
            if(clkDiv == 1023U)
            {
                status = SystemP_FAILURE;
                break;
            }
            clkDiv++;
        }

        if(status == SystemP_SUCCESS)
        {
            CSL_REG16_FINS(&pReg->CLOCK_CONTROL, MMC_CTLCFG_CLOCK_CONTROL_SDCLK_FRQSEL, (clkDiv & 0xFF));
            CSL_REG16_FINS(&pReg->CLOCK_CONTROL, MMC_CTLCFG_CLOCK_CONTROL_SDCLK_FRQSEL_UPBITS, ((clkDiv >> 8) & 0x03U));

            status = MMCSD_halIsClockStable(ctrlBaseAddr, 0xFFFFU);
        }

        if(status == SystemP_SUCCESS)
        {
            CSL_REG16_FINS(&pReg->CLOCK_CONTROL, MMC_CTLCFG_CLOCK_CONTROL_SD_CLK_ENA, CSL_MMC_CTLCFG_CLOCK_CONTROL_SD_CLK_ENA_VAL_ENABLE);
        }
    }

    return status;
}

static int32_t MMCSD_halSetUHSMode(uint32_t ctrlBaseAddr, uint32_t uhsMode)
{
    int32_t status = SystemP_SUCCESS;
    const CSL_mmc_ctlcfgRegs *pReg = (const CSL_mmc_ctlcfgRegs *)ctrlBaseAddr;

    CSL_REG16_FINS(&pReg->CLOCK_CONTROL, MMC_CTLCFG_CLOCK_CONTROL_SD_CLK_ENA, CSL_MMC_CTLCFG_CLOCK_CONTROL_SD_CLK_ENA_VAL_DISABLE);

    if((uhsMode == MMCSD_UHS_MODE_SDR12)  ||
       (uhsMode == MMCSD_UHS_MODE_SDR25)  ||
       (uhsMode == MMCSD_UHS_MODE_SDR50)  ||
       (uhsMode == MMCSD_UHS_MODE_SDR104) ||
       (uhsMode == MMCSD_UHS_MODE_DDR50)  ||
       (uhsMode == MMCSD_UHS_MODE_HS400)
       )
    {
        CSL_REG16_FINS(&pReg->HOST_CONTROL2, MMC_CTLCFG_HOST_CONTROL2_UHS_MODE_SELECT, uhsMode);
    }

    CSL_REG16_FINS(&pReg->CLOCK_CONTROL, MMC_CTLCFG_CLOCK_CONTROL_SD_CLK_ENA, CSL_MMC_CTLCFG_CLOCK_CONTROL_SD_CLK_ENA_VAL_ENABLE);

    return status;
}

static int32_t MMCSD_halSendCommand(uint32_t ctrlBaseAddr, MMCSD_Transaction *trans)
{
    int32_t status = SystemP_SUCCESS;
    const CSL_mmc_ctlcfgRegs *pReg = (const CSL_mmc_ctlcfgRegs *)ctrlBaseAddr;

    uint32_t multiBlock = (trans->blockCount > 1U) ? TRUE : FALSE;

    /* Clear transfer mode and command registers */
    CSL_REG16_WR(&pReg->TRANSFER_MODE, 0U);

    /* Set command argument */
    CSL_REG16_WR(&pReg->ARGUMENT1_LO, (trans->arg & 0xFFFFU));
    CSL_REG16_WR(&pReg->ARGUMENT1_HI, ((trans->arg >> 16) & 0xFFFFU));

    if(trans->cmd & CSL_MMC_CTLCFG_COMMAND_DATA_PRESENT_MASK)
    {
        if(multiBlock == TRUE)
        {
            CSL_REG16_WR(&pReg->BLOCK_COUNT, trans->blockCount);
            CSL_REG16_WR(&pReg->BLOCK_SIZE, trans->blockSize);
            CSL_REG16_WR(&pReg->TRANSFER_MODE, ((trans->enableDma << CSL_MMC_CTLCFG_TRANSFER_MODE_DMA_ENA_SHIFT) |
                                                (1 << CSL_MMC_CTLCFG_TRANSFER_MODE_BLK_CNT_ENA_SHIFT) |
                                                (trans->autoCmdEn << CSL_MMC_CTLCFG_TRANSFER_MODE_AUTO_CMD_ENA_SHIFT) |
                                                (trans->dir << CSL_MMC_CTLCFG_TRANSFER_MODE_DATA_XFER_DIR_SHIFT) |
                                                (1 << CSL_MMC_CTLCFG_TRANSFER_MODE_MULTI_BLK_SEL_SHIFT)));

        }
        else
        {
            CSL_REG16_WR(&pReg->TRANSFER_MODE, ((trans->enableDma << CSL_MMC_CTLCFG_TRANSFER_MODE_DMA_ENA_SHIFT) |
                                                (0 << CSL_MMC_CTLCFG_TRANSFER_MODE_BLK_CNT_ENA_SHIFT) |
                                                (trans->autoCmdEn << CSL_MMC_CTLCFG_TRANSFER_MODE_AUTO_CMD_ENA_SHIFT) |
                                                (trans->dir << CSL_MMC_CTLCFG_TRANSFER_MODE_DATA_XFER_DIR_SHIFT) |
                                                (0 << CSL_MMC_CTLCFG_TRANSFER_MODE_MULTI_BLK_SEL_SHIFT)));
        }
    }
    CSL_REG16_WR(&pReg->COMMAND, (uint16_t)trans->cmd);

    return status;
}

static int32_t MMCSD_halCmdResponseGet(uint32_t ctrlBaseAddr, uint32_t *rsp)
{
    int32_t status = SystemP_SUCCESS;
    const CSL_mmc_ctlcfgRegs *pReg = (const CSL_mmc_ctlcfgRegs *)ctrlBaseAddr;

    if(rsp != NULL)
    {
        rsp[0] = (uint32_t)CSL_REG16_RD(&pReg->RESPONSE[0]) | ((uint32_t)CSL_REG16_RD(&pReg->RESPONSE[1]) << 16);
        rsp[1] = (uint32_t)CSL_REG16_RD(&pReg->RESPONSE[2]) | ((uint32_t)CSL_REG16_RD(&pReg->RESPONSE[3]) << 16);
        rsp[2] = (uint32_t)CSL_REG16_RD(&pReg->RESPONSE[4]) | ((uint32_t)CSL_REG16_RD(&pReg->RESPONSE[5]) << 16);
        rsp[3] = (uint32_t)CSL_REG16_RD(&pReg->RESPONSE[6]) | ((uint32_t)CSL_REG16_RD(&pReg->RESPONSE[7]) << 16);
    }
    else
    {
        status = SystemP_FAILURE;
    }
    return status;
}

static inline uint32_t MMCSD_halNormalIntrStatusGet(uint32_t ctrlBaseAddr, uint16_t intrFlag)
{
    uint32_t regVal = 0U;
    const CSL_mmc_ctlcfgRegs *pReg = (const CSL_mmc_ctlcfgRegs *)ctrlBaseAddr;

    regVal = (uint32_t)(CSL_REG16_RD(&pReg->NORMAL_INTR_STS) & intrFlag);

    return regVal;
}

static inline void MMCSD_halNormalIntrStatusClear(uint32_t ctrlBaseAddr, uint16_t intrFlag)
{
    const CSL_mmc_ctlcfgRegs *pReg = (const CSL_mmc_ctlcfgRegs *)ctrlBaseAddr;

    CSL_REG16_WR(&pReg->NORMAL_INTR_STS, intrFlag);

}

static inline uint32_t MMCSD_halErrorIntrStatusGet(uint32_t ctrlBaseAddr, uint16_t intrFlag)
{
    uint32_t regVal = 0U;
    const CSL_mmc_ctlcfgRegs *pReg = (const CSL_mmc_ctlcfgRegs *)ctrlBaseAddr;

    regVal = (uint32_t)(CSL_REG16_RD(&pReg->ERROR_INTR_STS) & intrFlag);

    return regVal;

}

static inline void MMCSD_halErrorIntrStatusClear(uint32_t ctrlBaseAddr, uint16_t intrFlag)
{
    const CSL_mmc_ctlcfgRegs *pReg = (const CSL_mmc_ctlcfgRegs *)ctrlBaseAddr;

    CSL_REG16_WR(&pReg->ERROR_INTR_STS, intrFlag);

}

static inline void MMCSD_halNormalIntrStatusEnable(uint32_t ctrlBaseAddr, uint16_t intrFlag)
{
    const CSL_mmc_ctlcfgRegs *pReg = (const CSL_mmc_ctlcfgRegs *)ctrlBaseAddr;

    volatile uint16_t regVal = 0U;

    regVal = CSL_REG16_RD(&pReg->NORMAL_INTR_STS_ENA);
    regVal |= intrFlag;
    CSL_REG16_WR(&pReg->NORMAL_INTR_STS_ENA, regVal);

}

static inline void MMCSD_halNormalIntrStatusDisable(uint32_t ctrlBaseAddr, uint16_t intrFlag)
{
    const CSL_mmc_ctlcfgRegs *pReg = (const CSL_mmc_ctlcfgRegs *)ctrlBaseAddr;

    volatile uint16_t regVal = 0U;

    regVal = CSL_REG16_RD(&pReg->NORMAL_INTR_STS_ENA);
    regVal &= ~intrFlag;
    CSL_REG16_WR(&pReg->NORMAL_INTR_STS_ENA, regVal);

}

static inline void MMCSD_halErrorIntrStatusEnable(uint32_t ctrlBaseAddr, uint16_t intrFlag)
{
    const CSL_mmc_ctlcfgRegs *pReg = (const CSL_mmc_ctlcfgRegs *)ctrlBaseAddr;

    volatile uint16_t regVal = 0U;

    regVal = CSL_REG16_RD(&pReg->ERROR_INTR_STS_ENA);
    regVal |= intrFlag;
    CSL_REG16_WR(&pReg->ERROR_INTR_STS_ENA, regVal);

}

static inline void MMCSD_halErrorIntrStatusDisable(uint32_t ctrlBaseAddr, uint16_t intrFlag)
{
    const CSL_mmc_ctlcfgRegs *pReg = (const CSL_mmc_ctlcfgRegs *)ctrlBaseAddr;

    volatile uint16_t regVal = 0U;

    regVal = CSL_REG16_RD(&pReg->ERROR_INTR_STS_ENA);
    regVal &= ~intrFlag;
    CSL_REG16_WR(&pReg->ERROR_INTR_STS_ENA, regVal);

}

static inline void MMCSD_halNormalSigIntrDisable(uint32_t ctrlBaseAddr, uint16_t intrFlag)
{
    const CSL_mmc_ctlcfgRegs *pReg = (const CSL_mmc_ctlcfgRegs *)ctrlBaseAddr;
    volatile uint16_t regVal = 0U;

    regVal = CSL_REG16_RD(&pReg->NORMAL_INTR_SIG_ENA);
    regVal &= ~intrFlag;
    CSL_REG16_WR(&pReg->NORMAL_INTR_SIG_ENA, regVal);

}

static inline void MMCSD_halNormalSigIntrEnable(uint32_t ctrlBaseAddr, uint16_t intrFlag)
{
    const CSL_mmc_ctlcfgRegs *pReg = (const CSL_mmc_ctlcfgRegs *)ctrlBaseAddr;
    volatile uint16_t regVal = 0U;

    regVal = CSL_REG16_RD(&pReg->NORMAL_INTR_SIG_ENA);
    regVal |= intrFlag;
    CSL_REG16_WR(&pReg->NORMAL_INTR_SIG_ENA, regVal);

}

static inline void MMCSD_halErrorSigIntrDisable(uint32_t ctrlBaseAddr, uint16_t intrFlag)
{
    const CSL_mmc_ctlcfgRegs *pReg = (const CSL_mmc_ctlcfgRegs *)ctrlBaseAddr;
    volatile uint16_t regVal = 0U;

    regVal = CSL_REG16_RD(&pReg->ERROR_INTR_SIG_ENA);
    regVal &= ~intrFlag;
    CSL_REG16_WR(&pReg->ERROR_INTR_SIG_ENA, regVal);

}

static inline void MMCSD_halErrorSigIntrEnable(uint32_t ctrlBaseAddr, uint16_t intrFlag)
{
    const CSL_mmc_ctlcfgRegs *pReg = (const CSL_mmc_ctlcfgRegs *)ctrlBaseAddr;
    volatile uint16_t regVal = 0U;

    regVal = CSL_REG16_RD(&pReg->ERROR_INTR_SIG_ENA);
    regVal |= intrFlag;
    CSL_REG16_WR(&pReg->ERROR_INTR_SIG_ENA, regVal);

}

static void MMCSD_isr(void *arg)
{

}

