#pragma once

#include <stdint.h>

#define FT81x_COLOR_RGB(r, g, b) (((uint32_t)(r) << 16) | ((uint32_t)(g) << 8) | (uint32_t)(b))  ///< Color from RGB values

#define FT81x_ROTATE_LANDSCAPE                   0  ///< Use with setRotation() to rotate screen to landscape
#define FT81x_ROTATE_LANDSCAPE_INVERTED          1  ///< Use with setRotation() to invert and rotate screen to landscape
#define FT81x_ROTATE_PORTRAIT                    2  ///< Use with setRotation() to rotate screen to portrait
#define FT81x_ROTATE_PORTRAIT_INVERTED           3  ///< Use with setRotation() to invert and rotate screen to portrait
#define FT81x_ROTATE_LANDSCAPE_MIRRORED          4  ///< Use with setRotation() to mirror and rotate screen to landscape
#define FT81x_ROTATE_LANDSCAPE_INVERTED_MIRRORED 5  ///< Use with setRotation() to invert, mirror and rotate screen to landscape
#define FT81x_ROTATE_PORTRAIT_MIRRORED           6  ///< Use with setRotation() to mirror and rotate screen to portrait
#define FT81x_ROTATE_PORTRAIT_INVERTED_MIRRORED  7  ///< Use with setRotation() to invert, mirror and rotate screen to portrait

#define FT81x_SPI_CLOCK_SPEED 24000000  ///< Default SPI clock speed for Teensy 3.0, 3.1/3.2, 3.5, 3.6, 4.0

#ifndef FT81x_SPI_SETTINGS
#define FT81x_SPI_SETTINGS SPISettings(FT81x_SPI_CLOCK_SPEED, MSBFIRST, SPI_MODE0)  ///< Default SPI settings, can be overwritten
#endif

#define FT81x_CMD_ACTIVE 0x000000  ///< Switch from Standby/Sleep/PWRDOWN modes to active mode.
#define FT81x_CMD_STANDBY \
    0x410000  ///< Put FT81x core to standby mode. Clock gate STANDBY off, PLL and Oscillator remain on (default). ACTIVE command to wake up.
#define FT81x_CMD_SLEEP 0x420000  ///< Put FT81x core to sleep mode. Clock gate SLEEP off, PLL and Oscillator off. ACTIVE command to wake up.
#define FT81x_CMD_PWRDOWN \
    0x430000  ///< Switch off 1.2V core voltage to the digital PWRDOWN core circuits. Clock, PLL and Oscillator off. SPI is alive. ACTIVE command to wake up.
#define FT81x_CMD_CLKEXT \
    0x440000  ///< Select PLL input from external crystal oscillator or external input clock. No effect if external clock is already selected, otherwise a
              ///< system reset will be generated.
#define FT81x_CMD_CLKINT \
    0x480000  ///< Select PLL input from internal relaxation oscillator (default). No effect if internal clock is already selected, otherwise a system reset
              ///< will be generated.
#define FT81x_CMD_CLKSEL 0x610000  ///< Set clock frequency and PLL range. This command will only be effective when the PLL is stopped (SLEEP mode).
#define FT81x_CMD_RST_PULSE \
    0x680000  ///< Send reset pulse to FT81x core. The behaviour is the same as POR except that settings done through SPI commands will not be affected.
#define FT81x_CMD_PINDRIVE 0x700000  ///< This will set the drive strength for various pins.

