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

/**
 *  \file udma_priv.h
 *
 *  \brief UDMA private header file.
 */

#ifndef UDMA_PRIV_H_
#define UDMA_PRIV_H_

/* ========================================================================== */
/*                             Include Files                                  */
/* ========================================================================== */

/* Disable logs and assert - make it 1 and rebuild to enable */
#ifdef DebugP_LOG_ENABLED
#undef DebugP_LOG_ENABLED
#endif /* DebugP_LOG_ENABLED */

#define DebugP_LOG_ENABLED 0

#ifdef DebugP_ASSERT_ENABLED
#undef DebugP_ASSERT_ENABLED
#endif /* DebugP_ASSERT_ENABLED */

#define DebugP_ASSERT_ENABLED 0

/* This is needed for memset/memcpy */
#include <string.h>

#include <drivers/udma.h>

#include <kernel/dpl/HwiP.h>
#include <kernel/dpl/ClockP.h>
#include <kernel/dpl/CacheP.h>
#include <kernel/dpl/SemaphoreP.h>

#if (UDMA_SOC_CFG_RA_LCDMA_PRESENT == 1)
#include <drivers/udma/v1/hw_include/csl_lcdma_ringacc.h>
#endif
#if (UDMA_SOC_CFG_LCDMA_PRESENT == 1)
#include <drivers/udma/v1/hw_include/csl_bcdma.h>
#include <drivers/udma/v1/hw_include/csl_pktdma.h>
#endif

