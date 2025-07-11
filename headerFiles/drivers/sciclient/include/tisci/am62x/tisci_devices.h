/*
 *  Copyright (C) 2021-2025 Texas Instruments Incorporated
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
 */
/**
 * \ingroup TISCI
 * \defgroup tisci_devices tisci_devices
 *
 * DMSC controls the power management, security and resource management
 * of the device.
 *
 *
 * @{
 */
/**
 *
 *  \brief  This file contains:
 *
 *          WARNING!!: Autogenerated file from SYSFW. DO NOT MODIFY!!
 * Data version: 240823_113849
 *
 */
#ifndef SOC_TISCI_DEVICES_H
#define SOC_TISCI_DEVICES_H

#ifdef __cplusplus
extern "C"
{
#endif


#define TISCI_DEV_CMP_EVENT_INTROUTER0 1U
#define TISCI_DEV_DBGSUSPENDROUTER0 2U
#define TISCI_DEV_MAIN_GPIOMUX_INTROUTER0 3U
#define TISCI_DEV_WKUP_MCU_GPIOMUX_INTROUTER0 5U
#define TISCI_DEV_TIMESYNC_EVENT_ROUTER0 6U
#define TISCI_DEV_MCU_M4FSS0 7U
#define TISCI_DEV_MCU_M4FSS0_CBASS_0 8U
#define TISCI_DEV_MCU_M4FSS0_CORE0 9U
#define TISCI_DEV_CPSW0 13U
#define TISCI_DEV_CPT2_AGGR0 14U
#define TISCI_DEV_STM0 15U
#define TISCI_DEV_DCC0 16U
#define TISCI_DEV_DCC1 17U
#define TISCI_DEV_DCC2 18U
#define TISCI_DEV_DCC3 19U
#define TISCI_DEV_DCC4 20U
#define TISCI_DEV_DCC5 21U
#define TISCI_DEV_SMS0 22U
#define TISCI_DEV_MCU_DCC0 23U
#define TISCI_DEV_DEBUGSS_WRAP0 24U
#define TISCI_DEV_DMASS0 25U
#define TISCI_DEV_DMASS0_BCDMA_0 26U
#define TISCI_DEV_DMASS0_CBASS_0 27U
#define TISCI_DEV_DMASS0_INTAGGR_0 28U
#define TISCI_DEV_DMASS0_IPCSS_0 29U
#define TISCI_DEV_DMASS0_PKTDMA_0 30U
#define TISCI_DEV_DMASS0_RINGACC_0 33U
#define TISCI_DEV_MCU_TIMER0 35U
#define TISCI_DEV_TIMER0 36U
#define TISCI_DEV_TIMER1 37U
#define TISCI_DEV_TIMER2 38U
#define TISCI_DEV_TIMER3 39U
#define TISCI_DEV_TIMER4 40U
#define TISCI_DEV_TIMER5 41U
#define TISCI_DEV_TIMER6 42U
#define TISCI_DEV_TIMER7 43U
#define TISCI_DEV_MCU_TIMER1 48U
#define TISCI_DEV_MCU_TIMER2 49U
#define TISCI_DEV_MCU_TIMER3 50U
#define TISCI_DEV_ECAP0 51U
#define TISCI_DEV_ECAP1 52U
#define TISCI_DEV_ECAP2 53U
#define TISCI_DEV_ELM0 54U
#define TISCI_DEV_EMIF_DATA_ISO_VD 55U
#define TISCI_DEV_MMCSD0 57U
#define TISCI_DEV_MMCSD1 58U
#define TISCI_DEV_EQEP0 59U
#define TISCI_DEV_EQEP1 60U
#define TISCI_DEV_WKUP_GTC0 61U
#define TISCI_DEV_EQEP2 62U
#define TISCI_DEV_ESM0 63U
#define TISCI_DEV_WKUP_ESM0 64U
#define TISCI_DEV_FSS0 73U
#define TISCI_DEV_FSS0_FSAS_0 74U
#define TISCI_DEV_FSS0_OSPI_0 75U
#define TISCI_DEV_GICSS0 76U
#define TISCI_DEV_GPIO0 77U
#define TISCI_DEV_GPIO1 78U
#define TISCI_DEV_MCU_GPIO0 79U
#define TISCI_DEV_GPMC0 80U
#define TISCI_DEV_ICSSM0 81U
#define TISCI_DEV_LED0 83U
#define TISCI_DEV_DDPA0 85U
#define TISCI_DEV_EPWM0 86U
#define TISCI_DEV_EPWM1 87U
#define TISCI_DEV_EPWM2 88U
#define TISCI_DEV_WKUP_VTM0 95U
#define TISCI_DEV_MAILBOX0 96U
#define TISCI_DEV_MAIN2MCU_VD 97U
#define TISCI_DEV_MCAN0 98U
#define TISCI_DEV_MCU_MCRC64_0 100U
#define TISCI_DEV_MCU2MAIN_VD 101U
#define TISCI_DEV_I2C0 102U
#define TISCI_DEV_I2C1 103U
#define TISCI_DEV_I2C2 104U
#define TISCI_DEV_I2C3 105U
#define TISCI_DEV_MCU_I2C0 106U
#define TISCI_DEV_WKUP_I2C0 107U
#define TISCI_DEV_WKUP_TIMER0 110U
#define TISCI_DEV_WKUP_TIMER1 111U
#define TISCI_DEV_WKUP_UART0 114U
#define TISCI_DEV_MCRC64_0 116U
#define TISCI_DEV_WKUP_RTCSS0 117U
#define TISCI_DEV_WKUP_R5FSS0_SS0 118U
#define TISCI_DEV_WKUP_R5FSS0 119U
#define TISCI_DEV_WKUP_R5FSS0_CORE0 121U
#define TISCI_DEV_RTI0 125U
#define TISCI_DEV_RTI1 126U
#define TISCI_DEV_RTI2 127U
#define TISCI_DEV_RTI3 128U
#define TISCI_DEV_RTI15 130U
#define TISCI_DEV_MCU_RTI0 131U
#define TISCI_DEV_WKUP_RTI0 132U
#define TISCI_DEV_COMPUTE_CLUSTER0 134U
#define TISCI_DEV_A53SS0_CORE_0 135U
#define TISCI_DEV_A53SS0_CORE_1 136U
#define TISCI_DEV_A53SS0_CORE_2 137U
#define TISCI_DEV_A53SS0_CORE_3 138U
#define TISCI_DEV_PSCSS0 139U
#define TISCI_DEV_WKUP_PSC0 140U
#define TISCI_DEV_MCSPI0 141U
#define TISCI_DEV_MCSPI1 142U
#define TISCI_DEV_MCSPI2 143U
#define TISCI_DEV_UART0 146U
#define TISCI_DEV_MCU_MCSPI0 147U
#define TISCI_DEV_MCU_MCSPI1 148U
#define TISCI_DEV_MCU_UART0 149U
#define TISCI_DEV_SPINLOCK0 150U
#define TISCI_DEV_UART1 152U
#define TISCI_DEV_UART2 153U
#define TISCI_DEV_UART3 154U
#define TISCI_DEV_UART4 155U
#define TISCI_DEV_UART5 156U
#define TISCI_DEV_BOARD0 157U
#define TISCI_DEV_UART6 158U
#define TISCI_DEV_USB0 161U
#define TISCI_DEV_USB1 162U
#define TISCI_DEV_PBIST0 163U
#define TISCI_DEV_PBIST1 164U
#define TISCI_DEV_WKUP_PBIST0 165U
#define TISCI_DEV_A53SS0 166U
#define TISCI_DEV_COMPUTE_CLUSTER0_PBIST_0 167U
#define TISCI_DEV_PSC0_FW_0 168U
#define TISCI_DEV_PSC0 169U
#define TISCI_DEV_DDR16SS0 170U
#define TISCI_DEV_DEBUGSS0 171U
#define TISCI_DEV_A53_RS_BW_LIMITER0 172U
#define TISCI_DEV_A53_WS_BW_LIMITER1 173U
#define TISCI_DEV_GPU_RS_BW_LIMITER2 174U
#define TISCI_DEV_GPU_WS_BW_LIMITER3 175U
#define TISCI_DEV_WKUP_DEEPSLEEP_SOURCES0 176U
#define TISCI_DEV_EMIF_CFG_ISO_VD 177U
#define TISCI_DEV_MAIN_USB0_ISO_VD 178U
#define TISCI_DEV_MAIN_USB1_ISO_VD 179U
#define TISCI_DEV_MCU_MCU_16FF0 180U
#define TISCI_DEV_CPT2_AGGR1 181U
#define TISCI_DEV_CSI_RX_IF0 182U
#define TISCI_DEV_DCC6 183U
#define TISCI_DEV_MMCSD2 184U
#define TISCI_DEV_DPHY_RX0 185U
#define TISCI_DEV_DSS0 186U
#define TISCI_DEV_GPU0 187U
#define TISCI_DEV_MCU_MCAN0 188U
#define TISCI_DEV_MCU_MCAN1 189U
#define TISCI_DEV_MCASP0 190U
#define TISCI_DEV_MCASP1 191U
#define TISCI_DEV_MCASP2 192U
#define TISCI_DEV_CLK_32K_RC_SEL_DEV_VD 193U
#define TISCI_DEV_HSM0 225U
#define TISCI_DEV_MCU_OBSCLK_MUX_SEL_DEV_VD 227U


#ifdef __cplusplus
}
#endif

#endif /* SOC_TISCI_DEVICES_H */

/** @} */
