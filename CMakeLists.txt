# minimal cmake setup for stm32f103c8 (bluepill) using stm32cube f1 hal + cmsis
# comments are lowercase as requested

cmake_minimum_required(VERSION 3.20)
project(stm32f103c8_bluepill C ASM)

# ------------------------------------------------------------------------------
# toolchain
#   cmake -S . -B build -G Ninja -DCMAKE_TOOLCHAIN_FILE=cmake/arm-none-eabi-gcc.cmake
# ------------------------------------------------------------------------------

# path to the stm32cube f1 package (can be overridden with -DHAL_PATH=...)
if(NOT DEFINED HAL_PATH)
  set(HAL_PATH "$ENV{HOME}/hal/STM32CubeF1" CACHE PATH "path to stm32cube f1")
endif()

# ------------------------------------------------------------------------------
# cmsis/hal directories from the cube package
# ------------------------------------------------------------------------------

set(CMSIS_DEVICE_DIR ${HAL_PATH}/Drivers/CMSIS/Device/ST/STM32F1xx)
set(CMSIS_CORE_DIR   ${HAL_PATH}/Drivers/CMSIS/Include)
set(HAL_SRC_DIR      ${HAL_PATH}/Drivers/STM32F1xx_HAL_Driver/Src)
set(HAL_INC_DIR      ${HAL_PATH}/Drivers/STM32F1xx_HAL_Driver/Inc)

# ------------------------------------------------------------------------------
# sources
# ------------------------------------------------------------------------------

# startup and system (system is a local copy under board/)
set(STARTUP_ASM ${CMAKE_SOURCE_DIR}/startup/startup_stm32f103c8tx.s)
set(SYSTEM_SRC  ${CMAKE_SOURCE_DIR}/board/system_stm32f1xx.c)

# minimal hal needed for clock + gpio + systick + i2c
set(HAL_SOURCES
  ${HAL_SRC_DIR}/stm32f1xx_hal.c
  ${HAL_SRC_DIR}/stm32f1xx_hal_rcc.c
  ${HAL_SRC_DIR}/stm32f1xx_hal_rcc_ex.c
  ${HAL_SRC_DIR}/stm32f1xx_hal_gpio.c
  ${HAL_SRC_DIR}/stm32f1xx_hal_cortex.c
  ${HAL_SRC_DIR}/stm32f1xx_hal_pwr.c
  ${HAL_SRC_DIR}/stm32f1xx_hal_flash.c
  ${HAL_SRC_DIR}/stm32f1xx_hal_flash_ex.c
  ${HAL_SRC_DIR}/stm32f1xx_hal_i2c.c          # i2c hal enabled
  # add ${HAL_SRC_DIR}/stm32f1xx_hal_exti.c if you enable hal exti in hal_conf
)

