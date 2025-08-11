#include <array>
#include <cstdint>
#include <iostream>
#include <emscripten.h>

EM_JS(void, redenderizar_js, (uint8_t* gfx_ptr), {
  const ROWS = 32, COLS = 64;
  const memory = new Uint8Array(Module.HEAPU8.buffer, gfx_ptr, ROWS * COLS);
  for (let i = 0; i < ROWS * COLS; i++) {
    if (memory[i]) {
      window.pixels[i].classList.add("on");
    } else {
      window.pixels[i].classList.remove("on");
    }
  }
});

class Chip8 {
public:
  uint16_t pc = 0x200;
  uint16_t I = 0;
  std::array<uint8_t, 4096> memory{};
  std::array<uint8_t, 16> V{};
  std::array<std::array<uint8_t, 64>, 32> gfx{};
  uint8_t delay_timer = 0;
  uint8_t sound_timer = 0;

  using OpcodeFn = void (Chip8::*)(uint16_t);
  std::array<OpcodeFn, 16> table_opcode{};

  Chip8() {
    table_opcode.fill(nullptr);
    table_opcode[0x0] = &Chip8::clear_screen;
    table_opcode[0x6] = &Chip8::load_normalRegister;
    table_opcode[0xA] = &Chip8::load_IndexRegister;
    table_opcode[0xD] = &Chip8::draw_sprite;
  }

  void clear_screen(uint16_t opcode) {
    if (opcode == 0x00E0) {
      for (auto& row : gfx) row.fill(0);
    } else {
      opcode_warning(opcode);
    }
    pc += 2;
  }

  void load_normalRegister(uint16_t opcode) {
    uint8_t x = (opcode & 0x0F00) >> 8;
    uint8_t nn = opcode & 0x00FF;
    V[x] = nn;
    pc += 2;
  }

  void load_IndexRegister(uint16_t opcode) {
    I = opcode & 0x0FFF;
    pc += 2;
  }

  void draw_sprite(uint16_t opcode) {
    uint8_t x = V[(opcode & 0x0F00) >> 8] % 64;
    uint8_t y = V[(opcode & 0x00F0) >> 4] % 32;
    uint8_t height = opcode & 0x000F;
    V[0xF] = 0;
    for (int row = 0; row < height; ++row) {
      uint8_t sprite_byte = memory[I + row];
      for (int col = 0; col < 8; ++col) {
        uint8_t sprite_pixel = (sprite_byte >> (7 - col)) & 1;
        uint8_t& screen_pixel = gfx[(y + row) % 32][(x + col) % 64];
        if (sprite_pixel == 1) {
          if (screen_pixel == 1) {
            V[0xF] = 1;
          }
          screen_pixel ^= 1;
        }
      }
    }
    pc += 2;
  }

  void opcode_warning(uint16_t opcode) {
    std::cout << "Opcode desconocido: 0x" << std::hex << opcode << std::dec << "\n";
    pc += 2;
  }

  void emulate_cycle() {
    uint16_t opcode = (memory[pc] << 8) | memory[pc + 1];
    uint8_t op = (opcode & 0xF000) >> 12;
    if (table_opcode[op]) {
      (this->*table_opcode[op])(opcode);
    } else {
      opcode_warning(opcode);
    }

    if (delay_timer > 0) --delay_timer;
    if (sound_timer > 0) --sound_timer;

    redenderizar_js(&gfx[0][0]);
  }

  // Cargar ROM desde buffer y tamaño, para llamar desde JS
  void load_rom(const uint8_t* data, size_t size) {
    if (size > (4096 - 0x200)) {
      std::cerr << "ROM demasiado grande\n";
      return;
    }
    std::copy(data, data + size, memory.begin() + 0x200);
    pc = 0x200;
  }
};

Chip8 chip8;

extern "C" {
  EMSCRIPTEN_KEEPALIVE
  void step() {
    chip8.emulate_cycle();
  }

  EMSCRIPTEN_KEEPALIVE
  void load_rom(uint8_t* data, int size) {
    chip8.load_rom(data, size);
  }
}
