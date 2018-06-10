# VGA-demo-on-bluepill
VGA demo on bluepill (using STM32F103 microcontroller)

## What it can do
There are 4 demos:
* Displaying color image (taken from http://www.worldofspectrum.org/pub/sinclair/screens/load/b/scr/BubbleBobble.scr)
* Text typing and moving cursor (font is taken from http://caxapa.ru/149446.html)
* Drawing (taken from https://github.com/Avamander/arduino-tvout/blob/master/examples/DemoNTSC/DemoNTSC.pde)
* Game (taken from http://http.debian.net/debian/pool/main/x/xonix/xonix_1.4.orig.tar.gz)
Video:
<a href="http://www.youtube.com/watch?feature=player_embedded&v=97oakB1NX68
" target="_blank"><img src="http://img.youtube.com/vi/97oakB1NX68/0.jpg" 
alt="VGA Demo on Bluepill" width="480" height="360" border="10" /></a>

## Introduction
My idea of “fun” maybe odd, but here it is. This is a demo that displays a color VGA video with resolution of 256x192 and 64 colors without any specialized video hardware.
This project was born some time ago when I first installed a great [TVOut](https://playground.arduino.cc/Main/TVout) library on my Arduino Mega. I was really impressed with the fact that so small board (it is based on a tiny 16 MHz 8-bit processor) can display a video with reasonable quality.
I started to look at similar projects and found a ["Glitch"](http://cliffle.com/project/glitch-demo/) demo, which is using more powerful, but still tiny, 32-bit processor. The “Glitch” was an inspiration for this project. I decided to get something in between and chose a ["blue pill"](http://wiki.stm32duino.com/index.php?title=Blue_Pill), which is using a 32-bit STM32F103 microcontroller, similar to the one in “Glitch” demo, but cheaper and less powerful (and as a bonus, it supports Arduino).

## Installation
If you want to try my project, this is the only part that you need.

| Hardware      |    Qty|
| ------------- | -----:|
| "blue pill" STM32F103C8 board | 1
| Resistors 470 Ohm | 3
| Resistors 680 Ohm | 3
| Breadboard | 1
| VGA connector (I used one from the old video card) | 1
| Jumper wires | 15
| ST-Link v2 or clone | 1

Software (under Windows): Install [Visual Studio Code](https://code.visualstudio.com/), then [PlatformIO](http://docs.platformio.org/en/latest/ide/vscode.html) plug-in.

How to connect wires:

| PIN | Description | Connect To | Output |
| --- | ----------- | ---------- | ------ |
| PA0 | Red 1 | Resistor 470 Ohm | VGA red (1)
| PA1 | Red 2 | Resistor 680 Ohm | VGA red (1)
| PA2 | Green 1 | Resistor 470 Ohm | VGA green (2)
| PA3 | Green 2 | Resistor 680 Ohm | VGA green (2)
| PA4 | Blue 1 | Resistor 470 Ohm | VGA blue (3)
| PA5 | Blue 2 | Resistor 680 Ohm | VGA blue (3)
| B0 | HSync | | VGA HSync (13)
| B6 | VSync | | VGA VSync (14)
| G | Ground | | VGA Ground (5,6,7,8,10)

