/*
 *  Copyright (C) 2018-2025 Texas Instruments Incorporated
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

#include <drivers/bootloader/bootloader_uniflash.h>
#include <drivers/bootloader/bootloader_priv.h>
#include <stdlib.h>
#include <string.h>
#include <board/flash.h>
#include "kernel/dpl/CacheP.h"

#if defined(DRV_VERSION_MMCSD_V0) || defined(DRV_VERSION_MMCSD_V1)
#include <drivers/mmcsd.h>
#include <drivers/bootloader/bootloader_mmcsd_raw.h>
#endif

#if defined (DRV_BOOTLOADER_FORMAT_MCELF)
#include <drivers/bootloader/tiELFuParser/tielfup32.h>
#endif

static void Bootloader_uniflashInitRespHeader(Bootloader_UniflashResponseHeader *respHeader);
static int32_t Bootloader_uniflashFlashFile(uint32_t flashIndex, uint8_t *buf, uint32_t fileSize, uint32_t flashOffset);
static int32_t Bootloader_uniflashFlashVerifyFile(uint32_t flashIndex, uint8_t *fileBuf, uint32_t fileSize, uint8_t *verifyBuf, uint32_t verifyBufSize, uint32_t flashOffset);
static int32_t Bootloader_uniflashFlashErase(uint32_t flashIndex, uint32_t flashOffset, uint32_t flashSize);
static int32_t Bootloader_uniflashFlashXipFile(uint32_t flashIndex, uint8_t *fileBuf, uint32_t fileSize);
static int32_t Bootloader_uniflashFlashVerifyXipFile(uint32_t flashIndex, uint8_t *fileBuf, uint32_t fileSize, uint8_t *verifyBuf, uint32_t verifyBufSize);
static int32_t Bootloader_uniflashFlashPhyTuningData(uint32_t flashIndex);

#if defined(DRV_VERSION_MMCSD_V0) || defined(DRV_VERSION_MMCSD_V1)
static int32_t Bootloader_uniflashFlashFileMMCSDRaw(uint32_t flashIndex, uint8_t *buf, uint32_t fileSize, uint32_t flashOffset);
static int32_t Bootloader_uniflashFlashVerifyFileMMCSDRaw(uint32_t flashIndex, uint8_t *fileBuf, uint32_t fileSize, uint8_t *verifyBuf, uint32_t verifyBufSize, uint32_t flashOffset);
#endif

#if defined (DRV_BOOTLOADER_FORMAT_MCELF)
static int32_t Bootloader_uniflashFlashMcelfXipFile(uint32_t flashIndex, uint8_t *fileBuf, uint32_t fileSize);
static int32_t Bootloader_uniflashFlashVerifyMcelfXipFile(uint32_t flashIndex, uint8_t *fileBuf, uint32_t fileSize, uint8_t *verifyBuf, uint32_t verifyBufSize);
#endif

int32_t Bootloader_uniflashProcessFlashCommands(Bootloader_UniflashConfig *config, Bootloader_UniflashResponseHeader *respHeader)
{
	int32_t status = SystemP_SUCCESS;
	Bootloader_UniflashFileHeader fileHeader;

	memcpy(&fileHeader, config->buf, sizeof(Bootloader_UniflashFileHeader));

	Bootloader_uniflashInitRespHeader(respHeader);

	if(fileHeader.magicNumber != BOOTLOADER_UNIFLASH_FILE_HEADER_MAGIC_NUMBER)
	{
	    respHeader->statusCode = BOOTLOADER_UNIFLASH_STATUSCODE_MAGIC_ERROR;
	    status = SystemP_FAILURE;
	}

	/* Obtain the actual filesize */
	config->bufSize = fileHeader.actualFileSize;

	/* Check if the actual filesize is 16 B aligned. This is smallest program granularity suppported by NOR flashes.
	 * If it is not aligned, we have to write 1-15 bytes extra. Since xmodem would have already padded zeros into the
	 * file buffer for 1024B alignment, we can assume that these 1-15 bytes would be zero.
	 */
	uint32_t remainder = config->bufSize % (uint32_t)16;
	if(remainder != (uint32_t)0)
	{
        /* Defensive call to pad zeroes make the image 16B aligned*/
        memset((void*)(config->buf + sizeof(Bootloader_UniflashFileHeader) + config->bufSize), 0x0, (uint32_t)16 - remainder);

        config->bufSize += ((uint32_t)16 - remainder);
	}
	else
	{
		/* do nothing */
	}

    config->verifyBufSize = config->bufSize;

	if(SystemP_SUCCESS == status)
	{
	    uint32_t opType = (fileHeader.operationTypeAndFlags) & (uint32_t)0xFF;

	    switch(opType)
	    {
	        case BOOTLOADER_UNIFLASH_OPTYPE_FLASH:
	            /* flash the file at the given offset*/
	            status = Bootloader_uniflashFlashFile(config->flashIndex, config->buf + sizeof(Bootloader_UniflashFileHeader), config->bufSize, fileHeader.offset);
	            if(status != SystemP_SUCCESS)
	            {
	                respHeader->statusCode = BOOTLOADER_UNIFLASH_STATUSCODE_FLASH_ERROR;
	            }
	            else
	            {
	                /* verify the file at the given offset */
	                status = Bootloader_uniflashFlashVerifyFile(config->flashIndex, config->buf + sizeof(Bootloader_UniflashFileHeader), config->bufSize, config->verifyBuf, config->verifyBufSize, fileHeader.offset);
	                if(status != SystemP_SUCCESS)
	                {
	                    respHeader->statusCode = BOOTLOADER_UNIFLASH_STATUSCODE_FLASH_VERIFY_ERROR;
	                }
	            }
	            break;

	        case BOOTLOADER_UNIFLASH_OPTYPE_FLASH_VERIFY:
	            /* verify the file at the given offset */
	            status = Bootloader_uniflashFlashVerifyFile(config->flashIndex, config->buf + sizeof(Bootloader_UniflashFileHeader), config->bufSize, config->verifyBuf, config->verifyBufSize, fileHeader.offset);
	            if(status != SystemP_SUCCESS)
	            {
	                respHeader->statusCode = BOOTLOADER_UNIFLASH_STATUSCODE_FLASH_VERIFY_ERROR;
	            }
	            break;

	        case BOOTLOADER_UNIFLASH_OPTYPE_FLASH_ERASE:
	        	/* erase the flash from given offset for the given size */
	        	status = Bootloader_uniflashFlashErase(config->flashIndex, fileHeader.offset, fileHeader.eraseSize);
	        	if(status != SystemP_SUCCESS)
	            {
	                respHeader->statusCode = BOOTLOADER_UNIFLASH_STATUSCODE_FLASH_ERASE_ERROR;
	            }
	            break;

	        case BOOTLOADER_UNIFLASH_OPTYPE_FLASH_XIP:
	            /* flash the XIP file, flash offsets are within the file itself */

#if defined (DRV_BOOTLOADER_FORMAT_MCELF)
					status = Bootloader_uniflashFlashMcelfXipFile(config->flashIndex, config->buf + sizeof(Bootloader_UniflashFileHeader), config->bufSize);
#else
					status = Bootloader_uniflashFlashXipFile(config->flashIndex, config->buf + sizeof(Bootloader_UniflashFileHeader), config->bufSize);
#endif
	            if(status != SystemP_SUCCESS)
	            {
	                respHeader->statusCode = BOOTLOADER_UNIFLASH_STATUSCODE_FLASH_ERROR;
	            }
	            else
	            {
	                /* verify the file at the given offset */
#if defined (DRV_BOOTLOADER_FORMAT_MCELF)
						status = Bootloader_uniflashFlashVerifyMcelfXipFile(config->flashIndex, config->buf + sizeof(Bootloader_UniflashFileHeader), config->bufSize, config->verifyBuf, config->verifyBufSize);
#else
						status = Bootloader_uniflashFlashVerifyXipFile(config->flashIndex, config->buf + sizeof(Bootloader_UniflashFileHeader), config->bufSize, config->verifyBuf, config->verifyBufSize);
#endif
	                if(status != SystemP_SUCCESS)
	                {
	                    respHeader->statusCode = BOOTLOADER_UNIFLASH_STATUSCODE_FLASH_VERIFY_ERROR;
	                }
	            }
	            break;
	        case BOOTLOADER_UNIFLASH_OPTYPE_FLASH_VERIFY_XIP:
	            /* verify the file. Flash offsets are within the file itself */
#if defined (DRV_BOOTLOADER_FORMAT_MCELF)
					status = Bootloader_uniflashFlashVerifyMcelfXipFile(config->flashIndex, config->buf + sizeof(Bootloader_UniflashFileHeader), config->bufSize, config->verifyBuf, config->verifyBufSize);
#else
					status = Bootloader_uniflashFlashVerifyXipFile(config->flashIndex, config->buf + sizeof(Bootloader_UniflashFileHeader), config->bufSize, config->verifyBuf, config->verifyBufSize);
#endif
	            if(status != SystemP_SUCCESS)
	            {
	                respHeader->statusCode = BOOTLOADER_UNIFLASH_STATUSCODE_FLASH_VERIFY_ERROR;
	            }
	            break;

	        case BOOTLOADER_UNIFLASH_OPTYPE_FLASH_TUNING_DATA:
	            /* verify the file. Flash offsets are within the file itself */
	            status = Bootloader_uniflashFlashPhyTuningData(config->flashIndex);
	            if(status != SystemP_SUCCESS)
	            {
	                respHeader->statusCode = BOOTLOADER_UNIFLASH_STATUSCODE_FLASH_ERROR;
	            }
	            break;

#if defined(DRV_VERSION_MMCSD_V0) || defined(DRV_VERSION_MMCSD_V1)
            case BOOTLOADER_UNIFLASH_OPTYPE_EMMC_FLASH:
                /* flash the file at the given offset of MMCSD */
	            status = Bootloader_uniflashFlashFileMMCSDRaw(config->flashIndex, config->buf + sizeof(Bootloader_UniflashFileHeader), config->bufSize, fileHeader.offset);
	            if(status != SystemP_SUCCESS)
	            {
	                respHeader->statusCode = BOOTLOADER_UNIFLASH_STATUSCODE_FLASH_ERROR;
	            }
                else
	            {
	                /* verify the file at the given offset */
	                status = Bootloader_uniflashFlashVerifyFileMMCSDRaw(config->flashIndex, config->buf + sizeof(Bootloader_UniflashFileHeader), config->bufSize, config->verifyBuf, config->verifyBufSize, fileHeader.offset);
	                if(status != SystemP_SUCCESS)
	                {
	                    respHeader->statusCode = BOOTLOADER_UNIFLASH_STATUSCODE_FLASH_VERIFY_ERROR;
	                }
	            }
                break;

            case BOOTLOADER_UNIFLASH_OPTYPE_EMMC_VERIFY:
                /* verify the file at the given offset */
	            status = Bootloader_uniflashFlashVerifyFileMMCSDRaw(config->flashIndex, config->buf + sizeof(Bootloader_UniflashFileHeader), config->bufSize, config->verifyBuf, config->verifyBufSize, fileHeader.offset);
	            if(status != SystemP_SUCCESS)
	            {
	                respHeader->statusCode = BOOTLOADER_UNIFLASH_STATUSCODE_FLASH_VERIFY_ERROR;
	            }
	            break;
#endif
	        default:
	            respHeader->statusCode = BOOTLOADER_UNIFLASH_STATUSCODE_OPTYPE_ERROR;
	            break;
	    }
	}

	return status;
}

