/*
 * System Firmware Source File
 *
 * Software defined Device groups (devgrps) for AM275 device.
 *
 * Data version: 220316_180112
 *
 * Copyright (C) 2022 Texas Instruments Incorporated - http://www.ti.com/
 * ALL RIGHTS RESERVED
 */
#ifndef SOC_AM275X_DEVGRPS_H
#define SOC_AM275X_DEVGRPS_H

#include <types/devgrps.h>

/**
 * Set of MCU domain peripherals to initialize at boot and use during runtime.
 */
#define SOC_DEVGRP_AM275X_MCU_WAKEUP DEVGRP_01

/**
 * Set of Main domain peripherals to initialize at boot and use during
 * runtime.
 */
#define SOC_DEVGRP_AM275X_MAIN DEVGRP_00

/**
 * Set of peripherals to initialize for System Firmware internal usage
 */
#define SOC_DEVGRP_AM275X_TIFS_INTERNAL DEVGRP_DMSC


#endif /* SOC_AM275X_DEVGRPS_H */
