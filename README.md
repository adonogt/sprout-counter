# sprout counter – stm32f103

firmware project for counting seedlings detected by optical sensors and displaying totals on a 0.91" i²c oled. built with cmake + hal.

---

## features

- **mcu:** stm32f103 (blue pill default)  
- **inputs:** 3 optical sensors + 3 buttons (all interrupt-driven)  
- **output:** i²c oled display (ssd1306) + buzzer  
- **toolchain:** arm-none-eabi-gcc, cmake, ninja/make  
- **debug:** openocd + st-link, gdb  

---

## quick start

```bash
git submodule update --init --recursive
cmake -S . -B build -G Ninja -DCMAKE_BUILD_TYPE=Release
cmake --build build

openocd -f openocd/interface-stlink.cfg -f openocd/target-stm32f1x.cfg   -c "program build/sprout_counter.elf verify reset exit"
```

### debug with:
```bash
openocd -f openocd/interface-stlink.cfg -f openocd/target-stm32f1x.cfg &

gdb-multiarch -q   -ex "target remote localhost:3333"   -ex "monitor reset halt"   build/sprout_counter.elf
```

---

## hardware (default mapping)

- **oled (i²c1):** pb6 (scl), pb7 (sda)  
- **buzzer:** pb10  
- **buttons:** pa0, pa1, pa2 (active-low)  
- **optical sensors:** pb3, pb4, pb5 (active-high)  

---

## project structure

```
sprout-counter/
├── cmake/
│   └── toolchain-arm-gcc.cmake
├── include/
│   ├── board.h
│   ├── app.h
│   └── ssd1306.h
├── src/
│   ├── main.c
│   ├── board.c
│   ├── app.c
│   └── ssd1306.c
├── lib/
│   └── stm32cubef1/        # hal as git submodule
├── openocd/
│   ├── interface-stlink.cfg
│   └── target-stm32f1x.cfg
├── CMakeLists.txt
└── README.md
```

---

## notes

- adjust pinout and oled geometry in `include/board.h` and `src/ssd1306.c`.  
- debounce logic is basic (10 ms optics, 50 ms buttons).  
- requires stm32cubef1 hal as a submodule.  

---

## license
