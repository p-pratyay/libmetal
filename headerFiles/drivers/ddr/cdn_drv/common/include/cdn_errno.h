/******************************************************************************
 * Copyright (C) 2012-2022 Cadence Design Systems, Inc.
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 
 * 1. Redistributions of source code must retain the above copyright
 * notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 * notice, this list of conditions and the following disclaimer in the
 * documentation and/or other materials provided with the distribution.
 * 3. Neither the name of the copyright holder nor the names of its
 * contributors may be used to endorse or promote products derived from
 * this software without specific prior written permission.
 * 
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE
 * LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 *
 ******************************************************************************
 * cdn_errno.h
 * Cadence error codes
 ******************************************************************************
 */

#ifndef INCLUDE_CDN_ERRNO_H
#define INCLUDE_CDN_ERRNO_H

#ifdef __cplusplus
extern "C"
{
#endif
#ifndef ERRNO_H_NOT_PRESENT
#include <errno.h>
#endif //ERRNO_H_NOT_PRESENT

#ifndef REMOVE_OLD_ERRORCODES

#ifndef EOK
#define EOK             0      /* no error */
#endif //EOK

#ifndef EPERM
#define EPERM           1      /* Operation not permitted */
#endif //EPERM

#ifndef ENOENT
#define ENOENT          2      /* No such file or directory */
#endif //ENOENT

#ifndef EIO
#define EIO             5      /* I/O error */
#endif //EIO

#ifndef ENOEXEC
#define ENOEXEC         8      /* Exec format error */
#endif //ENOEXEC

#ifndef EAGAIN
#define EAGAIN          11      /* Try again */
#endif //EAGAIN

#ifndef ENOMEM
#define ENOMEM          12      /* Out of memory */
#endif //ENOMEM

#ifndef EFAULT
#define EFAULT          14      /* Bad address */
#endif //EFAULT

#ifndef EBUSY
#define EBUSY           16      /* Device or resource busy */
#endif //EBUSY

#ifndef EINVAL
#define EINVAL          22      /* Invalid argument */
#endif //EINVAL

#ifndef ENOSPC
#define ENOSPC          28      /* No space left on device */
#endif //ENOSPC

#ifndef EBADSLT
#define EBADSLT         57      /* Invalid slot */
#endif //EBADSLT

#ifndef EPROTO
#define EPROTO          71      /* Protocol error */
#endif //EPROTO

#ifndef EOVERFLOW
#define EOVERFLOW       75      /* Value too large for defined data type */
#endif //EOVERFLOW

#ifndef EOPNOTSUPP
#define EOPNOTSUPP      95      /* Operation not supported */
#endif //EOPNOTSUPP

#ifndef ETIMEDOUT
#define ETIMEDOUT       110     /* Connection timed out */
#endif //ETIMEDOUT

#ifndef EINPROGRESS
#define EINPROGRESS     115     /* Operation now in progress */
#endif //EINPROGRESS

#ifndef EDQUOT
#define EDQUOT          122     /* Quota exceeded */
#endif //EDQUOT

#ifndef ENOTSUP
#define ENOTSUP         EOPNOTSUPP
#endif //ENOTSUP

#ifndef ECANCELED
#define ECANCELED       126      /* Cancelled */
#endif //ECANCELED

#endif //REMOVE_OLD_ERRORCODES

#define CDN_EOK             0U      /* no error */

#define CDN_EPERM           1U      /* Operation not permitted */

#define CDN_ENOENT          2U      /* No such file or directory */

#define CDN_EIO             5U      /* I/O error */

#define CDN_ENOEXEC         8U      /* Exec format error */

#define CDN_EAGAIN          11U      /* Try again */

#define CDN_ENOMEM          12U      /* Out of memory */

#define CDN_EFAULT          14U      /* Bad address */

#define CDN_EBUSY           16U      /* Device or resource busy */

#define CDN_EINVAL          22U      /* Invalid argument */

#define CDN_ENOSPC          28U      /* No space left on device */

#define CDN_EBADSLT         57U      /* Invalid slot */

#define CDN_EPROTO          71U      /* Protocol error */

#define CDN_EOVERFLOW       75U      /* Value too large for defined data type */

#define CDN_EOPNOTSUPP      95U      /* Operation not supported */

#define CDN_ETIMEDOUT       110U     /* Connection timed out */

#define CDN_EINPROGRESS     115U     /* Operation now in progress */

#define CDN_EDQUOT          122U     /* Quota exceeded */

#define CDN_ENOTSUP         CDN_EOPNOTSUPP

#define CDN_ECANCELED       126U      /* Cancelled */

#ifdef __cplusplus
}
#endif

#endif //INCLUDE_CDN_ERRNO_H
