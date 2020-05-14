/**
 * gb.teensy Emulation Software
 * Copyright (C) 2020  Raphael Stäbler
 * 
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 * 
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <https://www.gnu.org/licenses/>.
 **/

#include "CPU.h"
#include <time.h>
#include <Arduino.h>

#include "Memory.h"

/**
 * Debuging settings
 */

// #define HALT_AT_ZERO
// #define HALT_AFTER_CYCLE 100000000
// #define DEBUG_AFTER_CYCLE 0
// #define DEBUG_AFTER_PC PC_TIMER

/**
 * Registers
 */

uint16_t AF = 0x01B0; // AAAAAAAAZNHCxxxx
uint16_t BC = 0x0013; // BBBBBBBBCCCCCCCC
uint16_t DE = 0x00D8; // DDDDDDDDEEEEEEEE
uint16_t HL = 0x014D; // HHHHHHHHLLLLLLLL
uint16_t SP = 0xFFFE; // Stack Pointer
uint16_t PC = PC_START; // Program Counter

/**
 * Compiler macros
 */

// LD nn,n
#define LD_Nn_n(t, s) (((s) << 8) | ((t) & 0x00FF))
#define LD_nN_n(t, s) ((s) | ((t) & 0xFF00))

// LD r1,r2
#define LD_Nn_Nn(t, s) (((s) & 0xFF00) | ((t) & 0x00FF))
#define LD_Nn_nN(t, s) LD_Nn_n(t, s)
#define LD_nN_Nn(t, s) (((s) >> 8) | ((t) & 0xFF00))
#define LD_nN_nN(t, s) (((s) & 0x00FF) | ((t) & 0xFF00))

// AND n
#define AND_Nn_Nn(nn1, nn2) (((nn1) & (nn2) & 0xFF00) >> 8)
#define AND_Nn_nN(nn1, nn2) (((nn1) >> 8) & (nn2) & 0x00FF)

// OR n
#define OR_Nn_Nn(nn1, nn2) ((((nn1) | (nn2)) & 0xFF00) >> 8)
#define OR_Nn_nN(nn1, nn2) ((((nn1) >> 8) | (nn2)) & 0x00FF)

// XOR n
#define XOR_Nn_Nn(nn1, nn2) ((((nn1) ^ (nn2)) & 0xFF00) >> 8)
#define XOR_Nn_nN(nn1, nn2) ((((nn1) >> 8) ^ (nn2)) & 0x00FF)

// Flags
#define ZERO_V 0x80
#define SUB_V 0x40
#define HALF_V 0x20
#define CARRY_V 0x10
#define ZERO_F(nn) (nn & ZERO_V)
#define SUB_F(nn) (nn & SUB_V)
#define HALF_F(nn) (nn & HALF_V)
#define CARRY_F(nn) (nn & CARRY_V)
#define ZERO_S(nn) (((nn) == 0) ? ZERO_V : 0)
#define HALF_S(n1, n2) ((((n1) & 0x0F) + ((n2) & 0x0F) > 0x0F) << 5)
#define HALF_Sc(n1, n2, c) ((((n1) & 0x0F) + (((n2) & 0x0F) + c) > 0x0F) << 5)
#define HALF_Snn(nn1, nn2) ((((nn1) & 0x0FFF) + ((nn2) & 0x0FFF) > 0x0FFF) << 5)
#define CARRY_S(n, n1, n2) ((((n) < (n1)) | ((n) < (n2))) << 4)
#define CARRY_Sc(n, n1, n2, c) (((((n) - (c)) < (n1)) | (((n) - (c)) < (n2))) << 4)
#define HBORROW_S(n1, n2) ((((n1) & 0x0F) < ((n2) & 0x0F)) << 5)
#define HBORROW_Sc(n1, n2, c) ((((n1) & 0x0F) < (((n2) & 0x0F) + c)) << 5)
#define BORROW_S(n1, n2) (((n1) < (n2)) << 4)
#define BORROW_Sc(n1, n2, c) ((((n1) < (n2)) | ((n1) < ((n2) + (c)))) << 4)

/**
 * Variables
 */

// Virtual power
volatile bool CPU::cpuEnabled = false;

// Keep count of cycles
volatile uint64_t CPU::totalCycles = 0;

// Init OP
uint8_t op = 0x00;

// Init IME
bool IME = 0;

// Virtual HALT
bool halted = 0;

// Define benchmark and debugging options
double start, stop;

// IRQ control
uint8_t enableIRQ = 0, disableIRQ = 0;

// Divider interval
uint8_t divider = 0;

// Timer control
uint8_t timerA = 0, timerB = 0xFF;

// Debug variables
#ifdef DEBUG_AFTER_CYCLE
uint64_t debugAfterCycle = DEBUG_AFTER_CYCLE;
#endif

/**
 * Functions
 */

uint8_t CPU::readOp() {
    return Memory::readByte(PC++);
}

uint16_t CPU::readNn()
{
    uint8_t n1 = Memory::readByte(PC++);
    uint8_t n2 = Memory::readByte(PC++);
    return n1 | (n2 << 8);
}

void CPU::pushStack(const uint16_t data) {
    SP--;
    Memory::writeByte(SP, data >> 8);
    SP--;
    Memory::writeByte(SP, data & 0x00FF);
}

uint16_t CPU::popStack() {
    uint8_t n1 = Memory::readByte(SP);
    SP++;
    uint8_t n2 = Memory::readByte(SP);
    SP++;
    return (n2 << 8) | n1;
}

void dumpRegister() {
    Serial.printf("AF: %04x, BC: %04x, DE: %04x, HL: %04x, SP: %04x\n", AF, BC, DE, HL, SP);
}

void dumpStack() {
    for (uint16_t p = 0xCFFF; p >= SP; p--) {
        Serial.printf("%02x ", Memory::readByte(p));
    }
    Serial.printf("\n");
}

void stopAndRestart() {
    for(;;) {
        Serial.printf("Cycles: %llu\n", CPU::totalCycles);
        Serial.printf("Restarting in %d seconds...\n", 10);
        Serial.printf("Halting now.\n");
    }
}