static void Bootloader_uniflashInitRespHeader(Bootloader_UniflashResponseHeader *respHeader)
{
	if(respHeader != 0)
	{
	    respHeader->magicNumber = BOOTLOADER_UNIFLASH_RESP_HEADER_MAGIC_NUMBER;
	    respHeader->statusCode  = BOOTLOADER_UNIFLASH_STATUSCODE_SUCCESS;
	    respHeader->rsv0        = 0xDEADBABE;
	    respHeader->rsv1        = 0xDEADBABE;
	}
}

static int32_t Bootloader_uniflashFlashFile(uint32_t flashIndex, uint8_t *buf, uint32_t fileSize, uint32_t flashOffset)
{
	int32_t status = SystemP_SUCCESS;

	Flash_Attrs *flashAttrs;
	Flash_Handle flashHandle;
	uint32_t eraseBlockSize = 0xFFFFFFFFu;

	flashAttrs = Flash_getAttrs(flashIndex);
	flashHandle = Flash_getHandle(flashIndex);

	if((flashAttrs == 0) || (flashHandle == NULL))
	{
	   status=SystemP_FAILURE;
	}
	else
	{
	    eraseBlockSize = flashAttrs->pageCount * flashAttrs->pageSize;
	}

	if((status == SystemP_SUCCESS) && ((flashOffset % eraseBlockSize) != (uint32_t)0))
	{
		/* Only flash to offsets which are a multiple of blockSize */
		status=SystemP_FAILURE;
	}

	if(status==SystemP_SUCCESS)
	{
	    uint32_t curOffset, totalChunks, curChunk, chunkSize, remainSize, blockNum, pageNum;
	    uint8_t *srcAddr;

	    /* start writing from buffer to flash */
	    chunkSize = eraseBlockSize;

	    srcAddr = buf;
	    remainSize = fileSize;
	    curOffset = flashOffset;
	    curChunk = 1;
	    totalChunks = (fileSize + (chunkSize-(uint32_t)1))/chunkSize;
	    while((curChunk <= totalChunks) && (status == SystemP_SUCCESS))
	    {
	        if(remainSize < chunkSize)
	        {
	            chunkSize = remainSize;
	        }

	        status = Flash_offsetToBlkPage(flashHandle, curOffset, &blockNum, &pageNum);
	        if(status == SystemP_SUCCESS)
	        {
	            status = Flash_eraseBlk(flashHandle, blockNum);
	        }
	        curOffset += chunkSize;
	        srcAddr = srcAddr + chunkSize;
	        remainSize -= chunkSize;
	        curChunk++;
	    }

        if(status == SystemP_SUCCESS)
	    {
	        status = Flash_write(flashHandle, flashOffset, buf, fileSize);
	    }
	}

	return status;
}

