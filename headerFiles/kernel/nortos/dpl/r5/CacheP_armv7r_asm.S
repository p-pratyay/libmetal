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

        .text

/* FUNCTION DEF: void CacheP_disableL1d(void) */
        .global CacheP_disableL1d
        .type CacheP_disableL1d,%function
        .section ".text.cache","ax",%progbits
        .arm
        .align 2
CacheP_disableL1d:
        push    {r0-r7, r9-r11, lr}
        mrc     p15, #0, r0, c1, c0, #0 // read SCR register
        bic     r0, r0, #0x0004         // clear C bit
        dsb
        mcr     p15, #0, r0, c1, c0, #0 // L1D cache disabled
                                        // clean entire L1D cache
        movw    r0, :lower16:CacheP_wbInvAllAsm
        movt    r0, :upper16:CacheP_wbInvAllAsm
        blx     r0
        pop     {r0-r7, r9-r11, lr}
        bx      lr

/* FUNCTION DEF: void CacheP_disableL1p(void) */
        .global CacheP_disableL1p
        .type CacheP_disableL1p,%function
        .section ".text.cache","ax",%progbits
        .arm
        .align 2
CacheP_disableL1p:
        mrc     p15, #0, r0, c1, c0, #0 // read SCR register
        bic     r0, r0, #0x1000         // clear I bit
        mcr     p15, #0, r0, c1, c0, #0 // L1P cache disabled
        mcr     p15, #0, r1, c7, c5, #0 // Invalidate entire instruction cache
        isb
        bx      lr

/* FUNCTION DEF: void CacheP_enableL1d(void) */
        .global CacheP_enableL1d
        .type CacheP_enableL1d,%function
        .section ".text.cache","ax",%progbits
        .arm
        .align 2
CacheP_enableL1d:
        mrc     p15, #0, r0, c1, c0, #0  // read SCR register
        orr     r0, r0, #0x0004          // set C bit (bit 2) to 1
        dsb
        mcr     p15, #0, r1, c15, c5, #0 // Invalidate entire data cache
        mcr     p15, #0, r0, c1, c0, #0  // L1D cache enabled
        bx      lr

/* FUNCTION DEF: void CacheP_enableL1p(void) */
        .global CacheP_enableL1p
        .type CacheP_enableL1p,%function
        .section ".text.cache","ax",%progbits
        .arm
        .align 2
CacheP_enableL1p:
        mrc     p15, #0, r0, c1, c0, #0 // read SCR register
        orr     r0, r0, #0x1000         // set I bit (bit 12) to 1
        mcr     p15, #0, r1, c7, c5, #0 // Invalidate entire instruction cache
        mcr     p15, #0, r0, c1, c0, #0 // ICache enabled
        isb
        bx      lr

/* FUNCTION DEF: void CacheP_invL1d(uint32_t blockPtr, uint32_t byteCnt)
 *
 *       r0 - contains blockPtr
 *       r1 - contains byteCnt
 */
        .global CacheP_invL1d
        .type CacheP_invL1d,%function
        .section ".text.cache","ax",%progbits
        .arm
        .align 2
CacheP_invL1d:
        push    {r4}
        add     r1, r0, r1              // calculate last address
        ldr     r3, l1dCacheLineSizeInvL1dAddr
        ldr     r3, [r3]
        sub     r4, r3, #1
        bic     r0, r0, r4              // align blockPtr to cache line
invL1dCache_loop:
        mcr     p15, #0, r0, c7, c6, #1 // invalidate single entry in L1D cache
        add     r0, r0, r3              // increment address by cache line size
        cmp     r0, r1                  // compare to last address
        blo     invL1dCache_loop        // loop if > 0
        dsb                             // drain write buffer
        pop     {r4}
        bx      lr                      // return

l1dCacheLineSizeInvL1dAddr: .long  gCacheL1dCacheLineSize


/* FUNCTION DEF: void CacheP_invL1p(uint32_t blockPtr, uint32_t byteCnt)
 *
 *       r0 - contains blockPtr
 *       r1 - contains byteCnt
 */
        .global CacheP_invL1p
        .type CacheP_invL1p,%function
        .section ".text.cache","ax",%progbits
        .arm
        .align 2
