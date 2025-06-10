# Modify to match your needs.  These settings can also be overridden at the
# command line.  (eg. cmake -DCMAKE_C_FLAGS="-O3")

set (CMAKE_SYSTEM_PROCESSOR "arm"            CACHE STRING "")
set (MACHINE                "template"       CACHE STRING "")
set (CROSS_PREFIX           "arm-none-eabi-" CACHE STRING "")

set (CMAKE_C_FLAGS          ""               CACHE STRING "")
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -mfpu=vfpv3-d16 -mfloat-abi=hard")
set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -mfpu=vfpv3-d16 -mfloat-abi=hard")

include (cross-freertos-gcc)
include_directories(/home/pratyay/attempt3/libmetal/headerFiles) 
include_directories(/home/pratyay/attempt3/libmetal/headerFiles/kernel/freertos/FreeRTOS-Kernel/include)  
link_directories(/home/pratyay/attempt3/libmetal/headerFiles/kernel/freertos/lib)
