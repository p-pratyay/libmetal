/********************************************************************
 * Copyright (C) 2022 Texas Instruments Incorporated.
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
 *  Name        : cslr_mcu_sec_mmr.h
*/
#ifndef CSLR_MCU_SEC_MMR_H_
#define CSLR_MCU_SEC_MMR_H_

#ifdef __cplusplus
extern "C"
{
#endif
#include <drivers/hw_include/cslr.h>
#include <stdint.h>

/**************************************************************************
* Module Base Offset Values
**************************************************************************/

#define CSL_MCU_SEC_MMR_CFG0_REGS_BASE                                    (0x00000000U)
#define CSL_MCU_SEC_MMR_CFG2_REGS_BASE                                    (0x00000000U)


/**************************************************************************
* Hardware Region  : MMRs in region 0
**************************************************************************/


/**************************************************************************
* Register Overlay Structure
**************************************************************************/

typedef struct {
    volatile uint32_t PID;                       /* PID register */
    volatile uint8_t  Resv_32[28];
    volatile uint32_t CLSTR0_DEF;
    volatile uint8_t  Resv_64[28];
    volatile uint32_t CLSTR0_CFG;
    volatile uint8_t  Resv_128[60];
    volatile uint32_t CLSTR0_PMCTRL;
    volatile uint8_t  Resv_144[12];
    volatile uint32_t CLSTR0_PMSTAT;
    volatile uint8_t  Resv_256[108];
    volatile uint32_t CLSTR0_CORE0_CFG;
    volatile uint8_t  Resv_272[12];
    volatile uint32_t CLSTR0_CORE0_BOOTVECT_LO;
    volatile uint32_t CLSTR0_CORE0_BOOTVECT_HI;
    volatile uint8_t  Resv_288[8];
    volatile uint32_t CLSTR0_CORE0_PMCTRL;
    volatile uint8_t  Resv_304[12];
    volatile uint32_t CLSTR0_CORE0_PMSTAT;
} CSL_mcu_sec_mmr_cfg0Regs;


/**************************************************************************
* Register Macros
**************************************************************************/

#define CSL_MCU_SEC_MMR_CFG0_PID                                          (0x00000000U)
#define CSL_MCU_SEC_MMR_CFG0_CLSTR0_DEF                                   (0x00000020U)
#define CSL_MCU_SEC_MMR_CFG0_CLSTR0_CFG                                   (0x00000040U)
#define CSL_MCU_SEC_MMR_CFG0_CLSTR0_PMCTRL                                (0x00000080U)
#define CSL_MCU_SEC_MMR_CFG0_CLSTR0_PMSTAT                                (0x00000090U)
#define CSL_MCU_SEC_MMR_CFG0_CLSTR0_CORE0_CFG                             (0x00000100U)
#define CSL_MCU_SEC_MMR_CFG0_CLSTR0_CORE0_BOOTVECT_LO                     (0x00000110U)
#define CSL_MCU_SEC_MMR_CFG0_CLSTR0_CORE0_BOOTVECT_HI                     (0x00000114U)
#define CSL_MCU_SEC_MMR_CFG0_CLSTR0_CORE0_PMCTRL                          (0x00000120U)
#define CSL_MCU_SEC_MMR_CFG0_CLSTR0_CORE0_PMSTAT                          (0x00000130U)

/**************************************************************************
* Field Definition Macros
**************************************************************************/


/* PID */

#define CSL_MCU_SEC_MMR_CFG0_PID_PID_MINOR_MASK                           (0x0000003FU)
#define CSL_MCU_SEC_MMR_CFG0_PID_PID_MINOR_SHIFT                          (0x00000000U)
#define CSL_MCU_SEC_MMR_CFG0_PID_PID_MINOR_MAX                            (0x0000003FU)

#define CSL_MCU_SEC_MMR_CFG0_PID_PID_CUSTOM_MASK                          (0x000000C0U)
#define CSL_MCU_SEC_MMR_CFG0_PID_PID_CUSTOM_SHIFT                         (0x00000006U)
#define CSL_MCU_SEC_MMR_CFG0_PID_PID_CUSTOM_MAX                           (0x00000003U)

#define CSL_MCU_SEC_MMR_CFG0_PID_PID_MAJOR_MASK                           (0x00000700U)
#define CSL_MCU_SEC_MMR_CFG0_PID_PID_MAJOR_SHIFT                          (0x00000008U)
#define CSL_MCU_SEC_MMR_CFG0_PID_PID_MAJOR_MAX                            (0x00000007U)

#define CSL_MCU_SEC_MMR_CFG0_PID_PID_MISC_MASK                            (0x0000F800U)
#define CSL_MCU_SEC_MMR_CFG0_PID_PID_MISC_SHIFT                           (0x0000000BU)
#define CSL_MCU_SEC_MMR_CFG0_PID_PID_MISC_MAX                             (0x0000001FU)

#define CSL_MCU_SEC_MMR_CFG0_PID_PID_MSB16_MASK                           (0xFFFF0000U)
#define CSL_MCU_SEC_MMR_CFG0_PID_PID_MSB16_SHIFT                          (0x00000010U)
#define CSL_MCU_SEC_MMR_CFG0_PID_PID_MSB16_MAX                            (0x0000FFFFU)

/* CLSTR0_DEF */

#define CSL_MCU_SEC_MMR_CFG0_CLSTR0_DEF_ARM_CORE_TYPE_MASK                (0x000000FFU)
#define CSL_MCU_SEC_MMR_CFG0_CLSTR0_DEF_ARM_CORE_TYPE_SHIFT               (0x00000000U)
#define CSL_MCU_SEC_MMR_CFG0_CLSTR0_DEF_ARM_CORE_TYPE_MAX                 (0x000000FFU)

#define CSL_MCU_SEC_MMR_CFG0_CLSTR0_DEF_DSP_CORE_TYPE_MASK                (0x0000FF00U)
#define CSL_MCU_SEC_MMR_CFG0_CLSTR0_DEF_DSP_CORE_TYPE_SHIFT               (0x00000008U)
#define CSL_MCU_SEC_MMR_CFG0_CLSTR0_DEF_DSP_CORE_TYPE_MAX                 (0x000000FFU)

#define CSL_MCU_SEC_MMR_CFG0_CLSTR0_DEF_CORE_NUM_MASK                     (0x00030000U)
#define CSL_MCU_SEC_MMR_CFG0_CLSTR0_DEF_CORE_NUM_SHIFT                    (0x00000010U)
#define CSL_MCU_SEC_MMR_CFG0_CLSTR0_DEF_CORE_NUM_MAX                      (0x00000003U)

/* CLSTR0_CFG */

#define CSL_MCU_SEC_MMR_CFG0_CLSTR0_CFG_LOCKSTEP_MASK                     (0x00000001U)
#define CSL_MCU_SEC_MMR_CFG0_CLSTR0_CFG_LOCKSTEP_SHIFT                    (0x00000000U)
#define CSL_MCU_SEC_MMR_CFG0_CLSTR0_CFG_LOCKSTEP_MAX                      (0x00000001U)

#define CSL_MCU_SEC_MMR_CFG0_CLSTR0_CFG_TEINIT_MASK                       (0x00000002U)
#define CSL_MCU_SEC_MMR_CFG0_CLSTR0_CFG_TEINIT_SHIFT                      (0x00000001U)
#define CSL_MCU_SEC_MMR_CFG0_CLSTR0_CFG_TEINIT_MAX                        (0x00000001U)

#define CSL_MCU_SEC_MMR_CFG0_CLSTR0_CFG_DBG_NO_CLKSTOP_MASK               (0x00000004U)
#define CSL_MCU_SEC_MMR_CFG0_CLSTR0_CFG_DBG_NO_CLKSTOP_SHIFT              (0x00000002U)
#define CSL_MCU_SEC_MMR_CFG0_CLSTR0_CFG_DBG_NO_CLKSTOP_MAX                (0x00000001U)

#define CSL_MCU_SEC_MMR_CFG0_CLSTR0_CFG_LOCKSTEP_EN_MASK                  (0x00000008U)
#define CSL_MCU_SEC_MMR_CFG0_CLSTR0_CFG_LOCKSTEP_EN_SHIFT                 (0x00000003U)
#define CSL_MCU_SEC_MMR_CFG0_CLSTR0_CFG_LOCKSTEP_EN_MAX                   (0x00000001U)

#define CSL_MCU_SEC_MMR_CFG0_CLSTR0_CFG_MEM_INIT_DIS_MASK                 (0x00000010U)
#define CSL_MCU_SEC_MMR_CFG0_CLSTR0_CFG_MEM_INIT_DIS_SHIFT                (0x00000004U)
#define CSL_MCU_SEC_MMR_CFG0_CLSTR0_CFG_MEM_INIT_DIS_MAX                  (0x00000001U)

#define CSL_MCU_SEC_MMR_CFG0_CLSTR0_CFG_SINGLE_CORE_MASK                  (0x00000020U)
#define CSL_MCU_SEC_MMR_CFG0_CLSTR0_CFG_SINGLE_CORE_SHIFT                 (0x00000005U)
#define CSL_MCU_SEC_MMR_CFG0_CLSTR0_CFG_SINGLE_CORE_MAX                   (0x00000001U)

#define CSL_MCU_SEC_MMR_CFG0_CLSTR0_CFG_SINGLE_CORE_ONLY_MASK             (0x00000040U)
#define CSL_MCU_SEC_MMR_CFG0_CLSTR0_CFG_SINGLE_CORE_ONLY_SHIFT            (0x00000006U)
#define CSL_MCU_SEC_MMR_CFG0_CLSTR0_CFG_SINGLE_CORE_ONLY_MAX              (0x00000001U)

/* CLSTR0_PMCTRL */


/* CLSTR0_PMSTAT */


/* CLSTR0_CORE0_CFG */

#define CSL_MCU_SEC_MMR_CFG0_CLSTR0_CORE0_CFG_ATCM_EN_MASK                (0x00000008U)
#define CSL_MCU_SEC_MMR_CFG0_CLSTR0_CORE0_CFG_ATCM_EN_SHIFT               (0x00000003U)
#define CSL_MCU_SEC_MMR_CFG0_CLSTR0_CORE0_CFG_ATCM_EN_MAX                 (0x00000001U)

#define CSL_MCU_SEC_MMR_CFG0_CLSTR0_CORE0_CFG_BTCM_EN_MASK                (0x00000080U)
#define CSL_MCU_SEC_MMR_CFG0_CLSTR0_CORE0_CFG_BTCM_EN_SHIFT               (0x00000007U)
#define CSL_MCU_SEC_MMR_CFG0_CLSTR0_CORE0_CFG_BTCM_EN_MAX                 (0x00000001U)

#define CSL_MCU_SEC_MMR_CFG0_CLSTR0_CORE0_CFG_TCM_RSTBASE_MASK            (0x00000800U)
#define CSL_MCU_SEC_MMR_CFG0_CLSTR0_CORE0_CFG_TCM_RSTBASE_SHIFT           (0x0000000BU)
#define CSL_MCU_SEC_MMR_CFG0_CLSTR0_CORE0_CFG_TCM_RSTBASE_MAX             (0x00000001U)

#define CSL_MCU_SEC_MMR_CFG0_CLSTR0_CORE0_CFG_NMFI_EN_MASK                (0x00008000U)
#define CSL_MCU_SEC_MMR_CFG0_CLSTR0_CORE0_CFG_NMFI_EN_SHIFT               (0x0000000FU)
#define CSL_MCU_SEC_MMR_CFG0_CLSTR0_CORE0_CFG_NMFI_EN_MAX                 (0x00000001U)

/* CLSTR0_CORE0_BOOTVECT_LO */

#define CSL_MCU_SEC_MMR_CFG0_CLSTR0_CORE0_BOOTVECT_LO_VECT_ADDR_MASK      (0xFFFFFF80U)
#define CSL_MCU_SEC_MMR_CFG0_CLSTR0_CORE0_BOOTVECT_LO_VECT_ADDR_SHIFT     (0x00000007U)
#define CSL_MCU_SEC_MMR_CFG0_CLSTR0_CORE0_BOOTVECT_LO_VECT_ADDR_MAX       (0x01FFFFFFU)

/* CLSTR0_CORE0_BOOTVECT_HI */

#define CSL_MCU_SEC_MMR_CFG0_CLSTR0_CORE0_BOOTVECT_HI_VECT_ADDR_MASK      (0x0000FFFFU)
#define CSL_MCU_SEC_MMR_CFG0_CLSTR0_CORE0_BOOTVECT_HI_VECT_ADDR_SHIFT     (0x00000000U)
#define CSL_MCU_SEC_MMR_CFG0_CLSTR0_CORE0_BOOTVECT_HI_VECT_ADDR_MAX       (0x0000FFFFU)

/* CLSTR0_CORE0_PMCTRL */

#define CSL_MCU_SEC_MMR_CFG0_CLSTR0_CORE0_PMCTRL_CORE_HALT_MASK           (0x00000001U)
#define CSL_MCU_SEC_MMR_CFG0_CLSTR0_CORE0_PMCTRL_CORE_HALT_SHIFT          (0x00000000U)
#define CSL_MCU_SEC_MMR_CFG0_CLSTR0_CORE0_PMCTRL_CORE_HALT_MAX            (0x00000001U)

/* CLSTR0_CORE0_PMSTAT */

#define CSL_MCU_SEC_MMR_CFG0_CLSTR0_CORE0_PMSTAT_WFI_MASK                 (0x00000001U)
#define CSL_MCU_SEC_MMR_CFG0_CLSTR0_CORE0_PMSTAT_WFI_SHIFT                (0x00000000U)
#define CSL_MCU_SEC_MMR_CFG0_CLSTR0_CORE0_PMSTAT_WFI_MAX                  (0x00000001U)

#define CSL_MCU_SEC_MMR_CFG0_CLSTR0_CORE0_PMSTAT_WFE_MASK                 (0x00000002U)
#define CSL_MCU_SEC_MMR_CFG0_CLSTR0_CORE0_PMSTAT_WFE_SHIFT                (0x00000001U)
#define CSL_MCU_SEC_MMR_CFG0_CLSTR0_CORE0_PMSTAT_WFE_MAX                  (0x00000001U)

#define CSL_MCU_SEC_MMR_CFG0_CLSTR0_CORE0_PMSTAT_CLK_GATE_MASK            (0x00000008U)
#define CSL_MCU_SEC_MMR_CFG0_CLSTR0_CORE0_PMSTAT_CLK_GATE_SHIFT           (0x00000003U)
#define CSL_MCU_SEC_MMR_CFG0_CLSTR0_CORE0_PMSTAT_CLK_GATE_MAX             (0x00000001U)

/**************************************************************************
* Hardware Region  : MMRs in region 2
**************************************************************************/


/**************************************************************************
* Register Overlay Structure
**************************************************************************/

typedef struct {
    volatile uint32_t CLSTR0_CORE0_DBG_CFG;
    volatile uint8_t  Resv_64[60];
    volatile uint32_t CLSTR0_CORE1_DBG_CFG;
} CSL_mcu_sec_mmr_cfg2Regs;


/**************************************************************************
* Register Macros
**************************************************************************/

#define CSL_MCU_SEC_MMR_CFG2_CLSTR0_CORE0_DBG_CFG                         (0x00000000U)
#define CSL_MCU_SEC_MMR_CFG2_CLSTR0_CORE1_DBG_CFG                         (0x00000040U)

/**************************************************************************
* Field Definition Macros
**************************************************************************/


/* CLSTR0_CORE0_DBG_CFG */

#define CSL_MCU_SEC_MMR_CFG2_CLSTR0_CORE0_DBG_CFG_NIDEN_MASK              (0x00000F00U)
#define CSL_MCU_SEC_MMR_CFG2_CLSTR0_CORE0_DBG_CFG_NIDEN_SHIFT             (0x00000008U)
#define CSL_MCU_SEC_MMR_CFG2_CLSTR0_CORE0_DBG_CFG_NIDEN_MAX               (0x0000000FU)

#define CSL_MCU_SEC_MMR_CFG2_CLSTR0_CORE0_DBG_CFG_DBGEN_MASK              (0x0000F000U)
#define CSL_MCU_SEC_MMR_CFG2_CLSTR0_CORE0_DBG_CFG_DBGEN_SHIFT             (0x0000000CU)
#define CSL_MCU_SEC_MMR_CFG2_CLSTR0_CORE0_DBG_CFG_DBGEN_MAX               (0x0000000FU)

/* CLSTR0_CORE1_DBG_CFG */


#ifdef __cplusplus
}
#endif
#endif
