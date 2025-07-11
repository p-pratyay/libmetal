/*
 * Copyright (c) 2017-2025, Texas Instruments Incorporated
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 *
 * *  Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 *
 * *  Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 *    Neither the name of Texas Instruments Incorporated nor the names of
 *    its contributors may be used to endorse or promote products derived
 *    from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO,
 * THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
 * PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR
 * CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
 * EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
 * PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS;
 * OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY,
 * WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR
 * OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE,
 * EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */
/**
 *  \file sciclient_direct.c
 *
 *  \brief File containing the SCICLIENT services for DM on R5.
 *
 */

/* ========================================================================== */
/*                             Include Files                                  */
/* ========================================================================== */
#include <stdint.h>
#include <string.h>
#include <kernel/dpl/SystemP.h>
#include <kernel/dpl/DebugP.h>
#include <drivers/sciclient/csl_sec_proxy.h>
#include <drivers/sciclient/sciclient_romMessages.h>
#include <drivers/sciclient/soc/sciclient_soc_priv.h>
#include <drivers/device_manager/sciclient.h>
#include <drivers/device_manager/sciserver.h>
#include <drivers/device_manager/sciclient_direct/sciclient_priv.h>

/* ========================================================================== */
/*                           Macros & Typedefs                                */
/* ========================================================================== */

#if defined (SOC_AM62X) || defined (SOC_AM62AX)
#define SCICLIENT_COMMON_X509_HEADER_ADDR               (0x43c3f1e0)
#define SCICLIENT_SCECURE_PROXY_MESSAGE_SIZE            64U
#elif defined(SOC_AM62PX)
#define SCICLIENT_COMMON_X509_HEADER_ADDR               (0x43c4f1e0)
#define SCICLIENT_SCECURE_PROXY_MESSAGE_SIZE            64U
#elif defined(SOC_AM275X)
#define SCICLIENT_COMMON_X509_HEADER_ADDR               (0x720ff1e0)
#define SCICLIENT_SCECURE_PROXY_MESSAGE_SIZE            64U
#elif defined(SOC_J722S)
#define SCICLIENT_COMMON_X509_HEADER_ADDR               (0x43c7f1e0)
#define SCICLIENT_SCECURE_PROXY_MESSAGE_SIZE            64U
#endif

#if defined(CONFIG_LPM_DM)
#define R5F_TCMB_ADDR                                   (0x41010000)
#endif

/* ========================================================================== */
/*                         Structure Declarations                             */
/* ========================================================================== */

/** \brief This structure contains configuration parameters for
*       the sec_proxy IP */
CSL_SecProxyCfg gSciclientRomSecProxyCfg =
{
    (CSL_sec_proxyRegs *)(CSL_SA3_SS0_IPCSS_SEC_PROXY_CFG_MMRS_BASE),
    /*< pSecProxyRegs */
    (CSL_sec_proxy_scfgRegs *)(CSL_SA3_SS0_IPCSS_SEC_PROXY_CFG_SCFG_BASE),
    /*< pSecProxyScfgRegs */
    (CSL_sec_proxy_rtRegs *)(CSL_SA3_SS0_IPCSS_SEC_PROXY_CFG_RT_BASE),
    /*< pSecProxyRtRegs */
    (uint64_t)(CSL_SA3_SS0_SEC_PROXY_SRC_TARGET_DATA_BASE),
    /*< proxyTargetAddr */
    0                                          // maxMsgSize
};

void _vectors(void);
uint8_t _freertosresetvectors[0x40];

#if defined(CONFIG_LPM_DM)
/** \brief Variable to store the LPM context save address required by TISCI APIs */
static uint64_t *gpLPMCtxt = NULL;

/** \brief Variable to store the LPM FS stub load address required for IO only plus DDR resume */
static uint64_t *gpLPMFSStub = NULL;

/** \brief Variable to get the entry point of device manager for use in LPM */
extern uint64_t _self_reset_start;

/**
 * Variables to save the application's LPM suspend and resume hooks
 */
