/*
 *  Copyright (C)2018-2024 Texas Instruments Incorporated
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
 *  \defgroup DRV_I2C_HLD_MODULE APIs for I2C
 *  \ingroup DRV_MODULE
 *
 *  This file containing the I2C API.
 *
 *  The I2C header file should be included in an application as follows:
 *  \code
 *  #include <drivers/i2c.h>
 *  \endcode
 *
 *  # Operation #
 *  The I2C driver operates as a controller or a target on a single-controller I2C bus,
 *  in either I2C_MODE_BLOCKING or I2C_MODE_CALLBACK.
 *  In blocking mode, the task's execution is blocked during the I2C
 *  transaction. When the transfer has completed, code execution will resume.
 *  In callback mode, the task's execution is not blocked, allowing for other
 *  transactions to be queued up or to process some other code. When the
 *  transfer has completed, the I2C driver will call a user-specified callback
 *  function (from a HWI context).
 *
 *  The APIs in this driver serve as an interface to a typical TI-RTOS
 *  application. The specific peripheral implementations are responsible to
 *  create all the SYS/BIOS specific primitives to allow for thread-safe
 *  operation.
 *
 *  ## Opening the driver #
 *
 *  \code
 *  I2C_Handle      handle;
 *  I2C_Params      params;
 *
 *  I2C_Params_init(&params);
 *  params.transferMode  = I2C_MODE_BLOCKING;
 *  I2C_init();
 *  handle = I2C_open(someI2C_configIndexValue, &params);
 *  if (!handle) {
 *      System_printf("I2C did not open");
 *  }
 *  \endcode
 *
 *  ## Transferring data #
 *  A I2C transaction with a I2C peripheral is started by calling
 *  I2C_transfer(). The details of the I2C transaction is specified with a
 *  I2C_Transaction data structure. This structure allows for any of the three
 *  types of transactions: Write, Read, or Write/Read. Each transfer is
 *  performed atomically with the I2C controller or target peripheral.
 *
 *  \code
 *  I2C_Transaction i2cTransaction;
 *  I2C_Transaction_init(&i2cTransaction);
 *
 *  i2cTransaction.writeBuf = someWriteBuffer;
 *  i2cTransaction.writeCount = numOfBytesToWrite;
 *
 *  i2cTransaction.readBuf = someReadBuffer;
 *  i2cTransaction.readCount = numOfBytesToRead;
 *
 *  i2cTransaction.targetAddress = some7BitI2CTargetAddress;
 *
 *  ret = I2C_transfer(handle, &i2cTransaction);
 *  if (!ret) {
 *      System_printf("Unsuccessful I2C transfer");
 *  }
 *  \endcode
 *
 *  @{
 */

#ifndef I2C_H_
#define I2C_H_

/* ========================================================================== */
/*                             Include Files                                  */
/* ========================================================================== */

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>
#include <kernel/dpl/SemaphoreP.h>
#include <kernel/dpl/HwiP.h>
#include <kernel/dpl/ClockP.h>
#include <drivers/i2c/v0/lld/i2c_lld.h>
/* ========================================================================== */
/*                           Macros & Typedefs                                */
/* ========================================================================== */

#define I2C_MAX_NUM_OWN_TARGET_ADDR     (4U)

/**
 *  \anchor I2C_TransferMode
 *  \name MACROS used to select the transfer mode
 *
 *  I2C_MODE_BLOCKING block task execution while a I2C transfer is in progress
 *  I2C_MODE_CALLBACK does not block task execution; but calls a callback
 *  function when the I2C transfer has completed
 *
 *  @{
 */
#define I2C_MODE_BLOCKING               ((uint8_t) 0U)
#define I2C_MODE_CALLBACK               ((uint8_t) 1U)

/** @} */

/**
 * \anchor   I2C_memTransactionDir
 * \name MACROS for the possible values of I2C_Mem_Transaction member
 * memDataDir.
 * Deafults to I2C_TXN_DIR_INVALID
 * @{
 */

