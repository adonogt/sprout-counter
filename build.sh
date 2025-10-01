#!/usr/bin/env bash
set -euo pipefail
# Tiny helper for one-liners. Comments in English.

BOARD="${BOARD:-bluepill_f103}"
APP="${APP:-.}"
BUILD_TYPE="${BUILD_TYPE:-Release}"
BUILD_DIR="${BUILD_DIR:-build}"
GEN="${GEN:-Ninja}"
TOOLCHAIN="${TOOLCHAIN:-cmake/arm-gcc-toolchain.cmake}"
EXTRA_CMAKE="${EXTRA_CMAKE:--DCMAKE_EXPORT_COMPILE_COMMANDS=ON}"

cmd="${1:-build}"

configure() {
  echo ">> configure (BOARD=${BOARD}, APP=${APP}, TYPE=${BUILD_TYPE})"
  cmake -S . -B "${BUILD_DIR}" -G "${GEN}" \
    -DCMAKE_TOOLCHAIN_FILE="${TOOLCHAIN}" \
    -DBOARD="${BOARD}" -DAPP="${APP}" \
    -DCMAKE_BUILD_TYPE="${BUILD_TYPE}" \
    ${EXTRA_CMAKE}
}

build() {
  configure
  echo ">> build"
  cmake --build "${BUILD_DIR}" -j
}

flash() {
  build
  echo ">> flash"
  ( cd "${BUILD_DIR}" && openocd -f interface/stlink.cfg -f target/stm32f1x.cfg \
      -c "program sprout_counter.elf verify reset exit" )
}

clean() {
  echo ">> clean ${BUILD_DIR}"
  rm -rf "${BUILD_DIR}"
}

size() {
  arm-none-eabi-size "${BUILD_DIR}/sprout_counter.elf" || true
}

case "${cmd}" in
  clean) clean ;;
  configure) configure ;;
  build) build ;;
  flash) flash ;;
  size) size ;;
  *) echo "usage: $0 {clean|configure|build|flash|size}"; exit 2 ;;
esac