static int32_t Bootloader_uniflashFlashVerifyFile(uint32_t flashIndex, uint8_t *fileBuf, uint32_t fileSize, uint8_t *verifyBuf, uint32_t verifyBufSize, uint32_t flashOffset)
{
	int32_t status = SystemP_SUCCESS;

	Flash_Attrs *flashAttrs;
	Flash_Handle flashHandle;
	flashAttrs = Flash_getAttrs(flashIndex);
	flashHandle = Flash_getHandle(flashIndex);

	if((flashAttrs == 0) || (flashHandle == NULL))
	{
	   status=SystemP_FAILURE;
	}

	if(status==SystemP_SUCCESS)
	{
	    int32_t diff;

	    /* clear verify buf to avoid comparing stale data */
	    memset(verifyBuf, 0, (size_t)fileSize);

	    status = Flash_read(flashHandle, flashOffset, verifyBuf, fileSize);

	    if(status == SystemP_SUCCESS)
	    {
	        /* check if data read from flash matches, data read from file */
	        diff = memcmp(verifyBuf, fileBuf, (size_t)fileSize);

	        if(diff != (int32_t)0U)
	        {
	            status = SystemP_FAILURE;
	        }
	    }
	}

	return status;
}

static int32_t Bootloader_uniflashFlashErase(uint32_t flashIndex, uint32_t flashOffset, uint32_t eraseSize)
{
	int32_t status = SystemP_SUCCESS;

	Flash_Attrs *flashAttrs;
	Flash_Handle flashHandle;
	uint32_t eraseBlockSize = 0xFFFFFFFFu;
	uint32_t flashSize = 0U;

	flashAttrs = Flash_getAttrs(flashIndex);
	flashHandle = Flash_getHandle(flashIndex);

	if((flashAttrs == 0) || (flashHandle == NULL))
	{
	   status=SystemP_FAILURE;
	}
	else
	{
	    eraseBlockSize = flashAttrs->pageCount * flashAttrs->pageSize;
	    flashSize = eraseBlockSize * flashAttrs->blockCount;
	}

	if((status == SystemP_SUCCESS) && ((flashOffset % eraseBlockSize) != (uint32_t)0))
	{
		/* Only flash to offsets which are a multiple of blockSize */
		status=SystemP_FAILURE;
	}

	if((status == SystemP_SUCCESS) && (eraseSize > flashSize))
	{
		status=SystemP_FAILURE;
	}

	if(status==SystemP_SUCCESS)
	{
	    uint32_t curOffset, totalChunks, curChunk, chunkSize, remainSize, blockNum, pageNum;

	    /* start writing from buffer to flash */
	    chunkSize = eraseBlockSize;

	    remainSize = eraseSize;
	    curOffset = flashOffset;
	    curChunk = 1;
	    totalChunks = (eraseSize + (chunkSize-(uint32_t)1))/chunkSize;
	    while((curChunk <= totalChunks) && (status == SystemP_SUCCESS))
	    {
	        if(remainSize < chunkSize)
	        {
	            chunkSize = remainSize;
	        }

	        status = Flash_offsetToBlkPage(flashHandle, curOffset, &blockNum, &pageNum);
	        if(status == SystemP_SUCCESS)
	        {
	            status = Flash_eraseBlk(flashHandle, blockNum);
	        }
	        curOffset += chunkSize;
	        remainSize -= chunkSize;
	        curChunk++;
	    }
	}
	return status;
}

