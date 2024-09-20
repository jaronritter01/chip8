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


/**
 * Dxyn - DRW Vx, Vy, nibble
 *
 * Display n-byte sprite starting at memory location I at (Vx, Vy), set VF = collision.
 */
void Chip8::OP_Dxyn()
{
    uint8_t Vx = (opcode & 0x0F00u) >> 8u;
    uint8_t Vy = (opcode & 0x00F0u) >> 4u;
    uint8_t height = opcode & 0x000Fu;

    // Wrap if going beyond screen boundaries
    uint8_t xPos = registers[Vx] % VIDEO_WIDTH;
    uint8_t yPos = registers[Vy] % VIDEO_HEIGHT;

    registers[0xF] = 0;

    for (unsigned int row = 0; row < height; ++row)
    {
        uint8_t spriteByte = memory[index + row];

        for (unsigned int col = 0; col < 8; ++col)
        {
            // This is effectively iterating over each byte in the sprite
            uint8_t spritePixel = spriteByte & (0x80u >> col);
            // This is a neat trick to map (x,y) cord to a single dimension array
            uint32_t* screenPixel = &display[(yPos + row) * VIDEO_WIDTH + (xPos + col)];

            // Sprite pixel is on
            if (spritePixel)
            {
                // Screen pixel also on - collision
                if (*screenPixel == 0xFFFFFFFF)
                {
                    registers[0xF] = 1;
                }

                // Effectively XOR with the sprite pixel
                *screenPixel ^= 0xFFFFFFFF;
            }
        }
    }
}

/**
 * Cxkk - RND Vx, byte
 *
 * Set Vx = random byte AND kk.
 */
void Chip8::OP_Cxkk()
{
    uint8_t Vx = (opcode & 0x0F00u) >> 8u;
    uint8_t byte = opcode & 0x00FFu;

    registers[Vx] = randByte(randGen) & byte;
}

/**
 * Bnnn - JP V0, addr
 *
 * Jump to location nnn + V0.
 */
void Chip8::OP_Bnnn()
{
    uint16_t address = opcode & 0x0FFFu;

    pc = registers[0] + address;
}

/**
 * Annn - LD I, addr
 *
 * Set I = nnn.
 */
void Chip8::OP_Annn()
{
    uint16_t address = opcode & 0x0FFFu;

    index = address;
}

/**
 * 9xy0 - SNE Vx, Vy
 *
 * Skip next instruction if Vx != Vy.
 */
void Chip8::OP_9xy0()
{
    uint8_t Vx = (opcode & 0x0F00u) >> 8u;
    uint8_t Vy = (opcode & 0x00F0u) >> 4u;

    if (registers[Vx] != registers[Vy])
    {
        pc += 2;
    }
}

/**
 * 8xyE - SHL Vx {, Vy}
 *
 * Set Vx = Vx SHL 1.
 */
void Chip8::OP_8xyE()
{
    uint8_t Vx = (opcode & 0x0F00u) >> 8u;

    // Save MSB in VF
    registers[0xF] = (registers[Vx] & 0x80u) >> 7u;

    registers[Vx] <<= 1;
}

/**
 * 8xy7 - SUBN Vx, Vy
 * Nearly the same as the other sub function except the operands are switched.
 */
void Chip8::OP_8xy7()
{
    uint8_t Vx = (opcode & 0x0F00u) >> 8u;
    uint8_t Vy = (opcode & 0x00F0u) >> 4u;

    if (registers[Vy] > registers[Vx])
    {
        registers[0xF] = 1;
    }
    else
    {
        registers[0xF] = 0;
    }

    registers[Vx] = registers[Vy] - registers[Vx];
}

/**
 * 8xy6 - SHR Vx
 *
 * Set Vx = Vx SHR 1.
 */
void Chip8::OP_8xy6()
{
    uint8_t Vx = (opcode & 0x0F00u) >> 8u;

    // Save LSB in VF
    registers[0xF] = (registers[Vx] & 0x1u);

    registers[Vx] >>= 1;
}

/**
 * 8xy5 - SUB Vx, Vy
 *
 * Set Vx = Vx - Vy, set VF = NOT borrow.
 *
 */