LPMSuspendHook gLPMSuspendHook;
LPMResumeHook gLPMResumeHook;
#endif

/* ========================================================================== */
/*                          Function Declarations                             */
/* ========================================================================== */

/**
 *  \brief   Validate thread has no errors and has space to accept the next
 *           message.
 *
 *  \param   thread    Index of the thread.
 *
 *  \return  status    Status of the message.
 */
static int32_t Sciclient_secProxyVerifyThread(uint32_t thread);


/**
 *  \brief   Gives the address for status register for a particular thread.
 *
 *  \param   thread    Index of the thread.
 *
 *  \return  address   address of the thread status
 */
static inline uint32_t Sciclient_secProxyThreadStatusReg(uint32_t thread);

/**
 *  \brief   Read a 32 bit word from the thread.
 *
 *  \param   thread    Index of the thread to be read from.
 *  \param   idx       Index of the word to be read from the thread.
 *
 *  \return  word      Value read back.
 */
static inline uint32_t Sciclient_secProxyReadThread32(uint32_t thread, uint8_t idx);

/**
 *  \brief   Check if there are credits to write to the thread.
 *
 *  \param   thread    Index of the thread.
 *  \param   timeout   Wait for timeout if operation is complete.
 *
 *  \return  status    Status of the message.
 */
static int32_t Sciclient_secProxyWaitThread(uint32_t thread, uint32_t timeout);

/* ========================================================================== */
/*                          Function Definitions                              */
/* ========================================================================== */

int32_t Sciclient_direct_init(void)
{
    int32_t ret = SystemP_SUCCESS;
    Sciclient_ConfigPrms_t clientPrms;

    memcpy((void *)_freertosresetvectors, (void *)_vectors , 0x40);

    ret = Sciclient_configPrmsInit(&clientPrms);

    if (ret == SystemP_SUCCESS)
    {
        ret = Sciclient_boardCfgParseHeader(
            (uint8_t *) SCICLIENT_COMMON_X509_HEADER_ADDR,
            &clientPrms.inPmPrms, &clientPrms.inRmPrms);
    }

    if (ret == SystemP_SUCCESS)
    {
        ret = Sciclient_init(&clientPrms);
    }

    if (ret == SystemP_SUCCESS)
    {
        DebugP_log("Sciclient direct init..... SUCCESS\n");
    }
    else
    {
        DebugP_log("Sciclient direct init..... FAILED\n");
    }

    return ret;
}

int32_t Sciclient_getVersionCheck(uint32_t doLog)
{
    int32_t status;
    struct tisci_msg_version_req req = {0};
    const Sciclient_ReqPrm_t      reqPrm =
    {
        TISCI_MSG_VERSION,
        TISCI_MSG_FLAG_AOP,
        (const uint8_t *)&req,
        sizeof(req),
        SystemP_WAIT_FOREVER,
        SCISERVER_NO_FORWARD_MSG
    };

    struct tisci_msg_version_resp response;
    Sciclient_RespPrm_t           respPrm =
    {
        0,
        (uint8_t *) &response,
        sizeof (response)
    };

    status = Sciclient_service(&reqPrm, &respPrm);
    if ( (SystemP_SUCCESS == status) && (respPrm.flags == TISCI_MSG_FLAG_ACK))
    {
        if(doLog != 0U)
        {
            DebugP_log("\r\n");
            DebugP_log("SYSFW Firmware Version %s\r\n",
                                (char *) response.str);
            DebugP_log("SYSFW Firmware revision 0x%x\r\n", response.version);
            DebugP_log("SYSFW ABI revision %d.%d\r\n", response.abi_major,
                                response.abi_minor);
            DebugP_log("\r\n");
        }
    }
    else
    {
        status = SystemP_FAILURE;
        if(doLog != 0U)
        {
            DebugP_log("\r\n");
            DebugP_logError("[ERROR] Sciclient get version failed !!!\r\n");
        }
    }
    return status;
}

