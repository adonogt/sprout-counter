# minimal arm-none-eabi toolchain file for stm32f103 (cortex-m3)
# comments are lowercase as requested

set(CMAKE_SYSTEM_NAME Generic)
set(CMAKE_SYSTEM_PROCESSOR arm)

# compilers
set(CMAKE_C_COMPILER   arm-none-eabi-gcc)
set(CMAKE_ASM_COMPILER arm-none-eabi-gcc)

# binutils
set(CMAKE_OBJCOPY arm-none-eabi-objcopy)
set(CMAKE_OBJDUMP arm-none-eabi-objdump)
set(CMAKE_SIZE    arm-none-eabi-size)

# base c flags for cortex-m3 bare metal
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -mcpu=cortex-m3 -mthumb -ffunction-sections -fdata-sections -fno-common")

# build types
set(CMAKE_C_FLAGS_RELEASE "-O2 -DNDEBUG")
set(CMAKE_C_FLAGS_DEBUG   "-O0 -g3")

# do not try to run test binaries on the host
set(CMAKE_TRY_COMPILE_TARGET_TYPE STATIC_LIBRARY)
