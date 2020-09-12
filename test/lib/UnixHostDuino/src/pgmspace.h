/*
 * Copyright (c) 2019 Brian T. Park
 * MIT License
 */

/**
 * @file pgmspace.h
 *
 * A version of the <avr/pgmspace.h> file in the Arduino environment with
 * enough mappings to allow AUnit tests to compile and run on Linux or MacOS.
 */

#pragma once

#include <string.h>

#define PGM_P      const char*
#define PGM_VOID_P const void*
#define PSTR(str)  (str)

#define PROGMEM

#define pgm_read_byte(p)  (*(const uint8_t*)(p))
#define pgm_read_word(p)  (*(const uint16_t*)(p))
#define pgm_read_dword(p) (*(const uint32_t*)(p))
#define pgm_read_float(p) (*(const float*)(p))
#define pgm_read_ptr(p)   (*(const void* const*)(p))

#define strlen_P     strlen
#define strcat_P     strcat
#define strcpy_P     strcpy
#define strncpy_P    strncpy
#define strcmp_P     strcmp
#define strncmp_P    strncmp
#define strcasecmp_P strcasecmp
#define strchr_P     strchr
#define strrchr_P    strrchr
