//
// Created by Jaron on 9/3/2024.
//

#include "Chip8.h"
#include "Font.h"
#include <fstream>
#include <chrono>
#include <random>
#include <string.h>

const unsigned int START_ADDRESS = 0x200;
const unsigned int FONTSET_START_ADDRESS = 0x50;

Chip8::Chip8() : randGen(std::chrono::system_clock::now().time_since_epoch().count()) {
    // Initialize
    pc = START_ADDRESS;

    // Initialize RNG
    randByte = std::uniform_int_distribution<uint8_t>(0, 255U);

    for (unsigned int i = 0; i < FONTSET_SIZE; ++i) {
        memory[FONTSET_START_ADDRESS + i] = fontset[i];
    }
}

void Chip8::OP_4xkk() {
    // Bit mask to get register to check
    uint8_t Vx = (opcode & 0x0F00u) >> 8u;
    // Bit mask to get value passed for check
    uint8_t byte = opcode & 0x00FFu;

    // If the value at the register is not equal to the passed byte, skip the next instruction.
    if (registers[Vx] != byte) {
        pc += 2;
    }
}

void Chip8::OP_3xkk() {
    // Bit mask to get the bits from the second half of the first byte in the opcode, or register location of value to
    // check against
    uint8_t Vx = (opcode & 0x0F00u) >> 8u;
    // Bit mask to get the last byte of the opcode or the equality value
    uint8_t byte = opcode & 0x00FFu;

    // If the value in the passed register is equal to the value passed, skip the next instruction.
    if (registers[Vx] == byte) {
        pc += 2;
    }

}

void Chip8::OP_2nnn() {
    // Get address location from instruction
    uint16_t address = opcode & 0x0FFFu;
    // Set current stack frame to the address currently in the pc.
    stack[sp] = pc;
    // Increment stack pointer
    ++sp;
    // Set the pc to the address we "call"
    pc = address;
}

void Chip8::OP_1nnn() {
    // Bit mask to keep the lower 12 bits from opcode
    uint16_t address = opcode & 0x0FFFu;
    pc = address;
}

void Chip8::OP_00EE() {
    // Decrement stack pointer to last instruction
    --sp;
    // Set the pc to the last instruction on the stack
    pc = stack[sp];
}

void Chip8::OP_00E0() {
    // This is very c. Need to see if there is a better way to do this.
    memset(display, 0, sizeof(display));
}

void Chip8::LoadROM(char const* filename) {
    // Open the file as a stream of binary and move the file pointer to the end
    std::ifstream file(filename, std::ios::binary | std::ios::ate);

    if (!file.is_open()) {
        // Get size of file and allocate a buffer to hold the contents
        std::streampos size = file.tellg();
        char* buffer = new char[size];

        // Go back to the beginning of the file and fill the buffer
        file.seekg(0, std::ios::beg);
        file.read(buffer, size);
        file.close();

        // Move the contents we read into the memory of the emulator starting at the correct starting address.
        for (long i = 0; i < size; ++i) {
            memory[START_ADDRESS + i] = buffer[i];
        }

        delete[] buffer;
    }
}