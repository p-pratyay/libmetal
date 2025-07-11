/*
 *  Copyright (C) 2024-25 Texas Instruments Incorporated
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

#ifndef CACHEP_ARMV7R_H
#define CACHEP_ARMV7R_H

#ifdef __cplusplus
extern "C"
{
#endif


/**
 *  \brief Get number of cache sets
 *
 *  \param None
 *
 *  \return number of cache sets
 */
uint32_t CacheP_GetNumSets(void);

/**
 *  \brief Get number of cache ways
 *
 *  \param None
 *
 *  \return number of cache ways
 */
uint32_t CacheP_GetNumWays(void);

/**
 *  \brief Invalidate Data cache with given sets and ways
 *
 *  \param enable value 1 to enable
 *
 *  \return None
 */
void CacheP_CleanInvalidateDcacheSetWay( uint32_t set, uint32_t way );

/**
 *  \brief Invalidate all entries of ICache
 *
 *  \param None
 *
 *  \return None
 */
void CacheP_invL1pAll(void);

/**
 *  \brief Invalidate all entries of DCache
 *
 *  \param None
 *
 *  \return None
 */
void CacheP_invL1dAll(void);

/**
 *  \brief Get the instruction cache line size
 *
 *  This function is used to get the instruction cache line size for MCU.
 *  Implementation of this API/code is use-case specific.
 *
 *  \param None
 *
 *  \return the instruction cache line size in bytes
 */
uint32_t CacheP_GetIcacheLineSize( void );

/**
 *  \brief Invalidate an instruction cache line by MVA
 *  This function is used to invalidate an instruction cache Line by MVA.
 *
 *  \param address  [IN]    Modified virtual address
 *
 *  \return None
 *
 */
void CacheP_InvalidateIcacheMva( uint32_t address );

/**
 *  \brief Invalidate a data cache line by set and way
 *
 *  This function is used to invalidate a data cache line by set and way.
 *
 *  \param set      [IN]    Indicates the cache set to invalidate
 *  \param way      [IN]    Indicates the cache way to invalidate
 *
 *  \return None
 */
void CacheP_InvalidateDcacheSetWay( uint32_t set, uint32_t way );

/**
 *  \brief Clean a data cache line by set and way
 *
 *  This function is used to clean a data cache line by set and way.
 *
 *  \param set      [IN]    Indicates the cache set to clean
 *  \param way      [IN]    Indicates the cache way to clean
 *
 *  \return None
 *
 */
void CacheP_CleanDcacheSetWay( uint32_t set, uint32_t way );

/**
 *  \brief Disable ECC (parity) checking on cache rams
 *
 *  This function is used to disable ECC (parity) checking on cache rams.
 *
 *  \param None
 *
 *  \return None
 */
void CacheP_DisableEcc( void );

/**
 *  \brief Enable AXI slave access to cache RAM
 *
 *  This function is used to enable AXI slave access to cache RAM.
 *
 *  \param None
 *
 *  \return None
 */
void CacheP_EnableAxiAccess( void );

#ifdef __cplusplus
}
#endif

#endif /* CACHEP_ARMV7R_H */