uint32_t Sciclient_getSelfDevIdCore(void)
{
    return TISCI_DEV_WKUP_R5FSS0_CORE0;
}

int32_t Sciclient_waitForBootNotification(void)
{
    int32_t status = SystemP_FAILURE;
    uint32_t rxThread = SCICLIENT_ROM_R5_RX_NORMAL_THREAD;
    uint32_t secHeaderSizeWords = sizeof(struct tisci_sec_header)/sizeof(uint32_t);
    volatile Sciclient_RomFirmwareLoadHdr_t *pLocalRespHdr = NULL;

    status = Sciclient_secProxyVerifyThread(rxThread);

    if (status == SystemP_SUCCESS)
    {
        while ((CSL_REG32_RD(Sciclient_secProxyThreadStatusReg(rxThread)) &
             CSL_SEC_PROXY_RT_THREAD_STATUS_CUR_CNT_MASK) == 0U) {;}

        pLocalRespHdr = (Sciclient_RomFirmwareLoadHdr_t *)(CSL_secProxyGetDataAddr(
                                            &gSciclientRomSecProxyCfg, rxThread, 0U)
                                            + ((uintptr_t) secHeaderSizeWords * (uintptr_t) 4U));
        /* Check the message type and flag of the response */
        if (pLocalRespHdr->type ==
            TISCI_MSG_BOOT_NOTIFICATION)
        {
            status = SystemP_SUCCESS;
        }
        else
        {
            status = SystemP_FAILURE;
        }
        /* Reading from the last register of rxThread*/
        (void) Sciclient_secProxyReadThread32(rxThread,
                        (uint8_t)(((SCICLIENT_SCECURE_PROXY_MESSAGE_SIZE -
                                    CSL_SEC_PROXY_RSVD_MSG_BYTES)/4U)-1U));
    }
    else
    {
        status = SystemP_FAILURE;
    }

    return status;
}

