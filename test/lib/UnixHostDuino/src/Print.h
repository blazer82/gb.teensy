/*
  Print.h - Base class that provides print() and println()
  Copyright (c) 2008 David A. Mellis.  All right reserved.
  Modified by Brian T. Park 2019.

  This library is free software; you can redistribute it and/or
  modify it under the terms of the GNU Lesser General Public
  License as published by the Free Software Foundation; either
  version 2.1 of the License, or (at your option) any later version.

  This library is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
  Lesser General Public License for more details.

  You should have received a copy of the GNU Lesser General Public
  License along with this library; if not, write to the Free Software
  Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
*/

#pragma once

#include <inttypes.h>
#include <math.h>
#include <stdarg.h>
#include <stdio.h>  // for size_t
#include <stdlib.h>
#include <string.h>  // strlen()

#include "Print.h"
#include "Printable.h"
// #include "WString.h"
#include "pgmspace.h"

#define DEC 10
#define HEX 16
#define OCT 8
#ifdef BIN  // Prevent warnings if BIN is previously defined in "iotnx4.h" or similar
#undef BIN
#endif
#define BIN 2

// Size of the internal printf() buffer
#define PRINTF_BUFFER_SIZE 250

class Print {
   private:
    int write_error;
    size_t printNumber(unsigned long n, uint8_t base) {
        char buf[8 * sizeof(long) + 1];  // Assumes 8-bit chars plus zero byte.
        char *str = &buf[sizeof(buf) - 1];

        *str = '\0';

        // prevent crash if called with base == 1
        if (base < 2) base = 10;

        do {
            char c = n % base;
            n /= base;

            *--str = c < 10 ? c + '0' : c + 'A' - 10;
        } while (n);

        return write(str);
    }
    size_t printFloat(double number, uint8_t digits) {
        size_t n = 0;

        if (isnan(number)) return print("nan");
        if (isinf(number)) return print("inf");
        if (number > 4294967040.0) return print("ovf");   // constant determined empirically
        if (number < -4294967040.0) return print("ovf");  // constant determined empirically

        // Handle negative numbers
        if (number < 0.0) {
            n += print('-');
            number = -number;
        }

        // Round correctly so that print(1.999, 2) prints as "2.00"
        double rounding = 0.5;
        for (uint8_t i = 0; i < digits; ++i) rounding /= 10.0;

        number += rounding;

        // Extract the integer part of the number and print it
        unsigned long int_part = (unsigned long)number;
        double remainder = number - (double)int_part;
        n += print(int_part);

        // Print the decimal point, but only if there are digits beyond
        if (digits > 0) {
            n += print('.');
        }

        // Extract digits from the remainder one at a time
        while (digits-- > 0) {
            remainder *= 10.0;
            unsigned int toPrint = (unsigned int)(remainder);
            n += print(toPrint);
            remainder -= toPrint;
        }

        return n;
    }

   protected:
    void setWriteError(int err = 1) { write_error = err; }

   public:
    Print() : write_error(0) {}

    int getWriteError() { return write_error; }
    void clearWriteError() { setWriteError(0); }

    virtual size_t write(uint8_t) = 0;
    size_t write(const char *str) {
        if (str == NULL) return 0;
        return write((const uint8_t *)str, strlen(str));
    }
    virtual size_t write(const uint8_t *buffer, size_t size) {
        size_t n = 0;
        while (size--) {
            if (write(*buffer++))
                n++;
            else
                break;
        }
        return n;
    }
    size_t write(const char *buffer, size_t size) { return write((const uint8_t *)buffer, size); }

    // default to zero, meaning "a single write may block"
    // should be overriden by subclasses with buffering
    virtual int availableForWrite() { return 0; }

    /*size_t print(const __FlashStringHelper *ifsh) {
        PGM_P p = reinterpret_cast<PGM_P>(ifsh);
        size_t n = 0;
        while (1) {
            unsigned char c = pgm_read_byte(p++);
            if (c == 0) break;
            if (write(c))
                n++;
            else
                break;
        }
        return n;
    }*/
    // size_t print(const String &s) { return write(s.c_str(), s.length()); }
    size_t print(const char str[]) { return write(str); }
    size_t print(char c) { return write(c); }
    size_t print(unsigned char b, int base = DEC) { return print((unsigned long)b, base); }
    size_t print(int n, int base = DEC) { return print((long)n, base); }
    size_t print(unsigned int n, int base = DEC) { return print((unsigned long)n, base); }
    size_t print(long n, int base = DEC) {
        if (base == 0) {
            return write(n);
        } else if (base == 10) {
            if (n < 0) {
                int t = print('-');
                n = -n;
                return printNumber(n, 10) + t;
            }
            return printNumber(n, 10);
        } else {
            return printNumber(n, base);
        }
    }
    size_t print(unsigned long n, int base = DEC) {
        if (base == 0)
            return write(n);
        else
            return printNumber(n, base);
    }
    size_t print(double n, int digits = 2) { return printFloat(n, digits); }
    size_t print(const Printable &x) { return x.printTo(*this); }

    /*size_t Print::println(const __FlashStringHelper *ifsh) {
        size_t n = print(ifsh);
        n += println();
        return n;
    }*/
    /*size_t println(const String &s) {
        size_t n = print(s);
        n += println();
        return n;
    }*/
    size_t println(const char c[]) {
        size_t n = print(c);
        n += println();
        return n;
    }
    size_t println(char c) {
        size_t n = print(c);
        n += println();
        return n;
    }
    size_t println(unsigned char b, int base = DEC) {
        size_t n = print(b, base);
        n += println();
        return n;
    }
    size_t println(int num, int base = DEC) {
        size_t n = print(num, base);
        n += println();
        return n;
    }
    size_t println(unsigned int num, int base = DEC) {
        size_t n = print(num, base);
        n += println();
        return n;
    }
    size_t println(long num, int base = DEC) {
        size_t n = print(num, base);
        n += println();
        return n;
    }
    size_t println(unsigned long num, int base = DEC) {
        size_t n = print(num, base);
        n += println();
        return n;
    }
    size_t println(double num, int digits = 2) {
        size_t n = print(num, digits);
        n += println();
        return n;
    }
    size_t println(const Printable &x) {
        size_t n = print(x);
        n += println();
        return n;
    }
    size_t println() { return write("\r\n"); }

    // printf() extension supported by many microcontrollers including
    // Teensy, ESP8266 and ESP32 (but not AVR).
    size_t printf(const char *fmt, ...) {
        char buf[PRINTF_BUFFER_SIZE];
        va_list args;
        va_start(args, fmt);
        int status = vsnprintf(buf, PRINTF_BUFFER_SIZE, fmt, args);
        va_end(args);
        if (status >= 0) {
            buf[PRINTF_BUFFER_SIZE - 1] = '\0';
            size_t n = print(buf);
            return n;
        } else {
            return 0;
        }
    }

    virtual void flush() { /* Empty implementation for backward compatibility */
    }
};