void Chip8::OP_8xy5()
{
    uint8_t Vx = (opcode & 0x0F00u) >> 8u;
    uint8_t Vy = (opcode & 0x00F0u) >> 4u;

    if (registers[Vx] > registers[Vy])
    {
        registers[0xF] = 1;
    }
    else
    {
        registers[0xF] = 0;
    }

    registers[Vx] -= registers[Vy];
}

/**
 * 8xy4 - ADD Vx, Vy
 *
 * Set Vx = Vx + Vy, set VF = carry
 */
void Chip8::OP_8xy4() {
    uint8_t Vx = (opcode & 0x0F00) >> 8u;
    uint8_t Vy = (opcode & 0x00F0) >> 4u;

    uint16_t sum = registers[Vx] + registers[Vy];

    if (sum > 255U)
    {
        registers[0xF] = 1;
    }
    else
    {
        registers[0xF] = 0;
    }

    registers[Vx] = sum & 0xFFu;

}

/**
 * 8xy3 - XOR Vx, Vy
 *
 * Set Vx = Vx XOR Vy
 */
void Chip8::OP_8xy3() {
    uint8_t Vx = (opcode & 0x0F00) >> 8u;
    uint8_t Vy = (opcode & 0x00F0) >> 4u;

    registers[Vx] ^= registers[Vy];
}


/**
 * 8xy2 - AND Vx, Vy
 *
 * Set Vx = Vx AND Vy
 */
void Chip8::OP_8xy2() {
    uint8_t Vx = (opcode & 0x0F00) >> 8u;
    uint8_t Vy = (opcode & 0x00F0) >> 4u;

    registers[Vx] &= registers[Vy];
}

/**
 * 8xy1 - OR Vx, Vy
 *
 * Set Vx = Vx OR Vy
 */
void Chip8::OP_8xy1() {
    uint8_t Vx = (opcode & 0x0F00u) >> 8u;
    uint8_t Vy = (opcode & 0x00F0u) >> 4u;

    registers[Vx] |= registers[Vy];

}

/**
 * 8xy0 - LD Vs, Vy
 *
 * Set Vx = Vy
 */
void Chip8::OP_8xy0() {
    uint8_t Vx = (opcode & 0x0F00u) >> 8u;
    uint8_t Vy = (opcode & 0x00F0u) >> 4u;

    registers[Vx] = registers[Vy];
}

/**
 * 7xkk - ADD Vx, byte
 *
 * Set Vx = Vx + kk
 */
void Chip8::OP_7xkk() {
    uint8_t Vx = (opcode & 0x0F00u) >> 8u;
    uint8_t byte = opcode & 0x00FFu;

    registers[Vx] += byte;
}

/**
 * 6xkk - LD Vx, byte
 *
 * Set Vx = kk.
 */
void Chip8::OP_6xkk() {
    uint8_t Vx = (opcode & 0x0F00u) >> 8u;
    uint8_t byte = opcode & 0x00FFu;

    registers[Vx] = byte;
}

/**
 * 5xy0 - SE Vx, Vy
 *
 * Skip next instruction if Vx = Vy.
 */
void Chip8::OP_5xy0() {
    uint8_t Vx = (opcode & 0x0F00u) >> 8u;
    uint8_t Vy = (opcode & 0x00F0u) >> 4u;

    if (registers[Vx] == registers[Vy])
    {
        pc += 2;
    }
}

/**
 * SNE Vx, byte
 *
 * Skip next instruction if Vx != kk.
 */
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

/**
 * SE Vx, byte
 *
 * Skip next instruction if Vx = kk.
 */
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

/**
 * CALL addr
 *
 * Call subroutine at nnn.
 */
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
/**
 * JP addr
 *
 * Jump to location nnn.
 */
void Chip8::OP_1nnn() {
    // Bit mask to keep the lower 12 bits from opcode
    uint16_t address = opcode & 0x0FFFu;
    pc = address;
}

/**
 * RET
 *
 * Return from a subroutine
 */
void Chip8::OP_00EE() {
    // Decrement stack pointer to last instruction
    --sp;
    // Set the pc to the last instruction on the stack
    pc = stack[sp];
}

/**
 * CLS
 *
 * Clear The Display
 */
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