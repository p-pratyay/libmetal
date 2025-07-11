/*
 *  Copyright (C) 2018-2021 Texas Instruments Incorporated
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

#include <drivers/ipc_rpmsg/ipc_rpmsg_priv.h>
#include <drivers/utils/utils.h>

/* NOTE:
 * For RTOS to RTOS IPC RPMessage
 *   - AVAIL in vring = EMPTY in ipc rpmsg implementation
 *   - USED in vring = FULL  in ipc rpmsg implementation
 *
 * For Linux to RTOS IPC RPMessage
 * - When doing TX from RTOS
 *   - AVAIL in vring = EMPTY in ipc rpmsg implementation
 *   - USED in vring = FULL  in ipc rpmsg implementation
 * - When doing RX from RTOS
 *   - AVAIL in vring = FULL in ipc rpmsg implementation
 *   - USED in vring = EMTPY  in ipc rpmsg implementation
 *
 * RPMessage_isLinuxCore is used in Rx APIs to switch the meaning when
 * receiving messages from core ID which runs Linux
 */

#define VRING_USED_F_NO_NOTIFY  (1U)

int32_t RPMessage_vringGetEmptyTxBuf(uint16_t remoteCoreId, uint16_t *vringBufId, uint32_t timeout)
{
    RPMessage_Core *coreObj = &gIpcRpmsgCtrl.coreObj[remoteCoreId];
    RPMessage_Vring *vringObj = &coreObj->vringTxObj;
    uintptr_t oldIntState;
    uint16_t head;
    int32_t status = SystemP_FAILURE;
    uint32_t done = 0;

    oldIntState = HwiP_disable();

    do
    {
        /* There's nothing available */
        if (vringObj->lastAvailIdx == vringObj->avail->idx)
        {
            /* We need to know about added buffers */
            vringObj->used->flags &= (uint16_t)~VRING_USED_F_NO_NOTIFY;

            HwiP_restore(oldIntState);

            status = SemaphoreP_pend(&coreObj->newEmptyVringBufSem, timeout);
            if(status==SystemP_TIMEOUT)
            {
                done = 1;
            }

            oldIntState = HwiP_disable();
        }
        else
        {
            head = vringObj->avail->ring[vringObj->lastAvailIdx % vringObj->vringNumBuf];
            vringObj->lastAvailIdx++;

            *vringBufId = head;
            done = 1;
            status = SystemP_SUCCESS;
        }
    } while(done != 1U);

    HwiP_restore(oldIntState);

    return status;
}

void RPMessage_vringPutFullTxBuf(uint16_t remoteCoreId, uint16_t vringBufId, uint16_t dataLen)
{
    RPMessage_Core *coreObj = &gIpcRpmsgCtrl.coreObj[remoteCoreId];
    RPMessage_Vring *vringObj = &coreObj->vringTxObj;
    struct vring_used_elem *used;
    uintptr_t oldIntState;
    uint32_t txMsgValue = RPMESSAGE_MSG_VRING_NEW_FULL;

    if(RPMessage_isLinuxCore(remoteCoreId) != 0U)
    {
        /* for linux we need to send the TX VRING ID in the mailbox message */
        txMsgValue = RPMESSAGE_LINUX_TX_VRING_ID;
    }
    else
    {
        if (gIpcRpmsgCtrl.vringAllocationPDK == 1u)
        {
            if(remoteCoreId > IpcNotify_getSelfCoreId())
            {
                txMsgValue = 1u;
            }
            else
            {
                txMsgValue = 0u;
            }
        }
    }

    oldIntState = HwiP_disable();

    used = &vringObj->used->ring[vringObj->used->idx % vringObj->vringNumBuf];
    used->id = (uint32_t)vringBufId;
    used->len = (uint32_t)dataLen;
    vringObj->used->idx++;

    Utils_dataAndInstructionBarrier();

    HwiP_restore(oldIntState);

    IpcNotify_sendMsg(remoteCoreId,
        IPC_NOTIFY_CLIENT_ID_RPMSG,
        txMsgValue,
        1 /* wait for message to be posted */
        );
}

void RPMessage_vringCheckEmptyTxBuf(uint16_t remoteCoreId)
{
    RPMessage_Core *coreObj = &gIpcRpmsgCtrl.coreObj[remoteCoreId];
    RPMessage_Vring *vringObj = &coreObj->vringTxObj;
    uint32_t isNewEmptyBuf = 1;
    uintptr_t oldIntState;

    oldIntState = HwiP_disable();

    if (vringObj->lastAvailIdx == vringObj->avail->idx)
    {
        isNewEmptyBuf = 0;
    }

    HwiP_restore(oldIntState);

    if(isNewEmptyBuf != 0U)
    {
        SemaphoreP_post(&coreObj->newEmptyVringBufSem);
    }
}