static int32_t Bootloader_uniflashFlashOrVerifyRprcXipFile(uint32_t flashIndex, uint8_t *buf, uint8_t *verifyBuf, uint32_t verifyBufSize)
{
    Bootloader_RprcFileHeader     header;
    Bootloader_RprcSectionHeader section;
    int32_t status = SystemP_SUCCESS;
    uint8_t *currentBuf = buf;

    memcpy(&header, currentBuf, sizeof(Bootloader_RprcFileHeader));
    currentBuf = currentBuf + sizeof(Bootloader_RprcFileHeader);
    if(header.magic != (uint32_t)BOOTLOADER_RPRC_MAGIC_NUMBER)
    {
        status = SystemP_FAILURE;
    }
    else
    {
        uint32_t i;

        for(i=0; i < header.sectionCount; i++)
        {
            memcpy(&section, currentBuf, sizeof(Bootloader_RprcSectionHeader));
            currentBuf = currentBuf + sizeof(Bootloader_RprcSectionHeader);

            if((verifyBuf != 0) && (verifyBufSize != (uint32_t)0))
            {
            	/* verify the section */
                status = Bootloader_uniflashFlashVerifyFile(flashIndex, currentBuf, section.size, verifyBuf, verifyBufSize, section.addr);
            }
            else
            {
                /* flash the section */
                status = Bootloader_uniflashFlashFile(flashIndex, currentBuf, section.size, section.addr);
            }
            currentBuf = currentBuf + section.size;

            if(status != SystemP_SUCCESS)
            {
                break;
            }
            else
            {
                /* do nothing */
            }
        }
    }
    return status;
}