CacheP_invL1p:
        push    {r4}
        add     r1, r0, r1              // calculate last address
        ldr     r3, l1pCacheLineSizeAddr
        ldr     r3, [r3]
        sub     r4, r3, #1
        bic     r0, r0, r4              // align blockPtr to cache line
invL1pCache_loop:
        mcr     p15, #0, r0, c7, c5, #1 // invalidate single entry in ICache
        add     r0, r0, r3              // increment address by cache line size
        cmp     r0, r1                  // compare to last address
        blo     invL1pCache_loop        // loop if > 0
        dsb                             // drain write buffer
        isb                             // flush instruction pipeline
        pop     {r4}
        bx      lr

l1pCacheLineSizeAddr:  .word   gCacheL1pCacheLineSize

/* FUNCTION DEF: void CacheP_invL1dAll()
 *
 * Invalidates all in data cache. Note: This is risky since data cache may
 * contain some stack variable or valid data that should not be invalidated.
 * Only use this function if you know for sure the data cache contains unwanted
 * information.
 */
        .global CacheP_invL1dAll
        .type CacheP_invL1dAll,%function
        .section ".text.cache","ax",%progbits
        .arm
        .align 2
CacheP_invL1dAll:
        mcr     p15, #0, r0, c15, c5, #0 // Invalidate entire data cache
        bx      lr                       // return


/* FUNCTION DEF: void CacheP_invL1pAll() */
        .global CacheP_invL1pAll
        .type CacheP_invL1pAll,%function
        .section ".text.cache","ax",%progbits
        .arm
        .align 2
CacheP_invL1pAll:
        mcr     p15, #0, r0, c7, c5, #0 // invalidate all entries in ICache
        bx      lr                      // return


/* FUNCTION DEF: void CacheP_wb(void *addr, uint32_t size, uint32_t type)
 * Writes back the range of MVA in data cache. First, wait on any previous cache
 * operation.
 *
 *       r0 - contains blockPtr
 *       r1 - contains byteCnt
 *       r2 - contains bit mask of cache type (unused)
 */
        .global CacheP_wb
        .type CacheP_wb,%function
        .section ".text.cache","ax",%progbits
        .arm
        .align 2
CacheP_wb:
        push    {r4, r5}
        dmb                              // Ensure all previous memory accesses
                                         // complete
        add     r1, r0, r1               // calculate last address
        ldr     r4, l1dCacheLineSizeWbAddr
        ldr     r4, [r4]
        sub     r5, r4, #1
        bic     r0, r0, r5               // align address to cache line
writeback:
        mcr     p15, #0, r0, c7, c10, #1 // write back a cache line
        add     r0, r0, r4               // increment address by cache line size
        cmp     r0, r1                   // compare to last address
        blo     writeback                // loop if count > 0
        dsb                              // drain write buffer
        pop     {r4, r5}
        bx      lr

l1dCacheLineSizeWbAddr: .long gCacheL1dCacheLineSize


/* FUNCTION DEF: void CacheP_wbInv(void *addr, uint32_t size, uint32_t type)
 *
 * Writes back and invalidates the range of MVA in data cache.
 * First, wait on any previous cache operation.
 *
 *       r0 - contains blockPtr
 *       r1 - contains byteCnt
 *       r2 - contains bitmask of cache type (unused)
 */
        .global CacheP_wbInv
        .type CacheP_wbInv,%function
        .section ".text.cache","ax",%progbits
        .arm
        .align 2
CacheP_wbInv:
        push    {r4, r5}
        dmb                              // Ensure all previous memory accesses
                                         // complete
        add     r1, r0, r1               // calculate last address
        ldr     r4, l1dCacheLineSizeWbInvAddr
        ldr     r4, [r4]
        sub     r5, r4, #1
        bic     r0, r0, r5               // align blockPtr to cache line
writebackInv:
        mcr     p15, #0, r0, c7, c14, #1 // writeback inv a cache line
        add     r0, r0, r4               // increment address by cache line size
        cmp     r0, r1                   // compare to last address
        blo     writebackInv             // loop if count > 0
        dsb                              // drain write buffer
        pop     {r4, r5}
        bx      lr

