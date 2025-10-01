# Bare-metal ARM GCC for Cortex-M3 (STM32F103xB).
set(CMAKE_SYSTEM_NAME Generic)
set(CMAKE_TRY_COMPILE_TARGET_TYPE STATIC_LIBRARY)

set(CMAKE_C_COMPILER arm-none-eabi-gcc)
set(CMAKE_ASM_COMPILER arm-none-eabi-gcc)

# IMPORTANT: make flags a CMake LIST (use semicolons) so they don't get quoted as one arg.
set(MCU_FLAGS
  -mcpu=cortex-m3
  -mthumb
  -ffunction-sections
  -fdata-sections
  -fno-builtin
  -fno-exceptions
)

add_compile_options(
  ${MCU_FLAGS}
  -O2
  -g3
  -Wall
  -Wextra
  -Wno-unused-parameter
)

# Optional (helps linker GC and consistent float ABI if you add libs later):
# set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -Wl,--gc-sections")