static int32_t Bootloader_uniflashFlashOrVerifyXipFile(uint32_t flashIndex, uint8_t *buf, uint32_t fileSize, uint8_t *verifyBuf, uint32_t verifyBufSize)
{
    int32_t status = SystemP_SUCCESS;
    Bootloader_MetaHeaderStart mHdrStr;
    Bootloader_MetaHeaderCore  mHdrCore[BOOTLOADER_MAX_INPUT_FILES];
    uint8_t *ptr = buf;

    memset(&mHdrCore[0], 0xFF, BOOTLOADER_MAX_INPUT_FILES*sizeof(Bootloader_MetaHeaderCore));
    memcpy(&mHdrStr, ptr, sizeof(Bootloader_MetaHeaderStart));
    ptr = ptr + sizeof(Bootloader_MetaHeaderStart);

    if(mHdrStr.magicStr != (uint32_t)BOOTLOADER_META_HDR_MAGIC_STR)
    {
        status = SystemP_FAILURE;
    }
    else
    {
        /* Read all the core offset addresses */
        uint32_t i;

        for(i=0U; i<mHdrStr.numFiles; i++)
        {
            memcpy(&mHdrCore[i], ptr, sizeof(Bootloader_MetaHeaderCore));
            ptr = ptr + sizeof(Bootloader_MetaHeaderCore);
        }

        /* Parse individual rprc files */
        for(i=0U; i<mHdrStr.numFiles; i++)
        {
            if(mHdrCore[i].coreId != (0xFFFFFFFFU))
            {
                /* flash or verify the file */
                status = Bootloader_uniflashFlashOrVerifyRprcXipFile(flashIndex, buf + mHdrCore[i].imageOffset, verifyBuf, verifyBufSize);
                if(status != SystemP_SUCCESS)
				{
                    break;
				}
				else
				{
					/* do nothing */
				}
            }
			else
			{
				/* do nothing */
			}
        }
    }
    return status;
}