# user sources (drivers + app; excludes vendor libs)
file(GLOB_RECURSE SRC_FILES ${CMAKE_SOURCE_DIR}/src/*.c)

set(BOARD_FILES
  ${CMAKE_SOURCE_DIR}/board/stm32f1xx_it.c
)

# linker script
set(LINKER_SCRIPT ${CMAKE_SOURCE_DIR}/linker/STM32F103C8TX_FLASH.ld)

# ------------------------------------------------------------------------------
# u8g2 vendor library (submodule at lib/u8g2)
# ------------------------------------------------------------------------------

file(GLOB U8G2_C_SOURCES ${CMAKE_SOURCE_DIR}/lib/u8g2/csrc/*.c)
add_library(u8g2 STATIC ${U8G2_C_SOURCES})
target_include_directories(u8g2 PUBLIC
  ${CMAKE_SOURCE_DIR}/lib/u8g2/csrc
)

# ------------------------------------------------------------------------------
# target
# ------------------------------------------------------------------------------

add_executable(${PROJECT_NAME}.elf
  ${STARTUP_ASM}
  ${SYSTEM_SRC}
  ${HAL_SOURCES}
  ${BOARD_FILES}
  ${SRC_FILES}
)

# link vendor u8g2 (fonts/drivers); unused sections will be gc’d
target_link_libraries(${PROJECT_NAME}.elf PRIVATE u8g2)

# device/hal defines for the whole target
target_compile_definitions(${PROJECT_NAME}.elf PRIVATE
  STM32F103x8
  STM32F103xB
  USE_HAL_DRIVER
)

# include order: board first so hal finds our stm32f1xx_hal_conf.h
# add lib/u8g2/csrc so headers like "u8g2.h" in display.h always resolve
target_include_directories(${PROJECT_NAME}.elf BEFORE PRIVATE
  ${CMAKE_SOURCE_DIR}/board
  ${HAL_INC_DIR}
  ${CMSIS_CORE_DIR}
  ${CMSIS_DEVICE_DIR}/Include

  ${CMAKE_SOURCE_DIR}/src
  ${CMAKE_SOURCE_DIR}/src/drivers
  ${CMAKE_SOURCE_DIR}/src/drivers/system
  ${CMAKE_SOURCE_DIR}/src/drivers/gpio
  ${CMAKE_SOURCE_DIR}/src/drivers/i2c
  ${CMAKE_SOURCE_DIR}/src/drivers/display

  ${CMAKE_SOURCE_DIR}/lib/u8g2/csrc           # <-- ensures #include "u8g2.h" works anywhere
)

# common warnings/opts (toolchain sets cpu/thumb/sections)
target_compile_options(${PROJECT_NAME}.elf PRIVATE
  -Wall -Wextra -Wundef -Wno-unused-parameter
  -O2 -g3
  -std=gnu11
  -fdata-sections -ffunction-sections -fno-common
)

# compile system_stm32f1xx.c as pure cmsis (no use_hal_driver here)
set_source_files_properties(${SYSTEM_SRC} PROPERTIES
  COMPILE_DEFINITIONS "STM32F103x8;STM32F103xB"
)

# link options
target_link_options(${PROJECT_NAME}.elf PRIVATE
  -T${LINKER_SCRIPT}
  -Wl,--gc-sections
  -Wl,-Map=${CMAKE_PROJECT_NAME}.map
  -Wl,--start-group -lc -lm -Wl,--end-group
)

# ------------------------------------------------------------------------------
# post-build (bin/hex) and optional size print
# ------------------------------------------------------------------------------

# locate objcopy/size if the toolchain file did not set them
if(NOT CMAKE_OBJCOPY)
  find_program(ARM_OBJCOPY arm-none-eabi-objcopy)
  if(NOT ARM_OBJCOPY)
    message(FATAL_ERROR "arm-none-eabi-objcopy not found")
  endif()
  set(CMAKE_OBJCOPY ${ARM_OBJCOPY})
endif()

add_custom_command(TARGET ${PROJECT_NAME}.elf POST_BUILD
  COMMAND ${CMAKE_OBJCOPY} -O binary  $<TARGET_FILE:${PROJECT_NAME}.elf> ${PROJECT_NAME}.bin
  COMMAND ${CMAKE_OBJCOPY} -O ihex    $<TARGET_FILE:${PROJECT_NAME}.elf> ${PROJECT_NAME}.hex
  COMMENT "generating .bin and .hex artifacts"
)

find_program(ARM_SIZE arm-none-eabi-size)
if(ARM_SIZE)
  add_custom_command(TARGET ${PROJECT_NAME}.elf POST_BUILD
    COMMAND ${ARM_SIZE} $<TARGET_FILE:${PROJECT_NAME}.elf>
    COMMENT "printing size with arm-none-eabi-size"
  )
else()
  message(WARNING "arm-none-eabi-size not found; skipping size print")
endif()

# ------------------------------------------------------------------------------
# flash target (st-link + openocd)
# ------------------------------------------------------------------------------

add_custom_target(flash
  COMMAND openocd -f interface/stlink.cfg -f target/stm32f1x.cfg
                  -c "program ${PROJECT_NAME}.elf verify reset exit"
  DEPENDS ${PROJECT_NAME}.elf
  COMMENT "flashing firmware via openocd/st-link"
)