l1dCacheLineSizeWbInvAddr: .long gCacheL1dCacheLineSize


/* FUNCTION DEF: void CacheP_wbAll()
 *
 * Write back all of L1 data cache
 */
        .global CacheP_wbAll
        .type CacheP_wbAll,%function
        .section ".text.cache","ax",%progbits
        .arm
        .align 2
CacheP_wbAll:
        stmfd   sp!, {r0-r7, r9-r11, lr}
        dmb                             // Ensure all previous memory accesses
                                        // complete
        mrc     p15, #1, r0, c0, c0, #1 // read clidr
        ands    r3, r0, #0x7000000      // extract loc from clidr
        mov     r3, r3, lsr #23         // left align loc bit field
        beq     wbafinished             // if loc is 0, then no need to clean

        mov     r10, #0                 // start clean at cache level 0

wbaloop1:
        add     r2, r10, r10, lsr #1    // work out 3x current cache level
        mov     r1, r0, lsr r2          // extract cache type bits from clidr
        and     r1, r1, #7              // mask of bits for current cache only
        cmp     r1, #2                  // see what cache we have at this level
        blt     wbaskip                 // skip if no cache, or just i-cache

        mrs     r6, cpsr
        cpsid   i                       // disable interrupts
        mcr     p15, #2, r10, c0, c0, #0// select current cache level in cssr
        isb                             // flush prefetch buffer
        mrc     p15, #1, r1, c0, c0, #0 // read the new csidr
        msr     cpsr_c, r6              // restore interrupts

        and     r2, r1, #7              // extract the length of the cache lines
        add     r2, r2, #4              // add 4 (line length offset)
        mov     r4, #0x3ff
        ands    r4, r4, r1, lsr #3      // find maximum number on the way size
        clz     r5, r4                  // find bit position of way size inc.
        mov     r7, #0x7fff
        ands    r7, r7, r1, lsr #13     // extract max number of the index size
wbaloop2:
        mov     r9, r4                  // create working copy of max way size
wbaloop3:
        orr     r11, r10, r9, lsl r5    // factor way and cache number into r11
        orr     r11, r11, r7, lsl r2    // factor index number into r11
        mcr     p15, #0, r11, c7, c10, #2 // clean line by set/way
        subs    r9, r9, #1              // decrement the way
        bge     wbaloop3
        subs    r7, r7, #1              // decrement the index
        bge     wbaloop2
wbaskip:
        add     r10, r10, #2            // increment cache number
        cmp     r3, r10
        bgt     wbaloop1

wbafinished:
        mov     r10, #0                 // switch back to cache level 0
        mcr     p15, #2, r10, c0, c0, #0// select current cache level in cssr
        dsb
        isb                             // flush prefetch buffer
        ldmfd   sp!, {r0-r7, r9-r11, lr}
        bx      lr



/* FUNCTION DEF: void CacheP_wbInvAll()
 *
 * Write back and invalidate entire data cache
 */
        .global CacheP_wbInvAll
        .type CacheP_wbInvAll,%function
        .section ".text.cache","ax",%progbits
        .arm
        .align 2
CacheP_wbInvAll:
        push    {r0-r7, r9-r11, lr}
        movw    r0, :lower16:CacheP_wbInvAllAsm
        movt    r0, :upper16:CacheP_wbInvAllAsm
        blx     r0
        pop     {r0-r7, r9-r11, lr}
        bx      lr

/* FUNCTION DEF: void CacheP_wbInvAllAsm()
 *
 * Write back and invalidate entire data cache
 */
        .global CacheP_wbInvAllAsm
        .type CacheP_wbInvAllAsm,%function
        .section ".text.cache","ax",%progbits
        .arm
        .align 2
CacheP_wbInvAllAsm:
        dmb                             // Ensure all previous memory accesses
                                        // complete
        mrc     p15, #1, r0, c0, c0, #1 // read clidr
        ands    r3, r0, #0x7000000      // extract loc from clidr
        mov     r3, r3, lsr #23         // left align loc bit field
        beq     finished                // if loc is 0, then no need to clean

        mov     r10, #0                 // start clean at cache level 0

