//
// Created by Jaron on 9/3/2024.
//

#ifndef CHIP8_H
#define CHIP8_H

#include <cstdint>
#include <random>

const unsigned int VIDEO_HEIGHT = 32;
const unsigned int VIDEO_WIDTH = 64;



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
    uint32_t display[VIDEO_WIDTH * VIDEO_HEIGHT]{};
    uint16_t opcode{};
    std::default_random_engine randGen;
    std::uniform_int_distribution<uint8_t> randByte;

    Chip8();

    void OP_Dxyn();

    void OP_Cxkk();

    void OP_Bnnn();

    void OP_Annn();

    void OP_9xy0();

    void OP_8xyE();

    void OP_8xy7();

    void OP_8xy6();

    void OP_8xy5();

    void OP_8xy4();

    void OP_8xy3();

    void OP_8xy2();

    void OP_8xy1();

    void OP_8xy0();

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