#define FT81x_REG_ID         0x302000  ///< Identification register, always reads as 7Ch
#define FT81x_REG_FRAMES     0x302004  ///< Frame counter, since reset
#define FT81x_REG_CLOCK      0x302008  ///< Clock cycles, since reset
#define FT81x_REG_FREQUENCY  0x30200C  ///< Main clock frequency (Hz)
#define FT81x_REG_RENDERMODE 0x302010  ///< Rendering mode: 0 = normal, 1 = single-line
#define FT81x_REG_SNAPY      0x302014  ///< Scanline select for RENDERMODE 1
#define FT81x_REG_SNAPSHOT   0x302018  ///< Trigger for RENDERMODE 1
#define FT81x_REG_SNAPFORMAT 0x30201C  ///< Pixel format for scanline readout
#define FT81x_REG_CPURESET   0x302020  ///< Graphics, audio and touch engines reset control. Bit2: audio, bit1: touch, bit0: graphics
#define FT81x_REG_TAP_CRC    0x302024  ///< Live video tap crc. Frame CRC is computed every DL SWAP.
#define FT81x_REG_TAP_MASK   0x302028  ///< Live video tap mask
#define FT81x_REG_HCYCLE     0x30202C  ///< Horizontal total cycle count
#define FT81x_REG_HOFFSET    0x302030  ///< Horizontal display start offset
#define FT81x_REG_HSIZE      0x302034  ///< Horizontal display pixel count
#define FT81x_REG_HSYNC0     0x302038  ///< Horizontal sync fall offset
#define FT81x_REG_HSYNC1     0x30203C  ///< Horizontal sync rise offset
#define FT81x_REG_VCYCLE     0x302040  ///< Vertical total cycle count
#define FT81x_REG_VOFFSET    0x302044  ///< Vertical display start offset
#define FT81x_REG_VSIZE      0x302048  ///< Vertical display line count
#define FT81x_REG_VSYNC0     0x30204C  ///< Vertical sync fall offset
#define FT81x_REG_VSYNC1     0x302050  ///< Vertical sync rise offset
#define FT81x_REG_DLSWAP     0x302054  ///< Display list swap control
#define FT81x_REG_ROTATE     0x302058  ///< Screen rotation control. Allow normal/mirrored/inverted for landscape or portrait orientation.
#define FT81x_REG_OUTBITS \
    0x30205C  ///< Output bit resolution, 3 bits each for R/G/B. Default is 6/6/6 bits for FT810/FT811, and 8/8/8 bits for FT812/FT813 (0b’000 means 8 bits)
#define FT81x_REG_DITHER           0x302060  ///< Output dither enable
#define FT81x_REG_SWIZZLE          0x302064  ///< Output RGB signal swizzle
#define FT81x_REG_CSPREAD          0x302068  ///< Output clock spreading enable
#define FT81x_REG_PCLK_POL         0x30206C  ///< PCLK polarity: 0 = output on PCLK rising edge, 1 = output on PCLK falling edge
#define FT81x_REG_PCLK             0x302070  ///< PCLK frequency divider, 0 = disable
#define FT81x_REG_TAG_X            0x302074  ///< Tag query X coordinate
#define FT81x_REG_TAG_Y            0x302078  ///< Tag query Y coordinate
#define FT81x_REG_TAG              0x30207C  ///< Tag query result
#define FT81x_REG_VOL_PB           0x302080  ///< Volume for playback
#define FT81x_REG_VOL_SOUND        0x302084  ///< Volume for synthesizer sound
#define FT81x_REG_SOUND            0x302088  ///< Sound effect select
#define FT81x_REG_PLAY             0x30208C  ///< Start effect playback
#define FT81x_REG_GPIO_DIR         0x302090  ///< Legacy GPIO pin direction, 0 = input, 1 = output
#define FT81x_REG_GPIO             0x302094  ///< Legacy GPIO read/write
#define FT81x_REG_GPIOX_DIR        0x302098  ///< Extended GPIO pin direction, 0 = input ,1 = output
#define FT81x_REG_GPIOX            0x30209C  ///< Extended GPIO read/write
#define FT81x_REG_INT_FLAGS        0x3020A8  ///< Interrupt flags, clear by read
#define FT81x_REG_INT_EN           0x3020AC  ///< Global interrupt enable, 1=enable
#define FT81x_REG_INT_MASK         0x3020B0  ///< Individual interrupt enable, 1=enable
#define FT81x_REG_PLAYBACK_START   0x3020B4  ///< Audio playback RAM start address
#define FT81x_REG_PLAYBACK_LENGTH  0x3020B8  ///< Audio playback sample length (bytes)
#define FT81x_REG_PLAYBACK_READPTR 0x3020BC  ///< Audio playback current read pointer
#define FT81x_REG_PLAYBACK_FREQ    0x3020C0  ///< Audio playback sampling frequency (Hz)
#define FT81x_REG_PLAYBACK_FORMAT  0x3020C4  ///< Audio playback format
#define FT81x_REG_PLAYBACK_LOOP    0x3020C8  ///< Audio playback loop enable
#define FT81x_REG_PLAYBACK_PLAY    0x3020CC  ///< Start audio playback
#define FT81x_REG_PWM_HZ           0x3020D0  ///< BACKLIGHT PWM output frequency (Hz)
#define FT81x_REG_PWM_DUTY         0x3020D4  ///< BACKLIGHT PWM output duty cycle 0=0%, 128=100%
#define FT81x_REG_MACRO_0          0x3020D8  ///< Display list macro command 0
#define FT81x_REG_MACRO_1          0x3020DC  ///< Display list macro command 1
#define FT81x_REG_CMD_READ         0x3020F8  ///< Command buffer read pointer
#define FT81x_REG_CMD_WRITE        0x3020FC  ///< Command buffer write pointer
#define FT81x_REG_CMD_DL           0x302100  ///< Command display list offset
#define FT81x_REG_BIST_EN          0x302174  ///< BIST memory mapping enable
#define FT81x_REG_TRIM             0x302180  ///< Internal relaxation clock trimming
#define FT81x_REG_ANA_COMP         0x302184  ///< Analogue control register
#define FT81x_REG_SPI_WIDTH        0x302188  ///< QSPI bus width setting, Bit [2]: extra dummy cycle on read, Bit [1:0]: bus width (0=1-bit, 1=2-bit, 2=4-bit)
#define FT81x_REG_DATESTAMP        0x302564  ///< Stamp date code
#define FT81x_REG_CMDB_SPACE       0x302574  ///< Command DL (bulk) space available
#define FT81x_REG_CMDB_WRITE       0x302578  ///< Command DL (bulk) write
#define FT81x_REG_MEDIAFIFO_READ   0x309014  ///< Read pointer for media FIFO in general purpose graphics RAM
#define FT81x_REG_MEDIAFIFO_WRITE  0x309018  ///< Write pointer for media FIFO in general purpose graphics RAM