uint8_t *RPMessage_vringGetTxBufAddr(uint16_t remoteCoreId, uint16_t vringBufId)
{
    RPMessage_Core *coreObj = &gIpcRpmsgCtrl.coreObj[remoteCoreId];
    RPMessage_Vring *vringObj = &coreObj->vringTxObj;

    return (uint8_t *)vringObj->desc[vringBufId].addr;
}

uint32_t RPMessage_vringGetTxBufLen(uint16_t remoteCoreId, uint16_t vringBufId)
{
    RPMessage_Core *coreObj = &gIpcRpmsgCtrl.coreObj[remoteCoreId];
    RPMessage_Vring *vringObj = &coreObj->vringTxObj;

    return (uint32_t)vringObj->desc[vringBufId].len;
}

int32_t RPMessage_vringGetFullRxBuf(uint16_t remoteCoreId, uint16_t *vringBufId)
{
    RPMessage_Core *coreObj = &gIpcRpmsgCtrl.coreObj[remoteCoreId];
    RPMessage_Vring *vringObj = &coreObj->vringRxObj;
    uint16_t head;
    uintptr_t oldIntState;
    int32_t status = SystemP_TIMEOUT;

    oldIntState = HwiP_disable();

    if(RPMessage_isLinuxCore(remoteCoreId) != 0U)
    {
        /* There's nothing available */
        if (vringObj->lastAvailIdx != vringObj->avail->idx)
        {
            head = vringObj->avail->ring[vringObj->lastAvailIdx % vringObj->vringNumBuf];
            vringObj->lastAvailIdx++;

            *vringBufId = head;
            status = SystemP_SUCCESS;
        }
        else
        {
            vringObj->used->flags &= (uint16_t)~VRING_USED_F_NO_NOTIFY;
        }
    }
    else
    {
        if (vringObj->lastUsedIdx != vringObj->used->idx)
        {
            head = (uint16_t)(vringObj->used->ring[vringObj->lastUsedIdx % vringObj->vringNumBuf].id);
            vringObj->lastUsedIdx++;

            *vringBufId = head;

            status = SystemP_SUCCESS;
        }
    }

    HwiP_restore(oldIntState);

    return status;
}

void RPMessage_vringPutEmptyRxBuf(uint16_t remoteCoreId, uint16_t vringBufId)
{
    RPMessage_Core *coreObj = &gIpcRpmsgCtrl.coreObj[remoteCoreId];
    RPMessage_Vring *vringObj = &coreObj->vringRxObj;
    uintptr_t oldIntState;
    uint32_t rxMsgValue;

    oldIntState = HwiP_disable();

    if(RPMessage_isLinuxCore(remoteCoreId) != 0U)
    {
        struct vring_used_elem *used;

        used = &vringObj->used->ring[vringObj->used->idx % vringObj->vringNumBuf];
        used->id = (uint32_t)vringBufId;
        used->len = vringObj->desc[vringBufId].len;
        vringObj->used->idx++;

        rxMsgValue = RPMESSAGE_LINUX_RX_VRING_ID; /* in case of linux this should be RX VRING ID */
    }
    else
    {
        uint16_t avail;

        avail = (uint16_t)(vringObj->avail->idx % vringObj->vringNumBuf);
        vringObj->avail->ring[avail] = vringBufId;
        vringObj->avail->idx++;

        rxMsgValue = RPMESSAGE_MSG_VRING_NEW_EMPTY;
    }

    Utils_dataAndInstructionBarrier();

    HwiP_restore(oldIntState);

    IpcNotify_sendMsg(remoteCoreId,
        IPC_NOTIFY_CLIENT_ID_RPMSG,
        rxMsgValue,
        1 /* wait for message to be posted */
        );
}

uint32_t RPMessage_vringIsFullRxBuf(uint16_t remoteCoreId)
{
    RPMessage_Core *coreObj = &gIpcRpmsgCtrl.coreObj[remoteCoreId];
    RPMessage_Vring *vringObj = &coreObj->vringRxObj;
    uint32_t isNewFullBuf = 1;
    uintptr_t oldIntState;

    oldIntState = HwiP_disable();

    if(RPMessage_isLinuxCore(remoteCoreId) != 0U)
    {
        if (vringObj->lastAvailIdx == vringObj->avail->idx)
        {
            isNewFullBuf = 0;
        }
    }
    else
    {
        if (vringObj->lastUsedIdx == vringObj->used->idx)
        {
            isNewFullBuf = 0;
        }
    }

    HwiP_restore(oldIntState);

    return isNewFullBuf;
}

