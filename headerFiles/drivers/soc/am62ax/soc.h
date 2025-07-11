/*
 *  Copyright (C) 2022-2025 Texas Instruments Incorporated
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

#ifndef SOC_AM62AX_H_
#define SOC_AM62AX_H_

#ifdef __cplusplus
extern "C"
{
#endif

/**
 *  \defgroup DRV_SOC_MODULE APIs for SOC Specific Functions
 *  \ingroup DRV_MODULE
 *
 * For more details and example usage, see \ref DRIVERS_SOC_PAGE
 *
 *  @{
 */

#include <stdint.h>
#include <kernel/dpl/SystemP.h>
#if (ENABLE_SCICLIENT_DIRECT)
#include <drivers/device_manager/sciclient.h>
#else
#include <drivers/sciclient.h>
#endif
/**
 *  \anchor SOC_DomainId_t
 *  \name SOC Domain ID
 *  @{
 */
#define SOC_DOMAIN_ID_MAIN     (0U)
#define SOC_DOMAIN_ID_MCU      (1U)
#define SOC_DOMAIN_ID_WKUP     (2U)
/** @} */

/**
 * \anchor SOC_PSCDomainId_t
 * \ SOC PSC Domain ID
 * @{
 */
/* PSC Instances */
#define SOC_PSC_DOMAIN_ID_MAIN      (0U)
#define SOC_PSC_DOMAIN_ID_MCU       (1U)
/** @} */

/**
 * \anchor SOC_PSCModuleState_t
 * \name SOC PSC Module State
 * @{
 */
/* PSC (Power Sleep Controller) Module states */
#define SOC_PSC_SYNCRESETDISABLE        (0x0U)
#define SOC_PSC_SYNCRESET               (0x1U)
#define SOC_PSC_DISABLE                 (0x2U)
#define SOC_PSC_ENABLE                  (0x3U)
/** @} */

/**
 * \anchor SOC_PSCDomainState_t
 * \name SOC PSC Domain State
 * @{
 */
#define SOC_PSC_DOMAIN_OFF              (0x0U)
#define SOC_PSC_DOMAIN_ON               (0x1U)
/** @} */

/**
 * \brief Switch value for SD card boot mode
 */
#define SOC_BOOTMODE_MMCSD      (0X36C3)


/**
 * \brief Enable clock to specified module
 *
 * \param moduleId [in] see \ref tisci_devices for list of device ID's
 * \param enable [in] 1: enable clock to the module, 0: disable clock to the module
 *
 * \return SystemP_SUCCESS Module clock is enabled
 * \return SystemP_FAILURE Module clock could not be enabled
 */
int32_t SOC_moduleClockEnable(uint32_t moduleId, uint32_t enable);

/**
 * \brief Set module clock to specified frequency and with a specific parent
 *
 * \param moduleId [in] see \ref tisci_devices for list of module ID's
 * \param clkId [in] see \ref tisci_clocks for list of clocks associated with the specified module ID
 * \param clkParent [in] see \ref tisci_clocks for list of clock parents associated with the specified module ID
 * \param clkRate [in] Frequency to set in Hz
 *
 * \return SystemP_SUCCESS Module clock is enabled
 * \return SystemP_FAILURE Module clock could not be enabled
 */
int32_t SOC_moduleSetClockFrequencyWithParent(uint32_t moduleId, uint32_t clkId, uint32_t clkParent, uint64_t clkRate);

/**
 * \brief Set module clock to specified frequency
 *
 * \param moduleId [in] see \ref tisci_devices for list of module ID's
 * \param clkId [in] see \ref tisci_clocks for list of clocks associated with the specified module ID
 * \param clkRate [in] Frequency to set in Hz
 *
 * \return SystemP_SUCCESS Module clock is enabled
 * \return SystemP_FAILURE Module clock could not be enabled
 */
