/*
 *  Copyright 2018-2021 (C) Texas Instruments Incorporated
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
 *  \file udma_soc.h
 *
 *  \brief UDMA Low Level Driver SOC specific file.
 */

#ifndef UDMA_SOC_TOP_H_
#define UDMA_SOC_TOP_H_

/* ========================================================================== */
/*                             Include Files                                  */
/* ========================================================================== */

#if defined (SOC_AM64X) || defined (SOC_AM243X)
#include <drivers/udma/v0/soc/am64x_am243x/udma_soc.h>
#endif

#if defined (SOC_AM62X)
#include <drivers/udma/v0/soc/am62x/udma_soc.h>
#endif

#if defined (SOC_AM62AX)
#include <drivers/udma/v0/soc/am62ax/udma_soc.h>
#endif

#if defined (SOC_AM62DX)
#include <drivers/udma/v0/soc/am62dx/udma_soc.h>
#endif

#if defined (SOC_AM62PX)
#include <drivers/udma/v0/soc/am62px/udma_soc.h>
#endif

#if defined (SOC_J722S)
#include <drivers/udma/v0/soc/j722s/udma_soc.h>
#endif

#if defined (SOC_AM275X)
#include <drivers/udma/v0/soc/am275x/udma_soc.h>
#endif

#ifdef __cplusplus
extern "C" {
#endif

/* ========================================================================== */
/*                           Macros & Typedefs                                */
/* ========================================================================== */

/* None */

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

#ifdef __cplusplus
}
#endif

#endif /* #ifndef UDMA_SOC_TOP_H_ */
