/*
 * Copyright (C) 2023-25 Texas Instruments Incorporated
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 *
 *   Redistributions of source code must retain the above copyright
 *   notice, this list of conditions and the following disclaimer.
 *
 *   Redistributions in binary form must reproduce the above copyright
 *   notice, this list of conditions and the following disclaimer in the
 *   documentation and/or other materials provided with the
 *   distribution.
 *
 *   Neither the name of Texas Instruments Incorporated nor the names of
 *   its contributors may be used to endorse or promote products derived
 *   from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 * "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 * LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
 * A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
 * OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
 * SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
 * LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
 * DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
 * THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

/**
 *  \file ddr_soc.h
 *
 *  \brief DDR Driver SOC-specific header file for J722S.
 */

#ifndef DDR_SOC_H_
#define DDR_SOC_H_

/* ========================================================================== */
/*                             Include Files                                  */
/* ========================================================================== */

#include <drivers/hw_include/cslr_soc.h>

#ifdef __cplusplus
extern "C" {
#endif

/* ========================================================================== */
/*                           Macros & Typedefs                                */
/* ========================================================================== */

#define DDR_CTL_CFG_BASE CSL_DDR32SS0_CTLPHY_WRAP_CTL_CFG_CTLCFG_BASE
#define DDR_SS_CFG_BASE CSL_DDR32SS0_REGS_SS_CFG_SSCFG_BASE

#define DDR_DRAM_START_ADDR     0x80000000U
#define DDR_DRAM_SIZE           0x80000000U
#define DDR_MHZ                (1600U)  /* DDR clock speed in MHZ */
#define DDR_BUS_WIDTH          (  32U)  /* in units of bits */
#define DDR_BURST_SIZE_BYTES   (  64U)  /* in units of bytes */

#define BIST_MODE_MEM_INIT          4
#define BIST_GO_START_TIMEOUT		10000

#define DDR_BIST_START_ADDRESS_0_REG    CSL_EMIF_CTLCFG_DENALI_CTL_285
#define DDR_BIST_START_ADDRESS_1_REG    CSL_EMIF_CTLCFG_DENALI_CTL_286
#define DDR_ADDRESS_SPACE_REG           CSL_EMIF_CTLCFG_DENALI_CTL_283
#define DDR_BIST_DATA_CHECK_REG         CSL_EMIF_CTLCFG_DENALI_CTL_284
#define DDR_BIST_ADDRESS_CHECK_REG      CSL_EMIF_CTLCFG_DENALI_CTL_284
#define DDR_BIST_TEST_MODE_REG          CSL_EMIF_CTLCFG_DENALI_CTL_289
#define DDR_BIST_DATA_PATTERN_0_REG     CSL_EMIF_CTLCFG_DENALI_CTL_290
#define DDR_BIST_DATA_PATTERN_1_REG     CSL_EMIF_CTLCFG_DENALI_CTL_291
#define DDR_BIST_GO_REG                 CSL_EMIF_CTLCFG_DENALI_CTL_283

#define DDR_BIST_START_ADDR_0_MASK      CSL_EMIF_CTLCFG_DENALI_CTL_285_BIST_START_ADDRESS_0_MASK
#define DDR_BIST_START_ADDR_1_MASK      CSL_EMIF_CTLCFG_DENALI_CTL_286_BIST_START_ADDRESS_1_MASK

#define CSL_DDR_ADDRESS_SPACE_MASK      CSL_EMIF_CTLCFG_DENALI_CTL_283_ADDR_SPACE_MASK
#define CSL_DDR_ADDRESS_SPACE_SHIFT     CSL_EMIF_CTLCFG_DENALI_CTL_283_ADDR_SPACE_SHIFT
#define CSL_DDR_ADDRESS_SPACE_MAX       CSL_EMIF_CTLCFG_DENALI_CTL_283_ADDR_SPACE_MAX

#define CSL_BIST_DATA_CHECK_REG_MASK    CSL_EMIF_CTLCFG_DENALI_CTL_284_BIST_DATA_CHECK_MASK
#define CSL_BIST_DATA_CHECK_REG_SHIFT   CSL_EMIF_CTLCFG_DENALI_CTL_284_BIST_DATA_CHECK_SHIFT
#define CSL_BIST_DATA_CHECK_REG_MAX     CSL_EMIF_CTLCFG_DENALI_CTL_284_BIST_DATA_CHECK_MAX

#define CSL_BIST_ADDRESS_CHECK_REG_MASK    CSL_EMIF_CTLCFG_DENALI_CTL_284_BIST_ADDR_CHECK_MASK
#define CSL_BIST_ADDRESS_CHECK_REG_SHIFT   CSL_EMIF_CTLCFG_DENALI_CTL_284_BIST_ADDR_CHECK_SHIFT
#define CSL_BIST_ADDRESS_CHECK_REG_MAX     CSL_EMIF_CTLCFG_DENALI_CTL_284_BIST_ADDR_CHECK_MAX

#define DDR_BIST_DATA_PATTERN0_MASK        CSL_EMIF_CTLCFG_DENALI_CTL_290_BIST_DATA_PATTERN_0_MASK
#define DDR_BIST_DATA_PATTERN1_MASK        CSL_EMIF_CTLCFG_DENALI_CTL_291_BIST_DATA_PATTERN_1_MASK

#define DDR_BIST_GO_MASK    CSL_EMIF_CTLCFG_DENALI_CTL_283_BIST_GO_MASK

#define DDR_IRQ_NUM         CSLR_WKUP_R5FSS0_CORE0_INTR_DDR32SS0_DDRSS_CONTROLLER_0

/* ========================================================================== */
/*                         Structure Declarations                             */
/* ========================================================================== */

/* None */

/* ========================================================================== */
/*                  Internal/Private Function Declarations                    */
/* ========================================================================== */

/* None */

/* ========================================================================== */
/*                          Function Declarations                             */
/* ========================================================================== */

/* None */

/* ========================================================================== */
/*                       Static Function Definitions                          */
/* ========================================================================== */

/* None */

#ifdef __cplusplus
}
#endif

#endif  /* #ifndef DDR_SOC_V1_H_ */
