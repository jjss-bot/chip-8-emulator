//
// Created by clip on 6/14/26.
//

#include "Chip8.h"

constexpr int CHIP8_FONT_START_ADDR = 0x50;

static constexpr std::array<uint8_t, 80> FONT {
     0xF0, 0x90, 0x90, 0x90, 0xF0, // 0
     0x20, 0x60, 0x20, 0x20, 0x70, // 1
     0xF0, 0x10, 0xF0, 0x80, 0xF0, // 2
     0xF0, 0x10, 0xF0, 0x10, 0xF0, // 3
     0x90, 0x90, 0xF0, 0x10, 0x10, // 4
     0xF0, 0x80, 0xF0, 0x10, 0xF0, // 5
     0xF0, 0x80, 0xF0, 0x90, 0xF0, // 6
     0xF0, 0x10, 0x20, 0x40, 0x40, // 7
     0xF0, 0x90, 0xF0, 0x90, 0xF0, // 8
     0xF0, 0x90, 0xF0, 0x10, 0xF0, // 9
     0xF0, 0x90, 0xF0, 0x90, 0x90, // A
     0xE0, 0x90, 0xE0, 0x90, 0xE0, // B
     0xF0, 0x80, 0x80, 0x80, 0xF0, // C
     0xE0, 0x90, 0x90, 0x90, 0xE0, // D
     0xF0, 0x80, 0xF0, 0x80, 0xF0, // E
     0xF0, 0x80, 0xF0, 0x80, 0x80  // F
 };

Chip8::Chip8() {
     /* Load font at address 0x50*/
     for (size_t i = 0; i < FONT.size(); i++) {
          ram_[CHIP8_FONT_START_ADDR + i] = FONT[i];
     }
}

void Chip8::loadRom(const std::vector<char> &data) {
     for (size_t i = 0; i < data.size(); i++) {
          ram_[0x200 + i] = data[i];
     }
}

uint16_t Chip8::pop() {
     return stack_[--sp_ % 16];
}
void Chip8::push(const uint16_t addr) {
     stack_[sp_++ % 16] = addr;
}

auto Chip8::vram() -> std::array<uint8_t, CHIP8_VRAM_SIZE> & {
     return vram_;
}

void Chip8::updateTimers() {
     if (delayTimer_ > 0) {
          delayTimer_--;
     }

     if (soundCounter_ > 0) {
          soundCounter_--;
     }
}

