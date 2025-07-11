
/*
 *  Copyright (C) 2024-2025 Texas Instruments Incorporated
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

/**
 *  \file tielfup32.c
 *
 *  \brief ELFUP apis.
 */

/* ========================================================================== */
/*                             Include Files                                  */
/* ========================================================================== */

#include <stdlib.h>
#include <kernel/dpl/SystemP.h>
#include "tielfup32.h"

/* ========================================================================== */
/*                             Macros & Typedefs                              */
/* ========================================================================== */

#define ELF_MAGIC_NUMBER (0x7FU)

/* ========================================================================== */
/*                             Global Variables                               */
/* ========================================================================== */

/* None */

/* ========================================================================== */
/*                          Function Declarations                             */
/* ========================================================================== */

/* None */

/* ========================================================================== */
/*                             Function Definitions                           */
/* ========================================================================== */

int16_t ELFUP_init(ELFUP_Handle *handle, ELFUP_ELFPH *pPhtArr, uint8_t pPhtArrSize)
{
    int16_t status = SystemP_SUCCESS;

    if((handle != NULL) && (pPhtArr != NULL) && (pPhtArrSize > 0U))
    {
        handle->pht = pPhtArr;
        handle->maxPhtSize = pPhtArrSize;
        handle->stateNext = ELFUP_PARSER_STATE_INIT;
        handle->statePrev = ELFUP_PARSER_STATE_INIT;
        handle->headerCnt = 0U;
        handle->elfFileStartOffset = 0U;
        handle->genericOffsetCounter = 0U;
        status = SystemP_SUCCESS;
    }
    return status;
}

int16_t ELFUP_update(ELFUP_Handle *handle, uint8_t byte)
{
    int16_t status = SystemP_FAILURE;

    char c = (char)byte;

    if(handle != NULL)
    {
        handle->stateNow = handle->stateNext;
        switch(handle->stateNow)
        {
            case ELFUP_PARSER_STATE_INIT:
                if((unsigned char)c == ELF_MAGIC_NUMBER)
                {
                    handle->elfFileStartOffset = handle->genericOffsetCounter;
                    handle->stateNext = ELFUP_PARSER_STATE_IDEN0;
                    handle->headerCnt = 0U;
                    handle->ELFHeader.buff[handle->headerCnt] = c;
                }
                break;
            case ELFUP_PARSER_STATE_IDEN0:
                if((handle->statePrev == ELFUP_PARSER_STATE_INIT) && (c == 'E'))
                {
                    handle->stateNext = ELFUP_PARSER_STATE_IDEN1;
                    handle->headerCnt = 1U;
                    handle->ELFHeader.buff[handle->headerCnt] = c;
                }
                break;
            case ELFUP_PARSER_STATE_IDEN1:
                if((handle->statePrev == ELFUP_PARSER_STATE_IDEN0) && (c == 'L'))
                {
                    handle->stateNext = ELFUP_PARSER_STATE_IDEN2;
                    handle->headerCnt = 2U;
                    handle->ELFHeader.buff[handle->headerCnt] = c;
                }
                break;
            case ELFUP_PARSER_STATE_IDEN2:
                if((handle->statePrev == ELFUP_PARSER_STATE_IDEN1) && (c == 'F'))
                {
                    handle->stateNext = ELFUP_PARSER_STATE_IDEN3;
                    handle->headerCnt = 3U;
                    handle->ELFHeader.buff[handle->headerCnt] = c;
                }
                break;
            case ELFUP_PARSER_STATE_IDEN3:
                handle->headerCnt += 1U;
                handle->ELFHeader.buff[handle->headerCnt] = c;

                if(handle->headerCnt == (ELF_HEADER_32_SIZE - 1U))
                {
                    handle->headerCnt = 0U;
                    if(handle->ELFHeader.ELFH.ePhnum > handle->maxPhtSize)
                    {
                        handle->stateNext = ELFUP_PARSER_STATE_ERROR;
                    }
                    else
                    {
                        handle->stateNext = ELFUP_PARSER_STATE_PHT;
                    }
                }
                else
                {
                    handle->stateNext = ELFUP_PARSER_STATE_IDEN3;
                }
                break;
            case ELFUP_PARSER_STATE_PHT:
                *(((uint8_t*)handle->pht) + handle->headerCnt) = (uint8_t)c;
                handle->headerCnt += 1U;
                if(handle->headerCnt >= ((handle->ELFHeader.ELFH.ePhentsize * handle->ELFHeader.ELFH.ePhnum)))
                {
                    handle->stateNext = ELFUP_PARSER_STATE_END;
                }
                break;
            case ELFUP_PARSER_STATE_END:
            case ELFUP_PARSER_STATE_ERROR:
            default:
                break;
        }
        handle->genericOffsetCounter += 1U;
        handle->statePrev = handle->stateNow;
        status = SystemP_SUCCESS;
    }

    return status;
}

int16_t ELFUP_isPartOfSegment(ELFUP_Handle *handle, uint32_t offset, ELFUP_ELFPH *phtInfo)
{
    int16_t status = SystemP_FAILURE;
    uint32_t ph_len;
    uint32_t ph_offset;

    if((handle != NULL) && (phtInfo != NULL))
    {
        if(handle->stateNow == ELFUP_PARSER_STATE_END)
        {
            for(uint8_t i = 0U; i < handle->ELFHeader.ELFH.ePhnum; i++)
            {
                ph_len = handle->pht[i].ELFPH.memsz;
                ph_offset = handle->pht[i].ELFPH.offset;
                if((ph_offset <= offset) && (offset < (ph_offset + ph_len)))
                {
                    *phtInfo = handle->pht[i];
                    status = SystemP_SUCCESS;
                    break;
                }
            }
        }
    }
    return status;
}
