/******************************************************************************
 * Copyright (c) 2024 Texas Instruments Incorporated - http://www.ti.com
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
 *
 *****************************************************************************/

/**
 *  \file qos_data.h
 *
 *  \brief am62x SOC Quality of Service (QoS) Configuration Data
 *         generated using K3 Resource Partitioning tool
 */

#ifndef QOS_DATA_H_
#define QOS_DATA_H_

/* ========================================================================== */
/*                             Include Files                                  */
/* ========================================================================== */

#include <drivers/qos/v0/soc/j722s/qos_soc.h>

#ifdef __cplusplus
extern "C" {
#endif

/* ========================================================================== */
/*                           Macros & Typedefs                                */
/* ========================================================================== */

/* None */

/* ========================================================================== */
/*                         Structure Declarations                             */
/* ========================================================================== */

/* None */

/* ========================================================================== */
/*                            Global Variables                                */
/* ========================================================================== */

QOS_Config gQosData[] = {
    /* modules_qosConfig0 - 1 endpoints, 4 channels */
    {
        .reg = K3_DSS_UL_MAIN_0_VBUSM_DMA + 0x100 + 0x4 * 0,
        .val = ORDERID_15,
    },
    {
        .reg = K3_DSS_UL_MAIN_0_VBUSM_DMA + 0x100 + 0x4 * 1,
        .val = ORDERID_15,
    },
    {
        .reg = K3_DSS_UL_MAIN_0_VBUSM_DMA + 0x100 + 0x4 * 2,
        .val = ORDERID_15,
    },
    {
        .reg = K3_DSS_UL_MAIN_0_VBUSM_DMA + 0x100 + 0x4 * 3,
        .val = ORDERID_15,
    },

    /* modules_qosConfig1 - 1 endpoints, 4 channels */
    {
        .reg = K3_DSS_UL_MAIN_1_VBUSM_DMA + 0x100 + 0x4 * 0,
        .val = ORDERID_15,
    },
    {
        .reg = K3_DSS_UL_MAIN_1_VBUSM_DMA + 0x100 + 0x4 * 1,
        .val = ORDERID_15,
    },
    {
        .reg = K3_DSS_UL_MAIN_1_VBUSM_DMA + 0x100 + 0x4 * 2,
        .val = ORDERID_15,
    },
    {
        .reg = K3_DSS_UL_MAIN_1_VBUSM_DMA + 0x100 + 0x4 * 3,
        .val = ORDERID_15,
    },

    /* modules_qosConfig2 - 3 endpoints, 1 channels */
    {
        .reg = PULSAR_UL_MAIN_0_CPU0_PMST + 0x100 + 0x4 * 0,
        .val = ORDERID_15,
    },
    {
        .reg = PULSAR_UL_MAIN_0_CPU0_RMST + 0x100 + 0x4 * 0,
        .val = ORDERID_15,
    },
    {
        .reg = PULSAR_UL_MAIN_0_CPU0_WMST + 0x100 + 0x4 * 0,
        .val = ORDERID_15,
    },

};

uint32_t gQosCount = sizeof(gQosData) / sizeof(gQosData[0]);

/* ========================================================================== */
/*                          Function Declarations                             */
/* ========================================================================== */

/* None */

#ifdef __cplusplus
}
#endif

#endif /* #ifndef QOS_DATA_H_ */
