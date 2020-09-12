# Changelog

* Unreleased
* 0.3 (2020-08-20)
    * Add `#define` for various digital and analog pins: `D0`-`D19`, `A0`-`A9`.
    * Add stub for `analogRead()`.
    * Add `Print.printf()` provided by a significant number of Arduino boards
      (ESP8266, ESP32, Teensy, etc).
* 0.2.1 (2020-05-02)
    * Add `MORE_CLEAN`, an optional user-provided make-target to do additional
      cleaning, such as removing directories created by code-generators.
* 0.2 (2020-03-21)
    * Mark `main()` as a weak reference (supported by g++ and clang++) so that
      UniHostDuino can be used as a library in programs that already provide a
      `main()` function. (Thanks to Max Prokhorov mcspr@).
    * Replace `ARDUINO_LIB_DIR` with `ARDUINO_LIB_DIRS` which supports
      multiple external library locations, in addition to siblings of the
      UnixHostDuino directory. The `ARDUINO_LIB_DIR` is no longer supported.
* 0.1.3 (2019-11-21)
    * Add 'Installation' section to the README.md.
    * Add `UNIX_HOST_DUINO` macro symbol  to the CPPFLAGS to allow conditional
      code that activates only when UnixHostDuino is used.
* 0.1.2 (2019-09-04)
    * Implement `StdioSerial::flush()` to enable `Serial.flush()`.
* 0.1.1 (2019-08-14)
    * If the STDIN is not a real tty, continue without putting it into raw mode
      or exiting with an error. This allows unit tests inside continuous build
      frameworks.
* 0.1 (2019-07-31)
    * Split from `AUnit` and renamed from `unitduino` to `UnixHostDuino`.
    * Add `UNIT_HOST_DUINO` macro.
    * Add `FPSTR()` macro for compatibilty with ESP8266 and ESP32.
    * Add `LED_BUILTIN` macro.
