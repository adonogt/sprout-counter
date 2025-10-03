# sprout-counter

firmware designed for seedling counting. minimal, modular, and easy to extend with new drivers and devices. supports integration with display modules, push buttons, and sensors for practical field usage.

---

## dependencies

install on arch linux:

```bash
sudo pacman -S arm-none-eabi-gcc arm-none-eabi-binutils arm-none-eabi-newlib \
             cmake ninja openocd gdb-multiarch make
```

external stm32cube f1 hal must be downloaded and placed in:

```
~/hal/STM32CubeF1
```

---

## structure

```
sprout-counter/
├── apps/
│   ├── main.c
│   ├── drivers/
│   ├── commons/
│   ├── devices/
│   ├── setup/
│   └── task/
├── boards/
│   └── bluepill_f103/
│       ├── config/
│       ├── linker.ld
│       └── startup/
├── cmake/
│   └── arm-gcc-toolchain.cmake
├── CMakeLists.txt
└── README.md
```

---

## build

### cmake + ninja
```bash
rm -rf build
cmake -S . -B build -G ninja \
  -DCMAKE_TOOLCHAIN_FILE=cmake/arm-gcc-toolchain.cmake \
  -DBOARD=bluepill_f103 \
  -DAPP=. \
  -DCMAKE_BUILD_TYPE=release \
  -DCMAKE_EXPORT_COMPILE_COMMANDS=on
cmake --build build -j
```

artifacts:
- `build/sprout_counter.elf`
- `build/sprout-counter.hex`
- `build/sprout-counter.bin`

### make wrapper
for convenience, use the provided makefile:
```bash
make build   # build firmware
make clean   # clean build directory
make flash   # flash to board
```

---

## flashing

```bash
openocd -f openocd/interface-stlink.cfg -f openocd/target-stm32f1x.cfg \
  -c "program build/sprout_counter.elf verify reset exit"
```

---

## debugging

```bash
openocd -f openocd/interface-stlink.cfg -f openocd/target-stm32f1x.cfg &
gdb-multiarch -q \
  -ex "target remote localhost:3333" \
  -ex "monitor reset halt" \
  build/sprout_counter.elf
```

---

## notes

- hal is external in `~/hal/STM32CubeF1`
- boards are in `boards/`
- drivers are in `apps/drivers/`
- supports future integration with displays, buttons, and sensors

---

## license

mit