void CPU::cpuStep() {
    uint8_t n, n1, n2, interrupt;
    int8_t sn;
    uint16_t nn, nn1, nn2;
    bool c;

    if (!cpuEnabled) return;

#ifdef HALT_AT_ZERO
    if (PC == 0) {
        Serial.printf("PC at %02x\n", PC);
        dumpRegister();
        stopAndRestart();
    }
#endif

#ifdef HALT_AFTER_CYCLE
    if (totalCycles > HALT_AFTER_CYCLE) {
        Serial.printf("0x8000 - 0x97FF (Tile Data):\n");
        for (uint16_t i = 0x8000; i < 0x97FF; i += 2) {
            Serial.printf("%02x-%02x ", Memory::readByte(i), Memory::readByte(i + 1));
        }
        Serial.printf("\n");
        Serial.printf("0x9800 - 0x9BFF (Background Map):\n");
        for (uint16_t i = 0x9800; i < 0x9BFF; i++) {
            Serial.printf("%02x ", Memory::readByte(i));
        }
        Serial.printf("\n");
        Serial.printf("0x9C00 - 0x9FFF (Background Maps):\n");
        for (uint16_t i = 0x9C00; i < 0x9FFF; i++) {
            Serial.printf("%02x ", Memory::readByte(i));
        }
        Serial.printf("\n");
        Serial.printf("0xFE00 - 0xFEA0 (OAM):\n");
        for (uint16_t i = 0xFE00; i < 0xFEA0; i += 4) {
            Serial.printf("%02x-%02x-%02x-%02x ", Memory::readByte(i), Memory::readByte(i + 1), Memory::readByte(i + 2), Memory::readByte(i + 3));
        }
        Serial.printf("\n");
        Serial.printf("0xFF40 (LCDC): %02x\n", Memory::readByte(0xFF40));
        Serial.printf("0xFF41 (STAT): %02x\n", Memory::readByte(0xFF41));
        stopAndRestart();
    }
#endif

    // Update timer
    if ((Memory::readByte(MEM_TIMER_CONTROL) & 0x04) == 0x04) {
        switch (Memory::readByte(MEM_TIMER_CONTROL) & 0x03)
        {
        case 3:
            timerB = totalCycles % 64;
            break;

        case 2:
            timerB = totalCycles % 16;
            break;
        
        case 1:
            timerB = totalCycles % 4;
            break;
        
        default:
            timerB = totalCycles % 250;
            break;
        }

        if (timerB == 0 || timerB < timerA) {
            Memory::writeByteInternal(MEM_TIMA, Memory::readByte(MEM_TIMA) + 1, true);

            if (Memory::readByte(MEM_TIMA) == 0) {
                Memory::writeByteInternal(MEM_TIMA, Memory::readByte(MEM_TMA), true);
                Memory::interrupt(IRQ_TIMER);
            }
        }
        timerA = timerB;
    }

    // Check for interrupts
    if (IME || halted) {
        interrupt = Memory::readByte(MEM_IRQ_FLAG) & Memory::readByte(MEM_IRQ_ENABLE);

        if ((interrupt & IRQ_VBLANK) == IRQ_VBLANK) {
            IME = 0;
            if (halted) {
                halted = 0;
            }
            else {
                Memory::writeByte(MEM_IRQ_FLAG, Memory::readByte(MEM_IRQ_FLAG) & (0xFF - IRQ_VBLANK));
                pushStack(PC);
                PC = PC_VBLANK;
            }
        }
        else if ((interrupt & IRQ_TIMER) == IRQ_TIMER) {
            IME = 0;
            if (halted) {
                halted = 0;
            }
            else {
                Memory::writeByte(MEM_IRQ_FLAG, Memory::readByte(MEM_IRQ_FLAG) & (0xFF - IRQ_TIMER));
                pushStack(PC);
                PC = PC_TIMER;
            }
        }
    }

    // Update divider register
    if (divider > (totalCycles % 61)) {
        divider = totalCycles % 61;
        Memory::writeByteInternal(MEM_DIVIDER, Memory::readByte(MEM_DIVIDER) + 1, true);
    }

    // Check if halted
    if (halted) {
        totalCycles += 1; // In order for the timer to work properly
        return;
    }

#ifdef DEBUG_AFTER_PC
    if (PC == DEBUG_AFTER_PC && debugAfterCycle == 0) {
        debugAfterCycle = totalCycles;
    }
#endif

    op = readOp();

#ifdef DEBUG_AFTER_CYCLE
    if (debugAfterCycle > 0 && totalCycles >= debugAfterCycle) {
        delay(20);
        Serial.printf("Cycle %llu: %02x at %04x - ", totalCycles, op, PC - 1);
        dumpRegister();

        /*for (uint16_t i = 0x8000; i <= 0x97FF; i++) {
            Serial.printf("%02x ", Memory::readByte(i));
        }

        Serial.printf("\n");
        stopAndRestart();*/
    }
#endif

    switch (op)
    {
    // NOP
    case 0x0:
        totalCycles += 1;
        break;

    // HALT
    case 0x76:
        halted = 1;
        totalCycles += 1;
        break;

    // LD nn,n
    case 0x06:
        BC = LD_Nn_n(BC, readOp());
        totalCycles += 2;
        break;
    case 0x0E:
        BC = LD_nN_n(BC, readOp());
        totalCycles += 2;
        break;
    case 0x16:
        DE = LD_Nn_n(DE, readOp());
        totalCycles += 2;
        break;
    case 0x1E:
        DE = LD_nN_n(DE, readOp());
        totalCycles += 2;
        break;
    case 0x26:
        HL = LD_Nn_n(HL, readOp());
        totalCycles += 2;
        break;
    case 0x2E:
        HL = LD_nN_n(HL, readOp());
        totalCycles += 2;
        break;

    // LD r1,r2
    case 0x7F:
        AF = LD_Nn_Nn(AF, AF);
        totalCycles += 1;
        break;
    case 0x78:
        AF = LD_Nn_Nn(AF, BC);
        totalCycles += 1;
        break;
    case 0x79:
        AF = LD_Nn_nN(AF, BC);
        totalCycles += 1;
        break;
    case 0x7A:
        AF = LD_Nn_Nn(AF, DE);
        totalCycles += 1;
        break;
    case 0x7B:
        AF = LD_Nn_nN(AF, DE);
        totalCycles += 1;
        break;
    case 0x7C:
        AF = LD_Nn_Nn(AF, HL);
        totalCycles += 1;
        break;
    case 0x7D:
        AF = LD_Nn_nN(AF, HL);
        totalCycles += 1;
        break;
    case 0x7E:
        AF = LD_Nn_nN(AF, Memory::readByte(HL));
        totalCycles += 2;
        break;
    case 0x40:
        BC = LD_Nn_Nn(BC, BC);
        totalCycles += 1;
        break;
    case 0x41:
        BC = LD_Nn_nN(BC, BC);
        totalCycles += 1;
        break;
    case 0x42:
        BC = LD_Nn_Nn(BC, DE);
        totalCycles += 1;
        break;
    case 0x43:
        BC = LD_Nn_nN(BC, DE);
        totalCycles += 1;
        break;
    case 0x44:
        BC = LD_Nn_Nn(BC, HL);
        totalCycles += 1;
        break;
    case 0x45:
        BC = LD_Nn_nN(BC, HL);
        totalCycles += 1;
        break;
    case 0x46:
        BC = LD_Nn_nN(BC, Memory::readByte(HL));
        totalCycles += 2;
        break;
    case 0x48:
        BC = LD_nN_Nn(BC, BC);
        totalCycles += 1;
        break;
    case 0x49:
        BC = LD_nN_nN(BC, BC);;
        totalCycles += 1;
        break;
    case 0x4A:
        BC = LD_nN_Nn(BC, DE);
        totalCycles += 1;
        break;
    case 0x4B:
        BC = LD_nN_nN(BC, DE);
        totalCycles += 1;
        break;
    case 0x4C:
        BC = LD_nN_Nn(BC, HL);
        totalCycles += 1;
        break;
    case 0x4D:
        BC = LD_nN_nN(BC, HL);
        totalCycles += 1;
        break;
    case 0x4E:
        BC = LD_nN_nN(BC, Memory::readByte(HL));
        totalCycles += 2;
        break;
    case 0x50:
        DE = LD_Nn_Nn(DE, BC);
        totalCycles += 1;
        break;
    case 0x51:
        DE = LD_Nn_nN(DE, BC);
        totalCycles += 1;
        break;
    case 0x52:
        DE = LD_Nn_Nn(DE, DE);
        totalCycles += 1;
        break;
    case 0x53:
        DE = LD_Nn_nN(DE, DE);
        totalCycles += 1;
        break;
    case 0x54:
        DE = LD_Nn_Nn(DE, HL);
        totalCycles += 1;
        break;
    case 0x55:
        DE = LD_Nn_nN(DE, HL);
        totalCycles += 1;
        break;
    case 0x56:
        DE = LD_Nn_nN(DE, Memory::readByte(HL));
        totalCycles += 2;
        break;
    case 0x58:
        DE = LD_nN_Nn(DE, BC);
        totalCycles += 1;
        break;
    case 0x59:
        DE = LD_nN_nN(DE, BC);
        totalCycles += 1;
        break;
    case 0x5A:
        DE = LD_nN_Nn(DE, DE);
        totalCycles += 1;
        break;
    case 0x5B:
        DE = LD_nN_nN(DE, DE);
        totalCycles += 1;
        break;
    case 0x5C:
        DE =LD_nN_Nn(DE, HL);
        totalCycles += 1;
        break;
    case 0x5D:
        DE = LD_nN_nN(DE, HL);
        totalCycles += 1;
        break;
    case 0x5E:
        DE = LD_nN_nN(DE, Memory::readByte(HL));
        totalCycles += 2;
        break;
    case 0x60:
        HL = LD_Nn_Nn(HL, BC);
        totalCycles += 1;
        break;
    case 0x61:
        HL = LD_Nn_nN(HL, BC);
        totalCycles += 1;
        break;
    case 0x62:
        HL = LD_Nn_Nn(HL, DE);
        totalCycles += 1;
        break;
    case 0x63:
        HL = LD_Nn_nN(HL, DE);
        totalCycles += 1;
        break;
    case 0x64:
        HL = LD_Nn_Nn(HL, HL);
        totalCycles += 1;
        break;
    case 0x65:
        HL = LD_Nn_nN(HL, HL);
        totalCycles += 1;
        break;
    case 0x66:
        HL = LD_Nn_nN(HL, Memory::readByte(HL));
        totalCycles += 2;
        break;
    case 0x68:
        HL = LD_nN_Nn(HL, BC);
        totalCycles += 1;
        break;
    case 0x69:
        HL = LD_nN_nN(HL, BC);
        totalCycles += 1;
        break;
    case 0x6A:
        HL = LD_nN_Nn(HL, DE);
        totalCycles += 1;
        break;
    case 0x6B:
        HL = LD_nN_nN(HL, DE);
        totalCycles += 1;
        break;
    case 0x6C:
        HL = LD_nN_Nn(HL, HL);
        totalCycles += 1;
        break;
    case 0x6D:
        HL = LD_nN_nN(HL, HL);
        totalCycles += 1;
        break;
    case 0x6E:
        HL = LD_nN_nN(HL, Memory::readByte(HL));
        totalCycles += 2;
        break;
    case 0x70:
        Memory::writeByte(HL, BC >> 8);
        totalCycles += 2;
        break;
    case 0x71:
        Memory::writeByte(HL, BC & 0x00FF);
        totalCycles += 2;
        break;
    case 0x72:
        Memory::writeByte(HL, DE >> 8);
        totalCycles += 2;
        break;
    case 0x73:
        Memory::writeByte(HL, DE & 0x00FF);
        totalCycles += 2;
        break;
    case 0x74:
        Memory::writeByte(HL, HL >> 8);
        totalCycles += 2;
        break;
    case 0x75:
        Memory::writeByte(HL, HL & 0x00FF);
        totalCycles += 2;
        break;
    case 0x36:
        Memory::writeByte(HL, readOp());
        totalCycles += 3;
        break;

    // LD A,n
    case 0x0A:
        AF = LD_Nn_nN(AF, Memory::readByte(BC));
        totalCycles += 2;
        break;
    case 0x1A:
        AF = LD_Nn_nN(AF, Memory::readByte(DE));
        totalCycles += 2;
        break;
    case 0xFA:
        AF = LD_Nn_nN(AF, Memory::readByte(readNn()));
        totalCycles += 4;
        break;
    case 0x3E:
        AF = LD_Nn_nN(AF, readOp());
        totalCycles += 2;
        break;

    // LD n,A
    case 0x47:
        BC = LD_Nn_Nn(BC, AF);
        totalCycles += 1;
        break;
    case 0x4F:
        BC = LD_nN_Nn(BC, AF);
        totalCycles += 1;
        break;
    case 0x57:
        DE = LD_Nn_Nn(DE, AF);
        totalCycles += 1;
        break;
    case 0x5F:
        DE = LD_nN_Nn(DE, AF);
        totalCycles += 1;
        break;
    case 0x67:
        HL = LD_Nn_Nn(HL, AF);
        totalCycles += 1;
        break;
    case 0x6F:
        HL = LD_nN_Nn(HL, AF);
        totalCycles += 1;
        break;
    case 0x02:
        Memory::writeByte(BC, AF >> 8);
        totalCycles += 2;
        break;
    case 0x12:
        Memory::writeByte(DE, AF >> 8);
        totalCycles += 2;
        break;
    case 0x77:
        Memory::writeByte(HL, AF >> 8);
        totalCycles += 2;
        break;
    case 0xEA:
        Memory::writeByte(readNn(), AF >> 8);
        totalCycles += 4;
        break;

    // LD A,(C)
    case 0xF2:
        AF = LD_Nn_n(AF, Memory::readByte(BC | 0xFF00));
        totalCycles += 8;
        break;

    // LD (C),A
    case 0xE2:
        Memory::writeByte(BC | 0xFF00, AF >> 8);
        totalCycles += 8;
        break;

    // LDH (n),A
    case 0xE0:
        Memory::writeByte(0xFF00 + readOp(), AF >> 8);
        totalCycles += 3;
        break;

    // LDH A,(n)
    case 0xF0:
        AF = LD_Nn_n(AF, Memory::readByte(0xFF00 + readOp()));
        totalCycles += 3;
        break;

    // LDD A,(HL)
    case 0x3A:
        AF = LD_Nn_n(AF, Memory::readByte(HL));
        HL--;
        totalCycles += 2;
        break;

    // LDD (HL),A
    case 0x32:
        Memory::writeByte(HL, AF >> 8);
        HL--;
        totalCycles += 2;
        break;

    // LDI (HL),A
    case 0x22:
        Memory::writeByte(HL, AF >> 8);
        HL++;
        totalCycles += 2;
        break;

    // LDI A,(HL)
    case 0x2A:
        AF = LD_Nn_n(AF, Memory::readByte(HL));
        HL++;
        totalCycles += 2;
        break;

    // LD n,nn
    case 0x01:
        BC = readNn();
        totalCycles += 3;
        break;
    case 0x11:
        DE = readNn();
        totalCycles += 3;
        break;
    case 0x21:
        HL = readNn();
        totalCycles += 3;
        break;
    case 0x31:
        SP = readNn();
        totalCycles += 3;
        break;

    // LD SP,HL
    case 0xF9:
        SP = HL;
        totalCycles += 2;
        break;

    // LDHL SP,n
    case 0xF8:
        sn = (int8_t)readOp();
        HL = SP + sn;
        AF = LD_nN_n(AF, HALF_S(SP, sn) | CARRY_S(HL & 0xFF, SP & 0xFF, sn));
        totalCycles += 3;
        break;

    // LD (nn),SP
    case 0x08:
        nn = readNn();
        Memory::writeByte(nn, SP & 0xFF);
        Memory::writeByte(nn + 1, SP >> 8);
        totalCycles += 5;
        break;

    // PUSH nn
    case 0xF5:
        pushStack(AF);
        totalCycles += 4;
        break;
    case 0xC5:
        pushStack(BC);
        totalCycles += 4;
        break;
    case 0xD5:
        pushStack(DE);
        totalCycles += 4;
        break;
    case 0xE5:
        pushStack(HL);
        totalCycles += 4;
        break;

    // POP nn
    case 0xF1:
        AF = popStack() & 0xFFF0;
        totalCycles += 3;
        break;
    case 0xC1:
        BC = popStack();
        totalCycles += 3;
        break;
    case 0xD1:
        DE = popStack();
        totalCycles += 3;
        break;
    case 0xE1:
        HL = popStack();
        totalCycles += 3;
        break;

    // ADD A,n
    case 0x87:
        n1 = AF >> 8;
        n2 = AF >> 8;
        AF = LD_Nn_n(AF, n1 + n2);
        n = AF >> 8;
        AF = LD_nN_n(AF, ZERO_S(AF & 0xFF00) | HALF_S(n1, n2) | CARRY_S(n, n1, n2));
        totalCycles += 1;
        break;
    case 0x80:
        n1 = AF >> 8;
        n2 = BC >> 8;
        AF = LD_Nn_n(AF, n1 + n2);
        n = AF >> 8;
        AF = LD_nN_n(AF, ZERO_S(AF & 0xFF00) | HALF_S(n1, n2) | CARRY_S(n, n1, n2));
        totalCycles += 1;
        break;
    case 0x81:
        n1 = AF >> 8;
        n2 = BC & 0x00FF;
        AF = LD_Nn_n(AF, n1 + n2);
        n = AF >> 8;
        AF = LD_nN_n(AF, ZERO_S(AF & 0xFF00) | HALF_S(n1, n2) | CARRY_S(n, n1, n2));
        totalCycles += 1;
        break;
    case 0x82:
        n1 = AF >> 8;
        n2 = DE >> 8;
        AF = LD_Nn_n(AF, n1 + n2);
        n = AF >> 8;
        AF = LD_nN_n(AF, ZERO_S(AF & 0xFF00) | HALF_S(n1, n2) | CARRY_S(n, n1, n2));
        totalCycles += 1;
        break;
    case 0x83:
        n1 = AF >> 8;
        n2 = DE & 0x00FF;
        AF = LD_Nn_n(AF, n1 + n2);
        n = AF >> 8;
        AF = LD_nN_n(AF, ZERO_S(AF & 0xFF00) | HALF_S(n1, n2) | CARRY_S(n, n1, n2));
        totalCycles += 1;
        break;
    case 0x84:
        n1 = AF >> 8;
        n2 = HL >> 8;
        AF = LD_Nn_n(AF, n1 + n2);
        n = AF >> 8;
        AF = LD_nN_n(AF, ZERO_S(AF & 0xFF00) | HALF_S(n1, n2) | CARRY_S(n, n1, n2));
        totalCycles += 1;
        break;
    case 0x85:
        n1 = AF >> 8;
        n2 = HL & 0x00FF;
        AF = LD_Nn_n(AF, n1 + n2);
        n = AF >> 8;
        AF = LD_nN_n(AF, ZERO_S(AF & 0xFF00) | HALF_S(n1, n2) | CARRY_S(n, n1, n2));
        totalCycles += 1;
        break;
    case 0x86:
        n1 = AF >> 8;
        n2 = Memory::readByte(HL);
        AF = LD_Nn_n(AF, n1 + n2);
        n = AF >> 8;
        AF = LD_nN_n(AF, ZERO_S(AF & 0xFF00) | HALF_S(n1, n2) | CARRY_S(n, n1, n2));
        totalCycles += 2;
        break;
    case 0xC6:
        n1 = AF >> 8;
        n2 = readOp();
        AF = LD_Nn_n(AF, n1 + n2);
        n = AF >> 8;
        AF = LD_nN_n(AF, ZERO_S(AF & 0xFF00) | HALF_S(n1, n2) | CARRY_S(n, n1, n2));
        totalCycles += 2;
        break;

    // ADC A,n
    case 0x8F:
        n1 = AF >> 8;
        n2 = AF >> 8;
        c = CARRY_F(AF) >> 4;
        AF = LD_Nn_n(AF, n1 + n2 + c);
        n = AF >> 8;
        AF = LD_nN_n(AF, ZERO_S(AF & 0xFF00) | HALF_Sc(n1, n2, c) | CARRY_Sc(n, n1, n2, c));
        totalCycles += 1;
        break;
    case 0x88:
        n1 = AF >> 8;
        n2 = BC >> 8;
        c = CARRY_F(AF) >> 4;
        AF = LD_Nn_n(AF, n1 + n2 + c);
        n = AF >> 8;
        AF = LD_nN_n(AF, ZERO_S(AF & 0xFF00) | HALF_Sc(n1, n2, c) | CARRY_Sc(n, n1, n2, c));
        totalCycles += 1;
        break;
    case 0x89:
        n1 = AF >> 8;
        n2 = BC & 0x00FF;
        c = CARRY_F(AF) >> 4;
        AF = LD_Nn_n(AF, n1 + n2 + c);
        n = AF >> 8;
        AF = LD_nN_n(AF, ZERO_S(AF & 0xFF00) | HALF_Sc(n1, n2, c) | CARRY_Sc(n, n1, n2, c));
        totalCycles += 1;
        break;
    case 0x8A:
        n1 = AF >> 8;
        n2 = DE >> 8;
        c = CARRY_F(AF) >> 4;
        AF = LD_Nn_n(AF, n1 + n2 + c);
        n = AF >> 8;
        AF = LD_nN_n(AF, ZERO_S(AF & 0xFF00) | HALF_Sc(n1, n2, c) | CARRY_Sc(n, n1, n2, c));
        totalCycles += 1;
        break;
    case 0x8B:
        n1 = AF >> 8;
        n2 = DE & 0x00FF;
        c = CARRY_F(AF) >> 4;
        AF = LD_Nn_n(AF, n1 + n2 + c);
        n = AF >> 8;
        AF = LD_nN_n(AF, ZERO_S(AF & 0xFF00) | HALF_Sc(n1, n2, c) | CARRY_Sc(n, n1, n2, c));
        totalCycles += 1;
        break;
    case 0x8C:
        n1 = AF >> 8;
        n2 = HL >> 8;
        c = CARRY_F(AF) >> 4;
        AF = LD_Nn_n(AF, n1 + n2 + c);
        n = AF >> 8;
        AF = LD_nN_n(AF, ZERO_S(AF & 0xFF00) | HALF_Sc(n1, n2, c) | CARRY_Sc(n, n1, n2, c));
        totalCycles += 1;
        break;
    case 0x8D:
        n1 = AF >> 8;
        n2 = HL & 0x00FF;
        c = CARRY_F(AF) >> 4;
        AF = LD_Nn_n(AF, n1 + n2 + c);
        n = AF >> 8;
        AF = LD_nN_n(AF, ZERO_S(AF & 0xFF00) | HALF_Sc(n1, n2, c) | CARRY_Sc(n, n1, n2, c));
        totalCycles += 1;
        break;
    case 0x8E:
        n1 = AF >> 8;
        n2 = Memory::readByte(HL);
        c = CARRY_F(AF) >> 4;
        AF = LD_Nn_n(AF, n1 + n2 + c);
        n = AF >> 8;
        AF = LD_nN_n(AF, ZERO_S(AF & 0xFF00) | HALF_Sc(n1, n2, c) | CARRY_Sc(n, n1, n2, c));
        totalCycles += 2;
        break;
    case 0xCE:
        n1 = AF >> 8;
        n2 = readOp();
        c = CARRY_F(AF) >> 4;
        AF = LD_Nn_n(AF, n1 + n2 + c);
        n = AF >> 8;
        AF = LD_nN_n(AF, ZERO_S(AF & 0xFF00) | HALF_Sc(n1, n2, c) | CARRY_Sc(n, n1, n2, c));
        totalCycles += 2;
        break;

    // SUB n
    case 0x97:
        n1 = AF >> 8;
        n2 = AF >> 8;
        AF = LD_Nn_n(AF, n1 - n2);
        AF = LD_nN_n(AF, ZERO_S(AF & 0xFF00) | SUB_V | HBORROW_S(n1, n2) | BORROW_S(n1, n2));
        totalCycles += 1;
        break;
    case 0x90:
        n1 = AF >> 8;
        n2 = BC >> 8;
        AF = LD_Nn_n(AF, n1 - n2);
        AF = LD_nN_n(AF, ZERO_S(AF & 0xFF00) | SUB_V | HBORROW_S(n1, n2) | BORROW_S(n1, n2));
        totalCycles += 1;
        break;
    case 0x91:
        n1 = AF >> 8;
        n2 = BC & 0x00FF;
        AF = LD_Nn_n(AF, n1 - n2);
        AF = LD_nN_n(AF, ZERO_S(AF & 0xFF00) | SUB_V | HBORROW_S(n1, n2) | BORROW_S(n1, n2));
        totalCycles += 1;
        break;
    case 0x92:
        n1 = AF >> 8;
        n2 = DE >> 8;
        AF = LD_Nn_n(AF, n1 - n2);
        AF = LD_nN_n(AF, ZERO_S(AF & 0xFF00) | SUB_V | HBORROW_S(n1, n2) | BORROW_S(n1, n2));
        totalCycles += 1;
        break;
    case 0x93:
        n1 = AF >> 8;
        n2 = DE & 0x00FF;
        AF = LD_Nn_n(AF, n1 - n2);
        AF = LD_nN_n(AF, ZERO_S(AF & 0xFF00) | SUB_V | HBORROW_S(n1, n2) | BORROW_S(n1, n2));
        totalCycles += 1;
        break;
    case 0x94:
        n1 = AF >> 8;
        n2 = HL >> 8;
        AF = LD_Nn_n(AF, n1 - n2);
        AF = LD_nN_n(AF, ZERO_S(AF & 0xFF00) | SUB_V | HBORROW_S(n1, n2) | BORROW_S(n1, n2));
        totalCycles += 1;
        break;
    case 0x95:
        n1 = AF >> 8;
        n2 = HL & 0x00FF;
        AF = LD_Nn_n(AF, n1 - n2);
        AF = LD_nN_n(AF, ZERO_S(AF & 0xFF00) | SUB_V | HBORROW_S(n1, n2) | BORROW_S(n1, n2));
        totalCycles += 1;
        break;
    case 0x96:
        n1 = AF >> 8;
        n2 = Memory::readByte(HL);
        AF = LD_Nn_n(AF, n1 - n2);
        AF = LD_nN_n(AF, ZERO_S(AF & 0xFF00) | SUB_V | HBORROW_S(n1, n2) | BORROW_S(n1, n2));
        totalCycles += 2;
        break;
    case 0xD6:
        n1 = AF >> 8;
        n2 = readOp();
        AF = LD_Nn_n(AF, n1 - n2);
        AF = LD_nN_n(AF, ZERO_S(AF & 0xFF00) | SUB_V | HBORROW_S(n1, n2) | BORROW_S(n1, n2));
        totalCycles += 2;
        break;

    // SBC n
    case 0x9F:
        n1 = AF >> 8;
        n2 = AF >> 8;
        c = CARRY_F(AF) >> 4;
        AF = LD_Nn_n(AF, n1 - n2 - c);
        AF = LD_nN_n(AF, ZERO_S(AF & 0xFF00) | SUB_V | HBORROW_Sc(n1, n2, c) | BORROW_Sc(n1, n2, c));
        totalCycles += 1;
        break;
    case 0x98:
        n1 = AF >> 8;
        n2 = BC >> 8;
        c = CARRY_F(AF) >> 4;
        AF = LD_Nn_n(AF, n1 - n2 - c);
        AF = LD_nN_n(AF, ZERO_S(AF & 0xFF00) | SUB_V | HBORROW_Sc(n1, n2, c) | BORROW_Sc(n1, n2, c));
        totalCycles += 1;
        break;
    case 0x99:
        n1 = AF >> 8;
        n2 = BC & 0x00FF;
        c = CARRY_F(AF) >> 4;
        AF = LD_Nn_n(AF, n1 - n2 - c);
        AF = LD_nN_n(AF, ZERO_S(AF & 0xFF00) | SUB_V | HBORROW_Sc(n1, n2, c) | BORROW_Sc(n1, n2, c));
        totalCycles += 1;
        break;
    case 0x9A:
        n1 = AF >> 8;
        n2 = DE >> 8;
        c = CARRY_F(AF) >> 4;
        AF = LD_Nn_n(AF, n1 - n2 - c);
        AF = LD_nN_n(AF, ZERO_S(AF & 0xFF00) | SUB_V | HBORROW_Sc(n1, n2, c) | BORROW_Sc(n1, n2, c));
        totalCycles += 1;
        break;
    case 0x9B:
        n1 = AF >> 8;
        n2 = DE & 0x00FF;
        c = CARRY_F(AF) >> 4;
        AF = LD_Nn_n(AF, n1 - n2 - c);
        AF = LD_nN_n(AF, ZERO_S(AF & 0xFF00) | SUB_V | HBORROW_Sc(n1, n2, c) | BORROW_Sc(n1, n2, c));
        totalCycles += 1;
        break;
    case 0x9C:
        n1 = AF >> 8;
        n2 = HL >> 8;
        c = CARRY_F(AF) >> 4;
        AF = LD_Nn_n(AF, n1 - n2 - c);
        AF = LD_nN_n(AF, ZERO_S(AF & 0xFF00) | SUB_V | HBORROW_Sc(n1, n2, c) | BORROW_Sc(n1, n2, c));
        totalCycles += 1;
        break;
    case 0x9D:
        n1 = AF >> 8;
        n2 = HL & 0x00FF;
        c = CARRY_F(AF) >> 4;
        AF = LD_Nn_n(AF, n1 - n2 - c);
        AF = LD_nN_n(AF, ZERO_S(AF & 0xFF00) | SUB_V | HBORROW_Sc(n1, n2, c) | BORROW_Sc(n1, n2, c));
        totalCycles += 1;
        break;
    case 0x9E:
        n1 = AF >> 8;
        n2 = Memory::readByte(HL);
        c = CARRY_F(AF) >> 4;
        AF = LD_Nn_n(AF, n1 - n2 - c);
        AF = LD_nN_n(AF, ZERO_S(AF & 0xFF00) | SUB_V | HBORROW_Sc(n1, n2, c) | BORROW_Sc(n1, n2, c));
        totalCycles += 2;
        break;
    case 0xDE:
        n1 = AF >> 8;
        n2 = readOp();
        c = CARRY_F(AF) >> 4;
        AF = LD_Nn_n(AF, n1 - n2 - c);
        AF = LD_nN_n(AF, ZERO_S(AF & 0xFF00) | SUB_V | HBORROW_Sc(n1, n2, c) | BORROW_Sc(n1, n2, c));
        totalCycles += 2;
        break;

    // AND n
    case 0xA7:
        AF = LD_Nn_n(AF, AND_Nn_Nn(AF, AF));
        AF = LD_nN_n(AF, ZERO_S(AF & 0xFF00) | HALF_V);
        totalCycles += 1;
        break;
    case 0xA0:
        AF = LD_Nn_n(AF, AND_Nn_Nn(AF, BC));
        AF = LD_nN_n(AF, ZERO_S(AF & 0xFF00) | HALF_V);
        totalCycles += 1;
        break;
    case 0xA1:
        AF = LD_Nn_n(AF, AND_Nn_nN(AF, BC));
        AF = LD_nN_n(AF, ZERO_S(AF & 0xFF00) | HALF_V);
        totalCycles += 1;
        break;
    case 0xA2:
        AF = LD_Nn_n(AF, AND_Nn_Nn(AF, DE));
        AF = (((DE >> 8) & (AF >> 8)) << 8) | (AF & 0x00FF);
        AF = LD_nN_n(AF, ZERO_S(AF & 0xFF00) | HALF_V);
        totalCycles += 1;
        break;
    case 0xA3:
        AF = LD_Nn_n(AF, AND_Nn_nN(AF, DE));
        AF = (((DE & 0x00FF) & (AF >> 8)) << 8) | (AF & 0x00FF);
        AF = LD_nN_n(AF, ZERO_S(AF & 0xFF00) | HALF_V);
        totalCycles += 1;
        break;
    case 0xA4:
        AF = LD_Nn_n(AF, AND_Nn_Nn(AF, HL));
        AF = LD_nN_n(AF, ZERO_S(AF & 0xFF00) | HALF_V);
        totalCycles += 1;
        break;
    case 0xA5:
        AF = LD_Nn_n(AF, AND_Nn_nN(AF, HL));
        AF = LD_nN_n(AF, ZERO_S(AF & 0xFF00) | HALF_V);
        totalCycles += 1;
        break;
    case 0xA6:
        AF = LD_Nn_n(AF, AND_Nn_nN(AF, Memory::readByte(HL)));
        AF = LD_nN_n(AF, ZERO_S(AF & 0xFF00) | HALF_V);
        totalCycles += 2;
        break;
    case 0xE6:
        AF = LD_Nn_n(AF, AND_Nn_nN(AF, readOp()));
        AF = LD_nN_n(AF, ZERO_S(AF & 0xFF00) | HALF_V);
        totalCycles += 2;
        break;

    // OR n
    case 0xB7:
        AF = LD_Nn_n(AF, OR_Nn_Nn(AF, AF));
        AF = LD_nN_n(AF, ZERO_S(AF & 0xFF00));
        totalCycles += 1;
        break;
    case 0xB0:
        AF = LD_Nn_n(AF, OR_Nn_Nn(AF, BC));
        AF = LD_nN_n(AF, ZERO_S(AF & 0xFF00));
        totalCycles += 1;
        break;
    case 0xB1:
        AF = LD_Nn_n(AF, OR_Nn_nN(AF, BC));
        AF = LD_nN_n(AF, ZERO_S(AF & 0xFF00));
        totalCycles += 1;
        break;
    case 0xB2:
        AF = LD_Nn_n(AF, OR_Nn_Nn(AF, DE));
        AF = LD_nN_n(AF, ZERO_S(AF & 0xFF00));
        totalCycles += 1;
        break;
    case 0xB3:
        AF = LD_Nn_n(AF, OR_Nn_nN(AF, DE));
        AF = LD_nN_n(AF, ZERO_S(AF & 0xFF00));
        totalCycles += 1;
        break;
    case 0xB4:
        AF = LD_Nn_n(AF, OR_Nn_Nn(AF, HL));
        AF = LD_nN_n(AF, ZERO_S(AF & 0xFF00));
        totalCycles += 1;
        break;
    case 0xB5:
        AF = LD_Nn_n(AF, OR_Nn_nN(AF, HL));
        AF = LD_nN_n(AF, ZERO_S(AF & 0xFF00));
        totalCycles += 1;
        break;
    case 0xB6:
        AF = LD_Nn_n(AF, OR_Nn_nN(AF, Memory::readByte(HL)));
        AF = LD_nN_n(AF, ZERO_S(AF & 0xFF00));
        totalCycles += 2;
        break;
    case 0xF6:
        AF = LD_Nn_n(AF, OR_Nn_nN(AF, readOp()));
        AF = LD_nN_n(AF, ZERO_S(AF & 0xFF00));
        totalCycles += 2;
        break;

    // XOR n
    case 0xAF:
        AF = LD_Nn_n(AF, XOR_Nn_Nn(AF, AF));
        AF = LD_nN_n(AF, ZERO_S(AF & 0xFF00));
        totalCycles += 1;
        break;
    case 0xA8:
        AF = LD_Nn_n(AF, XOR_Nn_Nn(AF, BC));
        AF = LD_nN_n(AF, ZERO_S(AF & 0xFF00));
        totalCycles += 1;
        break;
    case 0xA9:
        AF = LD_Nn_n(AF, XOR_Nn_nN(AF, BC));
        AF = LD_nN_n(AF, ZERO_S(AF & 0xFF00));
        totalCycles += 1;
        break;
    case 0xAA:
        AF = LD_Nn_n(AF, XOR_Nn_Nn(AF, DE));
        AF = LD_nN_n(AF, ZERO_S(AF & 0xFF00));
        totalCycles += 1;
        break;
    case 0xAB:
        AF = LD_Nn_n(AF, XOR_Nn_nN(AF, DE));
        AF = LD_nN_n(AF, ZERO_S(AF & 0xFF00));
        totalCycles += 1;
        break;
    case 0xAC:
        AF = LD_Nn_n(AF, XOR_Nn_Nn(AF, HL));
        AF = LD_nN_n(AF, ZERO_S(AF & 0xFF00));
        totalCycles += 1;
        break;
    case 0xAD:
        AF = LD_Nn_n(AF, XOR_Nn_nN(AF, HL));
        AF = LD_nN_n(AF, ZERO_S(AF & 0xFF00));
        totalCycles += 1;
        break;
    case 0xAE:
        AF = LD_Nn_n(AF, XOR_Nn_nN(AF, Memory::readByte(HL)));
        AF = LD_nN_n(AF, ZERO_S(AF & 0xFF00));
        totalCycles += 2;
        break;
    case 0xEE:
        AF = LD_Nn_n(AF, XOR_Nn_nN(AF, readOp()));
        AF = LD_nN_n(AF, ZERO_S(AF & 0xFF00));
        totalCycles += 2;
        break;

    // CP n
    case 0xBF:
        n1 = AF >> 8;
        n2 = AF >> 8;
        n = n1 - n2;
        AF = LD_nN_n(AF, ZERO_S(n) | SUB_V | HBORROW_S(n1, n2) | BORROW_S(n1, n2));
        totalCycles += 1;
        break;
    case 0xB8:
        n1 = AF >> 8;
        n2 = BC >> 8;
        n = n1 - n2;
        AF = LD_nN_n(AF, ZERO_S(n) | SUB_V | HBORROW_S(n1, n2) | BORROW_S(n1, n2));
        totalCycles += 1;
        break;
    case 0xB9:
        n1 = AF >> 8;
        n2 = BC & 0x00FF;
        n = n1 - n2;
        AF = LD_nN_n(AF, ZERO_S(n) | SUB_V | HBORROW_S(n1, n2) | BORROW_S(n1, n2));
        totalCycles += 1;
        break;
    case 0xBA:
        n1 = AF >> 8;
        n2 = DE >> 8;
        n = n1 - n2;
        AF = LD_nN_n(AF, ZERO_S(n) | SUB_V | HBORROW_S(n1, n2) | BORROW_S(n1, n2));
        totalCycles += 1;
        break;
    case 0xBB:
        n1 = AF >> 8;
        n2 = DE & 0x00FF;
        n = n1 - n2;
        AF = LD_nN_n(AF, ZERO_S(n) | SUB_V | HBORROW_S(n1, n2) | BORROW_S(n1, n2));
        totalCycles += 1;
        break;
    case 0xBC:
        n1 = AF >> 8;
        n2 = HL >> 8;
        n = n1 - n2;
        AF = LD_nN_n(AF, ZERO_S(n) | SUB_V | HBORROW_S(n1, n2) | BORROW_S(n1, n2));
        totalCycles += 1;
        break;
    case 0xBD:
        n1 = AF >> 8;
        n2 = HL & 0x00FF;
        n = n1 - n2;
        AF = LD_nN_n(AF, ZERO_S(n) | SUB_V | HBORROW_S(n1, n2) | BORROW_S(n1, n2));
        totalCycles += 1;
        break;
    case 0xBE:
        n1 = AF >> 8;
        n2 = Memory::readByte(HL);
        n = n1 - n2;
        AF = LD_nN_n(AF, ZERO_S(n) | SUB_V | HBORROW_S(n1, n2) | BORROW_S(n1, n2));
        totalCycles += 2;
        break;
    case 0xFE:
        n1 = AF >> 8;
        n2 = readOp();
        n = n1 - n2;
        AF = LD_nN_n(AF, ZERO_S(n) | SUB_V | HBORROW_S(n1, n2) | BORROW_S(n1, n2));
        totalCycles += 2;
        break;

    // INC n
    case 0x3C:
        AF = LD_Nn_Nn(AF, AF + 0x100);
        AF = LD_nN_n(AF, ZERO_S(AF & 0xFF00) | (((AF & 0x0F00) == 0) << 5) | CARRY_F(AF));
        totalCycles += 1;
        break;
    case 0x04:
        BC = LD_Nn_Nn(BC, BC + 0x100);
        AF = LD_nN_n(AF, ZERO_S(BC & 0xFF00) | (((BC & 0x0F00) == 0) << 5) | CARRY_F(AF));
        totalCycles += 1;
        break;
    case 0x0C:
        BC = LD_nN_nN(BC, BC + 1);
        AF = LD_nN_n(AF, ZERO_S(BC & 0x00FF) | (((BC & 0x000F) == 0) << 5) | CARRY_F(AF));
        totalCycles += 1;
        break;
    case 0x14:
        DE = LD_Nn_Nn(DE, DE + 0x100);
        AF = LD_nN_n(AF, ZERO_S(DE & 0xFF00) | (((DE & 0x0F00) == 0) << 5) | CARRY_F(AF));
        totalCycles += 1;
        break;
    case 0x1C:
        DE = LD_nN_nN(DE, DE + 1);
        AF = LD_nN_n(AF, ZERO_S(DE & 0x00FF) | (((DE & 0x000F) == 0) << 5) | CARRY_F(AF));
        totalCycles += 1;
        break;
    case 0x24:
        HL = LD_Nn_Nn(HL, HL + 0x100);
        AF = LD_nN_n(AF, ZERO_S(HL & 0xFF00) | (((HL & 0x0F00) == 0) << 5) | CARRY_F(AF));
        totalCycles += 1;
        break;
    case 0x2C:
        HL = LD_nN_nN(HL, HL + 1);
        AF = LD_nN_n(AF, ZERO_S(HL & 0x00FF) | (((HL & 0x000F) == 0) << 5) | CARRY_F(AF));
        totalCycles += 1;
        break;
    case 0x34:
        Memory::writeByte(HL, Memory::readByte(HL) + 1);
        AF = LD_nN_n(AF, ZERO_S(Memory::readByte(HL)) | (((Memory::readByte(HL) & 0x0F) == 0) << 5) | CARRY_F(AF));
        totalCycles += 3;
        break;

    // DEC n
    case 0x3D:
        AF = LD_Nn_Nn(AF, AF - 0x100);
        AF = LD_nN_n(AF, ZERO_S(AF & 0xFF00) | SUB_V | (((AF & 0x0F00) == 0x0F00) << 5)) | CARRY_F(AF);
        totalCycles += 1;
        break;
    case 0x05:
        BC = LD_Nn_Nn(BC, BC - 0x100);
        AF = LD_nN_n(AF, ZERO_S(BC & 0xFF00) | SUB_V | (((BC & 0x0F00) == 0x0F00) << 5) | CARRY_F(AF));
        totalCycles += 1;
        break;
    case 0x0D:
        BC = LD_nN_nN(BC, BC - 1);
        AF = LD_nN_n(AF, ZERO_S(BC & 0x00FF) | SUB_V | (((BC & 0x000F) == 0x000F) << 5) | CARRY_F(AF));
        totalCycles += 1;
        break;
    case 0x15:
        DE = LD_Nn_Nn(DE, DE - 0x100);
        AF = LD_nN_n(AF, ZERO_S(DE & 0xFF00) | SUB_V | (((DE & 0x0F00) == 0x0F00) << 5) | CARRY_F(AF));
        totalCycles += 1;
        break;
    case 0x1D:
        DE = LD_nN_nN(DE, DE - 1);
        AF = LD_nN_n(AF, ZERO_S(DE & 0x00FF) | SUB_V | (((DE & 0x000F) == 0x000F) << 5) | CARRY_F(AF));
        totalCycles += 1;
        break;
    case 0x25:
        HL = LD_Nn_Nn(HL, HL - 0x100);
        AF = LD_nN_n(AF, ZERO_S(HL & 0xFF00) | SUB_V | (((HL & 0x0F00) == 0x0F00) << 5) | CARRY_F(AF));
        totalCycles += 1;
        break;
    case 0x2D:
        HL = LD_nN_nN(HL, HL - 1);
        AF = LD_nN_n(AF, ZERO_S(HL & 0x00FF) | SUB_V | (((HL & 0x000F) == 0x000F) << 5) | CARRY_F(AF));
        totalCycles += 1;
        break;
    case 0x35:
        Memory::writeByte(HL, Memory::readByte(HL) - 1);
        AF = LD_nN_n(AF, ZERO_S(Memory::readByte(HL)) | SUB_V | (((Memory::readByte(HL) & 0x0F) == 0x0F) << 5) | CARRY_F(AF));
        totalCycles += 3;
        break;

    // ADD HL,n
    case 0x09:
        nn1 = HL;
        nn2 = BC;
        HL = nn1 + nn2;
        AF = LD_nN_n(AF, ZERO_F(AF) | HALF_Snn(nn1, nn2) | CARRY_S(HL, nn1, nn2));
        totalCycles += 2;
        break;
    case 0x19:
        nn1 = HL;
        nn2 = DE;
        HL = nn1 + nn2;
        AF = LD_nN_n(AF, ZERO_F(AF) | HALF_Snn(nn1, nn2) | CARRY_S(HL, nn1, nn2));
        totalCycles += 2;
        break;
    case 0x29:
        nn1 = HL;
        nn2 = HL;
        HL = nn1 + nn2;
        AF = LD_nN_n(AF, ZERO_F(AF) | HALF_Snn(nn1, nn2) | CARRY_S(HL, nn1, nn2));
        totalCycles += 2;
        break;
    case 0x39:
        nn1 = HL;
        nn2 = SP;
        HL = nn1 + nn2;
        AF = LD_nN_n(AF, ZERO_F(AF) | HALF_Snn(nn1, nn2) | CARRY_S(HL, nn1, nn2));
        totalCycles += 2;
        break;

    // ADD SP,n
    case 0xE8:
        nn = SP;
        sn = (int8_t)readOp();
        SP = nn + sn;
        AF = LD_nN_n(AF, HALF_S(nn, sn) | CARRY_S(SP & 0xFF, nn & 0xFF, sn));
        totalCycles += 4;
        break;

    // INC nn
    case 0x03:
        BC++;
        totalCycles += 2;
        break;
    case 0x13:
        DE++;
        totalCycles += 2;
        break;
    case 0x23:
        HL++;
        totalCycles += 2;
        break;
    case 0x33:
        SP++;
        totalCycles += 2;
        break;

    // DEC nn
    case 0x0B:
        BC--;
        totalCycles += 2;
        break;
    case 0x1B:
        DE--;
        totalCycles += 2;
        break;
    case 0x2B:
        HL--;
        totalCycles += 2;
        break;
    case 0x3B:
        SP--;
        totalCycles += 2;
        break;

    // RLCA
    case 0x07:
        c = (AF >> 15) & 0x01;
        AF = LD_Nn_Nn(AF, ((AF & 0xFF00) << 1) | (c << 8));
        AF = LD_nN_n(AF, c << 4);
        totalCycles += 1;
        break;

    // RLA
    case 0x17:
        c = (AF >> 15) & 0x01;
        AF = LD_Nn_Nn(AF, ((AF & 0xFF00) << 1) | (CARRY_F(AF) << 4));
        AF = LD_nN_n(AF, c << 4);
        totalCycles += 1;
        break;

    // RRCA
    case 0x0F:
        c = (AF >> 8) & 0x01;
        AF = LD_Nn_Nn(AF, (AF >> 1) | (c << 15));
        AF = LD_nN_n(AF, c << 4);
        totalCycles += 1;
        break;

    // RRA
    case 0x1F:
        c = (AF >> 8) & 0x01;
        AF = LD_Nn_Nn(AF, (AF >> 1) | (CARRY_F(AF) << 11));
        AF = LD_nN_n(AF, c << 4);
        totalCycles += 1;
        break;

    // Multiple OP codes depending on n
    case 0xCB:
        n = readOp();
        switch (n)
        {

        // RLC c
        case 0x07:
            c = (AF >> 15) & 0x01;
            AF = LD_Nn_Nn(AF, ((AF & 0xFF00) << 1) | (c << 8));
            AF = LD_nN_n(AF, ZERO_S(AF & 0xFF00) | (c << 4));
            totalCycles += 2;
            break;
        case 0x00:
            c = (BC >> 15) & 0x01;
            BC = LD_Nn_Nn(BC, ((BC & 0xFF00) << 1) | (c << 8));
            AF = LD_nN_n(AF, ZERO_S(BC & 0xFF00) | (c << 4));
            totalCycles += 2;
            break;
        case 0x01:
            c = (BC >> 7) & 0x01;
            BC = LD_nN_nN(BC, (BC << 1) | c);
            AF = LD_nN_n(AF, ZERO_S(BC & 0x00FF) | (c << 4));
            totalCycles += 2;
            break;
        case 0x02:
            c = (DE >> 15) & 0x01;
            DE = LD_Nn_Nn(DE, ((DE & 0xFF00) << 1) | (c << 8));
            AF = LD_nN_n(AF, ZERO_S(DE & 0xFF00) | (c << 4));
            totalCycles += 2;
            break;
        case 0x03:
            c = (DE >> 7) & 0x01;
            DE = LD_nN_nN(DE, (DE << 1) | c);
            AF = LD_nN_n(AF, ZERO_S(DE & 0x00FF) | (c << 4));
            totalCycles += 2;
            break;
        case 0x04:
            c = (HL >> 15) & 0x01;
            HL = LD_Nn_Nn(HL, ((HL & 0xFF00) << 1) | (c << 8));
            AF = LD_nN_n(AF, ZERO_S(HL & 0xFF00) | (c << 4));
            totalCycles += 2;
            break;
        case 0x05:
            c = (HL >> 7) & 0x01;
            HL = LD_nN_nN(HL, (HL << 1) | c);
            AF = LD_nN_n(AF, ZERO_S(HL & 0x00FF) | (c << 4));
            totalCycles += 2;
            break;
        case 0x06:
            c = (Memory::readByte(HL) >> 7) & 0x01;
            Memory::writeByte(HL, (Memory::readByte(HL) << 1) | c);
            AF = LD_nN_n(AF, ZERO_S(Memory::readByte(HL)) | (c << 4));
            totalCycles += 4;
            break;

        // RL n
        case 0x17:
            c = (AF >> 15) & 0x01;
            AF = LD_Nn_Nn(AF, ((AF & 0xFF00) << 1) | (CARRY_F(AF) << 4));
            AF = LD_nN_n(AF, ZERO_S(AF & 0xFF00) | (c << 4));
            totalCycles += 2;
            break;
        case 0x10:
            c = (BC >> 15) & 0x01;
            BC = LD_Nn_Nn(BC, ((BC & 0xFF00) << 1) | (CARRY_F(AF) << 4));
            AF = LD_nN_n(AF, ZERO_S(BC & 0xFF00) | (c << 4));
            totalCycles += 2;
            break;
        case 0x11:
            c = (BC >> 7) & 0x01;
            BC = LD_nN_nN(BC, (BC << 1) | (CARRY_F(AF) >> 4));
            AF = LD_nN_n(AF, ZERO_S(BC & 0x00FF) | (c << 4));
            totalCycles += 2;
            break;
        case 0x12:
            c = (DE >> 15) & 0x01;
            DE = LD_Nn_Nn(DE, ((DE & 0xFF00) << 1) | (CARRY_F(AF) << 4));
            AF = LD_nN_n(AF, ZERO_S(DE & 0xFF00) | (c << 4));
            totalCycles += 2;
            break;
        case 0x13:
            c = (DE >> 7) & 0x01;
            DE = LD_nN_nN(DE, (DE << 1) | (CARRY_F(AF) >> 4));
            AF = LD_nN_n(AF, ZERO_S(DE & 0x00FF) | (c << 4));
            totalCycles += 2;
            break;
        case 0x14:
            c = (HL >> 15) & 0x01;
            HL = LD_Nn_Nn(HL, ((HL & 0xFF00) << 1) | (CARRY_F(AF) << 4));
            AF = LD_nN_n(AF, ZERO_S(HL & 0xFF00) | (c << 4));
            totalCycles += 2;
            break;
        case 0x15:
            c = (HL >> 7) & 0x01;
            HL = LD_nN_nN(HL, (HL << 1) | (CARRY_F(AF) >> 4));
            AF = LD_nN_n(AF, ZERO_S(HL & 0x00FF) | (c << 4));
            totalCycles += 2;
            break;
        case 0x16:
            c = (Memory::readByte(HL) >> 7) & 0x01;
            Memory::writeByte(HL, (Memory::readByte(HL) << 1) | (CARRY_F(AF) >> 4));
            AF = LD_nN_n(AF, ZERO_S(Memory::readByte(HL)) | (c << 4));
            totalCycles += 4;
            break;

        // RRC n
        case 0x0F:
            c = (AF >> 8) & 0x01;
            AF = LD_Nn_Nn(AF, (AF >> 1) | (c << 15));
            AF = LD_nN_n(AF, ZERO_S(AF & 0xFF00) | (c << 4));
            totalCycles += 2;
            break;
        case 0x08:
            c = (BC >> 8) & 0x01;
            BC = LD_Nn_Nn(BC, (BC >> 1) | (c << 15));
            AF = LD_nN_n(AF, ZERO_S(BC & 0xFF00) | (c << 4));
            totalCycles += 2;
            break;
        case 0x09:
            c = BC & 0x01;
            BC = LD_nN_nN(BC, ((BC & 0x00FF) >> 1) | (c << 7));
            AF = LD_nN_n(AF, ZERO_S(BC & 0x00FF) | (c << 4));
            totalCycles += 2;
            break;
        case 0x0A:
            c = (DE >> 8) & 0x01;
            DE = LD_Nn_Nn(DE, (DE >> 1) | (c << 15));
            AF = LD_nN_n(AF, ZERO_S(DE & 0xFF00) | (c << 4));
            totalCycles += 2;
            break;
        case 0x0B:
            c = DE & 0x01;
            DE = LD_nN_nN(DE, ((DE & 0x00FF) >> 1) | (c << 7));
            AF = LD_nN_n(AF, ZERO_S(DE & 0x00FF) | (c << 4));
            totalCycles += 2;
            break;
        case 0x0C:
            c = (HL >> 8) & 0x01;
            HL = LD_Nn_Nn(HL, (HL >> 1) | (c << 15));
            AF = LD_nN_n(AF, ZERO_S(HL & 0xFF00) | (c << 4));
            totalCycles += 2;
            break;
        case 0x0D:
            c = HL & 0x01;
            HL = LD_nN_nN(HL, ((HL & 0x00FF) >> 1) | (c << 7));
            AF = LD_nN_n(AF, ZERO_S(HL & 0x00FF) | (c << 4));
            totalCycles += 2;
            break;
        case 0x0E:
            c = Memory::readByte(HL) & 0x01;
            Memory::writeByte(HL, (Memory::readByte(HL) >> 1) | (c << 7));
            AF = LD_nN_n(AF, ZERO_S(Memory::readByte(HL)) | (c << 4));
            totalCycles += 4;
            break;

        // RR n
        case 0x1F:
            c = (AF >> 8) & 0x01;
            AF = LD_Nn_Nn(AF, (AF >> 1) | (CARRY_F(AF) << 11));
            AF = LD_nN_n(AF, ZERO_S(AF & 0xFF00) | (c << 4));
            totalCycles += 2;
            break;
        case 0x18:
            c = (BC >> 8) & 0x01;
            BC = LD_Nn_Nn(BC, (BC >> 1) | (CARRY_F(AF) << 11));
            AF = LD_nN_n(AF, ZERO_S(BC & 0xFF00) | (c << 4));
            totalCycles += 2;
            break;
        case 0x19:
            c = BC & 0x01;
            BC = LD_nN_nN(BC, ((BC & 0x00FF) >> 1) | (CARRY_F(AF) << 3));
            AF = LD_nN_n(AF, ZERO_S(BC & 0x00FF) | (c << 4));
            totalCycles += 2;
            break;
        case 0x1A:
            c = (DE >> 8) & 0x01;
            DE = LD_Nn_Nn(DE, (DE >> 1) | (CARRY_F(AF) << 11));
            AF = LD_nN_n(AF, ZERO_S(DE & 0xFF00) | (c << 4));
            totalCycles += 2;
            break;
        case 0x1B:
            c = DE & 0x01;
            DE = LD_nN_nN(DE, ((DE & 0x00FF) >> 1) | (CARRY_F(AF) << 3));
            AF = LD_nN_n(AF, ZERO_S(DE & 0x00FF) | (c << 4));
            totalCycles += 2;
            break;
        case 0x1C:
            c = (HL >> 8) & 0x01;
            HL = LD_Nn_Nn(HL, (HL >> 1) | (CARRY_F(AF) << 11));
            AF = LD_nN_n(AF, ZERO_S(HL & 0xFF00) | (c << 4));
            totalCycles += 2;
            break;
        case 0x1D:
            c = HL & 0x01;
            HL = LD_nN_nN(HL, ((HL & 0x00FF) >> 1) | (CARRY_F(AF) << 3));
            AF = LD_nN_n(AF, ZERO_S(HL & 0x00FF) | (c << 4));
            totalCycles += 2;
            break;
        case 0x1E:
            c = Memory::readByte(HL) & 0x01;
            Memory::writeByte(HL, (Memory::readByte(HL) >> 1) | (CARRY_F(AF) << 3));
            AF = LD_nN_n(AF, ZERO_S(Memory::readByte(HL)) | (c << 4));
            totalCycles += 4;
            break;

        // SLA n
        case 0x27:
            c = (AF >> 15) & 0x01;
            AF = LD_Nn_Nn(AF, (AF & 0xFF00) << 1);
            AF = LD_nN_n(AF, ZERO_S(AF & 0xFF00) | (c << 4));
            totalCycles += 2;
            break;
        case 0x20:
            c = (BC >> 15) & 0x01;
            BC = LD_Nn_Nn(BC, (BC & 0xFF00) << 1);
            AF = LD_nN_n(AF, ZERO_S(BC & 0xFF00) | (c << 4));
            totalCycles += 2;
            break;
        case 0x21:
            c = (BC >> 7) & 0x01;
            BC = LD_nN_nN(BC, BC << 1);
            AF = LD_nN_n(AF, ZERO_S(BC & 0x00FF) | (c << 4));
            totalCycles += 2;
            break;
        case 0x22:
            c = (DE >> 15) & 0x01;
            DE = LD_Nn_Nn(DE, (DE & 0xFF00) << 1);
            AF = LD_nN_n(AF, ZERO_S(DE & 0xFF00) | (c << 4));
            totalCycles += 2;
            break;
        case 0x23:
            c = (DE >> 7) & 0x01;
            DE = LD_nN_nN(DE, DE << 1);
            AF = LD_nN_n(AF, ZERO_S(DE & 0x00FF) | (c << 4));
            totalCycles += 2;
            break;
        case 0x24:
            c = (HL >> 15) & 0x01;
            HL = LD_Nn_Nn(HL, (HL & 0xFF00) << 1);
            AF = LD_nN_n(AF, ZERO_S(HL & 0xFF00) | (c << 4));
            totalCycles += 2;
            break;
        case 0x25:
            c = (HL >> 7) & 0x01;
            HL = LD_nN_nN(HL, HL << 1);
            AF = LD_nN_n(AF, ZERO_S(HL & 0x00FF) | (c << 4));
            totalCycles += 2;
            break;
        case 0x26:
            c = (Memory::readByte(HL) >> 7) & 0x01;
            Memory::writeByte(HL, Memory::readByte(HL) << 1);
            AF = LD_nN_n(AF, ZERO_S(Memory::readByte(HL)) | (c << 4));
            totalCycles += 4;
            break;

        // SRA n
        case 0x2F:
            c = (AF >> 8) & 0x01;
            AF = LD_Nn_Nn(AF, (AF >> 1) | (AF & 0x8000));
            AF = LD_nN_n(AF, ZERO_S(AF & 0xFF00) | (c << 4));
            totalCycles += 2;
            break;
        case 0x28:
            c = (BC >> 8) & 0x01;
            BC = LD_Nn_Nn(BC, (BC >> 1) | (BC & 0x8000));
            AF = LD_nN_n(AF, ZERO_S(BC & 0xFF00) | (c << 4));
            totalCycles += 2;
            break;
        case 0x29:
            c = BC & 0x01;
            BC = LD_nN_nN(BC, ((BC & 0x00FF) >> 1) | (BC & 0x0080));
            AF = LD_nN_n(AF, ZERO_S(BC & 0x00FF) | (c << 4));
            totalCycles += 2;
            break;
        case 0x2A:
            c = (DE >> 8) & 0x01;
            DE = LD_Nn_Nn(DE, (DE >> 1) | (DE & 0x8000));
            AF = LD_nN_n(AF, ZERO_S(DE & 0xFF00) | (c << 4));
            totalCycles += 2;
            break;
        case 0x2B:
            c = DE & 0x01;
            DE = LD_nN_nN(DE, ((DE & 0x00FF) >> 1) | (DE & 0x0080));
            AF = LD_nN_n(AF, ZERO_S(DE & 0x00FF) | (c << 4));
            totalCycles += 2;
            break;
        case 0x2C:
            c = (HL >> 8) & 0x01;
            HL = LD_Nn_Nn(HL, (HL >> 1) | (HL & 0x8000));
            AF = LD_nN_n(AF, ZERO_S(HL & 0xFF00) | (c << 4));
            totalCycles += 2;
            break;
        case 0x2D:
            c = HL & 0x01;
            HL = LD_nN_nN(HL, ((HL & 0x00FF) >> 1) | (HL & 0x0080));
            AF = LD_nN_n(AF, ZERO_S(HL & 0x00FF) | (c << 4));
            totalCycles += 2;
            break;
        case 0x2E:
            c = Memory::readByte(HL) & 0x01;
            Memory::writeByte(HL, (Memory::readByte(HL) >> 1) | (Memory::readByte(HL) & 0x0080));
            AF = LD_nN_n(AF, ZERO_S(Memory::readByte(HL)) | (c << 4));
            totalCycles += 4;
            break;

        // SRL n
        case 0x3F:
            c = (AF >> 8) & 0x01;
            AF = LD_Nn_Nn(AF, AF >> 1);
            AF = LD_nN_n(AF, ZERO_S(AF & 0xFF00) | (c << 4));
            totalCycles += 2;
            break;
        case 0x38:
            c = (BC >> 8) & 0x01;
            BC = LD_Nn_Nn(BC, BC >> 1);
            AF = LD_nN_n(AF, ZERO_S(BC & 0xFF00) | (c << 4));
            totalCycles += 2;
            break;
        case 0x39:
            c = BC & 0x01;
            BC = LD_nN_nN(BC, (BC & 0x00FF) >> 1);
            AF = LD_nN_n(AF, ZERO_S(BC & 0x00FF) | (c << 4));
            totalCycles += 2;
            break;
        case 0x3A:
            c = (DE >> 8) & 0x01;
            DE = LD_Nn_Nn(DE, DE >> 1);
            AF = LD_nN_n(AF, ZERO_S(DE & 0xFF00) | (c << 4));
            totalCycles += 2;
            break;
        case 0x3B:
            c = DE & 0x01;
            DE = LD_nN_nN(DE, (DE & 0x00FF) >> 1);
            AF = LD_nN_n(AF, ZERO_S(DE & 0x00FF) | (c << 4));
            totalCycles += 2;
            break;
        case 0x3C:
            c = (HL >> 8) & 0x01;
            HL = LD_Nn_Nn(HL, HL >> 1);
            AF = LD_nN_n(AF, ZERO_S(HL & 0xFF00) | (c << 4));
            totalCycles += 2;
            break;
        case 0x3D:
            c = HL & 0x01;
            HL = LD_nN_nN(HL, (HL & 0x00FF) >> 1);
            AF = LD_nN_n(AF, ZERO_S(HL & 0x00FF) | (c << 4));
            totalCycles += 2;
            break;
        case 0x3E:
            c = Memory::readByte(HL) & 0x01;
            Memory::writeByte(HL, Memory::readByte(HL) >> 1);
            AF = LD_nN_n(AF, ZERO_S(Memory::readByte(HL)) | (c << 4));
            totalCycles += 4;
            break;

        // BIT b,r
        case 0x47:
            AF = LD_nN_n(AF, ZERO_S(AF & 0x0100) | HALF_V | CARRY_F(AF));
            totalCycles += 2;
            break;
        case 0x40:
            AF = LD_nN_n(AF, ZERO_S(BC & 0x0100) | HALF_V | CARRY_F(AF));
            totalCycles += 2;
            break;
        case 0x41:
            AF = LD_nN_n(AF, ZERO_S(BC & 0x0001) | HALF_V | CARRY_F(AF));
            totalCycles += 2;
            break;
        case 0x42:
            AF = LD_nN_n(AF, ZERO_S(DE & 0x0100) | HALF_V | CARRY_F(AF));
            totalCycles += 2;
            break;
        case 0x43:
            AF = LD_nN_n(AF, ZERO_S(DE & 0x0001) | HALF_V | CARRY_F(AF));
            totalCycles += 2;
            break;
        case 0x44:
            AF = LD_nN_n(AF, ZERO_S(HL & 0x0100) | HALF_V | CARRY_F(AF));
            totalCycles += 2;
            break;
        case 0x45:
            AF = LD_nN_n(AF, ZERO_S(HL & 0x0001) | HALF_V | CARRY_F(AF));
            totalCycles += 2;
            break;
        case 0x46:
            AF = LD_nN_n(AF, ZERO_S(Memory::readByte(HL) & 0x01) | HALF_V | CARRY_F(AF));
            totalCycles += 4;
            break;
        case 0x4F:
            AF = LD_nN_n(AF, ZERO_S(AF & 0x0200) | HALF_V | CARRY_F(AF));
            totalCycles += 2;
            break;
        case 0x48:
            AF = LD_nN_n(AF, ZERO_S(BC & 0x0200) | HALF_V | CARRY_F(AF));
            totalCycles += 2;
            break;
        case 0x49:
            AF = LD_nN_n(AF, ZERO_S(BC & 0x0002) | HALF_V | CARRY_F(AF));
            totalCycles += 2;
            break;
        case 0x4A:
            AF = LD_nN_n(AF, ZERO_S(DE & 0x0200) | HALF_V | CARRY_F(AF));
            totalCycles += 2;
            break;
        case 0x4B:
            AF = LD_nN_n(AF, ZERO_S(DE & 0x0002) | HALF_V | CARRY_F(AF));
            totalCycles += 2;
            break;
        case 0x4C:
            AF = LD_nN_n(AF, ZERO_S(HL & 0x0200) | HALF_V | CARRY_F(AF));
            totalCycles += 2;
            break;
        case 0x4D:
            AF = LD_nN_n(AF, ZERO_S(HL & 0x0002) | HALF_V | CARRY_F(AF));
            totalCycles += 2;
            break;
        case 0x4E:
            AF = LD_nN_n(AF, ZERO_S(Memory::readByte(HL) & 0x02) | HALF_V | CARRY_F(AF));
            totalCycles += 4;
            break;
        case 0x57:
            AF = LD_nN_n(AF, ZERO_S(AF & 0x0400) | HALF_V | CARRY_F(AF));
            totalCycles += 2;
            break;
        case 0x50:
            AF = LD_nN_n(AF, ZERO_S(BC & 0x0400) | HALF_V | CARRY_F(AF));
            totalCycles += 2;
            break;
        case 0x51:
            AF = LD_nN_n(AF, ZERO_S(BC & 0x0004) | HALF_V | CARRY_F(AF));
            totalCycles += 2;
            break;
        case 0x52:
            AF = LD_nN_n(AF, ZERO_S(DE & 0x0400) | HALF_V | CARRY_F(AF));
            totalCycles += 2;
            break;
        case 0x53:
            AF = LD_nN_n(AF, ZERO_S(DE & 0x0004) | HALF_V | CARRY_F(AF));
            totalCycles += 2;
            break;
        case 0x54:
            AF = LD_nN_n(AF, ZERO_S(HL & 0x0400) | HALF_V | CARRY_F(AF));
            totalCycles += 2;
            break;
        case 0x55:
            AF = LD_nN_n(AF, ZERO_S(HL & 0x0004) | HALF_V | CARRY_F(AF));
            totalCycles += 2;
            break;
        case 0x56:
            AF = LD_nN_n(AF, ZERO_S(Memory::readByte(HL) & 0x04) | HALF_V | CARRY_F(AF));
            totalCycles += 4;
            break;
        case 0x5F:
            AF = LD_nN_n(AF, ZERO_S(AF & 0x0800) | HALF_V | CARRY_F(AF));
            totalCycles += 2;
            break;
        case 0x58:
            AF = LD_nN_n(AF, ZERO_S(BC & 0x0800) | HALF_V | CARRY_F(AF));
            totalCycles += 2;
            break;
        case 0x59:
            AF = LD_nN_n(AF, ZERO_S(BC & 0x0008) | HALF_V | CARRY_F(AF));
            totalCycles += 2;
            break;
        case 0x5A:
            AF = LD_nN_n(AF, ZERO_S(DE & 0x0800) | HALF_V | CARRY_F(AF));
            totalCycles += 2;
            break;
        case 0x5B:
            AF = LD_nN_n(AF, ZERO_S(DE & 0x0008) | HALF_V | CARRY_F(AF));
            totalCycles += 2;
            break;
        case 0x5C:
            AF = LD_nN_n(AF, ZERO_S(HL & 0x0800) | HALF_V | CARRY_F(AF));
            totalCycles += 2;
            break;
        case 0x5D:
            AF = LD_nN_n(AF, ZERO_S(HL & 0x0008) | HALF_V | CARRY_F(AF));
            totalCycles += 2;
            break;
        case 0x5E:
            AF = LD_nN_n(AF, ZERO_S(Memory::readByte(HL) & 0x08) | HALF_V | CARRY_F(AF));
            totalCycles += 4;
            break;
        case 0x67:
            AF = LD_nN_n(AF, ZERO_S(AF & 0x1000) | HALF_V | CARRY_F(AF));
            totalCycles += 2;
            break;
        case 0x60:
            AF = LD_nN_n(AF, ZERO_S(BC & 0x1000) | HALF_V | CARRY_F(AF));
            totalCycles += 2;
            break;
        case 0x61:
            AF = LD_nN_n(AF, ZERO_S(BC & 0x0010) | HALF_V | CARRY_F(AF));
            totalCycles += 2;
            break;
        case 0x62:
            AF = LD_nN_n(AF, ZERO_S(DE & 0x1000) | HALF_V | CARRY_F(AF));
            totalCycles += 2;
            break;
        case 0x63:
            AF = LD_nN_n(AF, ZERO_S(DE & 0x0010) | HALF_V | CARRY_F(AF));
            totalCycles += 2;
            break;
        case 0x64:
            AF = LD_nN_n(AF, ZERO_S(HL & 0x1000) | HALF_V | CARRY_F(AF));
            totalCycles += 2;
            break;
        case 0x65:
            AF = LD_nN_n(AF, ZERO_S(HL & 0x0010) | HALF_V | CARRY_F(AF));
            totalCycles += 2;
            break;
        case 0x66:
            AF = LD_nN_n(AF, ZERO_S(Memory::readByte(HL) & 0x10) | HALF_V | CARRY_F(AF));
            totalCycles += 4;
            break;
        case 0x6F:
            AF = LD_nN_n(AF, ZERO_S(AF & 0x2000) | HALF_V | CARRY_F(AF));
            totalCycles += 2;
            break;
        case 0x68:
            AF = LD_nN_n(AF, ZERO_S(BC & 0x2000) | HALF_V | CARRY_F(AF));
            totalCycles += 2;
            break;
        case 0x69:
            AF = LD_nN_n(AF, ZERO_S(BC & 0x0020) | HALF_V | CARRY_F(AF));
            totalCycles += 2;
            break;
        case 0x6A:
            AF = LD_nN_n(AF, ZERO_S(DE & 0x2000) | HALF_V | CARRY_F(AF));
            totalCycles += 2;
            break;
        case 0x6B:
            AF = LD_nN_n(AF, ZERO_S(DE & 0x0020) | HALF_V | CARRY_F(AF));
            totalCycles += 2;
            break;
        case 0x6C:
            AF = LD_nN_n(AF, ZERO_S(HL & 0x2000) | HALF_V | CARRY_F(AF));
            totalCycles += 2;
            break;
        case 0x6D:
            AF = LD_nN_n(AF, ZERO_S(HL & 0x0020) | HALF_V | CARRY_F(AF));
            totalCycles += 2;
            break;
        case 0x6E:
            AF = LD_nN_n(AF, ZERO_S(Memory::readByte(HL) & 0x20) | HALF_V | CARRY_F(AF));
            totalCycles += 4;
            break;
        case 0x77:
            AF = LD_nN_n(AF, ZERO_S(AF & 0x4000) | HALF_V | CARRY_F(AF));
            totalCycles += 2;
            break;
        case 0x70:
            AF = LD_nN_n(AF, ZERO_S(BC & 0x4000) | HALF_V | CARRY_F(AF));
            totalCycles += 2;
            break;
        case 0x71:
            AF = LD_nN_n(AF, ZERO_S(BC & 0x0040) | HALF_V | CARRY_F(AF));
            totalCycles += 2;
            break;
        case 0x72:
            AF = LD_nN_n(AF, ZERO_S(DE & 0x4000) | HALF_V | CARRY_F(AF));
            totalCycles += 2;
            break;
        case 0x73:
            AF = LD_nN_n(AF, ZERO_S(DE & 0x0040) | HALF_V | CARRY_F(AF));
            totalCycles += 2;
            break;
        case 0x74:
            AF = LD_nN_n(AF, ZERO_S(HL & 0x4000) | HALF_V | CARRY_F(AF));
            totalCycles += 2;
            break;
        case 0x75:
            AF = LD_nN_n(AF, ZERO_S(HL & 0x0040) | HALF_V | CARRY_F(AF));
            totalCycles += 2;
            break;
        case 0x76:
            AF = LD_nN_n(AF, ZERO_S(Memory::readByte(HL) & 0x40) | HALF_V | CARRY_F(AF));
            totalCycles += 4;
            break;
        case 0x7F:
            AF = LD_nN_n(AF, ZERO_S(AF & 0x8000) | HALF_V | CARRY_F(AF));
            totalCycles += 2;
            break;
        case 0x78:
            AF = LD_nN_n(AF, ZERO_S(BC & 0x8000) | HALF_V | CARRY_F(AF));
            totalCycles += 2;
            break;
        case 0x79:
            AF = LD_nN_n(AF, ZERO_S(BC & 0x0080) | HALF_V | CARRY_F(AF));
            totalCycles += 2;
            break;
        case 0x7A:
            AF = LD_nN_n(AF, ZERO_S(DE & 0x8000) | HALF_V | CARRY_F(AF));
            totalCycles += 2;
            break;
        case 0x7B:
            AF = LD_nN_n(AF, ZERO_S(DE & 0x0080) | HALF_V | CARRY_F(AF));
            totalCycles += 2;
            break;
        case 0x7C:
            AF = LD_nN_n(AF, ZERO_S(HL & 0x8000) | HALF_V | CARRY_F(AF));
            totalCycles += 2;
            break;
        case 0x7D:
            AF = LD_nN_n(AF, ZERO_S(HL & 0x0080) | HALF_V | CARRY_F(AF));
            totalCycles += 2;
            break;
        case 0x7E:
            AF = LD_nN_n(AF, ZERO_S(Memory::readByte(HL) & 0x80) | HALF_V | CARRY_F(AF));
            totalCycles += 4;
            break;

        // SET b,r
        case 0xC7:
            AF = AF | 0x0100;
            totalCycles += 2;
            break;
        case 0xC0:
            BC = BC | 0x0100;
            totalCycles += 2;
            break;
        case 0xC1:
            BC = BC | 0x0001;
            totalCycles += 2;
            break;
        case 0xC2:
            DE = DE | 0x0100;
            totalCycles += 2;
            break;
        case 0xC3:
            DE = DE | 0x0001;
            totalCycles += 2;
            break;
        case 0xC4:
            HL = HL | 0x0100;
            totalCycles += 2;
            break;
        case 0xC5:
            HL = HL | 0x0001;
            totalCycles += 2;
            break;
        case 0xC6:
            Memory::writeByte(HL, Memory::readByte(HL) | 0x01);
            totalCycles += 4;
            break;
        case 0xCF:
            AF = AF | 0x0200;
            totalCycles += 2;
            break;
        case 0xC8:
            BC = BC | 0x0200;
            totalCycles += 2;
            break;
        case 0xC9:
            BC = BC | 0x0002;
            totalCycles += 2;
            break;
        case 0xCA:
            DE = DE | 0x0200;
            totalCycles += 2;
            break;
        case 0xCB:
            DE = DE | 0x0002;
            totalCycles += 2;
            break;
        case 0xCC:
            HL = HL | 0x0200;
            totalCycles += 2;
            break;
        case 0xCD:
            HL = HL | 0x0002;
            totalCycles += 2;
            break;
        case 0xCE:
            Memory::writeByte(HL, Memory::readByte(HL) | 0x02);
            totalCycles += 4;
            break;
        case 0xD7:
            AF = AF | 0x0400;
            totalCycles += 2;
            break;
        case 0xD0:
            BC = BC | 0x0400;
            totalCycles += 2;
            break;
        case 0xD1:
            BC = BC | 0x0004;
            totalCycles += 2;
            break;
        case 0xD2:
            DE = DE | 0x0400;
            totalCycles += 2;
            break;
        case 0xD3:
            DE = DE | 0x0004;
            totalCycles += 2;
            break;
        case 0xD4:
            HL = HL | 0x0400;
            totalCycles += 2;
            break;
        case 0xD5:
            HL = HL | 0x0004;
            totalCycles += 2;
            break;
        case 0xD6:
            Memory::writeByte(HL, Memory::readByte(HL) | 0x04);
            totalCycles += 4;
            break;
        case 0xDF:
            AF = AF | 0x0800;
            totalCycles += 2;
            break;
        case 0xD8:
            BC = BC | 0x0800;
            totalCycles += 2;
            break;
        case 0xD9:
            BC = BC | 0x0008;
            totalCycles += 2;
            break;
        case 0xDA:
            DE = DE | 0x0800;
            totalCycles += 2;
            break;
        case 0xDB:
            DE = DE | 0x0008;
            totalCycles += 2;
            break;
        case 0xDC:
            HL = HL | 0x0800;
            totalCycles += 2;
            break;
        case 0xDD:
            HL = HL | 0x0008;
            totalCycles += 2;
            break;
        case 0xDE:
            Memory::writeByte(HL, Memory::readByte(HL) | 0x08);
            totalCycles += 4;
            break;
        case 0xE7:
            AF = AF | 0x1000;
            totalCycles += 2;
            break;
        case 0xE0:
            BC = BC | 0x1000;
            totalCycles += 2;
            break;
        case 0xE1:
            BC = BC | 0x0010;
            totalCycles += 2;
            break;
        case 0xE2:
            DE = DE | 0x1000;
            totalCycles += 2;
            break;
        case 0xE3:
            DE = DE | 0x0010;
            totalCycles += 2;
            break;
        case 0xE4:
            HL = HL | 0x1000;
            totalCycles += 2;
            break;
        case 0xE5:
            HL = HL | 0x0010;
            totalCycles += 2;
            break;
        case 0xE6:
            Memory::writeByte(HL, Memory::readByte(HL) | 0x10);
            totalCycles += 4;
            break;
        case 0xEF:
            AF = AF | 0x2000;
            totalCycles += 2;
            break;
        case 0xE8:
            BC = BC | 0x2000;
            totalCycles += 2;
            break;
        case 0xE9:
            BC = BC | 0x0020;
            totalCycles += 2;
            break;
        case 0xEA:
            DE = DE | 0x2000;
            totalCycles += 2;
            break;
        case 0xEB:
            DE = DE | 0x0020;
            totalCycles += 2;
            break;
        case 0xEC:
            HL = HL | 0x2000;
            totalCycles += 2;
            break;
        case 0xED:
            HL = HL | 0x0020;
            totalCycles += 2;
            break;
        case 0xEE:
            Memory::writeByte(HL, Memory::readByte(HL) | 0x20);
            totalCycles += 4;
            break;
        case 0xF7:
            AF = AF | 0x4000;
            totalCycles += 2;
            break;
        case 0xF0:
            BC = BC | 0x4000;
            totalCycles += 2;
            break;
        case 0xF1:
            BC = BC | 0x0040;
            totalCycles += 2;
            break;
        case 0xF2:
            DE = DE | 0x4000;
            totalCycles += 2;
            break;
        case 0xF3:
            DE = DE | 0x0040;
            totalCycles += 2;
            break;
        case 0xF4:
            HL = HL | 0x4000;
            totalCycles += 2;
            break;
        case 0xF5:
            HL = HL | 0x0040;
            totalCycles += 2;
            break;
        case 0xF6:
            Memory::writeByte(HL, Memory::readByte(HL) | 0x40);
            totalCycles += 4;
            break;
        case 0xFF:
            AF = AF | 0x8000;
            totalCycles += 2;
            break;
        case 0xF8:
            BC = BC | 0x8000;
            totalCycles += 2;
            break;
        case 0xF9:
            BC = BC | 0x0080;
            totalCycles += 2;
            break;
        case 0xFA:
            DE = DE | 0x8000;
            totalCycles += 2;
            break;
        case 0xFB:
            DE = DE | 0x0080;
            totalCycles += 2;
            break;
        case 0xFC:
            HL = HL | 0x8000;
            totalCycles += 2;
            break;
        case 0xFD:
            HL = HL | 0x0080;
            totalCycles += 2;
            break;
        case 0xFE:
            Memory::writeByte(HL, Memory::readByte(HL) | 0x80);
            totalCycles += 4;
            break;

        // RES b,r
        case 0x87:
            AF = AF & 0xFEFF;
            totalCycles += 2;
            break;
        case 0x80:
            BC = BC & 0xFEFF;
            totalCycles += 2;
            break;
        case 0x81:
            BC = BC & 0xFFFE;
            totalCycles += 2;
            break;
        case 0x82:
            DE = DE & 0xFEFF;
            totalCycles += 2;
            break;
        case 0x83:
            DE = DE & 0xFFFE;
            totalCycles += 2;
            break;
        case 0x84:
            HL = HL & 0xFEFF;
            totalCycles += 2;
            break;
        case 0x85:
            HL = HL & 0xFFFE;
            totalCycles += 2;
            break;
        case 0x86:
            Memory::writeByte(HL, Memory::readByte(HL) & 0xFE);
            totalCycles += 4;
            break;
        case 0x8F:
            AF = AF & 0xFDFF;
            totalCycles += 2;
            break;
        case 0x88:
            BC = BC & 0xFDFF;
            totalCycles += 2;
            break;
        case 0x89:
            BC = BC & 0xFFFD;
            totalCycles += 2;
            break;
        case 0x8A:
            DE = DE & 0xFDFF;
            totalCycles += 2;
            break;
        case 0x8B:
            DE = DE & 0xFFFD;
            totalCycles += 2;
            break;
        case 0x8C:
            HL = HL & 0xFDFF;
            totalCycles += 2;
            break;
        case 0x8D:
            HL = HL & 0xFFFD;
            totalCycles += 2;
            break;
        case 0x8E:
            Memory::writeByte(HL, Memory::readByte(HL) & 0xFD);
            totalCycles += 4;
            break;
        case 0x97:
            AF = AF & 0xFBFF;
            totalCycles += 2;
            break;
        case 0x90:
            BC = BC & 0xFBFF;
            totalCycles += 2;
            break;
        case 0x91:
            BC = BC & 0xFFFB;
            totalCycles += 2;
            break;
        case 0x92:
            DE = DE & 0xFBFF;
            totalCycles += 2;
            break;
        case 0x93:
            DE = DE & 0xFFFB;
            totalCycles += 2;
            break;
        case 0x94:
            HL = HL & 0xFBFF;
            totalCycles += 2;
            break;
        case 0x95:
            HL = HL & 0xFFFB;
            totalCycles += 2;
            break;
        case 0x96:
            Memory::writeByte(HL, Memory::readByte(HL) & 0xFB);
            totalCycles += 4;
            break;
        case 0x9F:
            AF = AF & 0xF7FF;
            totalCycles += 2;
            break;
        case 0x98:
            BC = BC & 0xF7FF;
            totalCycles += 2;
            break;
        case 0x99:
            BC = BC & 0xFFF7;
            totalCycles += 2;
            break;
        case 0x9A:
            DE = DE & 0xF7FF;
            totalCycles += 2;
            break;
        case 0x9B:
            DE = DE & 0xFFF7;
            totalCycles += 2;
            break;
        case 0x9C:
            HL = HL & 0xF7FF;
            totalCycles += 2;
            break;
        case 0x9D:
            HL = HL & 0xFFF7;
            totalCycles += 2;
            break;
        case 0x9E:
            Memory::writeByte(HL, Memory::readByte(HL) & 0xF7);
            totalCycles += 4;
            break;
        case 0xA7:
            AF = AF & 0xEFFF;
            totalCycles += 2;
            break;
        case 0xA0:
            BC = BC & 0xEFFF;
            totalCycles += 2;
            break;
        case 0xA1:
            BC = BC & 0xFFEF;
            totalCycles += 2;
            break;
        case 0xA2:
            DE = DE & 0xEFFF;
            totalCycles += 2;
            break;
        case 0xA3:
            DE = DE & 0xFFEF;
            totalCycles += 2;
            break;
        case 0xA4:
            HL = HL & 0xEFFF;
            totalCycles += 2;
            break;
        case 0xA5:
            HL = HL & 0xFFEF;
            totalCycles += 2;
            break;
        case 0xA6:
            Memory::writeByte(HL, Memory::readByte(HL) & 0xEF);
            totalCycles += 4;
            break;
        case 0xAF:
            AF = AF & 0xDFFF;
            totalCycles += 2;
            break;
        case 0xA8:
            BC = BC & 0xDFFF;
            totalCycles += 2;
            break;
        case 0xA9:
            BC = BC & 0xFFDF;
            totalCycles += 2;
            break;
        case 0xAA:
            DE = DE & 0xDFFF;
            totalCycles += 2;
            break;
        case 0xAB:
            DE = DE & 0xFFDF;
            totalCycles += 2;
            break;
        case 0xAC:
            HL = HL & 0xDFFF;
            totalCycles += 2;
            break;
        case 0xAD:
            HL = HL & 0xFFDF;
            totalCycles += 2;
            break;
        case 0xAE:
            Memory::writeByte(HL, Memory::readByte(HL) & 0xDF);
            totalCycles += 4;
            break;
        case 0xB7:
            AF = AF & 0xBFFF;
            totalCycles += 2;
            break;
        case 0xB0:
            BC = BC & 0xBFFF;
            totalCycles += 2;
            break;
        case 0xB1:
            BC = BC & 0xFFBF;
            totalCycles += 2;
            break;
        case 0xB2:
            DE = DE & 0xBFFF;
            totalCycles += 2;
            break;
        case 0xB3:
            DE = DE & 0xFFBF;
            totalCycles += 2;
            break;
        case 0xB4:
            HL = HL & 0xBFFF;
            totalCycles += 2;
            break;
        case 0xB5:
            HL = HL & 0xFFBF;
            totalCycles += 2;
            break;
        case 0xB6:
            Memory::writeByte(HL, Memory::readByte(HL) & 0xBF);
            totalCycles += 4;
            break;
        case 0xBF:
            AF = AF & 0x7FFF;
            totalCycles += 2;
            break;
        case 0xB8:
            BC = BC & 0x7FFF;
            totalCycles += 2;
            break;
        case 0xB9:
            BC = BC & 0xFF7F;
            totalCycles += 2;
            break;
        case 0xBA:
            DE = DE & 0x7FFF;
            totalCycles += 2;
            break;
        case 0xBB:
            DE = DE & 0xFF7F;
            totalCycles += 2;
            break;
        case 0xBC:
            HL = HL & 0x7FFF;
            totalCycles += 2;
            break;
        case 0xBD:
            HL = HL & 0xFF7F;
            totalCycles += 2;
            break;
        case 0xBE:
            Memory::writeByte(HL, Memory::readByte(HL) & 0x7F);
            totalCycles += 4;
            break;

        // SWAP n
        case 0x37:
            AF = LD_Nn_Nn(AF, ((AF & 0xF000) >> 4) | ((AF & 0x0F00) << 4));
            AF = LD_nN_n(AF, ZERO_S(AF & 0xFF00));
            totalCycles += 2;
            break;
        case 0x30:
            BC = LD_Nn_Nn(BC, ((BC & 0xF000) >> 4) | ((BC & 0x0F00) << 4));
            AF = LD_nN_n(AF, ZERO_S(BC & 0xFF00));
            totalCycles += 2;
            break;
        case 0x31:
            BC = LD_nN_nN(BC, ((BC & 0x00F0) >> 4) | ((BC & 0x000F) << 4));
            AF = LD_nN_n(AF, ZERO_S(BC & 0x00FF));
            totalCycles += 2;
            break;
        case 0x32:
            DE = LD_Nn_Nn(DE, ((DE & 0xF000) >> 4) | ((DE & 0x0F00) << 4));
            AF = LD_nN_n(AF, ZERO_S(DE & 0xFF00));
            totalCycles += 2;
            break;
        case 0x33:
            DE = LD_nN_nN(DE, ((DE & 0x00F0) >> 4) | ((DE & 0x000F) << 4));
            AF = LD_nN_n(AF, ZERO_S(DE & 0x00FF));
            totalCycles += 2;
            break;
        case 0x34:
            HL = LD_Nn_Nn(HL, ((HL & 0xF000) >> 4) | ((HL & 0x0F00) << 4));
            AF = LD_nN_n(AF, ZERO_S(HL & 0xFF00));
            totalCycles += 2;
            break;
        case 0x35:
            HL = LD_nN_nN(HL, ((HL & 0x00F0) >> 4) | ((HL & 0x000F) << 4));
            AF = LD_nN_n(AF, ZERO_S(HL & 0x00FF));
            totalCycles += 2;
            break;
        case 0x36:
            Memory::writeByte(HL, ((Memory::readByte(HL) & 0xF0) >> 4) | ((Memory::readByte(HL) & 0x0F) << 4));
            AF = LD_nN_n(AF, ZERO_S(Memory::readByte(HL)));
            totalCycles += 4;
            break;

        default:
            Serial.printf("%02x NOT IMPLEMENTED (at %04x)\n\n", op, PC - 2);
            stopAndRestart();
        }

        break;

    // DAA
    case 0x27:
        n = 0;
        if (HALF_F(AF) == HALF_V || (SUB_F(AF) == 0 && (AF & 0x0F00) > 0x0900)) {
            n = 6;
        }
        if (CARRY_F(AF) == CARRY_V || (SUB_F(AF) == 0 && (AF & 0xFF00) > 0x9900)) {
            n = n | 0x60;
        }
        AF = LD_Nn_n(AF, (AF >> 8) + (SUB_F(AF) == 0 ? n : -n));
        AF = LD_nN_n(AF, ZERO_S(AF & 0xFF00) | SUB_F(AF) | (n > 6 ? CARRY_V : 0));
        totalCycles += 1;
        break;

    // CPL
    case 0x2F:
        AF = LD_Nn_Nn(AF, ~AF);
        AF = LD_nN_n(AF, ZERO_F(AF) | SUB_V | HALF_V | CARRY_F(AF));
        totalCycles += 1;
        break;

    // CCF
    case 0x3F:
        AF = LD_nN_n(AF, ZERO_F(AF) | (CARRY_F(AF) == 0 ? CARRY_V : 0));
        totalCycles += 1;
        break;

    // SCF
    case 0x37:
        AF = LD_nN_n(AF, ZERO_F(AF) | CARRY_V);
        totalCycles += 1;
        break;

    // DI
    case 0xF3:
        disableIRQ = 2;
        totalCycles += 1;
        break;

    // EI
    case 0xFB:
        enableIRQ = 2;
        totalCycles += 1;
        break;

    // JP nn
    case 0xC3:
        PC = readNn();
        totalCycles += 3;
        break;

    // JP cc,nn
    case 0xC2:
        nn = readNn();
        if (ZERO_F(AF) == 0)
        {
            PC = nn;
        }
        totalCycles += 3;
        break;
    case 0xCA:
        nn = readNn();
        if (ZERO_F(AF) == ZERO_V)
        {
            PC = nn;
        }
        totalCycles += 3;
        break;
    case 0xD2:
        nn = readNn();
        if (CARRY_F(AF) == 0)
        {
            PC = nn;
        }
        totalCycles += 3;
        break;
    case 0xDA:
        nn = readNn();
        if (CARRY_F(AF) == CARRY_V)
        {
            PC = nn;
        }
        totalCycles += 3;
        break;

    // JP (HL)
    case 0xE9:
        PC = HL;
        totalCycles += 1;
        break;

    // JR n
    case 0x18:
        PC += (int8_t)readOp();
        totalCycles += 2;
        break;

    // JR cc,n
    case 0x20:
        n = readOp();
        if (ZERO_F(AF) == 0)
        {
            PC += (int8_t)n;
        }
        totalCycles += 2;
        break;
    case 0x28:
        n = readOp();
        if (ZERO_F(AF) == ZERO_V)
        {
            PC += (int8_t)n;
        }
        totalCycles += 2;
        break;
    case 0x30:
        n = readOp();
        if (CARRY_F(AF) == 0)
        {
            PC += (int8_t)n;
        }
        totalCycles += 2;
        break;
    case 0x38:
        n = readOp();
        if (CARRY_F(AF) == CARRY_V)
        {
            PC += (int8_t)n;
        }
        totalCycles += 2;
        break;

    // CALL nn
    case 0xCD:
        nn = readNn();
        pushStack(PC);
        PC = nn;
        totalCycles += 3;
        break;

    // CALL cc,nn
    case 0xC4:
        nn = readNn();
        if (ZERO_F(AF) == 0)
        {
            pushStack(PC);
            PC = nn;
        }
        totalCycles += 3;
        break;
    case 0xCC:
        nn = readNn();
        if (ZERO_F(AF) == ZERO_V)
        {
            pushStack(PC);
            PC = nn;
        }
        totalCycles += 3;
        break;
    case 0xD4:
        nn = readNn();
        if (CARRY_F(AF) == 0)
        {
            pushStack(PC);
            PC = nn;
        }
        totalCycles += 3;
        break;
    case 0xDC:
        nn = readNn();
        if (CARRY_F(AF) == CARRY_V)
        {
            pushStack(PC);
            PC = nn;
        }
        totalCycles += 3;
        break;

    // RST n
    case 0xC7:
        pushStack(PC);
        PC = 0x00;
        totalCycles += 8;
        break;
    case 0xCF:
        pushStack(PC);
        PC = 0x08;
        totalCycles += 8;
        break;
    case 0xD7:
        pushStack(PC);
        PC = 0x10;
        totalCycles += 8;
        break;
    case 0xDF:
        pushStack(PC);
        PC = 0x18;
        totalCycles += 8;
        break;
    case 0xE7:
        pushStack(PC);
        PC = 0x20;
        totalCycles += 8;
        break;
    case 0xEF:
        pushStack(PC);
        PC = 0x28;
        totalCycles += 8;
        break;
    case 0xF7:
        pushStack(PC);
        PC = 0x30;
        totalCycles += 8;
        break;
    case 0xFF:
        pushStack(PC);
        PC = 0x38;
        totalCycles += 8;
        break;

    // RET
    case 0xC9:
        PC = popStack();
        totalCycles += 2;
        break;

    // RET cc
    case 0xC0:
        if (ZERO_F(AF) == 0) {
            PC = popStack();
        }
        totalCycles += 2;
        break;
    case 0xC8:
        if (ZERO_F(AF) == ZERO_V) {
            PC = popStack();
        }
        totalCycles += 2;
        break;
    case 0xD0:
        if (CARRY_F(AF) == 0) {
            PC = popStack();
        }
        totalCycles += 2;
        break;
    case 0xD8:
        if (CARRY_F(AF) == CARRY_V) {
            PC = popStack();
        }
        totalCycles += 2;
        break;

    // RETI
    case 0xD9:
        PC = popStack();
        enableIRQ = 2;
        totalCycles += 2;
        break;

    default:
        Serial.printf("%02x NOT IMPLEMENTED (at %04x)\n\n", op, PC - 1);
        stopAndRestart();
        break;
    }

    if (enableIRQ != 0 && --enableIRQ == 0) {
        IME = 1;
    }

    if (disableIRQ != 0 && --disableIRQ == 0) {
        IME = 0;
    }
}