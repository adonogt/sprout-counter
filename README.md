# sprout-counter firmware

firmware for **stm32f103c8t6** to count seedlings detected by **infrared sensors** and display results on an **ssd1306 oled** using **u8g2** over **i2c1 (pb6/pb7)**.

## features

- 3 infrared sensors with exti interrupts
- 3 buttons for user control (inc, dec, ok/menu)
- ssd1306 oled display (u8g2 over i2c1)
- modular driver design: gpio, i2c, display, counter
- cmake + ninja build system
- flash and debug via openocd + st-link

## functionality

the firmware counts seedlings detected by the infrared sensors. the target count value and the current count can be adjusted using three buttons:

- inc: increases the target count (single step per press)
- dec: decreases the target count (single step per press)
- ok/menu: confirms the current selection; long press may open a simple menu to switch between editing the target value and resetting the current count (implementation detail in firmware)

the oled display shows the current count and the target count in real time.

## hardware setup

| peripheral | function | pin  | note |
|-------------|-----------|------|------|
| i2c1        | scl       | pb6  | ssd1306 oled |
|             | sda       | pb7  | ssd1306 oled |
| ir sensors  | sensor1   | pa0  | exti0 |
|             | sensor2   | pa1  | exti1 |
|             | sensor3   | pa2  | exti2 |
| buttons     | inc       | pb12 | increase target |
|             | dec       | pb13 | decrease target |
|             | ok/menu   | pb14 | confirm/menu |
| led         | user led  | pc13 | active low |

## repository structure

```
sprout-counter/
├── board/
│   ├── stm32f1xx_it.c
│   ├── system.c
│   └── system.h
├── src/
│   ├── main.c
│   ├── drivers/
│   │   ├── gpio/
│   │   ├── i2c/
│   │   ├── display/
│   │   ├── ir/
│   │   └── buttons/
├── lib/
│   └── u8g2/
├── linker/
│   └── stm32f103c8tx_flash.ld
├── startup/
│   └── startup_stm32f103c8tx.s
├── cmake/
│   └── arm-none-eabi-gcc.cmake
└── cmakelists.txt
```

## dependencies

note: this repository uses git submodules for third-party code (u8g2). ensure submodules are fetched (see display library section).

### system packages

```bash
sudo apt install cmake ninja-build gcc-arm-none-eabi binutils-arm-none-eabi openocd
```

### stm32cube hal

```bash
mkdir -p ~/hal && cd ~/hal
git clone https://github.com/stmicroelectronics/stm32cubef1.git
```

### display library (u8g2)

this repository uses git submodules. clone or update with submodules to fetch u8g2 correctly.

clone with submodules:

```bash
git clone --recurse-submodules <repo-url>
```

if already cloned, update submodules:

```bash
git submodule update --init --recursive
```

## build instructions

```bash
cmake -S . -B build -G ninja \
  -DCMAKE_TOOLCHAIN_FILE=cmake/arm-none-eabi-gcc.cmake \
  -DHAL_PATH=$HOME/hal/stm32cubef1

cmake --build build -v
```

output files:

```
build/stm32f103c8.elf
build/stm32f103c8.bin
build/stm32f103c8.hex
```

## flashing

```bash
ninja -C build flash
```

or manually:

```bash
openocd -f interface/stlink.cfg -f target/stm32f1x.cfg \
  -c "program build/stm32f103c8.elf verify reset exit"
```

## debugging

```bash
openocd -f interface/stlink.cfg -f target/stm32f1x.cfg
arm-none-eabi-gdb build/stm32f103c8.elf \
  -ex "target extended-remote :3333" \
  -ex "monitor reset halt" \
  -ex "load" \
  -ex "monitor reset run"
```

## license

mit license

copyright (c) 2025 adonogt

