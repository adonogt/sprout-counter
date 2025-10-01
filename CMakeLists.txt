cmake_minimum_required(VERSION 3.16)

# Use a CMake-safe target name
project(sprout_counter C ASM)

# ---------------- Toolchain ----------------
set(CMAKE_TOOLCHAIN_FILE ${CMAKE_SOURCE_DIR}/cmake/arm-gcc-toolchain.cmake)

# ---------------- HAL base path ----------------
set(HAL_BASE "" CACHE PATH "Base path to STM32CubeF1 (contains 'Drivers')")
if(NOT HAL_BASE)
  if(DEFINED ENV{STM32CUBE_F1_ROOT} AND NOT "$ENV{STM32CUBE_F1_ROOT}" STREQUAL "")
    set(HAL_BASE "$ENV{STM32CUBE_F1_ROOT}")
  else()
    file(TO_CMAKE_PATH "$ENV{HOME}/hal/STM32CubeF1" HAL_BASE)
  endif()
endif()

set(HAL_ROOT   "${HAL_BASE}/Drivers/STM32F1xx_HAL_Driver")
set(CMSIS_ROOT "${HAL_BASE}/Drivers/CMSIS")

if(NOT EXISTS "${HAL_ROOT}/Inc" OR NOT EXISTS "${CMSIS_ROOT}/Include")
  message(FATAL_ERROR
    "STM32CubeF1 not found.\n"
    "Tried HAL_BASE='${HAL_BASE}'\n"
    "Expected:\n"
    "  ${HAL_ROOT}/Inc\n"
    "  ${CMSIS_ROOT}/Include\n"
    "Set ENV STM32CUBE_F1_ROOT or pass -DHAL_BASE=/abs/path.")
endif()

# ---------------- Board/App selection ----------------
set(BOARD bluepill_f103 CACHE STRING "Board folder under boards/")
set(APP   .             CACHE STRING "App folder under apps/ ('.' means apps/main.c)")

# ---------------- Device/Defines ----------------
add_compile_definitions(USE_HAL_DRIVER STM32F103xB)

# ---------------- Include paths ----------------
include_directories(
  ${CMSIS_ROOT}/Include
  ${CMSIS_ROOT}/Device/ST/STM32F1xx/Include
  ${HAL_ROOT}/Inc
  ${CMAKE_SOURCE_DIR}/boards/${BOARD}/config

  ${CMAKE_SOURCE_DIR}/apps/${APP}/commons/include
  ${CMAKE_SOURCE_DIR}/apps/${APP}/drivers/include
  ${CMAKE_SOURCE_DIR}/apps/${APP}/devices/include
  ${CMAKE_SOURCE_DIR}/apps/${APP}/setup/include
  ${CMAKE_SOURCE_DIR}/apps/${APP}/task/include
)

# ---------------- HAL (lean selection) ----------------
set(HAL_MODULES rcc gpio cortex flash rcc_ex)
include(${CMAKE_SOURCE_DIR}/cmake/hal_f1_min.cmake)  # -> HAL_SRCS

# ---------------- Startup/System ----------------
set(STARTUP_LOCAL ${CMAKE_SOURCE_DIR}/boards/${BOARD}/startup/startup_stm32f103xb.s)
if(EXISTS ${STARTUP_LOCAL})
  set(STARTUP ${STARTUP_LOCAL})
else()
  set(STARTUP ${CMSIS_ROOT}/Device/ST/STM32F1xx/Source/Templates/gcc/startup_stm32f103xb.s)
endif()
set(SYSTEM  ${CMSIS_ROOT}/Device/ST/STM32F1xx/Source/Templates/system_stm32f1xx.c)

# ---------------- App sources ----------------
file(GLOB APP_COMMONS ${CMAKE_SOURCE_DIR}/apps/${APP}/commons/src/*.c)
file(GLOB APP_DRIVERS ${CMAKE_SOURCE_DIR}/apps/${APP}/drivers/src/*.c)
file(GLOB APP_DEVICES ${CMAKE_SOURCE_DIR}/apps/${APP}/devices/src/*.c)
file(GLOB APP_SETUP   ${CMAKE_SOURCE_DIR}/apps/${APP}/setup/src/*.c)
file(GLOB APP_TASK    ${CMAKE_SOURCE_DIR}/apps/${APP}/task/src/*.c)
set(APP_MAIN ${CMAKE_SOURCE_DIR}/apps/${APP}/main.c)

if(NOT EXISTS ${APP_MAIN})
  message(FATAL_ERROR "Expected app main at: ${APP_MAIN}")
endif()

# ---------------- Executable target ----------------
add_executable(${PROJECT_NAME}.elf
  ${STARTUP}
  ${SYSTEM}
  ${HAL_SRCS}
  ${APP_COMMONS}
  ${APP_DRIVERS}
  ${APP_DEVICES}
  ${APP_SETUP}
  ${APP_TASK}
  ${APP_MAIN}
)

# ---------------- Linker ----------------
set(LD_SCRIPT ${CMAKE_SOURCE_DIR}/boards/${BOARD}/linker.ld)
target_link_options(${PROJECT_NAME}.elf PRIVATE
  -T${LD_SCRIPT} -Wl,--gc-sections -Wl,-Map=${PROJECT_NAME}.map
)

# ---------------- Artifacts (robust: no hardcoded names) ----------------
add_custom_command(TARGET ${PROJECT_NAME}.elf POST_BUILD
  COMMAND arm-none-eabi-objcopy -O ihex   $<TARGET_FILE:${PROJECT_NAME}.elf> sprout-counter.hex
  COMMAND arm-none-eabi-objcopy -O binary $<TARGET_FILE:${PROJECT_NAME}.elf> sprout-counter.bin
)