/** \brief I2C LLD invalid dataDir */
#define I2C_MEM_TXN_DIR_INVALID             ((uint8_t) 0U)
/** \brief I2C LLD Write */
#define I2C_MEM_TXN_DIR_TX                  ((uint8_t) 1U)
/** \brief I2C LLD Read */
#define I2C_MEM_TXN_DIR_RX                  ((uint8_t) 2U)

/** @} */

/* ========================================================================== */
/*                         Structure Declarations                             */
/* ========================================================================== */

/**
 *  \brief A handle that is returned from a I2C_open() call.
 */
typedef struct I2C_Config_s      *I2C_Handle;

/**
 *  \brief  I2C Hardware attributes
 */
typedef struct I2C_HwAttrs_s {
/** I2C Peripheral base address */
    uint32_t            baseAddr;
/** I2C Peripheral interrupt vector */
    uint32_t            intNum;
/** I2C Peripheral event id*/
    uint32_t            eventId;
/** I2C input functional clock */
    uint32_t            funcClk;
/** enable Interrupt */
    bool                enableIntr;
/** Interrupt Priority */
    uint8_t             intrPriority;
/** I2C own target addresses for multi-target channels,
 *  if only one target channel is supported, set the
 *  target address to ownTargetAddr[0]
 */
    uint32_t            ownTargetAddr[I2C_MAX_NUM_OWN_TARGET_ADDR];
} I2C_HwAttrs;

/**
 *  \brief  I2C Memory Transaction
 *
 *  This structure stores parameters related to
 *  I2C memory transaction.
 *
 */
typedef struct {

    uint32_t            memAddr;
    /**< [IN] Memory address to write to or read from */
    uint8_t             memAddrSize;
    /**< [IN] Memory address size \ref I2CLLD_MemoryAddrSize */
    uint8_t             *buffer;
    /**< [IN] Pointer to Read or Write buffer */
    uint32_t            size;
    /**< [IN] Size of Read or Write buffer */
    uint8_t             memDataDir;
    /**< [IN] Target internal memory data direction \ref I2C_memTransactionDir*/
} I2C_Mem_Transaction;

/**
 *  \brief  I2C transaction
 *
 *  This structure defines the nature of the I2C transaction.
 *
 *  I2C controller mode:
 *  This structure specifies the buffer and buffer's size that is to be
 *  written to or read from the I2C target peripheral.
 *
 *  I2C target mode:
 *  This structure specifies the buffer and buffer's size that is to be
 *  read from or written to the I2C controller. In restart condition,
 *  readBuf/writeBuf and readCount/writeCount are used repeatedly for
 *  every start in one transfer. When each restart happens, driver will
 *  call back to application with the restart transfer status, and
 *  application should save the data transferred in the previous start,
 *  and provide the new data to the current start. When all the starts
 *  complete (stop condition), driver will call back to application with
 *  transfer success status, and readBuf/writeBuf and readCount/writeCount
 *  will only record the data transferred in the last start condition.
 *
 */
typedef struct I2C_Transaction_s {

/** controller mode: buffer containing data to be written to target
 *  target mode: buffer containing data to be written to controller */
    void                *writeBuf;
/** controller mode: number of bytes to be written to the target
 *  target mode: number of bytes to be written to the controller */
    size_t              writeCount;
/** controller mode: buffer to which data from target is to be read into
 *  target mode: buffer to which data from controller is to be read into */
    void                *readBuf;
/** controller mode: number of bytes to be read from the target
 *  target mode: number of bytes to be read to the controller */
    size_t              readCount;
/** controller mode: input field from user to set the address of I2C target
 *  target mode: output field from driver to report the address of a
 *  target channel when multi-target channels are supported, if only one
 *  channel is supported, this field is ignored */
    uint32_t            targetAddress;
/** used for queuing in I2C_MODE_CALLBACK mode */
    void                *nextPtr;
/** used for passing argument to callback function */
    void                *arg;
/** Timeout value for i2c transaction */
    uint32_t            timeout;
/** I2C controller or target mode */
    bool                controllerMode;
/** Expand target address: true: 10-bit address mode,
 *  false: 7-bit address mode */
    bool                expandSA;
    /** Transaction type: true: Memory read/write Operation,
 *  false: Simple read and write operation */
    bool                memTxnEnable;
/** Memory Transfer related Parameters */
    I2C_Mem_Transaction *memTransaction;
/** Transaction Status */
    int32_t             status;

} I2C_Transaction;

