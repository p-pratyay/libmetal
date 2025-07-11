/*
 *  Copyright (C) 2020-2025 Texas Instruments Incorporated
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
 * \defgroup tisci_sa2ul_rm tisci_sa2ul_rm
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
 * System Firmware
 *
 * TISCI Data structures and macros for SA2UL resource management
 *
 */
#ifndef TISCI_SA2UL_RM_H_
#define TISCI_SA2UL_RM_H_

#ifdef __cplusplus
extern "C"
{
#endif



/**
 * \brief Request to acquire the authentication resources from System Fimrware
 *
 * The following is to request for SYSFW authentication resources to be handed
 * off to the requesting host. The following occurs during this API request:
 *
 *   PKA firewall region ownership is transitioned to the requesting host. No
 *   permissions are updated at this time - the host must take further action to
 *   update the region permissions through firewall API.
 *
 *   Secure PSIL threads are unpaired by SYSFW so that the requesting host can
 *   pair to their own DMA channels
 *
 * Only the host who is listed as the auth_resources owner in the security board
 * configuration may make this request.
 *
 * \param hdr Common TI-SCI header
 */

struct tisci_msg_sa2ul_auth_res_acquire_req {
    struct tisci_header hdr;
} __attribute__((__packed__));

/**
 * \brief Response to authentication resource acquisition request
 *
 * \param hdr Generic TISCI message header. ACK response indicates that the
 *            secure threads are unpaired from System Firmware's DMA channels
 *            and the host has exclusive access to PKA registers. A NACK
 *            response indicates that the resources could not be released to the
 *            requesting host.
 */
struct tisci_msg_sa2ul_auth_res_acquire_resp {
    struct tisci_header hdr;
} __attribute__((__packed__));

/**
 * \brief Request to release the authentication resources back to System
 *        Fimrware
 *
 * The following is to request for SYSFW authentication resources to be given
 * back to SYSFW. The following occurs during this API request:
 *
 *   PKA firewall is updated so that SYSFW becomes owner once again, and access
 *   is limited to DMSC only
 *
 *   Secure PSIL threads are cleaned up so that they can be are re-paired by
 *   SYSFW
 *
 * Only the host who is listed as the auth_resources owner in the security board
 * configuration may make this request.
 *
 * \param hdr Common TI-SCI header
 */

struct tisci_msg_sa2ul_auth_res_release_req {
    struct tisci_header hdr;
} __attribute__((__packed__));

/**
 * \brief Response to authentication resource release request
 *
 * \param hdr Generic TISCI message header. ACK response indicates that the
 * secure threads are cleaned for pairing to System Firmware's DMA channels and
 * the DMSC access to PKA registers is restored. A NACK response indicates that
 * the resources could not be released to the requesting back to System
 * Firmware.
 */
struct tisci_msg_sa2ul_auth_res_release_resp {
    struct tisci_header hdr;
} __attribute__((__packed__));


#ifdef __cplusplus
}
#endif

#endif /* TISCI_SA2UL_RM_H_ */

/** @} */