#define FT81x_RAM_G         0x000000  ///< General purpose graphics RAM
#define FT81x_ROM_FONT      0x1E0000  ///< Font table and bitmap
#define FT81x_ROM_FONT_ADDR 0x2FFFFC  ///< Font table pointer address
#define FT81x_RAM_DL        0x300000  ///< Display List RAM
#define FT81x_RAM_REG       0x302000  ///< Registers
#define FT81x_RAM_CMD       0x308000  ///< Command buffer

#define FT81x_DLSWAP_LINE 0x1  ///< Graphics engine will render the screen immediately after current line is scanned out. It may cause tearing effect.
#define FT81x_DLSWAP_FRAME \
    0x2  ///< Graphics engine will render the screen immediately after current frame is scanned out. This is recommended in most of cases.

#define FT81x_BITMAP_LAYOUT_ARGB1555 0x1  ///< Bitmap pixel format ARGB1555
#define FT81x_BITMAP_LAYOUT_ARGB4    0x6  ///< Bitmap pixel format ARGB4
#define FT81x_BITMAP_LAYOUT_RGB565   0x7  ///< Bitmap pixel format RGB565

#define FT81x_BITMAP_SIZE_NEAREST  0x0  ///< Bitmap filtering mode: NEAREST
#define FT81x_BITMAP_SIZE_BILINEAR 0x1  ///< Bitmap filtering mode: BILINEAR. For bilinear filtered pixels, the drawing rate is reduced to 1⁄4 pixels per clock.

