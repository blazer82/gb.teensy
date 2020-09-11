#pragma once

#include <Arduino.h>

#define SD_CHIP_SELECT_PIN 1
#define BUILTIN_SDCARD     1

#define FILE_READ  0
#define FILE_WRITE 1

class File {
   public:
    // File(SdFile f, const char *name);  // wraps an underlying SdFile
    File(void) {}   // 'empty' constructor
    ~File(void) {}  // destructor
    virtual size_t write(uint8_t) { return 0; }
    virtual size_t write(const uint8_t *buf, size_t size) { return size; }
    virtual int read() { return 0; }
    virtual int peek() { return 0; };
    virtual int available() { return 0; }
    virtual void flush() {}
    int read(void *buf, uint16_t nbyte) { return 0; }
    bool seek(uint32_t pos) { return true; }
    uint32_t position() { return 0; }
    uint32_t size() { return 0; }
    void close() {}
    operator bool() { return true; }
    char *name();

    bool isDirectory(void) { return false; }
    File openNextFile(uint8_t mode = FILE_READ) {
        const File file;
        return file;
    }
    void rewindDirectory(void) {}

    // using Print::write;
};

class SDClass {
   private:
   public:
    // This needs to be called to set up the connection to the SD card
    // before other methods are used.
    bool begin(uint8_t csPin = SD_CHIP_SELECT_PIN) { return true; }

    // Open the specified file/directory with the supplied mode (e.g. read or
    // write, etc). Returns a File object for interacting with the file.
    // Note that currently only one file can be open at a time.
    File open(const char *filename, uint8_t mode = FILE_READ) {
        const File file;
        return file;
    }

    // Methods to determine if the requested file path exists.
    bool exists(const char *filepath) { return true; }

    // Create the requested directory heirarchy--if intermediate directories
    // do not exist they will be created.
    bool mkdir(const char *filepath) { return true; }

    // Delete the file.
    bool remove(const char *filepath) { return true; }

    bool rmdir(const char *filepath) { return true; }
};

extern SDClass SD;
