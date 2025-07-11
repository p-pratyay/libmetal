/*
 *  Copyright (C) 2019 Texas Instruments Incorporated
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
 *  \file soc/sciclient_soc_priv.h
 *
 *  \brief Private SOC file
 */

#ifndef SCICLIENT_SOC_PRIV_TOP_H_
#define SCICLIENT_SOC_PRIV_TOP_H_

/* ========================================================================== */
/*                             Include Files                                  */
/* ========================================================================== */

#if defined (SOC_AM64X) || defined (SOC_AM243X)
#include <drivers/sciclient/soc/am64x_am243x/sciclient_soc_priv.h>
#include <drivers/sciclient/soc/am64x_am243x/sciclient_irq_rm.h>
#endif

#if defined (SOC_AM62X)
#include <drivers/sciclient/soc/am62x/sciclient_soc_priv.h>
#include <drivers/sciclient/soc/am62x/sciclient_irq_rm.h>
#endif

#if defined (SOC_AM62AX)
#include <drivers/sciclient/soc/am62ax/sciclient_soc_priv.h>
#include <drivers/sciclient/soc/am62ax/sciclient_irq_rm.h>
#endif

#if defined (SOC_AM62DX)
#include <drivers/sciclient/soc/am62dx/sciclient_soc_priv.h>
#include <drivers/sciclient/soc/am62dx/sciclient_irq_rm.h>
#endif

#if defined (SOC_AM62PX)
#include <drivers/sciclient/soc/am62px/sciclient_soc_priv.h>
#include <drivers/sciclient/soc/am62px/sciclient_irq_rm.h>
#endif

#if defined (SOC_J722S)
#include <drivers/sciclient/soc/j722s/sciclient_soc_priv.h>
#include <drivers/sciclient/soc/j722s/sciclient_irq_rm.h>
#endif

#if defined (SOC_AM275X)
#include <drivers/sciclient/soc/am275x/sciclient_soc_priv.h>
#include <drivers/sciclient/soc/am275x/sciclient_irq_rm.h>
#endif

#if defined (SOC_AM62LX)
#include <drivers/sciclient/soc/am62lx/sciclient_soc_priv.h>
#include <drivers/sciclient/soc/am62lx/sciclient_irq_rm.h>
#endif

#ifdef __cplusplus
extern "C" {
#endif

/* ========================================================================== */
/*                           Macros & Typedefs                                */
/* ========================================================================== */

#define SCICLIENT_DEV_ID_MAX	0xFFFFU

/* ========================================================================== */
/*                         Structure Declarations                             */
/* ========================================================================== */

/* None */

/* ========================================================================== */
/*                          Function Declarations                             */
/* ========================================================================== */

/* None */

/* ========================================================================== */
/*                       Static Function Definitions                          */
/* ========================================================================== */

/* None */

uint32_t Sciclient_getContext(uint32_t contextType, uint32_t coreId);
uint32_t Sciclient_getDevId(uint32_t coreId);

#ifdef __cplusplus
}
#endif

#endif /* SCICLIENT_SOC_PRIV_TOP_H_ */
