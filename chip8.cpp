#include <array>
#include <cstdint>
#include <iostream>
#include <emscripten.h>
#include <iomanip> 

//Debugger expor to JS 
static std::string g_lastDebugString;

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
  
  uint16_t pc = 0x200; // progrma counter
  uint16_t I = 0; //register specialy
  uint16_t sp = 0; // stack pointer 
  std::array<uint16_t ,16> stack{};
  std::array<uint8_t, 4096> memory{};
  std::array<uint8_t, 16> V{}; //register
  std::array<std::array<uint8_t, 64>, 32> gfx{}; //graphics
  uint8_t delay_timer = 0;
  uint8_t sound_timer = 0;

  using OpcodeFn = void (Chip8::*)(uint16_t);
  std::array<OpcodeFn, 16> table_opcode{};

  Chip8() {
    table_opcode.fill(nullptr);
    table_opcode[0x00E0] = &Chip8::clear_screen;
    table_opcode[0x1] = &Chip8::jump;
    table_opcode[0x2] = &Chip8::call_subroutine;
    table_opcode[0x3] = &Chip8::skip_opcode;
    table_opcode[0x4] = &Chip8::skip_opcodeEquals;
    table_opcode[0x5] = &Chip8::skip_opcodeNextRegister;
    table_opcode[0x6] = &Chip8::load_normalRegister;
    table_opcode[0x7] = &Chip8::add;
    table_opcode[0x9] = &Chip8::skip_opcodeEqualsRegister;
    table_opcode[0xA] = &Chip8::load_IndexRegister;
    table_opcode[0xD] = &Chip8::draw_sprite;
    
    
    
    
  }
  
  //Opcodes fuctions
  
  void jump(uint16_t opcode){ //1NNN - Jump to address NNN
    pc = opcode & 0x0FFF;
  }

  void clear_screen(uint16_t opcode) { //00E0 - Clear the screen
   
    if (opcode == 0x00E0) {
      for (auto& row : gfx) row.fill(0);
    } else {
      opcode_warning(opcode);
    }
    pc += 2;
  }

  void load_normalRegister(uint16_t opcode) { // 6XNN - Load normal register with immediate value
    uint8_t x = (opcode & 0x0F00) >> 8;
    uint8_t nn = opcode & 0x00FF;
    V[x] = nn;
    pc += 2;

  }

  void load_IndexRegister(uint16_t opcode) { //ANNN - Load index register with immediate value
    I = (opcode & 0x0FFF);
    pc += 2;

  }

  void draw_sprite(uint16_t opcode) { //DXNN - Draw sprite to screen (only aligned)

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

  void add(uint16_t opcode){ //7XNN - Add the value NN to VX.
    uint8_t x = (opcode & 0x0F00) >> 8;
    uint8_t nn  = (opcode & 0x00FF);

    V[x] = (V[x] + nn) & 0x00FF;
    pc += 2;
  }
  
  void skip_opcode(uint16_t opcode){ //3XNN - skip next opcode if vX == NN
    uint8_t x = (opcode & 0x0F00) >> 8;
    uint8_t nn  = (opcode & 0x00FF);

    if (V[x] == nn)
    {
      pc += 4; //skip sigiente intructions
    } else {
      pc += 2;
    }
 
  }
  
  void skip_opcodeEquals(uint16_t opcode){ //4XNN - skip next opcode if vX != NN 
    uint8_t x = (opcode & 0x0F00) >> 8;
    uint8_t nn = (opcode & 0x00FF);

    if (V[x] != nn)
    {
      pc += 4;
    } else {
      pc += 2;
    }
    
  }
  
  void skip_opcodeNextRegister(uint16_t opcode){ //5XY0 - skip next opcode if vX == vY
    uint8_t x = (opcode & 0x0F00) >> 8;
    uint8_t y = (opcode & 0x00F0) >> 4;

    if (V[x] == V[y])
    {
      pc += 4;
    } else {
      pc += 2;
    }
  }

  void skip_opcodeEqualsRegister(uint16_t opcode){ //9XY0 - skip next opcode if vX != vY
    uint8_t x = (opcode & 0x0F00) >> 8;
    uint8_t y = (opcode & 0x00F0) >> 4;

    if (V[x] != V[y])
    {
      pc += 4;
    } else {
      pc += 2;
    }
    
  } 
  
  void call_subroutine(uint16_t opcode){ //2NNN  - push return address onto stack and call subroutine at address NNN
    stack[sp] = pc;
    ++sp;
    pc = opcode & 0X0FFF;
  }
 
  void return_subroutine(uint16_t opcode) { //00EE  - return from subroutine to address pulled from stack

    --sp;
    pc = stack[sp];
    
    pc += 2;
   
  }
 
 
 
 
  // opcodes debuggers
  
  void opcode_warning(uint16_t opcode) {
    std::cout << "Opcode desconocido: 0x" << std::hex << opcode << std::dec << "\n";
    pc += 2;
  }
  
  std::string to_hex(uint32_t val, int width) {
    char buffer[20];
    // %0*X => ancho variable, mayúsculas
    std::sprintf(buffer, "%0*X", width, val);
    return std::string(buffer);
}

  std::string ConsoleDebuggerStr(uint16_t opcode) {
    std::string out = "================= DEBUG CHIP-8 =================\n";

    // REGISTROS
    out += "[REGISTROS]\n";
    for (int i = 0; i < 16; i++) {
        out += "V[" + to_hex(i,1) + "]: 0x" + to_hex(V[i],2) + "  ";
        if ((i+1) % 8 == 0) out += "\n";
    }

    out += "I:  0x" + to_hex(I,4) + "    PC: 0x" + to_hex(pc,4) + "\n";

    // OPCODE
    out += "\n[OPCODE]\n";
    out += "Actual: 0x" + to_hex(opcode,4) + "\n";

    uint8_t x  = (opcode & 0x0F00) >> 8;
    uint8_t y  = (opcode & 0x00F0) >> 4;
    uint8_t kk = (opcode & 0x00FF);
    uint16_t nnn = opcode & 0x0FFF;
    uint8_t n  = opcode & 0x000F;

    out += "X: " + std::to_string(x) + "  Y: " + std::to_string(y) + 
           "  KK: 0x" + to_hex(kk,2) + "  NNN: 0x" + to_hex(nnn,3) + 
           "  N: " + std::to_string(n) + "\n";

    // STACK
    out += "\n[STACK] (SP=" + std::to_string(sp) + ")\n";
    for (int i = 0; i < 16; i++) {
        out += "S[" + to_hex(i,1) + "]: 0x" + to_hex(stack[i],4) + "  ";
        if ((i+1) % 4 == 0) out += "\n";
    }

    // MEMORIA
    out += "\n[MEMORY PREVIEW] desde PC\n";
    for (int i = 0; i < 10; i++) {
        uint16_t addr = pc + i * 2;
        if (addr >= 4096) break;
        uint16_t instr = (memory[addr] << 8) | memory[addr + 1];
        out += "0x" + to_hex(addr,4) + ": 0x" + to_hex(instr,4);
        if (i == 0) out += "   <-- actual";
        out += "\n";
    }

    out += "===============================================\n\n";

    return out;
}

  void emulate_cycle() {
    uint16_t opcode = (memory[pc] << 8) | memory[pc + 1];
    uint8_t op = (opcode & 0xF000) >> 12;
    if (table_opcode[op]) {
      (this->*table_opcode[op])(opcode);

      //print debugger 
      g_lastDebugString = ConsoleDebuggerStr(opcode);
    } else {
      //print opcodes desconocidos
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
    EMSCRIPTEN_KEEPALIVE
  const char* getDebuggerString() {
    return g_lastDebugString.c_str();
  }
}