bool Chip8::execute(const std::array<bool, 16> &keys, uint8_t rand) {
     bool halt = false;

     uint16_t op =  ram_[pc_++] << 8;
     op |= ram_[pc_++];

     switch (op & 0xf000) {
          case 0x00: {
               if (op == 0x00E0) {
                    /* Clear the display */
                    for (auto &byte : vram_) {
                         byte = 0;
                    }
               } else if (op == 0x00EE) {
                    /* Return from a subroutine. */
                    pc_ = pop();
               } else {
                    /* Jump to a machine code routine at nnn.*/
                    pc_ = op & 0x0fff;
               }

               break;
          }
          case 0x1000: {
               /* Jump to location nnn. */
               pc_ = op & 0x0fff;
               break;
          }
          case 0x2000: {
               /* Call subroutine at nnn. */
               push(pc_);
               pc_ = op & 0x0fff;
               break;
          }
          case 0x3000: {
               /* Skip next instruction if Vx = kk. */
               uint8_t ix = (op & 0x0f00) >> 8;
               uint8_t value = (op & 0x00ff);

               if (v_[ix] == value) {
                    pc_ += 2;
               }

               break;
          }
          case 0x4000: {
               /* Skip next instruction if Vx != kk. */
               uint8_t ix = (op & 0x0f00) >> 8;
               uint8_t value = (op & 0x00ff);

               if (v_[ix] != value) {
                    pc_ += 2;
               }

               break;
          }
          case 0x5000: {
               /* Skip next instruction if Vx = Vy. */
               uint8_t ix = (op & 0x0f00) >> 8;
               uint8_t iy = (op & 0x00f0) >> 4;

               if (v_[ix] == v_[iy]) {
                    pc_ += 2;
               }

               break;
          }
          case 0x6000: {
               /* Set Vx = kk. */
               uint8_t ix = (op & 0x0f00) >> 8;
               uint8_t value = (op & 0x00ff);

               v_[ix] = value;
               break;
          }
          case 0x7000: {
               /* Set Vx = Vx + kk. */
               uint8_t ix = (op & 0x0f00) >> 8;
               uint8_t value = (op & 0x00ff);

               v_[ix] += value;
               break;
          }
          case 0x8000: {
               uint8_t ix = (op & 0x0f00) >> 8;
               uint8_t iy = (op & 0x00f0) >> 4;

               switch (op & 0x000f) {
                    case 0x00: {
                         /* Set Vx = Vy. */
                         v_[ix] = v_[iy];
                         break;
                    }
                    case 0x01: {
                         /* Set Vx = Vx OR Vy. */
                         v_[ix] |= v_[iy];
                         v_[0x0f] = 0;
                         break;
                    }
                    case 0x02: {
                         /* Set Vx = Vx AND Vy. */
                         v_[ix] &= v_[iy];
                         v_[0x0f] = 0;
                         break;
                    }
                    case 0x03: {
                         /* Set Vx = Vx XOR Vy. */
                         v_[ix] ^= v_[iy];
                         v_[0x0f] = 0;
                         break;
                    }
                    case 0x04: {
                         /* Set Vx = Vx + Vy, set VF = carry. */
                         uint16_t tmp = v_[ix] + v_[iy];

                         v_[ix] = tmp & 0xff;
                         v_[0x0f] = (tmp & 0x0100) >> 8;

                         break;
                    }
                    case 0x05: {
                         /* Set Vx = Vx - Vy, set VF = NOT borrow. */
                         uint8_t flag = (v_[ix] >= v_[iy]) ? 1 : 0;
                         v_[ix] -= v_[iy];
                         v_[0x0f] = flag;

                         break;
                    }
                    case 0x06: {
                         /* Set Vx = Vx SHR 1. */
                         uint8_t flag = v_[ix] & 0x01;
                         v_[ix] >>= 1;
                         v_[0x0f] = flag;

                         break;
                    }
                    case 0x07: {
                         /* Set Vx = Vy - Vx, set VF = NOT borrow. */
                         uint8_t flag = (v_[iy] >= v_[ix]) ? 1 : 0;
                         v_[ix] = v_[iy] - v_[ix];
                         v_[0x0f] = flag;
                         break;
                    }
                    case 0x0E: {
                         /* Set Vx = Vx SHL 1. */
                         uint8_t flag = (v_[ix] & 0x80) >> 7;
                         v_[ix] <<= 1;
                         v_[0x0f] = flag;
                    }
                    default:
                         break;
               }

               break;
          }
          case 0x9000: {
               /* Skip next instruction if Vx != Vy. */
               uint8_t ix = (op & 0x0f00) >> 8;
               uint8_t iy = (op & 0x00f0) >> 4;

               if (v_[ix] != v_[iy]) {
                    pc_ += 2;
               }

               break;
          }
          case 0xa000: {
               /* Set I = nnn. */
               index_ = op & 0x0fff;
               break;
          }
          case 0xb000: {
               /* Jump to location nnn + V0. */
               pc_ = (op & 0x0fff) + v_[0];
               break;
          }
          case 0xc000: {
               /* Set Vx = random byte AND kk. */
               uint8_t val = op & 0xff;
               uint8_t ix = (op & 0x0f00) >> 8;

               v_[ix] = rand & val;
               break;
          }
          case 0xd000: {
               /* Display n-byte sprite starting at memory location I at (Vx, Vy), set VF = collision. */
               uint8_t ix = (op & 0x0f00) >> 8;
               uint8_t iy = (op & 0x00f0) >> 4;
               uint8_t height = op & 0x000f;

               uint8_t collision = 0;
               uint8_t x = v_[ix] % 64;
               uint8_t y  = v_[iy] % 32;

               for (int row = 0; row < height; row++) {
                    uint8_t pixel = ram_[index_ + row];
                    for (int col = 0; col < 8; col++) {
                         uint8_t new_pixel = (pixel & (0x80 >> col)) >> (7 - col);
                         uint8_t vy = y + row;
                         uint8_t vx = x + col;

                         if (vy < 32 && vx < 64) {
                              uint8_t *old_pixel = &vram_[(vy * 64) + vx];

                              if (*old_pixel != 0) {
                                  collision = 1;
                              }

                              *old_pixel ^= new_pixel;
                         }
                    }
               }

               halt = true;
               v_[0x0f] = collision;
               break;
          }
          case 0xe000: {
               uint8_t ix = (op & 0x0f00) >> 8;

               if ((op & 0xff) == 0x9e) {
                    /* Skip next instruction if key with the value of Vx is pressed. */
                    if (keys[v_[ix]]) {
                         pc_ += 2;
                    }

               } else if ((op & 0xff) == 0xa1) {
                    /* Skip next instruction if key with the value of Vx is not pressed. */
                    if (!keys[v_[ix]]) {
                         pc_ += 2;
                    }
               }

               break;
          }
          case 0xf000: {
               uint8_t ix = (op & 0x0f00) >> 8;

               switch (op & 0xff) {
                    case 0x07: {
                         /* Set Vx = delay timer value. */
                         v_[ix] = delayTimer_;
                         break;
                    }
                    case 0x0a: {
                         /* Wait for a key press, store the value of the key in Vx. All execution stops until a key is pressed */
                         if (keyState == 0) {
                              size_t i = 0;
                              for (; i < keys.size(); i++) {
                                   if (keys[i]) {
                                        keyState++;
                                        v_[ix] = i;
                                   }
                              }

                              if (i == keys.size()) {
                                   pc_ -= 2;
                              }
                         } else if (keyState == 1) {
                              if (keys[v_[ix]]) {
                                   pc_ -= 2;
                              } else {
                                   keyState = 0;
                              }
                         }

                         break;
                    }
                    case 0x15: {
                         /* Set delay timer = Vx. */
                         delayCounter_ = 0;
                         delayTimer_ = v_[ix];

                         break;
                    }
                    case 0x18: {
                         /* Set sound timer = Vx. */
                         soundCounter_ = 0;
                         soundTimer_ = v_[ix];

                         break;
                    }
                    case 0x1e: {
                         /* Set I = I + Vx. */
                         index_ += v_[ix];
                         break;
                    }
                    case 0x29: {
                         /* Set I = location of sprite for digit Vx. */
                         index_ = CHIP8_FONT_START_ADDR + v_[ix] * 5;
                         break;
                    }
                    case 0x33: {
                         /* Store BCD representation of Vx in memory locations I, I+1, and I+2. */
                         uint8_t hundreds = v_[ix] / 100;
                         uint8_t tens = (v_[ix] - (hundreds * 100)) / 10;
                         uint8_t ones = v_[ix] - (hundreds * 100) - (tens * 10);

                         ram_[index_] = hundreds;
                         ram_[index_ + 1] = tens;
                         ram_[index_ + 2] = ones;

                         break;
                    }
                    case 0x55: {
                         /* Store registers V0 through Vx in memory starting at location I. */
                         for (uint8_t i = 0; i <= ix; i++) {
                              ram_[index_++] = v_[i];
                         }

                         break;
                    }
                    case 0x65: {
                         /* Read registers V0 through Vx from memory starting at location I. */
                         for (uint8_t i = 0; i <= ix; i++) {
                              v_[i] = ram_[index_++];
                         }

                         break;
                    }
                    default:
                         break;
               }

               break;
          }
          default:
               break;
     }

     return halt;
}