loop1:
        add     r2, r10, r10, lsr #1    // work out 3x current cache level
        mov     r1, r0, lsr r2          // extract cache type bits from clidr
        and     r1, r1, #7              // mask of bits for current cache only
        cmp     r1, #2                  // see what cache we have at this level
        blt     skip                    // skip if no cache, or just i-cache

        mrs     r6, cpsr
        cpsid   i                       // disable interrupts
        mcr     p15, #2, r10, c0, c0, #0// select current cache level in cssr
        isb                             // flush prefetch buffer
        mrc     p15, #1, r1, c0, c0, #0 // read the new csidr
        msr     cpsr_c, r6              // restore interrupts

        and     r2, r1, #7              // extract the length of the cache lines
        add     r2, r2, #4              // add 4 (line length offset)
        mov     r4, #0x3ff
        ands    r4, r4, r1, lsr #3      // find maximum number on the way size
        clz     r5, r4                  // find bit position of way size inc.
        mov     r7, #0x7fff
        ands    r7, r7, r1, lsr #13     // extract max number of the index size
loop2:
        mov     r9, r4                  // create working copy of max way size
loop3:
        orr     r11, r10, r9, lsl r5    // factor way and cache number into r11
        orr     r11, r11, r7, lsl r2    // factor index number into r11
        mcr     p15, #0, r11, c7, c14, #2 // clean & invalidate by set/way
        subs    r9, r9, #1              // decrement the way
        bge     loop3
        subs    r7, r7, #1              // decrement the index
        bge     loop2
skip:
        add     r10, r10, #2            // increment cache number
        cmp     r3, r10
        bgt     loop1
finished:
        mov     r10, #0                 // swith back to cache level 0
        mcr     p15, #2, r10, c0, c0, #0// select current cache level in cssr
        dsb
        isb                             // flush prefetch buffer
        bx      lr


/* FUNCTION DEF: uint32_t CacheP_getEnabled()
 *
 * Determine the mask of enabled caches
 */
        .global CacheP_getEnabled
        .type CacheP_getEnabled,%function
        .section ".text.cache","ax",%progbits
        .arm
        .align 2
CacheP_getEnabled:
        mov     r0, #0
                                        // Do L1 first
        mrc     p15, #0, r1, c1, c0, #0 // fetch Control Register into r1

        tst     r1, #0x1000             // test I bit (bit 12) for L1P
        addne   r0, r0, #1              // if I is true, L1P is enabled

        tst     r1, #0x0004             // test C bit (bit 2) for L1D
        addne   r0, r0, #2              // if C bit is true, L1D is enabled

                                        // Do L2 next
        mrc     p15, #0, r1, c1, c0, #1 // fetch Auxiliary Ctrl Register into r1

        tst     r1, #0x0002             // test L2EN bit (bit 1) for L2EN
        beq     getEnabledDone

        tst     r0, #0x0001
        addne   r0, r0, #4              // If L2EN and L1P then L2P

        tst     r0, #0x0002
        addne   r0, r0, #8              // If L2EN and L1D then L2D

getEnabledDone:
        bx      lr


/* FUNCTION DEF: uint32_t CacheP_getCacheLevelInfo(uint32_t level) */
        .global CacheP_getCacheLevelInfo
        .type CacheP_getCacheLevelInfo,%function
        .section ".text.cache","ax",%progbits
        .arm
        .align 2
CacheP_getCacheLevelInfo:
        mcr     p15, #2, r0, c0, c0, #0 // write to Cache Size Selection Reg
        mrc     p15, #1, r0, c0, c0, #0 // read Cache Size Id Reg
        bx      lr


/* FUNCTION DEF: void CacheP_configForceWrThru(uint32_t enable) */
        .global CacheP_configForceWrThru
        .type CacheP_configForceWrThru,%function
        .section ".text.cache","ax",%progbits
        .arm
        .align 2
CacheP_configForceWrThru:
        mrc     p15, #0, r1, c1, c0, #1 // fetch Auxiliary Ctrl Register into r1
        cmp     r0, #0
        beq     FWT_disable
        orr     r1, r1, #(1 << 9)       // set (enable) force write-thru bit
        b       FWT_exit

