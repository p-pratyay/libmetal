/*
 *  Copyright (C) 2015-2025 Texas Instruments Incorporated
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
 * \defgroup tisci_pm_device tisci_pm_device
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
 * Cortex-M3 (CM3) firmware for power management
 *
 */
#ifndef TISCI_PM_TISCI_DEVICE_H
#define TISCI_PM_TISCI_DEVICE_H

#ifdef __cplusplus
extern "C"
{
#endif



/**
 * Configure the device to be a wake source. The meaning of this flag will
 * vary slightly from device to device and from SoC to SoC but it
 * generally allows the device to wake the SoC out of low power states.
 */
#define TISCI_MSG_FLAG_DEVICE_WAKE_ENABLED    TISCI_BIT(8)

/** Enable reset isolation for this device. */
#define TISCI_MSG_FLAG_DEVICE_RESET_ISO       TISCI_BIT(9)

/**
 * Claim a device exclusively. When passed with STATE_RETENTION or STATE_ON,
 * it will claim the device exclusively. If another host already has this
 * device set to STATE_RETENTION or STATE_ON, the message will fail. Once
 * successful, other hosts attempting to set STATE_RETENTION or STATE_ON
 * will fail.
 */
#define TISCI_MSG_FLAG_DEVICE_EXCLUSIVE       TISCI_BIT(10)

/**
 * Used by TISCI_MSG_SET_DEVICE to turn device off when possible. This
 * must be used in conjunction with SoC dependencies to identify the
 * overall power domain state being achieved.
 */
#define TISCI_MSG_VALUE_DEVICE_SW_STATE_AUTO_OFF    0U

/** Used by TISCI_MSG_SET_DEVICE to disable device but keep in retention. */
#define TISCI_MSG_VALUE_DEVICE_SW_STATE_RETENTION   1U

/** Used by TISCI_MSG_SET_DEVICE to turn device on for usage. */
#define TISCI_MSG_VALUE_DEVICE_SW_STATE_ON          2U

/**
 * TISCI_MSG_GET_DEVICE sets this as current state to indicate device
 * is off.
 */
#define TISCI_MSG_VALUE_DEVICE_HW_STATE_OFF         0U

/**
 * TISCI_MSG_GET_DEVICE sets this as current state to indicate device
 * is on.
 */
#define TISCI_MSG_VALUE_DEVICE_HW_STATE_ON          1U

/**
 * TISCI_MSG_GET_DEVICE sets this as current state to indicate device is
 * transitioning between states. When a device stays in this state it is
 * typically due to the fact that some resource that the device is
 * dependent on (example IRQs) are pending preventing completion of
 * hardware handshake. Please refer to Technical Reference Manual for
 * additional information.
 */
#define TISCI_MSG_VALUE_DEVICE_HW_STATE_TRANS       2U

/**
 * \brief Set the desired state of the device.
 *
 * Certain flags can also be set to alter the device state:
 * TISCI_MSG_FLAG_DEVICE_WAKE_ENABLED, TISCI_MSG_FLAG_DEVICE_RESET_ISO,
 * TISCI_MSG_FLAG_DEVICE_EXCLUSIVE
 *
 * \param hdr TISCI header
 *
 * \param id
 * Indicates which device to modify
 *
 * \param reserved
 * Field is ignored
 *
 * \param state
 * The desired state of the device.
 */
struct tisci_msg_set_device_req {
    struct tisci_header    hdr;
    uint32_t            id;
    uint32_t            reserved;
    uint8_t            state;
} __attribute__((__packed__));

/**
 * \brief Empty response for TISCI_MSG_SET_DEVICE
 *
 * Although this message is essentially empty and contains only a header
 * a full data structure is created for consistency in implementation.
 *
 * \param hdr TISCI header to provide ACK/NAK flags to the host.
 */
struct tisci_msg_set_device_resp {
    struct tisci_header hdr;
} __attribute__((__packed__));

/**
 * \brief Request to get device based on id.
 *
 * \param hdr TISCI header.
 *
 * \param id
 * Indicates which device to modify
 */
struct tisci_msg_get_device_req {
    struct tisci_header    hdr;
    uint32_t            id;
} __attribute__((__packed__));

/**
 * \brief Response to get device request.
 *
 * \param hdr TISCI header.
 *
 * \param context_loss_count
 * Indicates how many times the device has lost context. A driver can use this
 * monotonic counter to determine if the device has lost context since the
 * last time this message was exchanged.
 *
 * \param resets
 * Programmed state of the reset lines.
 *
 * \param programmed_state
 * The state as programmed by set_device.
 *
 * \param current_state
 * The actual state of the hardware.
 */
struct tisci_msg_get_device_resp {
    struct tisci_header    hdr;
    uint32_t            context_loss_count;
    uint32_t            resets;
    uint8_t            programmed_state;
    uint8_t            current_state;
} __attribute__((__packed__));

/**
 * \brief Set the desired state of the resets of a device.
 *
 * \param hdr TISCI header.
 *
 * \param id
 * Indicates which device to modify
 *
 * \param resets
 * A bit field of resets for the device. The meaning, behavior, and usage of
 * the reset flags are device specific. Altough the flags can be set
 * independently of device state, the firmware will control when resets are
 * actually set to insure compliance with the SoC specific reset requirements.
 */
struct tisci_msg_set_device_resets_req {
    struct tisci_header    hdr;
    uint32_t            id;
    uint32_t            resets;
} __attribute__((__packed__));

/**
 * \brief Empty response for TISCI_MSG_SET_DEVICE_RESETS
 *
 * Although this message is essentially empty and contains only a header
 * a full data structure is created for consistency in implementation.
 *
 * \param hdr TISCI header to provide ACK/NAK flags to the host.
 */
struct tisci_msg_set_device_resets_resp {
    struct tisci_header hdr;
} __attribute__((__packed__));

/**
 * \brief Drop any remaining powerup device references.
 *
 * At boot, any powered up devices receive a powerup reference. This keeps
 * the device on. When a host enables a device, the powerup reference is
 * dropped and replaced with a host reference. Similarly, when a device
 * disables a device the powerup reference is dropped.
 *
 * This messages drops powerup reference from all devices, turning off
 * any devices with a powerup reference.
 *
 * \param hdr TISCI header.
 */
struct tisci_msg_device_drop_powerup_ref_req {
    struct tisci_header hdr;
} __attribute__((__packed__));

/**
 * \brief Empty response for TISCI_MSG_DEVICE_DROP_POWERUP_REF
 *
 * Although this message is essentially empty and contains only a header
 * a full data structure is created for consistency in implementation.
 *
 * \param hdr TISCI header to provide ACK/NAK flags to the host.
 */
struct tisci_msg_device_drop_powerup_ref_resp {
    struct tisci_header hdr;
} __attribute__((__packed__));


#ifdef __cplusplus
}
#endif

#endif /* TISCI_PM_TISCI_DEVICE_H */

/** @} */
