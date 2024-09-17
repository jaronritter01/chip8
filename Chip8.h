//
// Created by Jaron on 9/3/2024.
//

#ifndef CHIP8_H
#define CHIP8_H

#include <cstdint>
#include <random>


class Chip8 {
public:
    uint8_t registers[16]{};
    uint8_t memory[4096]{};
    uint16_t index{};
    uint16_t pc{};
    uint16_t stack[16]{};
    uint8_t sp{};
    uint8_t delayTimer{};
    uint8_t soundTimer{};
    uint8_t keys[16]{};
    uint32_t display[64 * 32]{};
    uint16_t opcode{};
    std::default_random_engine randGen;
    std::uniform_int_distribution<uint8_t> randByte;

    Chip8();

    void OP_7xkk();

    void OP_6xkk();

    void OP_5xy0();

    void OP_4xkk();

    void OP_3xkk();

    void OP_2nnn();

    void OP_1nnn();

    void OP_00EE();

    void OP_00E0();

    void LoadROM(char const *filename);
};



#endif //CHIP8_H