FWT_disable:
        bic     r1, r1, #(1 << 9)       // clear (disable) force write-thru bit

FWT_exit:
        mcr     p15, #0, r1, c1, c0, #1 // write Auxiliary Ctrl Register

        bx      lr


/* FUNCTION DEF: void CacheP_setDLFO() */
        .global CacheP_setDLFO
        .type CacheP_setDLFO,%function
        .section ".text.cache","ax",%progbits
        .arm
        .align 2
CacheP_setDLFO:
        mrc     p15, #0, r1, c1, c0, #1 // fetch Auxiliary Ctrl Register into r1
        orr     r1, r1, #(1 << 13)      // set DLFO to disable LF optimization
        mcr     p15, #0, r1, c1, c0, #1 // write Auxiliary Ctrl Register

        bx      lr

/* FUNCTION DEF: uint32_t CacheP_GetNumSets(void ) */
        .global CacheP_GetNumSets
        .type CacheP_GetNumSets,%function
        .section ".text.cache","ax",%progbits
        .arm
        .align 2
CacheP_GetNumSets:
    dmb
    mrc       p15, #1, r0, c0, c0, #0   // Read Cache Size ID Register
    lsl       r0, r0, #(31-27)
    lsr       r0, r0, #(31-27+13)       // Extract bits from [27:13]
    add       r0, r0, #1
    bx        lr

/* FUNCTION DEF: uint32_t CacheP_GetNumWays(void ) */
        .global CacheP_GetNumWays
        .type CacheP_GetNumWays,%function
        .section ".text.cache","ax",%progbits
        .arm
        .align 2
CacheP_GetNumWays:
    dmb
    mrc       p15, #1, r0, c0, c0, #0   // Read Cache Size ID Register
    lsl       r0, r0, #(31-12)
    lsr       r0, r0, #(31-12+3)        // Extract bits from [12:3]
    add       r0, r0, #1
    bx        lr

/* FUNCTION DEF: void CacheP_CleanInvalidateDcacheSetWay( uint32_t set, uint32_t way ) */
        .global CacheP_CleanInvalidateDcacheSetWay
        .type CacheP_CleanInvalidateDcacheSetWay,%function
        .section ".text.cache","ax",%progbits
        .arm
        .align 2
CacheP_CleanInvalidateDcacheSetWay:
    dmb     // Ensure all previous memory accesses are completed
    and     r1, r1, #0x3
    lsl     r0, #(32-9)
    lsr     r0, #(32-9)                     // set &= 0x1FF
    lsl     r1, r1, #30
    orr     r0, r1, r0, LSL #5
    mcr     p15, #0, r0, c7, c14, #2
    dsb
    bx      lr

/* FUNCTION DEF: uint32_t CacheP_GetIcacheLineSize(void) */
        .global CacheP_GetIcacheLineSize
        .type CacheP_CleanInvalidateDcacheSetWay,%function
        .section ".text","ax",%progbits
        .arm
        .align 2
CacheP_GetIcacheLineSize:
    dmb
    mrc       p15, #0, r0, c0, c0, #1   // Read Cache Type Register
    ubfx      r0, r0, #0, #4            //Extract the iMinLine
    add       r0, r0, #2                // Convert words to bytes
    mov       r1, #1
    lsl       r0, r1, r0                // Calculate the line size
    bx        lr

/* FUNCTION DEF: void CacheP_InvalidateIcacheMva( uint32_t address ) */
        .global CacheP_InvalidateIcacheMva
        .type CacheP_CleanInvalidateDcacheSetWay,%function
        .section ".text","ax",%progbits
        .arm
        .align 2
CacheP_InvalidateIcacheMva:
    dmb     // Ensure all previous memory accesses are completed
    mcr     p15, #0, r0, c7, c5, #1
    dsb
    bx      lr

/* FUNCTION DEF: void CacheP_InvalidateDcacheSetWay( uint32_t set, uint32_t way ) */
        .global CacheP_InvalidateDcacheSetWay
        .type CacheP_CleanInvalidateDcacheSetWay,%function
        .section ".text","ax",%progbits
        .arm
        .align 2
