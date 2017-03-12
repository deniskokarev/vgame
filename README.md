STM32F3 and Nokia LCD mini-game console

* doc - various datasheets
* hard - DipTrace schematics and PCB design
* soft - CubeMX/HAL-based project compilable with GNU toolkit

Standard Makefile compilation on Mac and Linux. Requires:
* gcc-arm-none-eabi - ARM GCC compiler
* STM32Cube_FW_F3_V1.7.0 - HAL library
* stutils (st-flash/st-util) - to flash/debug firmware with STLink programmer

```
make && make flash
```
then to debug

```
xterm -e st-util &
make debug
```

The current software is just an adopted SparkFun demo

The board is double-sided suitable for DIY and fits the LCD screen component "natively"

![Complete Device](https://github.com/deniskokarev/vgame/blob/master/IMG_1202.jpg "Complete Device")

TODO:
- Write a game for it
