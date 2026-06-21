
#include "VM.h"
#include <iostream>
#include <stdexcept>

int main(int argc, char *argv[]) {
    if (argc != 2) {
        SDL_Log("Usage: chip8 rom_file_path");
        return EXIT_FAILURE;
    }

    try {
        VM vm{argv[1]};
        vm.run();
    } catch (const std::runtime_error e) {
        std::cerr << e.what() << std::endl;
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}