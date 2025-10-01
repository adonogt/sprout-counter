# Simple wrapper Makefile for STM32F1 (CMake + Ninja).
# Targets: clean, configure, build, size, flash, erase, gdb

# ----- User settings (can be overridden: make BOARD=... APP=... ) -----
BOARD        ?= bluepill_f103
APP          ?= .
BUILD_TYPE   ?= Release
BUILD_DIR    ?= build
GEN          ?= Ninja
TOOLCHAIN    ?= cmake/arm-gcc-toolchain.cmake

# Optional: where STM32CubeF1 lives. If empty, CMake falls back to env or $HOME/hal/STM32CubeF1
# HAL_BASE   ?= /abs/path/to/STM32CubeF1

# OpenOCD interface/target (adjust if needed)
OCD_IF       ?= interface/stlink.cfg
OCD_TGT      ?= target/stm32f1x.cfg

# Extra CMake flags (e.g., -DHAL_BASE=... -DCMAKE_EXPORT_COMPILE_COMMANDS=ON)
EXTRA_CMAKE  ?= -DCMAKE_EXPORT_COMPILE_COMMANDS=ON
# Example to force HAL path:
# EXTRA_CMAKE = -DHAL_BASE=$(HOME)/hal/STM32CubeF1 -DCMAKE_EXPORT_COMPILE_COMMANDS=ON

# ----- Derived -----
CONFIG_ARGS  = -S . -B $(BUILD_DIR) -G $(GEN) \
               -DCMAKE_TOOLCHAIN_FILE=$(TOOLCHAIN) \
               -DBOARD=$(BOARD) -DAPP=$(APP) \
               -DCMAKE_BUILD_TYPE=$(BUILD_TYPE) \
               $(EXTRA_CMAKE)

# ----- Phony -----
.PHONY: all clean configure build rebuild size flash erase gdb openocd

all: build

clean:
	@echo ">> clean $(BUILD_DIR)"
	rm -rf $(BUILD_DIR)

configure:
	@echo ">> cmake configure: BOARD=$(BOARD) APP=$(APP) BUILD_TYPE=$(BUILD_TYPE)"
	cmake $(CONFIG_ARGS)

build: configure
	@echo ">> build"
	cmake --build $(BUILD_DIR) -j

rebuild: clean build

size:
	@echo ">> size"
	arm-none-eabi-size $(BUILD_DIR)/sprout_counter.elf || true

flash: build
	@echo ">> flash with OpenOCD"
	cd $(BUILD_DIR) && openocd -f $(OCD_IF) -f $(OCD_TGT) \
	  -c "program sprout_counter.elf verify reset exit"

erase:
	@echo ">> mass erase (stm32f1x)"
	openocd -f $(OCD_IF) -f $(OCD_TGT) -c "init; stm32f1x mass_erase 0; reset run; exit"

gdb:
	@echo ">> start gdb (attach after openocd)"
	arm-none-eabi-gdb $(BUILD_DIR)/sprout_counter.elf

openocd:
	@echo ">> start openocd server"
	openocd -f $(OCD_IF) -f $(OCD_TGT)

