/*
 * R5F and A53 support for AM62Dx.
 */
let path = require('path');

const files_r5f = {
    common: [
        "boot_armv7r.c",
        "DebugP_shmLogWriter.c",
        "DebugP_uartLogWriter.c",
        "DebugP_uartScanf.c",
        "DebugP_memTraceLogWriter.c",
        "DebugP_log.c",
        "DebugP_nortos.c",
        "SemaphoreP_nortos.c",
        "ClockP_nortos.c",
        "ClockP_nortos_r5.c",
        "EventP_nortos.c",
        "HeapP_nortos.c",
        "TimerP.c",
        "TaskP_nortos.c",
        "HwiP_armv7r_vim.c",
        "HwiP_armv7r_handlers_nortos.c",
        "MpuP_armv7r.c",
        "CacheP_armv7r.c",
        "CpuId_armv7r.c",
        "HeapP_internal.c",
        "printf.c",
        "AddrTranslateP.c",
        "PmuP_armv7r.c",
        "QueueP_nortos.c",
    ]
};

const files_a53 = {
    common: [
        "AddrTranslateP.c",
        "boot_armv8.c",
        "CacheP_armv8.c",
        "ClockP_nortos.c",
        "ClockP_nortos_a53.c",
        "DebugP_log.c",
        "DebugP_memTraceLogWriter.c",
        "DebugP_nortos.c",
        "DebugP_shmLogWriter.c",
        "DebugP_uartLogWriter.c",
        "DebugP_uartScanf.c",
        "EventP_nortos.c",
        "HeapP_internal.c",
        "HeapP_nortos.c",
        "HwiP_armv8_gic.c",
        "HwiP_armv8_handlers_nortos.c",
        "MmuP_armv8.c",
        "printf.c",
        "PmuP_armv8.c",
        "QueueP_nortos.c",
        "SemaphoreP_nortos.c",
        "TaskP_nortos.c",
        "TimerP.c",
    ],
};

const defines_r5f = {
    common: [
        "SOC_AM62DX",
        "OS_NORTOS"
    ]
};

const cflags_r5f = {
    common: [
        "-Wno-extra"
    ]
};

const cflags_a53 = {
    common: [

    ]
};

const filedirs_r5f = {
    common: [
        "dpl/r5",
        "dpl/common",
    ],
};

const asmfiles_r5f = {
    common: [
        "HwiP_armv7r_handlers_nortos_asm.S",
        "HwiP_armv7r_vectors_nortos_asm.S",
        "HwiP_armv7r_vectors_nortos_sbl_asm.S",
        "boot_armv7r_asm.S",
        "HwiP_armv7r_asm.S",
        "MpuP_armv7r_asm.S",
        "CacheP_armv7r_asm.S",
        "CpuId_armv7r_asm.S",
        "PmuP_armv7r_asm.S",
    ],
};

const asmfiles_a53 = {
    common: [
        "boot_armv8_asm.S",
        "common_armv8_asm.S",
        "CacheP_armv8_asm.S",
        "HwiP_armv8_asm.S",
        "HwiP_armv8_handlers_nortos_asm.S",
        "HwiP_armv8_vectors_nortos_asm.S",
        "MmuP_armv8_asm.S",
        "PmuP_armv8_asm.S",
        "SpinlockP_armv8.S",
    ],
};

const filedirs_a53 = {
    common: [
        "dpl/common",
        "dpl/a53",
    ],
};

const defines_a53 = {
    common: [
        "SOC_AM62DX",
        "OS_NORTOS",
        "AMP_FREERTOS_A53"
    ]
};

const buildOptionCombos = [
    { device: device, cpu: "r5f", cgt: "ti-arm-clang", os: "nortos"},
    { device: device, cpu: "a53", cgt: "gcc-aarch64", os: "nortos"},
];

function getComponentProperty() {
    let property = {};

    property.dirPath = path.resolve(__dirname, "..");
    property.type = "library";
    property.name = "nortos";
    property.isInternal = false;
    property.buildOptionCombos = buildOptionCombos;

    return property;
}

function getComponentBuildProperty(buildOption) {
    let build_property = {};

    if(buildOption.cpu.match(/r5f*/)) {
        build_property.files = files_r5f;
        build_property.asmfiles = asmfiles_r5f;
        build_property.filedirs = filedirs_r5f;
        build_property.cflags = cflags_r5f;
        build_property.defines = defines_r5f;
    }

    if(buildOption.cpu.match(/a53*/)) {
        build_property.files = files_a53;
        build_property.asmfiles = asmfiles_a53;
        build_property.filedirs = filedirs_a53;
        build_property.cflags = cflags_a53;
        build_property.defines = defines_a53;
    }

    return build_property;
}

module.exports = {
    getComponentProperty,
    getComponentBuildProperty,
};
