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
 * Fx65 - LD Vx, [I]
 *
 * Read registers V0 through Vx from memory starting at location I.
 */
void Chip8::OP_Fx65()
{
    uint8_t Vx = (opcode & 0x0F00u) >> 8u;

    for (uint8_t i = 0; i <= Vx; ++i)
    {
        registers[i] = memory[index + i];
    }
}

/**
 * Fx55 - LD [I], Vx
 *
 * Store registers V0 through Vx in memory starting at location I.
 */
void Chip8::OP_Fx55() {
    uint8_t Vx = (opcode & 0x0F00u) >> 8u;

    for (uint8_t i = 0; i <= Vx; ++i)
    {
        memory[index + i] = registers[i];
    }
}

/**
 * Fx33 - LD B, Vx
 *
 * Store BCD representation of Vx in memory locations I, I+1, and I+2.
 */
void Chip8::OP_Fx33() {
    uint8_t Vx = (opcode & 0x0F00u) >> 8u;
    uint8_t value = registers[Vx];

    // Ones-place
    memory[index + 2] = value % 10;
    value /= 10;

    // Tens-place
    memory[index + 1] = value % 10;
    value /= 10;

    // Hundreds-place
    memory[index] = value % 10;
}

/**
 * Fx29 - LD F, Vx
 *
 * Set I = location of sprite for digit Vx.
 */
void Chip8::OP_Fx29() {
    uint8_t Vx = (opcode & 0x0F00u) >> 8u;
    uint8_t digit = registers[Vx];

    index = FONTSET_START_ADDRESS + (5 * digit);
}

/**
 * Fx1E - ADD I, Vx
 *
 * Set I = I + Vx.
 */
void Chip8::OP_Fx1E() {
    uint8_t Vx = (opcode & 0x0F00u) >> 8u;

    index += registers[Vx];
}

/**
 * Fx18 - LD ST, Vx
 *
 * Set sound timer = Vx.
 */
void Chip8::OP_Fx18() {
    uint8_t Vx = (opcode & 0x0F00u) >> 8u;

    soundTimer = registers[Vx];
}

/**
 * Fx15 - LD DT, Vx
 *
 * Set delay timer = Vx.
 */
void Chip8::OP_Fx15() {
    uint8_t Vx = (opcode & 0x0F00u) >> 8u;

    delayTimer = registers[Vx];
}


/**
 * Fx0A - LD Vx, K
 *
 * Wait for a key press, store the value of the key in Vx.
 */
void Chip8::OP_Fx0A() {
    uint8_t Vx = (opcode & 0x0F00u) >> 8u;

    if (keys[0]) {
        registers[Vx] = 0;
    }
    else if (keys[1]) {
        registers[Vx] = 1;
    }
    else if (keys[2]) {
        registers[Vx] = 2;
    }
    else if (keys[3]) {
        registers[Vx] = 3;
    }
    else if (keys[4]) {
        registers[Vx] = 4;
    }
    else if (keys[5]) {
        registers[Vx] = 5;
    }
    else if (keys[6]) {
        registers[Vx] = 6;
    }
    else if (keys[7]) {
        registers[Vx] = 7;
    }
    else if (keys[8]) {
        registers[Vx] = 8;
    }
    else if (keys[9]) {
        registers[Vx] = 9;
    }
    else if (keys[10]) {
        registers[Vx] = 10;
    }
    else if (keys[11]) {
        registers[Vx] = 11;
    }
    else if (keys[12]) {
        registers[Vx] = 12;
    }
    else if (keys[13]) {
        registers[Vx] = 13;
    }
    else if (keys[14]) {
        registers[Vx] = 14;
    }
    else if (keys[15]) {
        registers[Vx] = 15;
    }
    else {
        pc -= 2;
    }
}

/**
 * Fx07 - LD Vx, DT
 *
 * Set Vx = delay timer value.
 */
void Chip8::OP_Fx07() {
    uint8_t Vx = (opcode & 0x0F00u) >> 8u;

    registers[Vx] = delayTimer;
}

/**
 * ExA1 - SKNP Vx
 *
 * Skip next instruction if key with the value of Vx is not pressed.
 */
void Chip8::OP_ExA1() {
    uint8_t Vx = (opcode & 0x0F00u) >> 8u;

    uint8_t key = registers[Vx];

    if (!keys[key])
    {
        pc += 2;
    }
}


/**
 * Ex9E - SKP Vx
 *
 * Skip next instruction if key with the value of Vx is pressed.
 */
void Chip8::OP_Ex9E() {
    uint8_t Vx = (opcode & 0x0F00u) >> 8u;

    uint8_t key = registers[Vx];

    if (keys[key])
    {
        pc += 2;
    }
}


/**
 * Dxyn - DRW Vx, Vy, nibble
 *
 * Display n-byte sprite starting at memory location I at (Vx, Vy), set VF = collision.
 */
void Chip8::OP_Dxyn() {
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
void Chip8::OP_Cxkk() {
    uint8_t Vx = (opcode & 0x0F00u) >> 8u;
    uint8_t byte = opcode & 0x00FFu;

    registers[Vx] = randByte(randGen) & byte;
}

/**
 * Bnnn - JP V0, addr
 *
 * Jump to location nnn + V0.
 */
void Chip8::OP_Bnnn() {
    uint16_t address = opcode & 0x0FFFu;

    pc = registers[0] + address;
}

/**
 * Annn - LD I, addr
 *
 * Set I = nnn.
 */
void Chip8::OP_Annn() {
    uint16_t address = opcode & 0x0FFFu;

    index = address;
}

/**
 * 9xy0 - SNE Vx, Vy
 *
 * Skip next instruction if Vx != Vy.
 */
void Chip8::OP_9xy0() {
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
void Chip8::OP_8xyE() {
    uint8_t Vx = (opcode & 0x0F00u) >> 8u;

    // Save MSB in VF
    registers[0xF] = (registers[Vx] & 0x80u) >> 7u;

    registers[Vx] <<= 1;
}

/**
 * 8xy7 - SUBN Vx, Vy
 * Nearly the same as the other sub function except the operands are switched.
 */
void Chip8::OP_8xy7() {
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
void Chip8::OP_8xy6() {
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
void Chip8::OP_8xy5() {
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