uint8_t *RPMessage_vringGetRxBufAddr(uint16_t remoteCoreId, uint16_t vringBufId)
{
    RPMessage_Core *coreObj = &gIpcRpmsgCtrl.coreObj[remoteCoreId];
    RPMessage_Vring *vringObj = &coreObj->vringRxObj;

    return (uint8_t *)vringObj->desc[vringBufId].addr;
}

uint32_t RPMessage_vringGetSize(uint32_t numBuf, uint16_t msgSize, uint32_t align)
{
    uint32_t msg_align;
    if (gIpcRpmsgCtrl.vringAllocationPDK == 0U)
    {
        msg_align = RPMessage_align(
                    ((uint32_t)sizeof(struct vring_desc) * numBuf) /* buffer descriptors for each buffer */
                    + ((uint32_t)sizeof(uint16_t) * (uint16_t)(2U + numBuf))    /* avail queue */
                    , align) + RPMessage_align(
                    ((uint32_t)sizeof(uint16_t) * 2U) + ((uint32_t)sizeof(struct vring_used_elem) * numBuf) /* used queue */
                    , align) + (numBuf * msgSize); /* message buffers */
    }
    else
    {
        msg_align =  ((((uint32_t)sizeof(struct vring_desc) * numBuf) + ((uint32_t)sizeof(uint16_t) * (2U + numBuf)))
                    + (((uintptr_t)0x1000 - 1U) & ~((uintptr_t)0x1000 - 1U))
                    + ((uint32_t)sizeof(uint16_t) * 2U) + ((uint32_t)sizeof(struct vring_used_elem) * numBuf));
    }

    return msg_align;
}

void RPMessage_vringResetInternal(RPMessage_Vring *vringObj, uint32_t numBuf, uint16_t msgSize,
    uintptr_t vringBaseAddr,
    uint32_t offset_desc,
    uint32_t offset_avail,
    uint32_t offset_used,
    uint32_t offset_buf,
    uint32_t isTx
)
{
    uint8_t *bufAddr;
    uint16_t bufId;

    /* intialize locally visible variables */
    vringObj->lastUsedIdx  = 0;
    vringObj->lastAvailIdx = 0;
    vringObj->vringNumBuf  = (uint16_t)numBuf;

    /* set address to vring descriptors, avail Q, used Q, message buffers */
    vringObj->desc        = (struct vring_desc  *)(vringBaseAddr + offset_desc);
    vringObj->avail       = (struct vring_avail *)(vringBaseAddr + offset_avail);
    vringObj->used        = (struct vring_used  *)(vringBaseAddr + offset_used);
    vringObj->bufBaseAddr = (uint8_t            *)(vringBaseAddr + offset_buf);

    /* only initialize TX vring, RX vring is initialized by the remote core */
    if(isTx != 0U)
    {
        /* initialize descriptors with message buffer address and max len */
        bufAddr = vringObj->bufBaseAddr;
        for(bufId=0; bufId<numBuf; bufId++)
        {
            vringObj->desc[bufId].addr    = (uint32_t)bufAddr;
            vringObj->desc[bufId].padding = 0;
            vringObj->desc[bufId].len     = (uint32_t)msgSize;
            vringObj->desc[bufId].flags   = 0;
            vringObj->desc[bufId].next    = 0;
            bufAddr = bufAddr + msgSize;
        }
        /* initialize avail Q and add all buffers to avail Q */
        vringObj->avail->idx = 0;
        vringObj->avail->flags = 0;
        for(bufId=0; bufId<numBuf; bufId++)
        {
            vringObj->avail->ring[bufId] = bufId;
            vringObj->avail->idx++;
        }
        /* initialize used Q */
        vringObj->used->idx = 0;
        vringObj->used->flags = 0;
        for(bufId=0; bufId<numBuf; bufId++)
        {
            vringObj->used->ring[bufId].id = 0;
            vringObj->used->ring[bufId].len = 0;
        }
    }
}

