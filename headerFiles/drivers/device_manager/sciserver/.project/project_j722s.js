let path = require('path');

const files_r5f = {
    common: [
        "sciserver.c",
        "sciserver_secproxyTransfer.c",
        "sciserver_secproxyRoutingDescription.c",
        "sciserver_secproxyConfigData.c",
        "sciserver_dmTxRoutingData.c",
        "sciserver_tirtos.c",
        "sciserver_secproxySocData.c",
        "sciserver_init.c",
    ]
};

const includes_r5f = {
    common: [
        ".",
        "j722s",
        "../rm_pm_hal/rm_pm_hal_src/include",
        "../rm_pm_hal/rm_pm_hal_src/common/fw_caps",
        "../rm_pm_hal/rm_pm_hal_src/pm",
        "../rm_pm_hal/rm_pm_hal_src/pm/include",
        "../rm_pm_hal/rm_pm_hal_src/rm",
        "../rm_pm_hal/rm_pm_hal_src/rm/include",
        "../rm_pm_hal/rm_pm_hal_src/include/soc/j722s",
    ],
};

const filedirs_r5f = {
    common: [
        "j722s/",
        ".",
    ],
};

const defines_r5f = {
    common: [
        "MCU_PLUS_SDK",
        "BUILD_DM_R5",
        "BUILD_MCU1_0",
        "BUILD_MCU",
        "CONFIG_OSAL_MINIMAL_QUEUES",
        "MAKEFILE_BUILD",
    ],
};

const cflags = {
    remove: [
        "-Wno-unused-function",
    ],
};

const buildOptionCombos = [
    { device: device, cpu: "wkup-r5f", cgt: "ti-arm-clang"},
];

function getComponentProperty() {
    let property = {};

    property.dirPath = path.resolve(__dirname, "..");
    property.type = "library";
    property.name = "sciserver";
    property.isInternal = false;
    property.buildOptionCombos = buildOptionCombos;

    return property;
}

function getComponentBuildProperty(buildOption) {
    let build_property = {};

    if(buildOption.cpu == "wkup-r5f") {
        build_property.files = files_r5f;
        build_property.includes = includes_r5f;
        build_property.filedirs = filedirs_r5f;
        build_property.defines = defines_r5f;
        build_property.cflags = cflags;
    }

    return build_property;
}

module.exports = {
    getComponentProperty,
    getComponentBuildProperty,
};