#ifdef __cplusplus
extern "C" {
#endif

/* ========================================================================== */
/*                           Macros & Typedefs                                */
/* ========================================================================== */


/**
 *  \anchor Udma_InstanceType
 *  \name UDMA Instance Type
 *
 *  UDMA instance Type - UDMA/LCDMA_BCDMA/LCDMA_PKTDMA
 *
 *  @{
 */
/** \brief UDMA instance type*/
#define UDMA_INST_TYPE_NORMAL           (0U)
/** \brief LCDMA - Block Copy DMA instance type*/
#define UDMA_INST_TYPE_LCDMA_BCDMA      (1U)
/** \brief LCDMA - Packet DMA instance type*/
#define UDMA_INST_TYPE_LCDMA_PKTDMA     (2U)
/** \brief Maximum number of UDMA instance types */
#define UDMA_INST_TYPE_MAX              (3U)
/* @} */

/**
 *  \anchor Udma_RingAccType
 *  \name UDMA Ring Accelerator Type
 *
 *  UDMA ring accelerator Type - Normal RA/Lcdma RA
 *
 *  @{
 */
/** \brief Normal RA type*/
#define UDMA_RA_TYPE_NORMAL             (0U)
/** \brief Lcdma RA type*/
#define UDMA_RA_TYPE_LCDMA              (1U)
/** \brief Maximum number of RA types */
#define UDMA_RA_TYPE_MAX                (2U)
/* @} */

/** \brief TR Trigger type*/
/* Global trigger 0*/
#define UDMA_TR_FLAGS_TRIGGER_GLOBAL0          ((uint32_t) 1U)
/* Local event in TR trigger0*/
#define UDMA_TR_FLAGS_TRIGGER_LOCAL0           ((uint32_t) 2U)
/* Local event in TR trigger1*/
#define UDMA_TR_FLAGS_TRIGGER_LOCAL1           ((uint32_t) 3U)

/** \brief Macro used to specify that the thread ID is invalid. */
#define UDMA_THREAD_ID_INVALID          ((uint32_t) 0xFFFF0004U)

/** \brief UDMA driver handle */
typedef struct Udma_DrvObjectInt_t     *Udma_DrvHandleInt;
/** \brief UDMA channel handle */
typedef struct Udma_ChObjectInt_t      *Udma_ChHandleInt;
/** \brief UDMA event handle */
typedef struct Udma_EventObjectInt_t   *Udma_EventHandleInt;
/** \brief UDMA ring handle */
typedef struct Udma_RingObjectInt_t    *Udma_RingHandleInt;
/** \brief UDMA flow handle */
typedef struct Udma_FlowObjectInt_t    *Udma_FlowHandleInt;

/**
 *  \anchor Udma_RingLocalApiPrototypes
 *  \name UDMA Ring Local API's function prototypes
 *
 *  Function prototypes for various local UDMA Ring API's.
 *  For Normal RA / LCDMA RA, function pointers will be used
 *  to call the appropriate function.
 *
 *  @{
 */
/** \brief UDMA Ring handle clear register function prototype */
typedef void (*Udma_ringHandleClearRegsFxn)(Udma_RingHandleInt ringHandle);
/** \brief UDMA Ring set doorbell function prototype */
typedef void (*Udma_ringSetDoorBellFxn)(Udma_RingHandleInt ringHandle,
                                        int32_t count);
/** \brief UDMA Ring prime function prototype */
typedef void (*Udma_ringPrimeFxn)(Udma_RingHandleInt ringHandle,
                                  uint64_t phyDescMem);
/** \brief UDMA Ring prime read function prototype */
typedef void (*Udma_ringPrimeReadFxn)(Udma_RingHandleInt ringHandle,
                                      uint64_t *phyDescMem);
/** \brief UDMA Ring get mem pointer function prototype */
typedef void *(*Udma_ringGetMemPtrFxn)(Udma_RingHandleInt ringHandle);
/** \brief UDMA Ring get ring mode function prototype */
typedef uint32_t (*Udma_ringGetModeFxn)(Udma_RingHandleInt ringHandle);
/** \brief UDMA Ring get element count function prototype */
typedef uint32_t (*Udma_ringGetElementCntFxn)(Udma_RingHandleInt ringHandle);
/** \brief UDMA Ring get forward ring occupancy function prototype */
typedef uint32_t (*Udma_ringGetForwardRingOccFxn)(Udma_RingHandleInt ringHandle);
/** \brief UDMA Ring get reverse ring occupancy function prototype */
typedef uint32_t (*Udma_ringGetReverseRingOccFxn)(Udma_RingHandleInt ringHandle);
/** \brief UDMA Ring get write index value function prototype */
typedef uint32_t (*Udma_ringGetWrIdxFxn)(Udma_RingHandleInt ringHandle);
/** \brief UDMA Ring get read index value function prototype */
typedef uint32_t (*Udma_ringGetRdIdxFxn)(Udma_RingHandleInt ringHandle);
/** \brief UDMA Ring dequeue raw function prototype */
typedef int32_t (*Udma_ringDequeueRawFxn)(Udma_DrvHandleInt  drvHandle,
                                          Udma_RingHandleInt ringHandle,
                                          uint64_t *phyDescMem);
/** \brief UDMA Ring queue raw function prototype */
typedef int32_t (*Udma_ringQueueRawFxn)(Udma_DrvHandleInt  drvHandle,
                                        Udma_RingHandleInt ringHandle,
                                        uint64_t phyDescMem);
/** \brief UDMA Ring flush raw function prototype */
typedef int32_t (*Udma_ringFlushRawFxn)(Udma_DrvHandleInt  drvHandle,
                                        Udma_RingHandleInt ringHandle,
                                        uint64_t *phyDescMem);
/** \brief UDMA Ring set Cfg function prototype */
typedef void (*Udma_ringSetCfgFxn)(Udma_DrvHandleInt drvHandle,
                                   Udma_RingHandleInt ringHandle,
                                   const Udma_RingPrms *ringPrms);
/* @} */

/**
 *  \anchor Udma_RmMaxSize
 *  Resource management related macros.
 *
 *  These values are based on an optimal value typically used for allocation
 *  per core and not based on actual resources in a given SOC.
 *
 *  Note: Kept to be multiple of 32 to store as bit fields in uint32_t
 *  @{
 */
#define UDMA_RM_MAX_BLK_COPY_CH             (256U)
#define UDMA_RM_MAX_BLK_COPY_HC_CH          (256U)
#define UDMA_RM_MAX_BLK_COPY_UHC_CH         (256U)
#define UDMA_RM_MAX_TX_RX_CH                (256U)
#define UDMA_RM_MAX_TX_HC_CH                (256U)
#define UDMA_RM_MAX_TX_UHC_CH               (256U)
#define UDMA_RM_MAX_RX_HC_CH                (256U)
#define UDMA_RM_MAX_RX_UHC_CH               (256U)
#define UDMA_RM_MAX_IR_INTR                 (600U)

/* Array allocation macros */
#define UDMA_RM_BLK_COPY_CH_ARR_SIZE        (UDMA_RM_MAX_BLK_COPY_CH >> 5U)
#define UDMA_RM_BLK_COPY_HC_CH_ARR_SIZE     (UDMA_RM_MAX_BLK_COPY_HC_CH >> 5U)
#define UDMA_RM_BLK_COPY_UHC_CH_ARR_SIZE    (UDMA_RM_MAX_BLK_COPY_UHC_CH >> 5U)
#define UDMA_RM_TX_RX_CH_ARR_SIZE           (UDMA_RM_MAX_TX_RX_CH >> 5U)
#define UDMA_RM_TX_HC_CH_ARR_SIZE           (UDMA_RM_MAX_TX_HC_CH >> 5U)
#define UDMA_RM_TX_UHC_CH_ARR_SIZE          (UDMA_RM_MAX_TX_UHC_CH >> 5U)
#define UDMA_RM_RX_HC_CH_ARR_SIZE           (UDMA_RM_MAX_RX_HC_CH >> 5U)
#define UDMA_RM_RX_UHC_CH_ARR_SIZE          (UDMA_RM_MAX_RX_UHC_CH >> 5U)
#define UDMA_RM_IR_INTR_ARR_SIZE            (UDMA_RM_MAX_IR_INTR >> 5U)
/* @} */

/** \brief Default ring order ID */
#define UDMA_DEFAULT_RING_ORDER_ID      (0U)

/** \brief Default TX channel DMA priority */
#define UDMA_DEFAULT_TX_CH_DMA_PRIORITY                                     \
                                    (CSL_BCDMA_CHAN_SCHED_PRI_MED_HIGH)
/** \brief Default RX channel DMA priority */
#define UDMA_DEFAULT_RX_CH_DMA_PRIORITY                                     \
                                    (CSL_BCDMA_CHAN_SCHED_PRI_MED_HIGH)
/** \brief Default TX channel bus priority */
#define UDMA_DEFAULT_TX_CH_BUS_PRIORITY (4U)
/** \brief Default RX channel bus priority */
#define UDMA_DEFAULT_RX_CH_BUS_PRIORITY (4U)

/** \brief Default TX channel bus QOS */
#define UDMA_DEFAULT_TX_CH_BUS_QOS      (4U)
/** \brief Default RX channel bus QOS */
#define UDMA_DEFAULT_RX_CH_BUS_QOS      (4U)

/** \brief Default TX channel bus order ID */
#define UDMA_DEFAULT_TX_CH_BUS_ORDERID  (0U)
/** \brief Default RX channel bus order ID */
#define UDMA_DEFAULT_RX_CH_BUS_ORDERID  (0U)


/** \brief Macro used to specify that init is performed for an object. */
#define UDMA_INIT_DONE                  (0xABDCABCDU)
/** \brief Macro used to specify that deinit is performed for an object. */
#define UDMA_DEINIT_DONE                (0x00000000U)

/** \brief Macro used to specify that channel teardown status is not available */
#define UDMA_EVENT_CH_TEARDOWN_STATUS_NA           ((uint32_t) 0x0001U)
/** \brief Macro used to specify that channel teardown is complete */
#define UDMA_EVENT_CH_TEARDOWN_STATUS_COMPLETE     ((uint32_t) 0x0002U)
/** \brief Macro used to specify that channel teardown is not complete */
#define UDMA_EVENT_CH_TEARDOWN_STATUS_INCOMPLETE   ((uint32_t) 0x0003U)

/**
 * Maximum encoded size threshold setting
 */
#define  UDMAP_RX_FLOW_SIZE_THRESH_MAX       (7u)

/**
 * Overwrite with src_tag[7:0] from back end application.  Source tag
 * byte selector configuration
 */
#define UDMAP_RX_FLOW_SRC_SELECT_SRC_TAG     (4u)

/**
 * Drop the received packet on starvation error.  See the UDMAP section of the
 * TRM for more information on this setting.
 */
#define UDMAP_RX_FLOW_ERR_DROP              (0u)

/**
 * Configure receive flow to place the PS words at the end of the packet
 * descriptor.  See the UDMAP section of the TRM for more information on this
 * setting.
 */
#define UDMAP_RX_FLOW_PS_END_PD             (0u)

/**
 * Configure receive flow to handle extended packet info not present in the
 * received packet descriptor.  See the UDMAP section of the TRM for more
 * information on this setting.
 */
#define UDMAP_RX_FLOW_EINFO_NOT_PRESENT     (0u)
/**
 * Configure receive flow to handle extended packet info present in the
 * received packet descriptor.  See the UDMAP section of the TRM for more
 * information on this setting.
 */
#define UDMAP_RX_FLOW_EINFO_PRESENT         (1u)
/**
 * Configure receive flow to handle PS words not present in the received packet
 * descriptor.  See the UDMAP section of the TRM for more information on this
 * setting.
 */
#define UDMAP_RX_FLOW_PSINFO_NOT_PRESENT    (0u)
/**
 * Configure receive flow to handle PS words present in the received packet
 * descriptor.  See the UDMAP section of the TRM for more information on this
 * setting.
 */
#define UDMAP_RX_FLOW_PSINFO_PRESENT        (1u)
/**
 * Re-try descriptor allocation operation on starvation error.  See the UDMAP
 * section of the TRM for more information on this setting.
 */
#define UDMAP_RX_FLOW_ERR_RETRY             (1u)
/**
 * Host descriptor configure
 */
#define UDMAP_RX_FLOW_DESC_HOST             (0u)
/**
 * Monolithic descriptor configure
 */
#define UDMAP_RX_FLOW_DESC_MONO             (2u)

/* ========================================================================== */
/*                         Structure Declarations                             */
/* ========================================================================== */

/**
 *  \brief UDMA ring object.
 *
 *  Note: This is an internal/private driver structure and should not be
 *  used or modified by caller.
 */
typedef struct Udma_RingObjectInt_t
{
    Udma_DrvHandleInt           drvHandle;
    /**< Pointer to global driver handle. */

    uint16_t                    ringNum;
    /**< Ring number */

#if (UDMA_SOC_CFG_RA_LCDMA_PRESENT == 1)
    CSL_LcdmaRingaccRingCfg          lcdmaCfg;
    /**< Lcdma Ring config */
    volatile CSL_lcdma_ringacc_flow_cfg_v2Regs_flow  *pLcdmaCfgRegs;
    /**< Pointer to Lcdma RA config register overlay */
    volatile CSL_lcdma_ringacc_flowrt_v2Regs_flow *pLcdmaRtRegs;
    /**< Pointer to Lcdma RA RT config register overlay */
#endif

    uint32_t                    ringInitDone;
    /**< Flag to set the ring object is init. */

    uint32_t                    mappedRingGrp;
    /**< The allocated mapped ring group when channel type is
     *   #UDMA_CH_TYPE_TX_MAPPED or #UDMA_CH_TYPE_RX_MAPPED.
     *
     *   This is needed to free the mapped ring.
     *
     *   Refer \ref Udma_MappedTxGrpSoc macro for details about mapped TX ring groups
     *   or \ref Udma_MappedRxGrpSoc macro for details about mapped RX ring groups.
     *
     *   For unmapped case, this will be #UDMA_MAPPED_GROUP_INVALID
     */
    uint32_t                    mappedChNum;
    /**< The assigned mapped channel number when channel type is
     *   #UDMA_CH_TYPE_TX_MAPPED or #UDMA_CH_TYPE_RX_MAPPED.
     *
     *   This is needed to free the mapped ring.
     *
     *   For unmapped case, this will be #UDMA_DMA_CH_INVALID.
     */
} Udma_RingObjectInt;

/**
 *  \brief UDMA flow object.
 *
 *  Note: This is an internal/private driver structure and should not be
 *  used or modified by caller.
 */
typedef struct Udma_FlowObjectInt_t
{
    Udma_DrvHandleInt       drvHandle;
    /**< Pointer to global driver handle. */

    uint32_t                flowStart;
    /**< Flow ID start number.
     *
     *   Note: In case of mapped flow(in devices like AM64x), this indicates the
     *   mapped flow idx managed by this flow handle.
     *
    */
    uint32_t                flowCnt;
    /**< Number of flow IDs allocated - Contiguos flows are allocated
     *
     *   Note: In case of mapped flow(in devices like AM64x), this will be 1
     *   since only one mapped flow is managed by a flow handle.
    */

    uint32_t                flowIdx;
    /** Requested flow Index */

    uint32_t                flowInitDone;
    /**< Flag to set the flow object is init. */

    uint32_t                mappedFlowGrp;
    /**< The allocated mapped flow group when channel type is
     *   #UDMA_CH_TYPE_RX_MAPPED.
     *
     *   This is needed to free the mapped flow.
     *
     *   Refer \ref Udma_MappedRxGrpSoc macro for details about mapped RX flow groups.
     *
     *   For unmapped case, this will be #UDMA_MAPPED_GROUP_INVALID
     */
    uint32_t                mappedChNum;
    /**< The assigned mapped channel number when channel type is
     *   #UDMA_CH_TYPE_RX_MAPPED.
     *
     *   This is needed to free the mapped flow.
     *
     *   For unmapped case, this will be #UDMA_DMA_CH_INVALID.
     */
} Udma_FlowObjectInt;

/**
 *  \brief UDMA event object.
 *
 *  Note: This is an internal/private driver structure and should not be
 *  used or modified by caller.
 */
typedef struct Udma_EventObjectInt_t
{
    Udma_DrvHandleInt       drvHandle;
    /**< Pointer to global driver handle. */
    Udma_EventPrms          eventPrms;
    /**< Event parameters passed during event registeration. */

    uint32_t                coreIntrNum;
    /**< Allocated core interrupt number. */

    Udma_EventHandleInt     nextEvent;
    /**< Pointer to next event - used in shared event for traversing in ISR */
    Udma_EventHandleInt     prevEvent;
    /**< Pointer to previous event - used in shared event for traversing during
     *   event un-registration */

    void                   *hwiHandle;
    /**< HWI handle. */
    HwiP_Object             hwiObject;
    /**< HWI Object. */

    uint32_t                eventInitDone;
    /**< Flag to set the event object is init. */
    uint32_t                flowNum;
    /**< Flow attributes for a given channel. */
} Udma_EventObjectInt;

/**
 *  \brief UDMA mapped channel ring attributes.
 */
typedef struct
{
    uint32_t    peerChNum;
    /**< PeerchNum passed for PDMA channels. */
    uint32_t    ChNum;
    /**< Associated Tx/Rx channel number. */
    uint32_t    pktdmaIntNum;
    /**< Associated pktdma Tx/Rx interrupt number. */
    uint32_t    bcdmaIntNum;
    /**< Associated pktdma Tx/Rx interrupt number. */
    uint32_t    flowCnt;
    /**< Associated pktdma Tx/Rx flow count. */
    uint32_t    flowIdx;
    /**< Associated pktdma Tx/Rx flow Index. */
} Udma_ChAttributes;

/**
 *  \brief UDMA mapped channel ring attributes.
 */
typedef struct
{
    uint32_t    ChNum;
    /**< Associated Tx/Rx channel number. */
    uint32_t    flowCnt;
    /**< Associated pktdma Tx/Rx flow count. */
    uint32_t    flowIdx;
    /**< Associated pktdma Tx/Rx flow count. */
} Udma_FlowAttributes;

/**
 *  \brief UDMA channel object.
 *
 *  Note: This is an internal/private driver structure and should not be
 *  used or modified by caller.
 */
typedef struct Udma_ChObjectInt_t
{
    uint32_t                chType;
    /**< UDMA channel type. Refer \ref Udma_ChType. */
    Udma_ChPrms             chPrms;
    /**< UDMA channel attributes. Refer \ref Udma_ChType. */
    Udma_ChAttributes       chAttr;
    /**< Object to store the channel params. */
    Udma_DrvHandleInt       drvHandle;
    /**< Pointer to global driver handle. */

    uint32_t                txChNum;
    /**< Allocated TX channel number - this is relative channel number from
     *   base TX channel. This is valid only when the channel is opened for
     *   TX and block copy mode */
    uint32_t                rxChNum;
    /**< Allocated RX channel number - this is relative channel number from
     *   base RX channel. This is valid only when the channel is opened for
     *   RX and block copy mode */
    uint32_t                pdmaChNum;
    /**< Allocated peer PDMA channel number. This is valid only when the
     *   channel is opened for PDMA mode */
    uint32_t                peerThreadId;
    /**< Peer channel thread ID - this is or'ed with thread offset. */

    Udma_RingHandleInt      fqRing;
    /**< Free queue ring handle */
    Udma_RingHandleInt      cqRing;
    /**< Completion queue ring handle
    *    For AM64x kind of devices, where there is no seperate Completion queue,
    *    this points to fqRing itself.
    */
    Udma_RingHandleInt      tdCqRing;
    /**< Teardown completion queue ring handle */

    Udma_RingObjectInt      fqRingObj;
    /**< Free queue ring object */
    Udma_RingObjectInt      cqRingObj;
    /**< Completion queue ring object
    *    Not used for AM64x kind of devices, where there is no seperate Completion queue.
    */
    Udma_RingObjectInt      tdCqRingObj;
    /**< Teardown completion queue ring object
    *    Not used for AM64x kind of devices, where teardown function is not present.
    */

    Udma_FlowHandleInt      defaultFlow;
    /**< Default flow handle */
    Udma_FlowObjectInt      defaultFlowObj;
    /**< Default flow object - Flow ID equal to the RX channel is reserved
     *   as the default flow for the channel. This object is used for
     *   providing handle to the caller to re-program the default flow using
     *   the standard flow API's */

    Udma_ChTxPrms           txPrms;
    /**< TX channel parameter passed during channel config. */
    Udma_ChRxPrms           rxPrms;
    /**< RX channel parameter passed during channel config. */

#if (UDMA_SOC_CFG_LCDMA_PRESENT == 1)
    /* Below BCDMA register overlay pointers provided for debug purpose to
     * readily view the registers */
    volatile CSL_bcdma_chcfg_v2Regs_chan   *pBcdmaBcCfgRegs;
    /**< Pointer to BCDMA Block copy config register overlay */
    volatile CSL_bcdma_chrt_v2Regs_chan    *pBcdmaBcRtRegs;
    /* Below PKTDMA register overlay pointers provided for debug purpose to
     * readily view the registers */
    volatile CSL_pktdma_chcfg_v2Regs_chan  *pPktdmaCfgRegs;
    /**< Pointer to PKTDMA Block copy config register overlay for realtime register*/
    volatile CSL_pktdma_chrt_v2Regs_chan   *pPktdmaRtRegs;
#endif
    uint32_t                chInitDone;
    /**< Flag to set the channel object is init. */
    uint32_t                chOesAllocDone;
    /**< Flag to check if the channel's OES is allocated. This is required
     *   because the channel OES is used for chaining as well as for
     *   TR event registeration. This allows to check for error when both
     *   are requested by user on the same channel */
    uint32_t                trigger;
    /**< Channel trigger used when chaining channels - needed at the time of
     *   breaking the chaining */
} Udma_ChObjectInt;

/**
 *  \brief UDMA driver object.
 *
 *  Note: This is an internal/private driver structure and should not be
 *  used or modified by caller.
 */
typedef struct Udma_DrvObjectInt_t
{
    uint32_t                instType;
    /**< Udma Instance Type */
    uint32_t                raType;
    /**< Udma Ring Accelerator Type */

#if (UDMA_SOC_CFG_LCDMA_PRESENT == 1)
    /*
     * LCDMA DMSS specific instance parameters
     */
    CSL_BcdmaCfg             bcdmaRegs;
    /**< BCDMA register configuration */
    CSL_PktdmaCfg            pktdmaRegs;
    /**< PKTDMA register configuration */
#endif
#if (UDMA_SOC_CFG_RA_LCDMA_PRESENT == 1)
    CSL_LcdmaRingaccCfg     lcdmaRaRegs;
#endif
    /**< Interrupt Aggregator configuration */
    uint32_t                udmapSrcThreadOffset;
    /**< UDMAP Source/TX thread offset */
    uint32_t                udmapDestThreadOffset;
    /**< UDMAP Dest/RX thread offset */
    uint32_t                maxRings;
    /**< Maximun number of rings present in the NAVSS instance */

    /*
     * Channel Offsets
     */
    uint32_t                txChOffset;
    /**< TX channel offset. */
    uint32_t                extChOffset;
    /**< External channel offset. */
    uint32_t                rxChOffset;
    /**< RX channel offset. */
    /*
     *  The driver allocates ringNum = chNum (for BlkCpoy)
                                     = chNum + txChOffset (for SplitTR Tx)
                                     = chNum + rxChOffset (for SplitTR Rx)

        For CSL_bcdma* API's passed param ->channel_num = txChNum (for BlkCopy)
                                                        = txChNum + txChOffset (for SplitTR Tx)
                                                        = rxChNum + rxChOffset (for SplitTR Rx)
    */

    Udma_EventObjectInt     globalEventObj;
    /**< Object to store global event. */
    Udma_EventHandleInt     globalEventHandle;
    /**< Global event handle. */

    Udma_InitPrms           initPrms;
    /**< Object to store the init params. */
    Udma_RmInitPrms         rmInitPrms;
    /**< RM init parameters */
    uint32_t                drvInitDone;
    /**< Flag to check if the driver object is init properly or not. */

    /*
     * RM objects.
     * This is a bitwise flag
     * 1 - free, 0 - allocated
     */
    uint32_t                blkCopyChFlag[UDMA_RM_BLK_COPY_CH_ARR_SIZE];
    /**< UDMA Block copy channel allocation flag */
    uint32_t                blkCopyHcChFlag[UDMA_RM_BLK_COPY_HC_CH_ARR_SIZE];
    /**< UDMA high capacity Block copy channel allocation flag */
    uint32_t                blkCopyUhcChFlag[UDMA_RM_BLK_COPY_UHC_CH_ARR_SIZE];
    /**< UDMA ultra high capacity Block copy channel allocation flag */

    uint32_t                pktDmaChFlag[UDMA_RM_TX_RX_CH_ARR_SIZE];
    /**< UDMA TX channel allocation flag */
    uint32_t                txHcChFlag[UDMA_RM_TX_HC_CH_ARR_SIZE];
    /**< UDMA high capacity TX channel allocation flag */
    uint32_t                txUhcChFlag[UDMA_RM_TX_UHC_CH_ARR_SIZE];

    /**< UDMA ultra high capacity TX channel allocation flag */
    uint32_t                bcDmaChFlag[UDMA_RM_TX_RX_CH_ARR_SIZE];
    /**< UDMA RX channel allocation flag */
    uint32_t                rxHcChFlag[UDMA_RM_RX_HC_CH_ARR_SIZE];
    /**< UDMA high capacity RX channel allocation flag */
    uint32_t                rxUhcChFlag[UDMA_RM_RX_UHC_CH_ARR_SIZE];
    /**< UDMA ultra high capacity RX channel allocation flag */
    uint32_t                irIntrFlag[UDMA_RM_IR_INTR_ARR_SIZE];
    /**< IR interrupt allocation flag */

    void                   *rmLock;
    /**< Mutex to protect RM allocation. */
    SemaphoreP_Object       rmLockObj;
    /**< Mutex object. */
    /*
     * UDMA Ring Local API's function pointers
     * For Normal RA / LCDMA RA, these function pointers are used
     * to call the appropriate function.
     */
    Udma_ringDequeueRawFxn            ringDequeueRaw;
    /**< UDMA Ring dequeue raw function pointer */
    Udma_ringQueueRawFxn              ringQueueRaw;
    /**< UDMA Ring queue raw function pointer */
    Udma_ringFlushRawFxn              ringFlushRaw;
    /**< UDMA Ring flush raw function pointer */
    Udma_ringGetElementCntFxn         ringGetElementCnt;
    /**< UDMA Ring get element count function pointer */
    Udma_ringGetMemPtrFxn             ringGetMemPtr;
    /**< UDMA Ring get mem pointer function pointer */
    Udma_ringGetModeFxn               ringGetMode;
    /**< UDMA Ring get ring mode function pointer */
    Udma_ringGetForwardRingOccFxn     ringGetForwardRingOcc;
    /**< UDMA Ring get forward ring occupancy function pointer */
    Udma_ringGetReverseRingOccFxn     ringGetReverseRingOcc;
    /**< UDMA Ring get reverse ring occupancy function pointer */
    Udma_ringGetWrIdxFxn              ringGetWrIdx;
    /**< UDMA Ring get write index value function pointer */
    Udma_ringGetRdIdxFxn              ringGetRdIdx;
    /**< UDMA Ring get read index value function pointer */
    Udma_ringPrimeFxn                 ringPrime;
    /**< UDMA Ring prime function pointer */
    Udma_ringPrimeReadFxn             ringPrimeRead;
    /**< UDMA Ring prime read function pointer */
    Udma_ringSetDoorBellFxn           ringSetDoorBell;
    /**< UDMA Ring set doorbell function pointer */
    Udma_ringSetCfgFxn                ringSetCfg;
    /**< UDMA Ring set Cfg function pointer */
    Udma_ringHandleClearRegsFxn       ringHandleClearRegs;
    /**< UDMA Ring handle clear register function pointer */
} Udma_DrvObjectInt;

/* ========================================================================== */
/*                         Global Variables                                   */
/* ========================================================================== */

/* None */

/* ========================================================================== */
/*                          Function Declarations                             */
/* ========================================================================== */

/* SOC APIs */
void Udma_initDrvHandle(Udma_DrvHandleInt drvHandle);
int32_t UdmaRmInitPrms_init(uint32_t instId, Udma_DrvHandleInt drvHandle);
uint32_t Udma_getChAttr(uint32_t peerChNum, Udma_ChAttributes *chAttribute, uint32_t ChNum);

#if (UDMA_SOC_CFG_RA_LCDMA_PRESENT == 1)
/* LCDMA RA APIs*/
void Udma_lcdmaRingaccMemOps(void *pVirtAddr, uint32_t size, uint32_t opsType);
void Udma_ringHandleClearRegsLcdma(Udma_RingHandleInt ringHandle);
void Udma_ringSetDoorBellLcdma(Udma_RingHandleInt ringHandle, int32_t count);
void Udma_ringPrimeLcdma(Udma_RingHandleInt ringHandle, uint64_t phyDescMem);
void Udma_ringPrimeReadLcdma(Udma_RingHandleInt ringHandle, uint64_t *phyDescMem);
void *Udma_ringGetMemPtrLcdma(Udma_RingHandleInt ringHandle);
uint32_t Udma_ringGetModeLcdma(Udma_RingHandleInt ringHandle);
uint32_t Udma_ringGetElementCntLcdma(Udma_RingHandleInt ringHandle);
uint32_t Udma_ringGetForwardRingOccLcdma(Udma_RingHandleInt ringHandle);
uint32_t Udma_ringGetReverseRingOccLcdma(Udma_RingHandleInt ringHandle);
uint32_t Udma_ringGetWrIdxLcdma(Udma_RingHandleInt ringHandle);
uint32_t Udma_ringGetRdIdxLcdma(Udma_RingHandleInt ringHandle);
int32_t Udma_ringDequeueRawLcdma(Udma_DrvHandleInt  drvHandle,
                                 Udma_RingHandleInt ringHandle,
                                 uint64_t *phyDescMem);
int32_t Udma_ringQueueRawLcdma(Udma_DrvHandleInt  drvHandle,
                               Udma_RingHandleInt ringHandle,
                               uint64_t phyDescMem);
int32_t Udma_ringFlushRawLcdma(Udma_DrvHandleInt  drvHandle,
                               Udma_RingHandleInt ringHandle,
                               uint64_t *phyDescMem);
void Udma_ringSetCfgLcdma(Udma_DrvHandleInt drvHandle,
                          Udma_RingHandleInt ringHandle,
                          const Udma_RingPrms *ringPrms);
#endif

/*
 * RM APIs
 */
void Udma_rmInit(Udma_DrvHandleInt drvHandle);
int32_t Udma_rmDeinit(Udma_DrvHandleInt drvHandle);

/* Channel RM APIs */
uint32_t Udma_rmAllocBlkCopyCh(uint32_t preferredChNum, Udma_DrvHandleInt drvHandle);
void Udma_rmFreeBlkCopyCh(uint32_t chNum, Udma_DrvHandleInt drvHandle);
uint32_t Udma_rmAllocBlkCopyHcCh(uint32_t preferredChNum, Udma_DrvHandleInt drvHandle);
void Udma_rmFreeBlkCopyHcCh(uint32_t chNum, Udma_DrvHandleInt drvHandle);
uint32_t Udma_rmAllocBlkCopyUhcCh(uint32_t preferredChNum, Udma_DrvHandleInt drvHandle);
void Udma_rmFreeBlkCopyUhcCh(uint32_t chNum, Udma_DrvHandleInt drvHandle);
uint32_t Udma_rmAllocCh(uint32_t preferredChNum, Udma_DrvHandleInt drvHandle);
void Udma_rmFreeCh(uint32_t chNum, Udma_DrvHandleInt drvHandle);
uint32_t Udma_rmAllocTxHcCh(uint32_t preferredChNum, Udma_DrvHandleInt drvHandle);
void Udma_rmFreeTxHcCh(uint32_t chNum, Udma_DrvHandleInt drvHandle);
uint32_t Udma_rmAllocRxHcCh(uint32_t preferredChNum, Udma_DrvHandleInt drvHandle);
void Udma_rmFreeRxHcCh(uint32_t chNum, Udma_DrvHandleInt drvHandle);
uint32_t Udma_rmAllocTxUhcCh(uint32_t preferredChNum, Udma_DrvHandleInt drvHandle);
void Udma_rmFreeTxUhcCh(uint32_t chNum, Udma_DrvHandleInt drvHandle);
uint32_t Udma_rmAllocRxUhcCh(uint32_t preferredChNum, Udma_DrvHandleInt drvHandle);
void Udma_rmFreeRxUhcCh(uint32_t chNum, Udma_DrvHandleInt drvHandle);

uint16_t Udma_rmAllocFreeRing(Udma_DrvHandleInt drvHandle);
void Udma_rmFreeFreeRing(uint32_t ringNum, Udma_DrvHandleInt drvHandle);

/* Event RM APIs */
uint32_t Udma_rmAllocIrIntr(uint32_t preferredIrIntrNum,
                              Udma_DrvHandleInt drvHandle);
void Udma_rmFreeIrIntr(uint32_t irIntrNum, Udma_DrvHandleInt drvHandle);
uint32_t Udma_rmTranslateIrOutput(Udma_DrvHandleInt drvHandle, uint32_t irIntrNum);
uint32_t Udma_rmTranslateCoreIntrInput(Udma_DrvHandleInt drvHandle, uint32_t coreIntrNum);
void Udma_rmFreeCoreIntr(uint32_t coreIntrNum, Udma_DrvHandleInt drvHandle);

/* Utils APIs */
uint64_t Udma_virtToPhyFxn(const void *virtAddr,
                           Udma_DrvHandleInt drvHandle,
                           Udma_ChHandleInt chHandle);
void *Udma_phyToVirtFxn(uint64_t phyAddr,
                        Udma_DrvHandleInt drvHandle,
                        Udma_ChHandleInt chHandle);

/* ========================================================================== */
/*                       Static Function Definitions                          */
/* ========================================================================== */

/* None */

#ifdef __cplusplus
}
#endif

#endif /* #ifndef UDMA_PRIV_H_ */