/**
 *  \brief  I2C Parameters
 *
 *  I2C parameters are used to with the I2C_open() call. Default values for
 *  these parameters are set using I2C_Params_init().
 *
 *  If I2C_TransferMode is set to I2C_MODE_BLOCKING then I2C_transfer function
 *  calls will block thread execution until the transaction has completed.
 *
 *  If I2C_TransferMode is set to I2C_MODE_CALLBACK then I2C_transfer will not
 *  block thread execution and it will call the function specified by
 *  transferCallbackFxn. Sequential calls to I2C_transfer in I2C_MODE_CALLBACK
 *  mode will put the designated transaction onto an internal queue that
 *  automatically starts queued transactions after the previous transaction has
 *  completed. (regardless of error state).
 *
 *  \ref I2cBitRates specifies the I2C bus rate used for I2C communications.
 *
 */
typedef struct I2C_Params_s {
/** Blocking or Callback mode */
    uint8_t     transferMode;
/** I2C_CallbackFxn     transferCallbackFxn; Callback function pointer */
    void (*transferCallbackFxn)(I2C_Handle i2cHnd, I2C_Transaction * msg,
                                int32_t transferStatus);
/** I2C bus bit rate */
    uint8_t     bitRate;
} I2C_Params;

/**
 *  \brief  I2C Object
 *
 *  The application must not access any member variables of this structure!
 */
typedef struct I2C_Object_s {

/** Grants exclusive access to I2C */
    SemaphoreP_Object   mutex;
/** Notify finished I2C transfer */
    SemaphoreP_Object   transferComplete;
/** Hwi object */
    HwiP_Object         hwiObj;
/** I2C open parameters as provided by user */
    I2C_Params          i2cParams;
/** Stores the I2C state */
    uint8_t             state;
/** I2C transaction variables */
/** Pointer to current I2C transaction */
    I2C_Transaction    *currentTransaction;
/* I2C transaction pointers for I2C_MODE_CALLBACK */
/* Head ptr for queued transactions */
    I2C_Transaction    *headPtr;
/* Tail ptr for queued transactions */
    I2C_Transaction    *tailPtr;
/* flag to indicate module is open */
    bool                isOpen;
/* interrupt status error */
    uint32_t            intStatusErr;
/* I2C driver object for lld */
    I2CLLD_Object       i2cLldObject;
    I2CLLD_Handle       i2cLldHandle;

} I2C_Object;


/**
 *  \brief  I2C Global configuration
 *
 *  The I2C_Config structure contains a set of pointers used to characterize
 *  the I2C driver implementation.
 *
 *  This structure needs to be defined before calling I2C_init() and it must
 *  not be changed thereafter.
 *
 */
typedef struct I2C_Config_s {
/** Pointer to a driver specific data object */
    I2C_Object         *object;
/** Pointer to a driver specific hardware attributes structure */
    I2C_HwAttrs        *hwAttrs;
} I2C_Config;

