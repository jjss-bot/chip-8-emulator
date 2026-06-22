//
// Created by clip on 6/17/26.
//

#ifndef CHIP8_VM_H
#define CHIP8_VM_H

#include <string>
#include <vector>
#include <array>
#include <filesystem>
#include <SDL3/SDL.h>
#include "Chip8.h"

class VM {
public:
    explicit VM(std::string file_path);
    ~VM();
    void run();

private:
    bool quit_{};
    bool reset_{};
    Chip8 chip8_{};
    std::array<bool, 16> keys_{};
    std::array<uint32_t, CHIP8_VRAM_SIZE> pixels_{};

    SDL_Window *window_{};
    SDL_Texture *texture_{};
    SDL_Renderer *renderer_{};

    void draw();
    void handleInput();
    void updateGraphics();
    static std::vector<char> loadRomFromFile(std::filesystem::path path);
};


#endif //CHIP8_VM_H
