//
// Created by clip on 6/14/26.
//

#ifndef CHIP8_CHIP8_H
#define CHIP8_CHIP8_H

#include <cstdint>
#include <array>
#include <vector>

constexpr int CHIP8_VIDEO_WIDTH = 64;
constexpr int CHIP8_VIDEO_HEIGHT = 32;
constexpr int CHIP8_VRAM_SIZE =  CHIP8_VIDEO_WIDTH * CHIP8_VIDEO_HEIGHT;

class Chip8 {
public:
    Chip8();
    void reset();
    void updateTimers();
    void loadRom(const std::vector<char> &data);
    bool execute(const std::array<bool, 16> &keys, uint8_t rand);
    auto vram() -> std::array<uint8_t, CHIP8_VRAM_SIZE> &;

private:
    uint16_t sp_{};
    uint16_t pc_{};
    uint16_t index_{};

    uint8_t delayTimer_{};
    uint8_t soundTimer_{};

    char keyState{};
    std::array<uint8_t, 16> v_{};
    std::array<uint8_t, 4069> ram_{};
    std::array<uint16_t, 16> stack_{};
    std::array<uint8_t, CHIP8_VRAM_SIZE> vram_{};

    uint16_t pop();
    void push(uint16_t addr);
};

#endif //CHIP8_CHIP8_H