int32_t Sciclient_loadFirmware(const uint32_t *pSciclient_firmware)
{
    int32_t  status   = SystemP_SUCCESS;
    uint32_t txThread = SCICLIENT_ROM_R5_TX_NORMAL_THREAD;
    uint32_t rxThread = SCICLIENT_ROM_R5_RX_NORMAL_THREAD;
    Sciclient_RomFirmwareLoadHdr_t header      = {0};
    Sciclient_RomFirmwareLoadPayload_t payload = {0};
    uint8_t secHeaderSizeWords = sizeof(struct tisci_sec_header)/sizeof(uint32_t);
    const uint32_t  maxMsgSizeBytes = CSL_secProxyGetMaxMsgSize(&gSciclientRomSecProxyCfg) - CSL_SEC_PROXY_RSVD_MSG_BYTES;

    volatile Sciclient_RomFirmwareLoadHdr_t *pLocalRespHdr =
        (Sciclient_RomFirmwareLoadHdr_t *)CSL_secProxyGetDataAddr(&gSciclientRomSecProxyCfg, rxThread, 0U);
    uint8_t  payloadSize = sizeof (Sciclient_RomFirmwareLoadPayload_t) /
                           sizeof (uint8_t);
    uint64_t proxyTargetAddr = gSciclient_secProxyCfg.proxyTargetAddr;
    /* Require ROM secure proxy target address for SYSFW load */
    gSciclient_secProxyCfg.proxyTargetAddr = gSciclientRomSecProxyCfg.proxyTargetAddr;
    /* Construct header */
    header.type = SCICLIENT_ROM_MSG_R5_TO_M3_M3FW;

    header.host = TISCI_HOST_ID_DEVICE_MANAGER;
    /* ROM expects a sequence number of 0 */
    header.seq  = 0U;
    /* ROM doesn't check for flags */
    header.flags = 0U;

    if (pSciclient_firmware != NULL)
    {
        payload.bufferAddress = (uint32_t)(uintptr_t)pSciclient_firmware;

        /*Size is not needed actually.It is taken from x509 certificate*/
        payload.bufferSizeBytes = 0xffffffffU;

        /* Verify thread status before reading/writing */
        status = Sciclient_secProxyVerifyThread(txThread);
        if (SystemP_SUCCESS == status)
        {
            status = Sciclient_secProxyWaitThread(txThread, SystemP_WAIT_FOREVER);
        }
        if (SystemP_SUCCESS == status)
        {
            /* Writing header and payload */
            Sciclient_sendMessage(txThread, NULL, secHeaderSizeWords,(uint8_t *) &header,
                                  (uint8_t *)&payload, payloadSize, maxMsgSizeBytes);

            /* CHECKING FOR FIRMWARE LOAD ACK */
            /* Verify thread status before reading/writing */
            status = Sciclient_secProxyVerifyThread(rxThread);
        }
        if (SystemP_SUCCESS == status)
        {
            while ((CSL_REG32_RD(Sciclient_secProxyThreadStatusReg(rxThread)) &
                 CSL_SEC_PROXY_RT_THREAD_STATUS_CUR_CNT_MASK) == 0U) {;}
            /* Check the message type and flag of the response */
            if ((pLocalRespHdr->type ==
                SCICLIENT_ROM_MSG_M3_TO_R5_M3FW_RESULT)
                && (pLocalRespHdr->flags == SCICLIENT_ROM_MSG_CERT_AUTH_PASS))
            {
                status = SystemP_SUCCESS;
            }
            else
            {
                status = SystemP_FAILURE;
            }
            /* Reading from the last register of rxThread. This flushes the thread*/
            (void) Sciclient_secProxyReadThread32(rxThread,
                            (uint8_t)((maxMsgSizeBytes/4U)-1U));
        }

        /* CHECKING FOR TISCI_MSG_BOOT_NOTIFICATION from SYSFW*/
        pLocalRespHdr =
        (Sciclient_RomFirmwareLoadHdr_t *)(CSL_secProxyGetDataAddr(
                                            &gSciclientRomSecProxyCfg, rxThread, 0U)
                                            + ((uintptr_t) secHeaderSizeWords * (uintptr_t) 4U));
        if (status == SystemP_SUCCESS)
        {
            status = Sciclient_secProxyVerifyThread(rxThread);
        }
        if (status == SystemP_SUCCESS)
        {
            while ((CSL_REG32_RD(Sciclient_secProxyThreadStatusReg(rxThread)) &
                 CSL_SEC_PROXY_RT_THREAD_STATUS_CUR_CNT_MASK) == 0U) {;}
            /* Check the message type and flag of the response */
            if (pLocalRespHdr->type ==
                TISCI_MSG_BOOT_NOTIFICATION)
            {
                status = SystemP_SUCCESS;
            }
            else
            {
                status = SystemP_FAILURE;
            }
            /* Reading from the last register of rxThread*/
            (void) Sciclient_secProxyReadThread32(rxThread,
                            (uint8_t)((maxMsgSizeBytes/4U)-1U));
        }
    }
    else
    {
        status = SystemP_FAILURE;
    }
    /* Setting the previous target address */
    gSciclient_secProxyCfg.proxyTargetAddr = proxyTargetAddr;

    return status;
}

#ifdef CONFIG_LPM_DM
void Sciclient_initDeviceManagerLPMData(DM_LPMData_t *pLPMData)
{
    /* Update the addresses required for lpm */
    gpLPMCtxt = (uint64_t *) (&(pLPMData->fsCtxt));
    gpLPMFSStub = (uint64_t *) (&(pLPMData->fsStub));

    /* Update the LPM meta data section */
    pLPMData->metaData.dmEntryPoint = (uint64_t) (&_self_reset_start);
    pLPMData->metaData.fsCtxtAddr = (uint64_t) &(pLPMData->fsCtxt);

    /* Copy the FS stub to R5 local memory */
    memcpy((void *)R5F_TCMB_ADDR, (void *)(pLPMData->fsStub), LPM_FS_STUB_SIZE);
}

