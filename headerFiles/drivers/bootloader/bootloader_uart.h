/*
 *  Copyright (C) 2024 Texas Instruments Incorporated
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

#ifndef BOOTLOADER_UART_H
#define BOOTLOADER_UART_H

/* UART protocol commands */
#define BOOTLOADER_UART_MAGIC                      (0xBF0000BFU)
#define BOOTLOADER_UART_FILE_RECEIVE_COMPLETE      (0xBF000002U)
#define BOOTLOADER_UART_SEND_FILE                  (0xBF000003U)

#ifdef __cplusplus
extern "C"
{
#endif

#include <drivers/bootloader/bootloader_dma.h>

/**
 * \brief Arguments to be passed to driver implementation callbacks when boot media is SOC memory
 */
typedef struct Bootloader_UartArgs_s
{
    uint32_t curOffset;
    uint32_t appImageBaseAddr;
    uint32_t drvIdx;
    uint32_t virtMemOffset;
    uint32_t segmentSize;
    uint32_t loadAddress;
    uint8_t finalPacket;

} Bootloader_UartArgs;

/* UART protocol request header */
typedef struct Bootloader_UartProtocolReq_s
{
    uint64_t magic;
    uint64_t virtMemOffset;
    uint64_t cmd;
    uint64_t len;

} Bootloader_UartProtocolReq;

extern Bootloader_Fxns gBootloaderUartFxns;


#ifdef __cplusplus
}
#endif

#endif  /* BOOTLOADER_UART_H */