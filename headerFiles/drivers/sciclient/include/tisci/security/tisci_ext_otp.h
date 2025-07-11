/*
 *  Copyright (C) 2019-2025 Texas Instruments Incorporated
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
 * \defgroup tisci_ext_otp tisci_ext_otp
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
 * TISCI Data structures and macros for External OTP access
 *
 */

#ifndef TISCI_EXT_OTP_H
#define TISCI_EXT_OTP_H

#ifdef __cplusplus
extern "C"
{
#endif



/**
 * \brief Request message for reading OTP MMR
 *
 * \param hdr Common TI-SCI header
 * \param mmr_idx Index of the 32-bit MMR to be read. Index starts from zero.
 */
struct tisci_msg_read_otp_mmr_req {
    struct tisci_header    hdr;
    uint8_t            mmr_idx;
} __attribute__((__packed__));

/**
 * \brief Respone message for reading OTP MMR
 *
 * \param hdr Common TI-SCI header
 * \param mmr_val Value of the requested 32-bit MMR
 */
struct tisci_msg_read_otp_mmr_resp {
    struct tisci_header    hdr;
    uint32_t            mmr_val;
} __attribute__((__packed__));

/**
 * \brief Request message for performing a global soft write lock on OTP rows
 *
 * \param hdr Common TI-SCI header
 */
struct tisci_msg_soft_lock_otp_write_global_req {
    struct tisci_header hdr;
} __attribute__((__packed__));

/**
 * \brief Respone message for global soft write lock on OTP rows
 *
 * \param hdr Common TI-SCI header
 */
struct tisci_msg_soft_lock_otp_write_global_resp {
    struct tisci_header hdr;
} __attribute__((__packed__));

/**
 * \brief Request message for hardware locking an OTP row
 *
 * \param hdr Common TI-SCI header
 * \param row_idx Index of the otp row to be written. Index starts from zero.
 * \param hw_write_lock    indicates if write lock has to be applied in HW on the current row. Set to 0x5A for write lock.
 * \param hw_read_lock     indicates if read lock has to be applied in HW on the current row. Set to 0x5A for read lock.
 * \param row_soft_lock    indicates if soft lock has to be enabled on current row preventing writes. (ignored for now)
 *
 * In all cases 0x5A indicates true and 0xA5 indicates false. All other values are invalid.
 */
struct tisci_msg_lock_otp_row_req {
    struct tisci_header    hdr;
    uint8_t            row_idx;
    uint8_t            hw_write_lock;
    uint8_t            hw_read_lock;
    uint8_t            row_soft_lock;
} __attribute__((__packed__));

/**
 * \brief Respone message for hardware lock OTP row
 *
 * \param hdr Common TI-SCI header
 */
struct tisci_msg_lock_otp_row_resp {
    struct tisci_header hdr;
} __attribute__((__packed__));

/**
 * \brief Request message for writing to an OTP row
 *
 * \param hdr Common TI-SCI header
 * \param row_idx Index of the otp row to be written. Index starts from zero.
 * \param row_val value to be written to the otp row
 * \param row_mask mask specifying the bits in row_val to be written
 */
struct tisci_msg_write_otp_row_req {
    struct tisci_header    hdr;
    uint8_t            row_idx;
    uint32_t            row_val;
    uint32_t            row_mask;
} __attribute__((__packed__));

/**
 * \brief Respone message for writing to an OTP row
 *
 * \param hdr Common TI-SCI header
 * \param row_val value read back from the row after the write operation.
 */
struct tisci_msg_write_otp_row_resp {
    struct tisci_header    hdr;
    uint32_t            row_val;
} __attribute__((__packed__));

/**
 * \brief Request message for querying lock status of an OTP row
 *
 * \param hdr Common TI-SCI header
 * \param row_idx Index of the otp row to be queried. Index starts from zero.
 */
struct tisci_msg_get_otp_row_lock_status_req {
    struct tisci_header    hdr;
    uint8_t            row_idx;
} __attribute__((__packed__));

/**
 * \brief Respone message for Get OTP row lock status request
 *
 * \param hdr Common TI-SCI header
 * \param global_soft_lock indicates if global soft lock has been enabled preventing writes.
 * \param hw_write_lock    indicates if write lock has been applied in HW on the current row.
 * \param hw_read_lock     indicates if read lock has been applied in HW on the current row.
 * \param row_soft_lock    indicates if soft lock has been enabled on current row preventing writes.
 *
 * In all cases 0x5A indicates true and 0xA5 indicates false. All other values are invalid.
 */
struct tisci_msg_get_otp_row_lock_status_resp {
    struct tisci_header    hdr;
    uint8_t            global_soft_lock;
    uint8_t            hw_write_lock;
    uint8_t            hw_read_lock;
    uint8_t            row_soft_lock;
} __attribute__((__packed__));


#ifdef __cplusplus
}
#endif

#endif /* TISCI_EXT_OTP_H */

/** @} */
