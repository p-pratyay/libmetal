/*
 *  Copyright (C) 2022-24 Texas Instruments Incorporated
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
 *  LIMITED TO, SCICLIENT_PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
 *  DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
 *  THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 *  (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 *  OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *
 */

/**
 * \ingroup DRV_SCICLIENT_MODULE
 * \defgroup SCICLIENT_FMW_MSG_PARAMS sciclient_fmwMsgParams.h
 *
 * @{
 */

/**
 *  \file   am62dx/sciclient_fmwMsgParams.h
 *
 *  \brief  This file contains the definition of all the parameter IDs for
 *          PM, RM, Security.
 */

#ifndef SCICLIENT_FMWMSGPARAMS_H_
#define SCICLIENT_FMWMSGPARAMS_H_

/* ========================================================================== */
/*                             Include Files                                  */
/* ========================================================================== */

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/* ========================================================================== */
/*                           Macros & Typedefs                                */
/* ========================================================================== */

/** Undefined Param Undefined */
#define TISCI_PARAM_UNDEF                                        (0xFFFFFFFFU)

/**
 *  \anchor Sciclient_FirmwareABI
 *  \name Sciclient Firmware ABI revisions
 *  @{
 *  ABI revisions for compatibility check.
 */
/* ABI Major revision - Major revision changes
*       indicate backward compatibility breakage */
#define SCICLIENT_FIRMWARE_ABI_MAJOR                     (4U)
/* ABI Minor revision - Minor revision changes
*       indicate backward compatibility is maintained,
*       however, new messages OR extensions to existing
*       messages might have been adde */
#define SCICLIENT_FIRMWARE_ABI_MINOR                     (0U)
/** @} */

/** C7X_1(Non Secure): C7x context 1 on Main island */
#define SCICLIENT_CONTEXT_C7X_NONSEC_0 (10U)
#define SCICLIENT_CONTEXT_C7X_SEC_0 (9U)

#define SCICLIENT_C7X_NON_SECURE_INTERRUPT_NUM (9U)
#define SCICLIENT_C7X_SECURE_INTERRUPT_NUM     (10U)
/**
 *  \anchor Sciclient_ContextIds
 *  \name Sciclient Context Ids
 *  @{
 *  Context IDs for Sciclient_ConfigPrms_t .
 */
/** r5(Secure): Cortex R5 Context 0 */
#define SCICLIENT_CONTEXT_R5_SEC_0                      (0U)
/** r5(Non Secure): Cortex R5 Context 1 */
#define SCICLIENT_CONTEXT_R5_NONSEC_0                   (1U)
/** r5(Secure): Cortex R5 Context 2 */
#define SCICLIENT_CONTEXT_R5_SEC_1                      (2U)
/** r5(Non Secure): Cortex R5 Context 3 */
#define SCICLIENT_CONTEXT_R5_NONSEC_1                   (3U)
/** a53(Secure): Cortex A53 context 0 */
#define SCICLIENT_CONTEXT_A53_SEC_0                     (4U)
/** a53(Non Secure): Cortex A53 context 1 */
#define SCICLIENT_CONTEXT_A53_SEC_1                     (5U)
/** a53(Non Secure): Cortex A53 context 2 */
#define SCICLIENT_CONTEXT_A53_NONSEC_1                  (6U)
/** a53(Non Secure): Cortex A53 context 3 */
#define SCICLIENT_CONTEXT_A53_NONSEC_2                  (7U)
/** R5 (Non Secure): Cortex R5 context 1 */
#define SCICLIENT_CONTEXT_MCU_R5_0_NONSEC_0             (8U)
/** a53(Non Secure): Cortex A53 context 4 */
#define SCICLIENT_CONTEXT_A53_NONSEC_3                  (9U)
/** C7x(Non Secure): C7x core*/
#define SCICLIENT_CONTEXT_C7_NONSEC_0                   (10U)
/** DM2TIFS(Non Secure): DM to TIFS Message forwarding */
#define SCICLIENT_CONTEXT_DM2TIFS                       (11U)

/** Total number of possible contexts for application. */
#define SCICLIENT_CONTEXT_MAX_NUM                      (12U)
/** @} */

/**
 *  \anchor Sciclient_ProcessorIds
 *  \name Sciclient Processor Ids
 *  @{
 *  Processor IDs for the Processor Boot Configuration APIs.
 */

/**
 * AM62D_MAIN_SEC_MMR_MAIN_0: (Cluster 9 Processor 0)
 */
#define SCICLIENT_PROC_ID_A53SS0_CORE_0 (0x20U)

/**
 * AM62D_MAIN_SEC_MMR_MAIN_0: (Cluster 9 Processor 1)
 */
#define SCICLIENT_PROC_ID_A53SS0_CORE_1 (0x21U)

/**
 * AM62D_MAIN_SEC_MMR_MAIN_0: (Cluster 9 Processor 2)
 */
#define SCICLIENT_PROC_ID_A53SS0_CORE_2 (0x22U)

/**
 * AM62D_MAIN_SEC_MMR_MAIN_0: (Cluster 9 Processor 3)
 */
#define SCICLIENT_PROC_ID_A53SS0_CORE_3 (0x23U)

/**
 * AM62D_MAIN_SEC_MMR_MAIN_0: (Cluster 13 Processor 0)
 */
#define SCICLIENT_PROC_ID_C7X256V0_C7XV_CORE_0 (0x04U)

/**
 * AM62D_MCU_SEC_MMR_MCU_0: (Cluster 0 Processor 0)
 */
#define SCICLIENT_PROC_ID_MCU_R5FSS0_CORE0 (0x03U)

/**
 * AM62D_WKUP_SEC_MMR_WKUP_0: (Cluster 28 Processor 0)
 */
