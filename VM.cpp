//
// Created by clip on 6/17/26.
//

#include "VM.h"

#include <fstream>
#include <random>
#include <stdexcept>

static constexpr int SCALE = 10;
static constexpr int W_WIDTH = CHIP8_VIDEO_WIDTH * SCALE;
static constexpr int W_HEIGHT = CHIP8_VIDEO_HEIGHT * SCALE;

VM::VM(const std::string file_path) {
    chip8_.loadRom(loadRomFromFile(file_path));

    if (!SDL_Init(SDL_INIT_VIDEO)) {
        throw std::runtime_error{"SDL could not initialize: "+ std::string{SDL_GetError()}};
    }

    /* Create the window */
    if (!SDL_CreateWindowAndRenderer("CHIP-8", W_WIDTH, W_HEIGHT, 0, &window_, &renderer_)) {
        throw std::runtime_error{"Couldn't create window and renderer: " + std::string{SDL_GetError()}};
    }

    if (!SDL_SetRenderVSync(renderer_, 1)) {
        throw std::runtime_error{"Couldn't set vsync: " + std::string{SDL_GetError()}};
    }

    texture_ = SDL_CreateTexture(renderer_, SDL_PIXELFORMAT_RGBA8888, SDL_TEXTUREACCESS_STREAMING, CHIP8_VIDEO_WIDTH, CHIP8_VIDEO_HEIGHT);

    if (texture_ == nullptr) {
        throw std::runtime_error{"Couldn't create texture: " + std::string{SDL_GetError()}};
    }

    if (!SDL_SetTextureScaleMode(texture_, SDL_SCALEMODE_PIXELART)) {
        throw std::runtime_error{"Couldn't set texture scale: " + std::string{SDL_GetError()}};
    }
}

VM::~VM() {
    SDL_DestroyTexture(texture_);
    SDL_DestroyRenderer(renderer_);
    SDL_DestroyWindow(window_);
    SDL_Quit();
}

void VM::run() {
    std::random_device rd;  // a seed source for the random number engine
    std::mt19937 gen(rd()); // mersenne_twister_engine seeded with rd()
    std::uniform_int_distribution<> distribution(0, 255);

    while (!quit_) {
        bool halt{};

        if (reset_) {
            chip8_.reset();
            reset_ = false;
        }

        chip8_.updateTimers();

        for (int i = 0; i < 10 && !quit_ && !halt; i++) {
            handleInput();
            halt = chip8_.execute(keys_, distribution(gen));
        }

        updateGraphics();
        draw();
    }
}

void VM::handleInput() {
    SDL_Event event;
    SDL_zero(event);

    while(SDL_PollEvent( &event )) {
        if (event.type == SDL_EVENT_QUIT) {
            quit_ = true;
            break;
        }

        if (event.type == SDL_EVENT_KEY_DOWN || event.type == SDL_EVENT_KEY_UP) {
            const bool val = event.type == SDL_EVENT_KEY_DOWN;

            switch (event.key.key) {
                case SDLK_SPACE:
                    reset_ = true;
                    break;
                case SDLK_ESCAPE:
                    quit_ = true;
                    break;
                case SDLK_1:
                    keys_[1] = val;
                    break;
                case SDLK_2:
                    keys_[2] = val;
                    break;
                case SDLK_3:
                    keys_[3] = val;
                    break;
                case SDLK_4:
                    keys_[12] = val;
                    break;
                case SDLK_Q:
                    keys_[4] = val;
                    break;
                case SDLK_W:
                    keys_[5] = val;
                    break;
                case SDLK_E:
                    keys_[6] = val;
                    break;
                case SDLK_R:
                    keys_[13] = val;
                    break;
                case SDLK_A:
                    keys_[7] = val;
                    break;
                case SDLK_S:
                    keys_[8] = val;
                    break;
                case SDLK_D:
                    keys_[9] = val;
                    break;
                case SDLK_F:
                    keys_[14] = val;
                    break;
                case SDLK_Z:
                    keys_[10] = val;
                    break;
                case SDLK_X:
                    keys_[0] = val;
                    break;
                case SDLK_C:
                    keys_[11] = val;
                    break;
                case SDLK_V:
                    keys_[15] = val;
                    break;
                default:
                    break;
            }
        }
    }
}

void VM::updateGraphics() {
    const auto vram = chip8_.vram();

    for (int i = 0; i < CHIP8_VIDEO_HEIGHT; i++) {
        for (int j = 0; j < CHIP8_VIDEO_WIDTH; j++) {
            if (vram[i * CHIP8_VIDEO_WIDTH + j]) {
                pixels_[i * CHIP8_VIDEO_WIDTH + j] =  0xf3c004ff;
            } else {
                pixels_[i * CHIP8_VIDEO_WIDTH + j] =  0;
            }
        }
    }

    SDL_UpdateTexture(texture_, nullptr, &pixels_, sizeof(uint32_t) * CHIP8_VIDEO_WIDTH);
}

void VM::draw() {
    SDL_RenderClear(renderer_);
    SDL_RenderTexture(renderer_, texture_, nullptr, nullptr);
    SDL_RenderPresent(renderer_);
}

std::vector<char> VM::loadRomFromFile(const std::filesystem::path path) {
    std::ifstream rom_file{path, std::ios::binary};

    if (!rom_file) {
        throw std::runtime_error{"Unable to open " + path.string()};
    }

    auto file_size = std::filesystem::file_size(path);
    std::vector<char> buff(file_size);
    rom_file.read(buff.data(), file_size);

    return buff;
}

