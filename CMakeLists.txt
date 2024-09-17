cmake_minimum_required(VERSION 3.29)
project(chip8)

set(CMAKE_CXX_STANDARD 20)

add_executable(chip8 main.cpp
        Chip8.cpp
        Chip8.h
        Font.cpp
        Font.h)