#define FT81x_OPT_3D        0x0000  ///< 3D effect (CMD_BUTTON,CMD_CLOCK,CMD_KEYS, CMD_GAUGE,CMD_SLIDER, CMD_DIAL, CMD_TOGGLE,CMD_PROGRESS, CMD_SCROLLBAR)
#define FT81x_OPT_FLAT      0x0100  ///< No 3D effect (CMD_BUTTON,CMD_CLOCK,CMD_KEYS, CMD_GAUGE,CMD_SLIDER, CMD_DIAL, CMD_TOGGLE,CMD_PROGRESS, CMD_SCROLLBAR)
#define FT81x_OPT_SIGNED    0x0100  ///< The number is treated as a 32 bit signed integer (CMD_NUMBER)
#define FT81x_OPT_CENTERX   0x0200  ///< Horizontally-centred style (CMD_KEYS,CMD_TEXT , CMD_NUMBER)
#define FT81x_OPT_CENTERY   0x0400  ///< Vertically centred style (CMD_KEYS,CMD_TEXT , CMD_NUMBER)
#define FT81x_OPT_CENTER    0x0600  ///< Horizontally and vertically centred style (CMD_KEYS,CMD_TEXT , CMD_NUMBER)
#define FT81x_OPT_RIGHTX    0x0800  ///< Right justified style (CMD_KEYS,CMD_TEXT , CMD_NUMBER)
#define FT81x_OPT_NOBACK    0x1000  ///< No background drawn (CMD_CLOCK, CMD_GAUGE)
#define FT81x_OPT_NOTICKS   0x2000  ///< No Ticks (CMD_CLOCK, CMD_GAUGE)
#define FT81x_OPT_NOHM      0x4000  ///< No hour and minute hands (CMD_CLOCK)
#define FT81x_OPT_NOPOINTER 0x4000  ///< No pointer (CMD_GAUGE)
#define FT81x_OPT_NOSECS    0x8000  ///< No second hands (CMD_CLOCK)
#define FT81x_OPT_NOHANDS   0xC000  ///< Nohands (CMD_CLOCK)

class FT81x {
   public:
    FT81x(int8_t cs1, int8_t cs2, int8_t dc) {}
    void begin() {}
    void clear(const uint32_t color) {}
    void drawCircle(const int16_t x, const int16_t y, const uint8_t size, const uint32_t color) {}
    void drawRect(const int16_t x, const int16_t y, const uint16_t width, const uint16_t height, const uint8_t cornerRadius, const uint32_t color) {}
    void drawLine(const int16_t x1, const int16_t y1, const int16_t x2, const int16_t y2, const uint8_t width, const uint32_t color) {}
    void beginLineStrip(const uint8_t width, const uint32_t color) {}
    void addVertex(const int16_t x, const int16_t y) {}
    void endLineStrip() {}
    void drawLetter(const int16_t x, const int16_t y, const uint8_t font, const uint32_t color, const uint8_t letter) {}
    void drawText(const int16_t x, const int16_t y, const uint8_t font, const uint32_t color, const uint16_t options, const char text[]) {}
    void drawBitmap(const uint32_t offset, const uint16_t x, const uint16_t y, const uint16_t width, const uint16_t height, const uint8_t scale) {}
    void drawSpinner(const int16_t x, const int16_t y, const uint16_t style, const uint16_t scale, const uint32_t color) {}
    void drawButton(const int16_t x, const int16_t y, const int16_t width, const int16_t height, const uint8_t font, const uint32_t textColor,
                    const uint32_t buttonColor, const uint16_t options, const char text[]) {}
    void drawClock(const int16_t x, const int16_t y, const int16_t radius, const uint32_t handsColor, const uint32_t backgroundColor, const uint16_t options,
                   const uint16_t hours, const uint16_t minutes, const uint16_t seconds) {}
    void drawGauge(const int16_t x, const int16_t y, const int16_t radius, const uint32_t handsColor, const uint32_t backgroundColor, const uint16_t options,
                   const uint8_t major, const uint8_t minor, const uint16_t value, const uint16_t range) {}
    void drawGradient(const int16_t x1, const int16_t y1, const uint32_t color1, const int16_t x2, const int16_t y2, const uint32_t color2) {}
    void beginDisplayList() {}
    void swapScreen() {}
    void waitForCommandBuffer() {}
    void setRotation(const uint8_t rotation) {}
    void writeGRAM(const uint32_t offset, const uint32_t size, const uint8_t data[]) {}
    void loadImage(const uint32_t offset, const uint32_t size, const uint8_t data[]) {}
};