int32_t SOC_moduleSetClockFrequency(uint32_t moduleId, uint32_t clkId, uint64_t clkRate);

/**
 * \brief Convert a core ID to a user readable name
 *
 * \param coreId    [in] see \ref CSL_CoreID
 *
 * \return name as a string
 */
const char *SOC_getCoreName(uint16_t coreId);

/**
 * \brief Convert a core ID to a user readable name
 *
 * \param coreName    [in] core Name
 *
 * \return coreID
 */
uint32_t SOC_getCoreId(const char * coreName);

/**
 * \brief Get the clock frequency in Hz of the CPU on which the driver is running
 *
 * \return Clock frequency in Hz
 */
uint64_t SOC_getSelfCpuClk(void);

/**
 * \brief Enable or disable ePWM time base clock from Control MMR
 *
 * \param epwmInstance [in] ePWM instance number [0 - (CSL_EPWM_PER_CNT-1)]
 * \param enable       [in] TRUE to enable and FALSE to disable
 */
void SOC_setEpwmTbClk(uint32_t epwmInstance, uint32_t enable);


/**
 * \brief Lock control module partition to prevent writes into control MMRs
 *
 * \param domainId    [in] See SOC_DomainId_t
 * \param partition   [in] Partition number to unlock
 */
void SOC_controlModuleLockMMR(uint32_t domainId, uint32_t partition);

/**
 * \brief Unlock control module partition to allow writes into control MMRs
 *
 * \param domainId    [in] See SOC_DomainId_t
 * \param partition   [in] Partition number to unlock
 */
void SOC_controlModuleUnlockMMR(uint32_t domainId, uint32_t partition);

/**
 * \brief Unlocks all the control MMRs
 *
 */
void SOC_unlockAllMMR(void);

/**
 * \brief Enable or disable ePWM time base clock from Control MMR
 *
 * \param epwmInstance [in] ePWM instance number [0 - (CSL_EPWM_PER_CNT-1)]
 * \param enable       [in] TRUE to enable and FALSE to disable
 */
void SOC_setEpwmTbClk(uint32_t epwmInstance, uint32_t enable);

/**
 * \brief Get module clock frequency
 *
 * \param moduleId [in] see \ref tisci_devices for list of module ID's
 * \param clkId [in] see \ref tisci_clocks for list of clocks associated with the specified module ID
 * \param clkRate [out] Frequency of the clock
 *
 * \return SystemP_SUCCESS on success, else failure
 */
int32_t SOC_moduleGetClockFrequency(uint32_t moduleId, uint32_t clkId, uint64_t *clkRate);

/**
 * \brief Change boot mode by setting devstat register
 *
 * \param bootMode [IN] Boot mode switch value
 */
void SOC_setDevStat(uint32_t bootMode);

/**
 * \brief Generates the MCU IPC interrupt to DM R5 to wakeup the main domain from MCU only LPM mode.
 *
 */
void SOC_triggerMcuLpmWakeup(void);

/**
 * \brief Generate SW Warm Reset Main Domain
 */
void SOC_generateSwWarmResetMainDomain(void);

/**
 * \brief Generate SW POR Reset Main Domain
 */
void SOC_generateSwPORResetMainDomain(void);

/**
 * \brief Get the reset reason source for Main Domain
 *
 * \return Reset Reason Source Main Domain
 */
uint32_t SOC_getWarmResetCauseMainDomain(void);

/**
 * \brief Generate SW WARM Reset Mcu Domain
 */
void SOC_generateSwWarmResetMcuDomain(void);

/**
 * \brief Generate SW WARM Reset Main Domain from Mcu Domain
 */
void SOC_generateSwWarmResetMainDomainFromMcuDomain(void);

/**
 * \brief Generate SW POR Reset Main Domain from Mcu Domain
 */
void SOC_generateSwPORResetMainDomainFromMcuDomain(void);