static int32_t Bootloader_uniflashFlashVerifyXipFile(uint32_t flashIndex, uint8_t *fileBuf, uint32_t fileSize, uint8_t *verifyBuf, uint32_t verifyBufSize)
{
    return Bootloader_uniflashFlashOrVerifyXipFile(flashIndex, fileBuf, fileSize, verifyBuf, verifyBufSize);
}

static int32_t Bootloader_uniflashFlashXipFile(uint32_t flashIndex, uint8_t *fileBuf, uint32_t fileSize)
{
    return Bootloader_uniflashFlashOrVerifyXipFile(flashIndex, fileBuf, fileSize, 0, 0);
}

#if defined (DRV_BOOTLOADER_FORMAT_MCELF)
static int32_t Bootloader_uniflashFlashOrVerifyMcelfXipFile(uint32_t flashIndex, uint8_t *fileBuf, uint32_t fileSize, uint8_t *verifyBuf, uint32_t verifyBufSize)
{
	int32_t retval = SystemP_FAILURE;
	Flash_Attrs *flashAttrs;
	Flash_Handle flashHandle;
	uint32_t eraseBlockSize = 0xFFFFFFFFu;

	if(fileBuf != NULL)
	{
		ELFUP_ELFPH pht[20];
		ELFUP_Handle elfuph;
		int32_t status = SystemP_FAILURE;

		memset(pht, 0xff, sizeof(pht));

		ELFUP_init(&elfuph, pht, 20);

		flashAttrs = Flash_getAttrs(flashIndex);
		flashHandle = Flash_getHandle(flashIndex);
		if((flashAttrs == NULL) || (flashHandle == NULL))
		{
			status = SystemP_FAILURE;
		}
		else
		{
			eraseBlockSize = flashAttrs->pageCount * flashAttrs->pageSize;
		}

		for(uint32_t offset = 0; (offset < fileSize) && (!((elfuph.stateNext == ELFUP_PARSER_STATE_END || elfuph.stateNext == ELFUP_PARSER_STATE_ERROR))); offset++)
		{
			ELFUP_update(&elfuph, fileBuf[offset]);
		}

		if(elfuph.stateNext == ELFUP_PARSER_STATE_END)
		{
			/* now we have the program header table */
			for(uint32_t  i =0; i < elfuph.ELFHeader.ELFH.ePhnum; i++)
			{
				Bootloader_ELFPH32 * entry = &(pht[i].ELFPH);
				if(entry->type == PT_LOAD)
				{

					uint32_t dest_addr = entry->paddr & ~(0xF0000000U);
					uint32_t src_addr = (uint32_t)fileBuf + entry->offset;
					uint32_t size = entry->memsz;

					if((dest_addr % eraseBlockSize) == 0U)
					{
						/* Only flash to offsets which are a multiple of blockSize */

						if((NULL != verifyBuf) && (verifyBufSize > 0U))
						{
							status = Bootloader_uniflashFlashVerifyFile(flashIndex, (uint8_t*)src_addr, size, verifyBuf, verifyBufSize, dest_addr);
						}
						else
						{
							status = Bootloader_uniflashFlashFile(flashIndex, (uint8_t*)src_addr, size, dest_addr);
						}
						if(status == SystemP_FAILURE)
						{
							break;
						}
					}
					else
					{
						status = SystemP_FAILURE;
						break;
					}
				}
			}
		}
		retval = status;
	}
	return(retval);
}

static int32_t Bootloader_uniflashFlashVerifyMcelfXipFile(uint32_t flashIndex, uint8_t *fileBuf, uint32_t fileSize, uint8_t *verifyBuf, uint32_t verifyBufSize)
{
	return Bootloader_uniflashFlashOrVerifyMcelfXipFile(flashIndex, fileBuf, fileSize, verifyBuf, verifyBufSize);
}

static int32_t Bootloader_uniflashFlashMcelfXipFile(uint32_t flashIndex, uint8_t *fileBuf, uint32_t fileSize)
{
	return Bootloader_uniflashFlashOrVerifyMcelfXipFile(flashIndex, fileBuf, fileSize, NULL, 0);
}
#endif

