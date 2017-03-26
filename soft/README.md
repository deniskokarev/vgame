# Game of Reversy formware for our mini-console

* soft.ioc - CubeMX hardware project file: our hardware-specific stuff
* Src, Inc and TrueSTUDIO - HAL-based source code redndered by CubeMX
(Except few lines in Src/main.c where our code starts)
* cube3.mk - generic makefile for Cube F3 HAL library-based projects 
* Makefile - our project makefile
* STM32F334K8_FLASH.ld - linker script for our chip rendered by CubeMX

There are two external modules used:
* git submodule add https://github.com/deniskokarev/reversy
* git submodule add https://github.com/adafruit/Adafruit-GFX-Library

Also Adafruit-PCD8544-Nokia-5110-LCD-library library was ported to STM32 with HAL layer and DMA mode.

program.cpp has higher user-level API to work with our hardware

vgame_program.cpp is the actual game code

TODO:
- add extensive comments
+ setMainProgram() in the Program() consructor would be conflicting if we have had multiple global programs defined
  actually in this case programs will have to be incapsulated into a global object with explicit creation order
- rename sleep() methods
- observe infinite refresh rate
- pull void __cxa_pure_virtual(void) and __errno into cxx module
- add color and difficulty selection to the startWindow
