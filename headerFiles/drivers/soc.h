/*
 *  Copyright (C) 2021-2022 Texas Instruments Incorporated
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

#ifndef SOC_TOP_H_
#define SOC_TOP_H_

#include <stdint.h>

#ifdef __cplusplus
extern "C"
{
#endif

#if defined (SOC_AM64X) || defined (SOC_AM243X)
#include <drivers/soc/am64x_am243x/soc.h>
#endif

#if defined (SOC_AM263X)
#include <drivers/soc/am263x/soc.h>
#include <drivers/soc/am263x/soc_rcm.h>
#endif

#if defined (SOC_AM273X)
#include <drivers/soc/am273x/soc.h>
#endif

#if defined (SOC_AWR294X)
#include <drivers/soc/awr294x/soc.h>
#endif

#if defined (SOC_AM62X)
#include <drivers/soc/am62x/soc.h>
#endif

#if defined (SOC_AM62AX)
#include <drivers/soc/am62ax/soc.h>
#endif

#if defined (SOC_AM62DX)
#include <drivers/soc/am62dx/soc.h>
#endif

#if defined (SOC_AM62PX)
#include <drivers/soc/am62px/soc.h>
#endif

#if defined (SOC_AM275X)
#include <drivers/soc/am275x/soc.h>
#endif

#if defined (SOC_J722S)
#include <drivers/soc/j722s/soc.h>
#endif

#if defined (SOC_AM62LX)
#include <drivers/soc/am62lx/soc.h>
#endif

#ifdef __cplusplus
}
#endif

#endif
