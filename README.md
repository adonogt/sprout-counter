# sprout-counter firmware

firmware for stm32f103c8t6 (bluepill) to count seedlings detected by infrared sensors.  
the system runs fully bare metal using stm32cube f1 hal and cmsis.  
seed count and parameters are displayed and adjusted through a 0.91" ssd1306 oled using the u8g2 library.

---

## features

- bare-metal stm32f103 firmware (no rtos)
- infrared sensor pulse counting
- configurable target count
- 0.91" ssd1306 oled display (i2c, via u8g2)
- user configuration through onboard buttons
- modular drivers (gpio, system, display, counter)
- clang + cmake + ninja build system for linux development

---

## hardware

- mcu: stm32f103c8t6 ("bluepill")
- display: oled 0.91" ssd1306 (i2c)
- sensors: infrared beam detectors (digital output)
- buttons: momentary push buttons for configuration

connections (example):
```
oled sda  → pb7  
oled scl  → pb6  
ir sensor input → pa0  
buttons → pa1, pa2  
```

---

## repository structure

```
sprout-counter/
├── board/                # board-level config (hal_conf, interrupt handlers, system init)
├── cmake/                # toolchain and cmake helper files
├── linker/               # linker script for stm32f103c8
├── src/
│   ├── drivers/
│   │   ├── gpio/         # gpio driver with irq and callbacks
│   │   └── system/       # system clock + hal init
│   ├── main.c            # main application (counter logic + oled update loop)
│   └── ...
├── startup/              # startup assembly file
└── build/                # generated build output (ignored by git)
```

---

## dependencies

required packages on ubuntu/debian:

```bash
sudo apt install cmake ninja-build gcc-arm-none-eabi binutils-arm-none-eabi openocd
```

required stm32cube hal:

```bash
mkdir -p ~/hal && cd ~/hal
git clone https://github.com/STMicroelectronics/STM32CubeF1.git
```

optional (for display):
```bash
git clone https://github.com/olikraus/u8g2.git
```

---

## build

```bash
cmake -S . -B build -G Ninja \
  -DCMAKE_TOOLCHAIN_FILE=cmake/arm-none-eabi-gcc.cmake \
  -DHAL_PATH=$HOME/hal/STM32CubeF1

cmake --build build -v
```

artifacts generated in `build/`:
```
stm32f103c8_bluepill.elf
stm32f103c8_bluepill.bin
stm32f103c8_bluepill.hex
stm32f103c8_bluepill.map
```

---

## flash

with st-link and openocd:

```bash
ninja -C build flash
```

or manually:

```bash
openocd -f interface/stlink.cfg -f target/stm32f1x.cfg \
  -c "program build/stm32f103c8_bluepill.elf verify reset exit"
```

---

## debugging

```bash
# terminal 1
openocd -f interface/stlink.cfg -f target/stm32f1x.cfg

# terminal 2
arm-none-eabi-gdb build/stm32f103c8_bluepill.elf \
  -ex "target extended-remote :3333" \
  -ex "monitor reset halt" \
  -ex "load" \
  -ex "monitor reset run"
```

---

## license

this project is released under the [mit license](LICENSE).

```
copyright (c) 2025 adonogt
permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "software"), to deal
in the software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the software, and to permit persons to whom the software is
furnished to do so, subject to the following conditions:
...
```

---

## credits

- stm32cube f1 hal by stmicroelectronics  
- u8g2 graphics library by oliver kraus  
- project setup and build system using clang, cmake, and ninja