static int32_t Bootloader_uniflashFlashPhyTuningData(uint32_t flashIndex)
{
	int32_t status = SystemP_SUCCESS;

	Flash_Attrs *flashAttrs;
	Flash_Handle flashHandle;

	flashAttrs = Flash_getAttrs(flashIndex);
	flashHandle = Flash_getHandle(flashIndex);

	if((flashAttrs == 0) || (flashHandle == NULL))
	{
	   status = SystemP_FAILURE;
	}
	else
	{
		uint32_t phyTuningData, phyTuningDataSize;
		uint32_t phyTuningDataFlashOffset;
		uint32_t blockNum, pageNum;

		OSPI_phyGetTuningData(&phyTuningData, &phyTuningDataSize);
		phyTuningDataFlashOffset =  Flash_getPhyTuningOffset(flashHandle);

		status = Flash_offsetToBlkPage(flashHandle, phyTuningDataFlashOffset, &blockNum, &pageNum);
		if(status == SystemP_SUCCESS)
		{
		    status = Flash_eraseBlk(flashHandle, blockNum);
		}

		if(status == SystemP_SUCCESS)
		{
			status = Flash_write(flashHandle, phyTuningDataFlashOffset, (uint8_t *)phyTuningData, phyTuningDataSize);
		}
	}

	return status;
}

#if defined(DRV_VERSION_MMCSD_V0) || defined(DRV_VERSION_MMCSD_V1)
static int32_t Bootloader_uniflashFlashFileMMCSDRaw(uint32_t mmcsdIndex, uint8_t *buf, uint32_t fileSize, uint32_t flashOffset)
{
    int32_t status = SystemP_SUCCESS;

    MMCSD_Handle handle = MMCSD_getHandle(mmcsdIndex);
    if(handle == NULL)
    {
        status = SystemP_FAILURE;
    }
    else if (MMCSD_CARD_TYPE_NO_DEVICE == ((MMCSD_Config *)handle)->object->cardType)
    {
    	status = SystemP_FAILURE;
    }
    else
    {
        status = MMCSD_enableBootPartition(handle, 1);
    }

    if(status != SystemP_SUCCESS)
    {

    }
    else
    {
        status = Bootloader_MmcsdRaw_writeToOffset(handle, buf, fileSize, flashOffset);
    }

    return status;
}

static int32_t Bootloader_uniflashFlashVerifyFileMMCSDRaw(uint32_t mmcsdIndex, uint8_t *fileBuf, uint32_t fileSize, uint8_t *verifyBuf, uint32_t verifyBufSize, uint32_t flashOffset)
{
	int32_t status = SystemP_SUCCESS;

    MMCSD_Handle handle = MMCSD_getHandle(mmcsdIndex);

	if(handle == NULL)
	{
	   status = SystemP_FAILURE;
	}
    else
    {
        status = MMCSD_enableBootPartition(handle, 1);
    }

    if(status != SystemP_SUCCESS)
    {

    }
    else
	{
	    uint32_t curOffset, totalChunks, curChunk, chunkSize, remainSize;
	    uint8_t *srcAddr;
	    int32_t diff;


	    /* start writing from buffer to flash */
	    chunkSize = verifyBufSize;

	    srcAddr = fileBuf;
	    remainSize = fileSize;
	    curOffset = flashOffset;
	    curChunk = 1;
	    totalChunks = (fileSize + (chunkSize-(uint32_t)1))/chunkSize;
	    while(curChunk <= totalChunks)
	    {
	        if(remainSize < chunkSize)
	        {
	            chunkSize = remainSize;
	        }

	        /* clear verify buf to avoid comparing stale data */
	        memset(verifyBuf, 0, (size_t)verifyBufSize);

            status = Bootloader_MmcsdRaw_readFromOffset(handle, verifyBuf, chunkSize, curOffset);

	        if(status == SystemP_SUCCESS)
	        {
	            /* check if data read from flash matches, data read from file */
	            diff = memcmp(verifyBuf, srcAddr, (size_t)chunkSize);

	            if(diff != (int32_t)0)
	            {
	                status = SystemP_FAILURE;
	            }
	        }
	        curOffset += chunkSize;
	        srcAddr = srcAddr + chunkSize;
	        remainSize -= chunkSize;
	        curChunk++;
	    }
	}

	return status;
}

#endif

