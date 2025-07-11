/*
 *  Copyright (C) 2023-2024 Texas Instruments Incorporated
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
#ifndef SOC_CONFIG_IN_H_
#define SOC_CONFIG_IN_H_

#ifdef __cplusplus
extern "C"
{
#endif

/* IP versions */
#define IP_VERSION_ECAP_V0
#define IP_VERSION_EPWM_V0
#define IP_VERSION_EQEP_V0
#define IP_VERSION_GPIO_V0
#define IP_VERSION_I2C_V0
#define IP_VERSION_MCAN_V0
#define IP_VERSION_MCASP_V0
#define IP_VERSION_MCSPI_V0
#define IP_VERSION_INTAGGR_V0
#define IP_VERSION_INTR_ROUTER_V0
#define IP_VERSION_DDR_V1
#define IP_VERSION_MMCSD_V1
#define IP_VERSION_OSPI_V0
#define IP_VERSION_UART_V0
#define IP_VERSION_RTC_V1
#define IP_VERSION_GTC_V0
#define IP_VERSION_BCDMA_V0
#define IP_VERSION_LCDMA_RINGACC_V0
#define IP_VERSION_PKTDMA_V0
/* This IP version of VPAC points to VPAC3L */
#define IP_VERSION_VPAC_V3L
#define IP_VERSION_RINGACC_V0
#define IP_VERSION_DRU_V2
#define IP_VERSION_WATCHDOG_V1

/* Driver versions */
#define DRV_VERSION_ECAP_V0
#define DRV_VERSION_EPWM_V0
#define DRV_VERSION_EQEP_V0
#define DRV_VERSION_GPIO_V0
#define DRV_VERSION_DDR_V1
#define DRV_VERSION_I2C_V0
#define DRV_VERSION_MCAN_V0
#define DRV_VERSION_MCASP_V1
#define DRV_VERSION_MCSPI_V0
#define DRV_VERSION_SERIAL_FLASH_V0 /* OSPI NOR/NAND */
#define DRV_VERSION_MMCSD_V1
#define DRV_VERSION_OSPI_V0
#define DRV_VERSION_QOS_V0
#define DRV_VERSION_RTC_V0
#define DRV_VERSION_UART_V0
#define DRV_VERSION_UDMA_V0
#define DRV_VERSION_GTC_V0
#define DRV_VERSION_WATCHDOG_V1

/* Driver DMA integration */
#define DMA_VERSION_MCSPI_UDMA

/* Driver bootloader format */
#define DRV_BOOTLOADER_FORMAT_RPRC

#ifdef __cplusplus
}
#endif

#endif
