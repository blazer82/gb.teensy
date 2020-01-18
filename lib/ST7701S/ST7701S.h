#pragma once

#define ST7701S_NOP         0x00
#define ST7701S_SWRESET     0x01    // Software Reset
#define ST7701S_RDDID       0x04    // Read Display ID
#define ST7701S_RDNUMED     0x05    // Read Number of Errors on DSI
#define ST7701S_RDRED       0x06    // Read the first pixel of Red Color
#define ST7701S_RDGREEN     0x07    // Read the first pixel of Green Color
#define ST7701S_RDBLUE      0x08    // Read the first pixel of Blue Color
#define ST7701S_RDDPM       0x0A    // Read Display Power Mode
#define ST7701S_RDDMADCTL   0x0B    // Read Display MADCTL
#define ST7701S_RDDCOLMOD   0x0C    // Read Display Pixel Format
#define ST7701S_RDDIM       0x0D    // Read Display Image Mode
#define ST7701S_RDDSM       0x0E    // Read Display Signal Mode
#define ST7701S_RDDSDR      0x0F    // Read Display Self-Diagnostic Result
#define ST7701S_SLPIN       0x10    // Sleep in
#define ST7701S_SLPOUT      0x11    // Sleep Out
#define ST7701S_PTLON       0x12    // Partial Display Mode On
#define ST7701S_NORON       0x13    // Normal Display Mode On
#define ST7701S_INVOFF      0x20    // Display Inversion Off
#define ST7701S_INVON       0x21    // Display Inversion On
#define ST7701S_ALLPOFF     0x22    // All Pixel Off
#define ST7701S_ALLPON      0x23    // All Pixel On
#define ST7701S_GAMSET      0x26    // Gamma Set
#define ST7701S_DISPOFF     0x28    // Display Off
#define ST7701S_DISPON      0x29    // Display On
#define ST7701S_TEOFF       0x34    // Tearing Effect Line Off
#define ST7701S_TEON        0x35    // Tearing Effect Line On
#define ST7701S_MADCTL      0x36    // Display data access control
#define ST7701S_IDMOFF      0x38    // Idle Mode Off
#define ST7701S_IDMON       0x39    // Idle Mode On
#define ST7701S_COLMOD      0x3A    // Interface Pixel Format
#define ST7701S_GSL         0x45    // Get Scan Line
#define ST7701S_WRDISBV     0x51    // Write Display Brightness
#define ST7701S_RDDISBV     0x52    // Read Display Brightness Value
#define ST7701S_WRCTRLD     0x53    // Write CTRL Display
#define ST7701S_RDCTRLD     0x54    // Read CTRL Value Display
#define ST7701S_WRCACE      0x55    // Write Content Adaptive Brightness Control and Color Enhancement
#define ST7701S_RDCABC      0x56    // Read Content Adaptive Brightness Control
#define ST7701S_WRCABCMB    0x5E    // Write CABC Minimum Brightness
#define ST7701S_RDCABCMB    0x5F    // Read CABC Minimum Brightness
#define ST7701S_RDABCSDR    0x68    // Read Automatic Brightness Control Self-Diagnostic Result
#define ST7701S_RDBWLB      0x70    // Read Black/White Low Bits
#define ST7701S_RDBKX       0x71    // Read Bkx
#define ST7701S_RDBKY       0x72    // Read Bky
#define ST7701S_RDWX        0x73    // Read Wx
#define ST7701S_RDWY        0x74    // Read Wy
#define ST7701S_RDRGLB      0x75    // Read Red/Green Low Bits
#define ST7701S_RDRX        0x76    // Read Rx
#define ST7701S_RDRY        0x77    // Read Ry
#define ST7701S_RDGX        0x78    // Read Gx
#define ST7701S_RDGY        0x79    // Read Gy
#define ST7701S_RDBALB      0x7A    // Read Blue/A Color Low Bits
#define ST7701S_RDBX        0x7B    // Read Bx
#define ST7701S_RDBY        0x7C    // Read By
#define ST7701S_RDAX        0x7D    // Read Ax
#define ST7701S_RDAY        0x7E    // Read Ay
#define ST7701S_RDDDBS      0xA1    // Read DDB Start
#define ST7701S_RDDDBC      0xA8    // Read DDB Continue
#define ST7701S_RDFCS       0xAA    // Read First Checksum
#define ST7701S_RDCCS       0xAF    // Read Continue Checksum
#define ST7701S_RDID1       0xDA    // Read ID1
#define ST7701S_RDID2       0xDB    // Read ID2
#define ST7701S_RDID3       0xDC    // Read ID3