CacheP_InvalidateDcacheSetWay:
    dmb     // Ensure all previous memory accesses are completed
    and     r1, r1, #0x3
    lsl     r0, #(32-9)
    lsr     r0, #(32-9)                     // set &= 0x1FF
    lsl     r1, r1, #30
    orr     r0, r1, r0, LSL #5
    mcr     p15, #0, r0, c7, c6, #2
    dsb
    bx      lr

/* FUNCTION DEF: void CacheP_CleanDcacheSetWay( uint32_t set, uint32_t way ) */
        .global CacheP_CleanDcacheSetWay
        .type CacheP_CleanInvalidateDcacheSetWay,%function
        .section ".text","ax",%progbits
        .arm
        .align 2
CacheP_CleanDcacheSetWay:
    dmb     // Ensure all previous memory accesses are completed
    and     r1, r1, #0x3
    lsl     r0, #(32-9)
    lsr     r0, #(32-9)                     // set &= 0x1FF
    lsl     r1, r1, #30
    orr     r0, r1, r0, LSL #5
    mcr     p15, #0, r0, c7, c10, #2
    dsb
    bx      lr

/* FUNCTION DEF: void CacheP_DisableEcc( void ) */
        .global CacheP_DisableEcc
        .type CacheP_CleanInvalidateDcacheSetWay,%function
        .section ".text","ax",%progbits
        .arm
        .align 2
CacheP_DisableEcc:
    push    {lr}
    mov     r0, #0
    bl      CacheP_DisableAllCache    // Disable instruction and data caches

    mrc     p15, #0, r0, c1, c0, #1         // Read Auxiliary Control register
    bic     r0, r0, #(0x7<<3)               // Clear CEC field
    orr     r0, r0, #(1<<5)                 // Set CEC field to 3'b100 (disable parity checking)
    orr     r0, r0, #(1<<9)                 // Enable Write Through Cache RAMS
    mcr     p15, #0, r0, c1, c0, #1         // Write Auxiliary Control register

    bl      CacheP_InvalidateAllCache
    mov     r0, #1
    bl      CacheP_EnableAllCache    // Enable instruction and data caches
    pop     {lr}
    bx      lr

/* FUNCTION DEF: void CacheP_InvalidateAllCache( void ) */
        .global CacheP_InvalidateAllCache
        .type CacheP_CleanInvalidateDcacheSetWay,%function
        .section ".text","ax",%progbits
        .arm
        .align 2
CacheP_InvalidateAllCache:
    push    {lr}
    bl      CacheP_invL1pAll
    bl      CacheP_invL1dAll
    pop     {lr}
    bx      lr

/* FUNCTION DEF: void CacheP_EnableAllCache( uint32_t enable ) */
        .global CacheP_EnableAllCache
        .type CacheP_CleanInvalidateDcacheSetWay,%function
        .section ".text","ax",%progbits
        .arm
        .align 2
CacheP_EnableAllCache:
    push    {lr}
    bl      CacheP_enableL1p
    bl      CacheP_enableL1d
    pop     {lr}
    bx      lr

/* FUNCTION DEF: void CacheP_DisableAllCache( uint32_t enable ) */
        .global CacheP_DisableAllCache
        .type CacheP_CleanInvalidateDcacheSetWay,%function
        .section ".text","ax",%progbits
        .arm
        .align 2
CacheP_DisableAllCache:
    push    {lr}
    bl      CacheP_disableL1p
    bl      CacheP_disableL1d
    pop     {lr}
    bx      lr

/* FUNCTION DEF: void CacheP_EnableAxiAccess( void ) */
        .global CacheP_EnableAxiAccess
        .type CacheP_CleanInvalidateDcacheSetWay,%function
        .section ".text","ax",%progbits
        .arm
        .align 2
CacheP_EnableAxiAccess:
    mrc     p15, #0, r0, c1, c0, #1         // Read Auxiliary Control register
    orr     r0, r0, #(1<<24)                // Set AXISCEN bit (enable AXI slave cache RAM access)
    mcr     p15, #0, r0, c1, c0, #1         // Write Auxiliary Control register
    bx      lr

    .end