#define SCICLIENT_PROC_ID_R5FSS0_CORE0 (0x01U)

/**
 * AM62D_HSM_SEC_MMR_0: (Cluster 28 Processor 0)
 */
#define SCICLIENT_PROC_ID_HSM_M4FSS0_CORE0 (0x80U)

/**
 * Total Number of processors in AM62D
 */
#define SOC_NUM_SCICLIENT_PROCESSORS (0x08U)


/** @} */

/** -------------------- Resource Management Parameters ---------------------*/
#define TISCI_MSG_VALUE_RM_NULL_RING_TYPE     (0xFFFFu)
#define TISCI_MSG_VALUE_RM_NULL_RING_INDEX    (0xFFFFFFFFu)
#define TISCI_MSG_VALUE_RM_NULL_RING_ADDR     (0xFFFFFFFFu)
#define TISCI_MSG_VALUE_RM_NULL_RING_COUNT    (0xFFFFFFFFu)
/**
 * The ring mode field of the RING_SIZE register is not modified if this value
 * is used for:
 * @ref tisci_msg_rm_ring_cfg_req::mode
 */
#define TISCI_MSG_VALUE_RM_NULL_RING_MODE      (0xFFu)
#define TISCI_MSG_VALUE_RM_NULL_RING_SIZE      (0xFFu)
#define TISCI_MSG_VALUE_RM_NULL_ORDER_ID       (0xFFu)
#define TISCI_MSG_VALUE_RM_UDMAP_NULL_CH_TYPE  (0xFFu)
#define TISCI_MSG_VALUE_RM_UDMAP_NULL_CH_INDEX (0xFFFFFFFFu)

/** ------------ Power Management Messages Parameters -----------------------*/
/**
 *  \anchor Sciclient_PmDeviceIds
 *  \name Power Management Device IDs
 *  @{
 *  Power Management Module Device IDs
 */
#include <drivers/sciclient/include/tisci/am62dx/tisci_devices.h>
/** @} */

/**
 *  \anchor Sciclient_PmModuleClockIds
 *  \name Power Management Clock IDs Module Wise
 *  @{
 *  Power Management Module Clock IDs for individual modules.
 */
#include <drivers/sciclient/include/tisci/am62dx/tisci_clocks.h>
/** @} */


/**
 *  \anchor Sciclient_WkupR5fIds
 *  \name WKUP Pulsar IDs
 *  @{
 *  WKUP Device CPU IDs.
 */
#define SCICLIENT_DEV_WKUP_R5FSS0_CORE0  (TISCI_DEV_WKUP_R5FSS0_CORE0)
#define SCICLIENT_DEV_WKUP_R5FSS0_CORE1  (TISCI_DEV_WKUP_R5FSS0_CORE0)
/** @} */

/**
 * \brief Special ISC ID to refer to compute cluster privid registers
 */
#define TISCI_ISC_CC_ID                (160U)


/**
 *  \anchor Sciclient_IrqSrcIdxStart
 *  \name IRQ source index start
 *  @{
 *  Start offset of IRQ source index.
 */
#define TISCI_RINGACC0_OES_IRQ_SRC_IDX_START        (0U)
#define TISCI_RINGACC0_MON_IRQ_SRC_IDX_START        (1024U)
#define TISCI_RINGACC0_EOES_IRQ_SRC_IDX_START       (2048U)
#define TISCI_UDMAP0_TX_OES_IRQ_SRC_IDX_START       (0U)
#define TISCI_UDMAP0_TX_EOES_IRQ_SRC_IDX_START      (512U)
#define TISCI_UDMAP0_RX_OES_IRQ_SRC_IDX_START       (1024U)
#define TISCI_UDMAP0_RX_EOES_IRQ_SRC_IDX_START      (1280U)
#define TISCI_UDMAP0_RX_FLOW_EOES_IRQ_SRC_IDX_START (1536U)
#define TISCI_BCDMA0_BC_RC_OES_IRQ_SRC_IDX_START     (9216U)
#define TISCI_BCDMA0_TX_RC_OES_IRQ_SRC_IDX_START     (10752U)
#define TISCI_BCDMA0_RX_RC_OES_IRQ_SRC_IDX_START     (12288U)
#define TISCI_BCDMA0_BC_DC_OES_IRQ_SRC_IDX_START     (8704U)
#define TISCI_BCDMA0_TX_DC_OES_IRQ_SRC_IDX_START     (10240U)
#define TISCI_BCDMA0_RX_DC_OES_IRQ_SRC_IDX_START     (11776U)
#define TISCI_PKTDMA0_TX_FLOW_OES_IRQ_SRC_IDX_START  (4608U)
#define TISCI_PKTDMA0_RX_FLOW_OES_IRQ_SRC_IDX_START  (5632U)
/** @} */

/**
 *  \anchor Sciclient_WkupR5fProcIds
 *  \name WKUP Pulsar Processor IDs
 *  @{
 *  WKUP Device Processor IDs.
 */
#define SCICLIENT_DEV_WKUP_R5FSS0_CORE0_PROCID  \
    (SCICLIENT_PROC_ID_R5FSS0_CORE0)
#define SCICLIENT_DEV_WKUP_R5FSS0_CORE1_PROCID  \
    (SCICLIENT_PROC_ID_R5FSS0_CORE0)
/** @} */

/** Board config Base start address */
#define SCICLIENT_ALLOWED_BOARDCFG_BASE_START 1
/** Board config Base end address */
#define SCICLIENT_ALLOWED_BOARDCFG_BASE_END   0xFFFFFFFFU

/* ========================================================================== */
/*                         Structure Declarations                             */
/* ========================================================================== */

/* None */

#ifdef __cplusplus
}
#endif

#endif /* #ifndef SCICLIENT_FMWMSGPARAMS_H_ */

/** @} */
