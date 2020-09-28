# Teensy Gameboy

[![build-badge]](https://github.com/blazer82/gb.teensy/actions?workflow=build)
[![cpu-instrs-badge]](https://github.com/blazer82/gb.teensy/actions?workflow=cpu_instrs)

This is a work-in-progress Gameboy emulator for the Teensy platform. It requires some custom hardware and wiring in order to work properly.

![Breadboard Gameboy](https://raw.githubusercontent.com/blazer82/gb.teensy/master/assets/breadboard-gameboy.jpg)

## Features

- Runs Tetris
- Passes Blargg's CPU instruction tests
- Audio output (4 separate channels)

## Dependencies

The project uses a [Teensy 4.1](https://www.pjrc.com/store/teensy41.html) as its main processing unit and storage device (SD card). 

The [FT81x Arduino Driver](https://github.com/blazer82/FT81x_Arduino_Driver) is used as a display system.

Additional wiring is required for audio output and button input.

## Wiring

### Display

For display wiring please refer to [this guide](https://blazer82.github.io/FT81x_Arduino_Driver/) for the FT81x Arduino Driver.

### Buttons

Button wiring is straight forward and doesn't require any pull-up resistors:

![Button Wiring](https://raw.githubusercontent.com/blazer82/gb.teensy/master/assets/button-wiring.png)

### Audio

For proper audio output some filter and amplifier circuitry is required:

![Audio Wiring](https://raw.githubusercontent.com/blazer82/gb.teensy/master/assets/audio-wiring.png)

_Tip: Audio output is most authentic with a Gameboy replacement speaker._

## Running and Loading a Game

Game ROMs are not included with this repository and have to be provided via SD card! Make sure the SD card is formatted using the **FAT32** filesystem using **Master Boot Record** partitioning style. File names should follow the [8.3 MS-DOS style](https://en.wikipedia.org/wiki/8.3_filename).

As of now, the ROM to be loaded has to be specified inside `src/main.cpp`, e.g:

```
Cartridge::begin("tetris.gb");
```

## Contributing

Any contribution is welcome! Please have a look at open issues and pull requests or create your own.

## Credits

Substantial contributions have been made by:

- [@granthaack](https://github.com/granthaack)

The code builds upon the following libraries:

- [Arduino](https://github.com/arduino/Arduino)
- [Teensyduino](https://www.pjrc.com/teensy/teensyduino.html)
- [TeensyTimerTool](https://github.com/luni64/TeensyTimerTool)

[build-badge]: https://github.com/blazer82/gb.teensy/workflows/build/badge.svg
[cpu-instrs-badge]: https://github.com/blazer82/gb.teensy/workflows/cpu_instrs/badge.svg