int32_t Sciclient_getLPMCtxtSaveAddr(uint64_t *pCtxtAddr)
{
    int32_t ret = SystemP_SUCCESS;

    /* If the section is not allocated by application, return failure */
    if (gpLPMCtxt != NULL)
    {
        *pCtxtAddr = (uint64_t) gpLPMCtxt;
    }
    else
    {
        ret = SystemP_FAILURE;
    }

    return ret;
}

int32_t Sciclient_copyLPMFSStubToLocalMem(void)
{
    int32_t ret = SystemP_SUCCESS;

    /* If the section is not allocated by application, return failure */
    if (gpLPMFSStub != NULL)
    {
        /* Copy the FS stub to R5 local memory */
        memcpy((void *)R5F_TCMB_ADDR, (const void *)(gpLPMFSStub), LPM_FS_STUB_SIZE);
    }
    else
    {
        ret = SystemP_FAILURE;
    }

    return ret;
}

void Sciclient_initLPMSusResHook(LPMSuspendHook suspendHook, LPMResumeHook resumeHook)
{
    gLPMSuspendHook = suspendHook;
    gLPMResumeHook = resumeHook;
}

void Sciclient_ApplicationLPMSuspend(void)
{
    if(gLPMSuspendHook != NULL)
    {
        gLPMSuspendHook();
    }
}

void Sciclient_ApplicationLPMResume(void)
{
    if(gLPMResumeHook != NULL)
    {
        gLPMResumeHook();
    }
}

#else
void Sciclient_initDeviceManagerLPMData(DM_LPMData_t *pLPMData __attribute__((unused)))
{
    return;
}

int32_t Sciclient_getLPMCtxtSaveAddr(uint64_t *pCtxtAddr __attribute__((unused)))
{
    return SystemP_SUCCESS;
}

int32_t Sciclient_copyLPMFSStubToLocalMem(void)
{
    return SystemP_SUCCESS;
}

void Sciclient_ApplicationLPMSuspend(void)
{
}

void Sciclient_ApplicationLPMResume(void)
{
}
#endif

/* -------------------------------------------------------------------------- */
/*                 Internal Function Definitions                              */
/* -------------------------------------------------------------------------- */

static int32_t Sciclient_secProxyVerifyThread(uint32_t thread)
{
    int32_t status = SystemP_SUCCESS;
    /* Verify thread status before reading/writing */
    if ((CSL_REG32_RD(Sciclient_secProxyThreadStatusReg(thread)) &
        CSL_SEC_PROXY_RT_THREAD_STATUS_ERROR_MASK) != 0U)
    {
        status = SystemP_FAILURE;
    }
    return status;
}

static inline uint32_t Sciclient_secProxyThreadStatusReg(uint32_t thread)
{
    return ((uint32_t)(uintptr_t)(gSciclientRomSecProxyCfg.pSecProxyRtRegs) +
        CSL_SEC_PROXY_RT_THREAD_STATUS(thread));
}

static inline uint32_t Sciclient_secProxyReadThread32(uint32_t thread, uint8_t idx)
{
    uint32_t ret;
    ret = CSL_REG32_RD(CSL_secProxyGetDataAddr(&gSciclientRomSecProxyCfg,thread,0U) +
        ((uintptr_t) (0x4U) * (uintptr_t) idx));
    return ret;
}

static int32_t Sciclient_secProxyWaitThread(uint32_t thread, uint32_t timeout)
{
    int32_t  status     = CSL_ETIMEOUT;
    uint32_t timeToWait = timeout;
    /* Checks the thread count is > 0 */
    while (timeToWait > 0U)
    {
        if ((CSL_REG32_RD(Sciclient_secProxyThreadStatusReg(thread)) &
            CSL_SEC_PROXY_RT_THREAD_STATUS_CUR_CNT_MASK) > 0U)
        {
            status = SystemP_SUCCESS;
            break;
        }
        timeToWait--;
    }
    return status;
}