int32_t RPMessage_vringReset(uint16_t remoteCoreId, uint16_t isTx, const RPMessage_Params *params)
{
    RPMessage_Core *coreObj = &gIpcRpmsgCtrl.coreObj[remoteCoreId];
    RPMessage_Vring *vringObj;
    uintptr_t vringBaseAddr;
    uint32_t offset_desc, offset_avail, offset_used, offset_buf, vringSize;
    size_t align;
    uint32_t numBuf, msgSize;
    int32_t status = SystemP_SUCCESS;

    if(isTx != 0U)
    {
        vringObj = &coreObj->vringTxObj;
        vringBaseAddr = params->vringTxBaseAddr[remoteCoreId];
    }
    else
    {
        vringObj = &coreObj->vringRxObj;
        vringBaseAddr = params->vringRxBaseAddr[remoteCoreId];
    }
    align            = sizeof(uint32_t);
    numBuf           = params->vringNumBuf;
    msgSize          = params->vringMsgSize;

    /* get vring size, including descriptors, avail Q, used Q, message buffers and alignment */
    vringSize = RPMessage_vringGetSize(numBuf, (uint16_t)msgSize, align);

    /* check if vring ID is within limits of the memory available for vring */
    if( vringSize <= params->vringSize)
    {
        if (gIpcRpmsgCtrl.vringAllocationPDK == 0U)
        {
            /* calculate offset to vring descriptors, avail Q, used Q, message buffers
            * relative to vringBaseAddr
            */
            offset_desc  = 0;
            offset_avail = offset_desc  + (sizeof(struct vring_desc) * numBuf);
            offset_used  = offset_avail + RPMessage_align( (sizeof(uint16_t) * (uint16_t)(2U + numBuf)), align);
            offset_buf   = offset_used  + RPMessage_align( (sizeof(uint16_t) * 2U) + (sizeof(struct vring_used_elem) * numBuf), align);
        }
        else
        {
            /* calculate offset to vring descriptors, avail Q, used Q, message buffers
            * relative to vringBaseAddr
            */
            offset_desc  = 0;
            offset_avail = offset_desc  + (sizeof(struct vring_desc) * numBuf);
            offset_used  = RPMessage_align( offset_avail + (sizeof(uint16_t) * (uint16_t)(2U + numBuf)), 0x1000);
            offset_buf   = numBuf * msgSize;
        }
        RPMessage_vringResetInternal(vringObj,
        numBuf, (uint16_t)msgSize,
        vringBaseAddr,
        offset_desc, offset_avail, offset_used, offset_buf,
        isTx
        );
    }
    else
    {
        status = SystemP_FAILURE;
    }
    return status;
}

/* VRING reset for Linux+RTOS is different vs RTOS+RTOS.
 * This function has the logic to handle these differences.
 */
void RPMessage_vringResetLinux(uint16_t remoteCoreId, uint16_t isTx, const RPMessage_ResourceTable *rscTable)
{
    RPMessage_Core *coreObj = &gIpcRpmsgCtrl.coreObj[remoteCoreId];
    RPMessage_Vring *vringObj;
    uintptr_t vringBaseAddr;
    uint32_t offset_desc, offset_avail, offset_used, offset_buf;
    uint32_t align, numBuf;
    uint16_t msgSize;

    if(isTx != 0U)
    {
        vringObj = &coreObj->vringTxObj;
        vringBaseAddr = (uintptr_t)rscTable->vring0.da;
        align         = rscTable->vring0.align;
        numBuf        = rscTable->vring0.num;
    }
    else
    {
        vringObj = &coreObj->vringRxObj;
        vringBaseAddr = (uintptr_t)rscTable->vring1.da;
        align         = rscTable->vring1.align;
        numBuf        = rscTable->vring1.num;
    }

    msgSize          = RPMESSAGE_LINUX_MSG_SIZE;

    /* calculate offset to vring descriptors, avail Q, used Q, message buffers
     * relative to vringBaseAddr
     */
    offset_desc  = 0;
    offset_avail = offset_desc  + (sizeof(struct vring_desc) * numBuf);
    offset_used  = offset_avail + RPMessage_align( (sizeof(uint16_t) * (uint16_t)(2U + numBuf)), align);
    offset_buf   = offset_used  + RPMessage_align( (sizeof(uint16_t) * 2U) + (sizeof(struct vring_used_elem) * numBuf), align);
    /* buffer offset is aligned to numBuf*msgSize*2, eg, 512*256*2 = 256KB after offset_used */
    offset_buf   = RPMessage_align( offset_buf, (uint32_t)(numBuf * msgSize * 2U));

    if(isTx != 0U)
    {
        /* offset_buf points to TX buffers already */
    }
    else
    {
        /* we dont really use offset buf for RX VRING but showing the calculation here for completeness
         * RX buffers are initialized by Linux side
         */
        offset_buf += (numBuf*msgSize) - (rscTable->vring1.da - rscTable->vring0.da);
    }

    RPMessage_vringResetInternal(vringObj,
        numBuf, msgSize,
        vringBaseAddr,
        offset_desc, offset_avail, offset_used, offset_buf,
        isTx
        );
}
