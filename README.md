STM32F3 and Nokia LCD mini-game console

* doc - various datasheets
* hard - DipTrace schematics and PCB design
* soft - CubeMX/HAL-based project compilable with GNU toolkit

Standard Makefile compilation on Mac and Linux. Requires:
* gcc-arm-none-eabi - ARM GCC compiler: https://launchpad.net/gcc-arm-embedded
* STM32Cube_FW_F3_V1.7.0 - HAL library: http://www.st.com/en/embedded-software/stm32cubef3.html - click on "Get Software"
* stlink tools (st-flash/st-util) - to flash/debug firmware with STLink programmer: https://github.com/texane/stlink
* STLink programmer itself: http://www.st.com/en/development-tools/st-link-v2.html
* if you plan to change hardware you will need STM32CubeMX tool: http://www.st.com/en/development-tools/stm32cubemx.html - click on "Get Software" 

```
make && make flash
```
then to debug

```
xterm -e st-util &		# spawn st-util on different console
make debug
```

The current software is the Game of Reversy (Otello)

The board is double-sided suitable for DIY and fits the LCD screen component "natively"

![Complete Device](https://github.com/deniskokarev/vgame/blob/master/IMG_1202.jpg "Complete Device")

TODO:
- Write more games