/**
 *  \brief  I2C callback function
 *
 *  User definable callback function prototype. The I2C driver will call the
 *  defined function and pass in the I2C driver's handle, the pointer to the I2C
 *  transaction that just completed, and the return value of I2C_transfer.
 *
 *  In target mode, when there is a restart condtion,the driver calls back
 *  to the application with received data and I2C_STS_RESTART transfer status,
 *  application needs to provide the restart transmit data in I2C_Transaction
 *  rsWrToMstBuf. Restart condition only works in callback mode.
 *
 *  \param  handle          I2C_Handle
 *  \param  msg             Address of the I2C_Transaction performed
 *  \param  transferStatus  Results of the I2C transaction
 */
typedef void (*I2C_CallbackFxn)(I2C_Handle handle, I2C_Transaction * msg,
                                int32_t transferStatus);

/* ========================================================================== */
/*                          Function Declarations                             */
/* ========================================================================== */

/**
 *  \brief Initialize the I2C module
 */
void I2C_init(void);

/**
 *  \brief De-nitialize the I2C module
 */
void I2C_deinit(void);

/**
 *  \brief Function to set default values of I2C_Params in params
 *
 *  \param params           [IN] pointer to the structure to be initialized
 */
void I2C_Params_init(I2C_Params *params);

/**
 *  \brief Open the I2C at index idx with parameters params
 *
 *  \param idx              [IN] Index of I2C to open in global config
 *  \param params           [IN] I2C_Params values to use for opening
 *
 *  \return I2C_Handle
 */
I2C_Handle  I2C_open(uint32_t idx, const I2C_Params *params);

/**
 *  \brief Function to close the I2C Peripheral specified by the handle
 *
 *  \pre #I2C_open() has to be called first
 *
 *  \param handle           [IN] #I2C_Handle returned from I2C_open()
 *
 *  \sa #I2C_open()
 */
void I2C_close(I2C_Handle handle);

/**
 *  \brief Function to set default values of I2C_Mem_Transaction in
 *  memTransaction
 *
 *  \param memTransaction   [IN] pointer to the structure to be initialized
 */
void I2C_Memory_Transaction_init(I2C_Mem_Transaction *memTransaction);

/**
 *  \brief Function to set default values of I2C_Transaction in transaction
 *
 *  \param transaction      [IN] pointer to the structure to be initialized
 */
void I2C_Transaction_init(I2C_Transaction *transaction);

/**
 *  \brief Function to initiate a transfer from I2C
 *
 *  \param handle           [IN] handle to the I2C
 *  \param transaction      [IN] I2C_Transaction structure that contains
 *                               values for this specific transfer
 *
 *  \return \ref I2C_StatusCode
 */
int32_t I2C_transfer(I2C_Handle handle, I2C_Transaction *transaction);

/**
 *  \brief Function to probe I2C
 *
 *  \param handle           [IN] handle to the I2C
 *  \param targetAddr       [IN] address of the target to probe
 *
 *  \return \ref I2C_StatusCode
 */
int32_t I2C_probe(I2C_Handle handle, uint32_t targetAddr);

/**
 *  \brief Function to set the bus frequency
 *
 *  \param handle           [IN] handle to the I2C
 *  \param busFrequency     [IN] frequency value to be set
 *
 *  \return \ref I2C_StatusCode
 */
int32_t I2C_setBusFrequency(I2C_Handle handle, uint32_t busFrequency);

/**
 *  \brief Function to recover the bus in case of error
 *
 *  \param handle           [IN] handle to the I2C
 *  \param i2cDelay         [IN] the length of delay for sending
 *                             clock pulses to target
 *
 *  \return \ref I2C_StatusCode
 */
int32_t I2C_recoverBus(I2C_Handle handle, uint32_t i2cDelay);

/**
 *  \brief  This function returns the handle of an open I2C instance from the instance index
 *
 *  \pre    I2C controller has been opened using #I2C_open()
 *
 *  \param  index           [IN] Index of config to use in the *I2C_Config* array
 *
 *  \return An #I2C_Handle if it has been opened already or NULL otherwise
 */
I2C_Handle I2C_getHandle(uint32_t index);

#ifdef __cplusplus
}
#endif

#endif /* I2C_H_ */

/** @} */