/**
 * \brief Get the reset reason source for Mcu Domain
 *
 * \return Reset Reason Source Mcu Domain
 */
uint32_t SOC_getWarmResetCauseMcuDomain(void);

/**
 * \brief Clears reason for Warm and Main/Mcu Domain Power On Resets.
 *        CTRLMMR_RST_SRC is just a mirror of CTRLMMR_MCU_RST_SRC register. It
 *        is read only. So we need to write 1 to CTRLMMR_MCU_RST_SRC to clear
 *        the reset reason.
 *
 * \param resetCause [IN] Reset reason value to clear.
 */
void SOC_clearResetCauseMainMcuDomain(uint32_t resetCause);

/**
 * \brief Enable reset isolation of MCU domain for safety applications.
 *
 * \param main2McuIsolation [IN] Flag to enable isolation of mcu domain from main domain
 *                          Setting this flag restricts the access of MCU resources
 *                          by main domain
 * \param mcu2MainIsolation [IN] Flag to enable isolation of MCU domain from DM
 *                          Setting this flag restricts the access of MCU resources
 *                          by DM
 * \param mcu2dmIsolation  [IN] Flag to enable isolation of DM from mcu domain
 *                          Setting this flag restricts the access of DM resources
 *                          by MCU domain
 * \param debugIsolationEnable [IN] Enable debug isolation. Setting this would restrict
 *                             JTAG access to MCU domain
 */
int32_t SOC_enableResetIsolation(uint32_t main2McuIsolation,
                                uint32_t mcu2MainIsolation,
                                uint32_t mcu2dmIsolation,
                                uint32_t debugIsolationEnable);
/**
 * \brief Set MCU reset isolation done flag.
 *
 * \param value [IN] : 0 - Allow main domain reset to propogate
 *                   : 1 - Do not allow main domain reset to propogate
 */
void SOC_setMCUResetIsolationDone(uint32_t value);

/**
 * \brief Wait for main domain reset to complete.
 */
void SOC_waitMainDomainReset(void);

/**
 * \brief Get PSC (Power Sleep Controller) state
 *
 * \param instNum [IN]      : PSC Instance. See SOC_PSCDomainId_t
 * \param domainNum [IN]    : Power domain number
 * \param moduleNum [IN]    : Module number
 * \param domainState [OUT] : Domain state (1 : ON, 0 : OFF)
 * \param moduleState [OUT] : Module State. See SOC_PSCModuleState_t
 *
 * \return SystemP_SUCCESS on success, else failure
 */
int32_t SOC_getPSCState(uint32_t instNum, uint32_t domainNum, uint32_t moduleNum,
                    uint32_t *domainState, uint32_t *moduleState);

/**
 * \brief Set PSC (Power Sleep Controller) state
 *
 * \param instNum [IN]      : PSC Instance. See SOC_PSCDomainId_t
 * \param domainNum [IN]    : Power domain number
 * \param moduleNum [IN]    : Module number
 * \param pscState [IN]     : PSC module state. See SOC_PSCModuleState_t
 *
 * \return SystemP_SUCCESS on success, else failure
 */
int32_t SOC_setPSCState(uint32_t instNum, uint32_t domainNum, uint32_t moduleNum, uint32_t pscState);

/**
 * \brief Update the boot block size for FSS subsystem.
 *
 * Selects the size of the boot block to be used for the OSPI flash
 * interface. Default value is 1'b0 - S0_BOOT_SIZE_64MB for the MMR
 * register. Set 1'b1 - S0_BOOT_SIZE_128MB to update the value.
 *
 */
void SOC_setFSSCtrlFlashBootSize(void);

/**
 * \brief Get Physical Address from virtual address.
 *
 * \param virtAddr [IN] : Virtual Address(Alias)
 * 
 * \return Physical Address
 */
uint64_t Soc_getPhyAddr(uint64_t virtAddr);

/** @} */

#ifdef __cplusplus
}
#endif

#endif
