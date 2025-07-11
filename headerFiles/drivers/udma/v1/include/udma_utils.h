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

/**
 *  \ingroup DRV_UDMA_MODULE
 *  \defgroup DRV_UDMA_UTILS_MODULE UDMA Utils API
 *            This is UDMA driver utilty parameters and API
 *
 *  @{
 */

/**
 *  \file udma_utils.h
 *
 *  \brief UDMA utility API to make TR, get TR descriptor memory requirement.
 */

#ifndef UDMA_UTILS_H_
#define UDMA_UTILS_H_

/* ========================================================================== */
/*                             Include Files                                  */
/* ========================================================================== */

/* None */

#ifdef __cplusplus
extern "C" {
#endif

/* ========================================================================== */
/*                           Macros & Typedefs                                */
/* ========================================================================== */

/**
 *  \anchor Udma_TrType
 *  \name UDMA TR type
 *
 *  Various UDMA TR type supported
 *
 *  @{
 */
#define UDMA_TR_TYPE_0                  (CSL_UDMAP_TR_FLAGS_TYPE_1D_DATA_MOVE)
#define UDMA_TR_TYPE_1                  (CSL_UDMAP_TR_FLAGS_TYPE_2D_DATA_MOVE)
#define UDMA_TR_TYPE_2                  (CSL_UDMAP_TR_FLAGS_TYPE_3D_DATA_MOVE)
#define UDMA_TR_TYPE_3                  (CSL_UDMAP_TR_FLAGS_TYPE_4D_DATA_MOVE)
#define UDMA_TR_TYPE_4                  (CSL_UDMAP_TR_FLAGS_TYPE_4D_DATA_MOVE_FORMATTING)
#define UDMA_TR_TYPE_5                  (CSL_UDMAP_TR_FLAGS_TYPE_4D_CACHE_WARM)
#define UDMA_TR_TYPE_8                  (CSL_UDMAP_TR_FLAGS_TYPE_4D_BLOCK_MOVE)
#define UDMA_TR_TYPE_9                  (CSL_UDMAP_TR_FLAGS_TYPE_4D_BLOCK_MOVE_REPACKING)
#define UDMA_TR_TYPE_10                 (CSL_UDMAP_TR_FLAGS_TYPE_2D_BLOCK_MOVE)
#define UDMA_TR_TYPE_11                 (CSL_UDMAP_TR_FLAGS_TYPE_2D_BLOCK_MOVE_REPACKING)
#define UDMA_TR_TYPE_15                 (CSL_UDMAP_TR_FLAGS_TYPE_4D_BLOCK_MOVE_REPACKING_INDIRECTION)
/** @} */

/**
 *  \brief UDMA TR15 packet descriptor memory size in bytes.
 *  This contains the CSL_UdmapCppi5TRPD + Padding to sizeof(CSL_UdmapTR15) +
 *  N* Type_15 TR (CSL_UdmapTR15) + N* TR response of 4 bytes.
 *  Since CSL_UdmapCppi5TRPD header is less than CSL_UdmapTR15, CSL_UdmapTR15
 *  itself is used for size alignment.
 *
 *  n - Number of TR's present in the TRPD
 */
#define UDMA_GET_TRPD_TR15_SIZE(n)      (UDMA_ALIGN_SIZE(sizeof(CSL_UdmapTR15) + ((n) * (sizeof(CSL_UdmapTR15) + 4U))))

/**
 *  \brief UDMA TR3 packet descriptor memory size in bytes.
 *  This contains the CSL_UdmapCppi5TRPD + Padding to sizeof(CSL_UdmapTR3) +
 *  N* Type_3 TR (CSL_UdmapTR3) + N* TR response of 4 bytes.
 *  Since CSL_UdmapCppi5TRPD header is less than CSL_UdmapTR3, CSL_UdmapTR3
 *  itself is used for size alignment.
 *
 *  n - Number of TR's present in the TRPD
 */
#define UDMA_GET_TRPD_TR3_SIZE(n)       (UDMA_ALIGN_SIZE(sizeof(CSL_UdmapTR3) + ((n) * (sizeof(CSL_UdmapTR3) + 4U))))

/**
 * \brief Offset for TR1 type TR from TRPD start address
 */
#define UDMA_TR1_OFFSET (32U)

/**
 * \brief Offset for TR3 type TR from TRPD start address
 */
#define UDMA_TR3_OFFSET (32U)

/* ========================================================================== */
/*                         Structure Declarations                             */
/* ========================================================================== */

/* None */

/* ========================================================================== */
/*                          Function Declarations                             */
/* ========================================================================== */

/**
 *  \brief UDMA get ring size utility API.
 *
 *  \param mode         [IN] Ring mode.
 *  \param elemCnt      [IN] Ring element count.
 *  \param elemSize     [IN] Ring element size. Refer \ref Udma_RingElemSize
 *
 *  \return Size of the ring memory required in bytes
 */
uint32_t UdmaUtils_getRingMemSize(uint8_t mode,
                                  uint32_t elemCnt,
                                  uint8_t elemSize);

/**
 *  \brief UDMA utility API to make TRPD with TR15 TR type. Other TRPD make
 *  APIs should be called only after this API.
 *
 *  Note: This sets the most commonly used  value for all the fields.
 *  User can override the fields after calling this API if required.
 *
 *  \param trpdMem      [IN] Pointer to TRPD memory
 *  \param trCnt        [IN] Number of TR entries present in the TRPD
 *  \param cqRingNum    [IN] Completion ring number where the TRPD is returned
 *                           after processing
 */
void UdmaUtils_makeTrpdTr15(uint8_t *trpdMem, uint32_t trCnt, uint32_t cqRingNum);

/**
 *  \brief UDMA utility API to make TRPD. Other TRPD make APIs should be
 *  called only after this API.
 *
 *  Note: This sets the most commonly used  value for all the fields.
 *  User can override the fields after calling this API if required.
 *
 *  \param trpdMem      [IN] Pointer to TRPD memory
 *  \param trType       [IN] TR type. Refer \ref Udma_TrType
 *  \param trCnt        [IN] Number of TR entries present in the TRPD
 *  \param cqRingNum    [IN] Completion ring number where the TRPD is returned
 *                           after processing
 */
void UdmaUtils_makeTrpd(uint8_t *trpdMem,
                        uint32_t trType,
                        uint32_t trCnt,
                        uint32_t cqRingNum);

/**
 *  \brief Returns the size of TR (encoded) based on the type
 *
 *  \param trType       [IN] TR type. Refer \ref Udma_TrType
 *
 *  \return Size of the TR record in encoded value to be programmed in TRPD.
 */
uint32_t UdmaUtils_getTrSizeEncoded(uint32_t trType);

/**
 *  \brief Returns the size of TR in bytes based on the type
 *
 *  \param trType       [IN] TR type. Refer \ref Udma_TrType
 *
 *  \return Size of the TR record in bytes.
 */
uint32_t UdmaUtils_getTrSizeBytes(uint32_t trType);

/**
 *  \brief Map L2G event for BCDMA trigger
 *
 *  \param drvHandle    [IN] UDMA driver handle - static memory needs to
 *                           allocated by caller. This is used by the driver to
 *                           maintain the driver states.
 *                           This cannot be NULL.
 *  \param chHandle     [IN] UDMA channel handle.
 *                           This parameter can't be NULL.
 *  \param localeventID [IN] Local event ID for the corresponding IP.
 *
 *  \param eventMode    [IN] Edge triggering signal or pulsed signal.
 *
 *
 *  \return SYSTEMP_SUCESS OR SYSTEMP_FAILURE.
 */
int32_t UdmaUtils_mapLocaltoGlobalEvent(Udma_DrvHandle drvHandle, Udma_ChHandle chHandle,
                                        uint32_t localeventID, uint32_t eventMode);

/**
 * \brief Sets the reload count and index for the TRPD
 *
 * \param trpdMem       [IN] Address to TRPD memory
 * \param reloadEnable  [IN] Reload enable flag
 * \param reloadIdx     [IN] Reload index
*/
int32_t UdmaUtils_setTrpdReload(uint8_t *trpdMem,
                                uint32_t reloadEnable, uint32_t reloadIdx);

/**
 *  \brief Returns the TR response in the TRPD memory based on the index
 *
 *  \param trpdMem  [IN] TRPD memory pointer
 *  \param trCnt    [IN] Number of TR entries present in the TRPD
 *  \param trIndex  [IN] Index to the TR to get the response from
 *
 *  \return TR response Refer \ref CSL_UdmapTrResponseStatus
 */
static inline uint32_t UdmaUtils_getTrpdTr15Response(const uint8_t *trpdMem,
                                                     uint32_t trCnt,
                                                     uint32_t trIndex);

/**
 *  \brief Returns the TR15 pointer in the TRPD memory based on the index
 *
 *  \param trpdMem  [IN] TRPD memory pointer
 *  \param trIndex  [IN] Index to the TR to get the TR15 pointer address
 *
 *  \return TR15 pointer address
 */
static inline CSL_UdmapTR15 *UdmaUtils_getTrpdTr15Pointer(uint8_t *trpdMem,
                                                          uint32_t trIndex);

/**
 *  \brief Returns the TR1 pointer in the TRPD memory based on the index
 *
 *  \param trpdMem  [IN] TRPD memory pointer
 *  \param trIndex  [IN] Index to the TR to get the TR1 pointer address
 *
 *  \return TR1 pointer address
 */
static inline CSL_UdmapTR1 *UdmaUtils_getTrpdTr1Pointer(uint8_t *trpdMem,
                                                          uint32_t trIndex);

/* ========================================================================== */
/*                       Static Function Definitions                          */
/* ========================================================================== */

static inline uint32_t UdmaUtils_getTrpdTr15Response(const uint8_t *trpdMem,
                                                     uint32_t trCnt,
                                                     uint32_t trIndex)
{
    uint32_t        trRespStatus;
    const uint32_t *pTrResp;;

    /* Get TR index */
    pTrResp = (const uint32_t *) (trpdMem +
                             (sizeof(CSL_UdmapTR15) +
                             (sizeof(CSL_UdmapTR15) * trCnt) +
                             (sizeof(uint32_t) * trIndex)));
    trRespStatus = CSL_FEXT(*pTrResp, UDMAP_TR_RESPONSE_STATUS_TYPE);

    return (trRespStatus);
}

static inline CSL_UdmapTR15 *UdmaUtils_getTrpdTr15Pointer(uint8_t *trpdMem,
                                                          uint32_t trIndex)
{
    CSL_UdmapTR15  *pTr;

    pTr = (CSL_UdmapTR15 *)(trpdMem + sizeof(CSL_UdmapTR15) + (sizeof(CSL_UdmapTR15) * trIndex));

    return (pTr);
}

static inline CSL_UdmapTR1 *UdmaUtils_getTrpdTr1Pointer(uint8_t *trpdMem,
                                                          uint32_t trIndex)
{
    CSL_UdmapTR1  *pTr;

    pTr = (CSL_UdmapTR1 *)(trpdMem + UDMA_TR1_OFFSET + (UDMA_TR1_OFFSET * trIndex));

    return (pTr);
}

static inline CSL_UdmapTR3 *UdmaUtils_getTrpdTr3Pointer(uint8_t *trpdMem,
                                                          uint32_t trIndex)
{
    CSL_UdmapTR3  *pTr;

    pTr = (CSL_UdmapTR3 *)(trpdMem + UDMA_TR3_OFFSET + (UDMA_TR3_OFFSET * trIndex));

    return (pTr);
}

#ifdef __cplusplus
}
#endif

#endif /* #ifndef UDMA_UTILS_H_ */

/** @} */
