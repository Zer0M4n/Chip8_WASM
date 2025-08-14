#include <array>
#include <cstdint>
#include <iostream>
#include <emscripten.h>
#include <emscripten/html5.h>
#include <chrono>
#include <thread>
#include <iomanip>

// Debugger expor to JS
static std::string g_lastDebugString;

EM_JS(void, redenderizar_js, (uint8_t *gfx_ptr), {
  const ROWS = 32, COLS = 64;
  const memory = new Uint8Array(Module.HEAPU8.buffer, gfx_ptr, ROWS * COLS);
  for (let i = 0; i < ROWS * COLS; i++)
  {
    if (memory[i])
    {
      window.pixels[i].classList.add("on");
    }
    else
    {
      window.pixels[i].classList.remove("on");
    }
  }
});

// KEYBOARD CODE
EM_BOOL key_callback(int eventType, const EmscriptenKeyboardEvent *e, void *userData)
{

  
  uint8_t keycode = e->keyCode ;
switch (keycode)
{
    // Números
    case '1': 
        printf("key 1\n"); 
        break;
    case '2': 
        printf("key 2\n"); 
        break;
    case '3': 
        printf("key 3\n"); 
        break;
    case '4': 
        printf("key 4\n"); 
        break;

    // QWER
    case 'q': case 'Q':
        printf("key Q\n"); 
        break;
    case 'w': case 'W':
        printf("key W\n"); 
        break;
    case 'e': case 'E':
        printf("key E\n"); 
        break;
    case 'r': case 'R':
        printf("key R\n"); 
        break;

    // ASDF
    case 'a': case 'A':
        printf("key A\n"); 
        break;
    case 's': case 'S':
        printf("key S\n"); 
        break;
    case 'd': case 'D':
        printf("key D\n"); 
        break;
    case 'f': case 'F':
        printf("key F\n"); 
        break;

    // ZXCV
    case 'z': case 'Z':
        printf("key Z\n"); 
        break;
    case 'x': case 'X':
        printf("key X\n"); 
        break;
    case 'c': case 'C':
        printf("key C\n"); 
        break;
    case 'v': case 'V':
        printf("key V\n"); 
        break;
}
  
  return EM_TRUE;                      // true para consumir el evento

}

class Chip8
{
public:
  uint16_t pc = 0x200; // progrma counter
  uint16_t I;          // register specialy
  uint16_t sp = 0;     // stack pointer
  std::array<uint16_t, 16> stack{};
  std::array<uint8_t, 4096> memory{};
  std::array<uint8_t, 16> V{};                   // register
  std::array<std::array<uint8_t, 64>, 32> gfx{}; // graphics
  uint8_t delay_timer = 0;
  uint8_t sound_timer = 0;

  using OpcodeFn = void (Chip8::*)(uint16_t);
  std::array<OpcodeFn, 16> table_opcode{};

  uint8_t Keymap[16] =
      {};

  Chip8()
  {
    table_opcode.fill(nullptr);
    table_opcode[0x0] = &Chip8::helper0x0;
    table_opcode[0x1] = &Chip8::jump;
    table_opcode[0x2] = &Chip8::call_subroutine;
    table_opcode[0x3] = &Chip8::skip_opcode;
    table_opcode[0x4] = &Chip8::skip_opcodeEquals;
    table_opcode[0x5] = &Chip8::skip_opcodeNextRegister;
    table_opcode[0x6] = &Chip8::load_normalRegister;
    table_opcode[0x7] = &Chip8::add;
    table_opcode[0x8] = &Chip8::helper0x8;
    table_opcode[0x9] = &Chip8::skip_opcodeEqualsRegister;
    table_opcode[0xA] = &Chip8::load_IndexRegister;
    table_opcode[0xF] = &Chip8::helper0xF;
    table_opcode[0xD] = &Chip8::draw_sprite;
  }

  // Opcodes fuctions

  void jump(uint16_t opcode) // 1NNN - Jump to address NNN
  {
    pc = opcode & 0x0FFF;
    g_lastDebugString = ConsoleDebuggerStr(opcode);
  }

  void clear_screen(uint16_t opcode) // 00E0 - Clear the screen
  {

    if (opcode == 0x00E0)
    {
      for (auto &row : gfx)
        row.fill(0);
    }
    pc += 2;
    g_lastDebugString = ConsoleDebuggerStr(opcode);
  }

  void load_normalRegister(uint16_t opcode) // 6XNN - Load normal register with immediate value
  {
    uint8_t x = (opcode & 0x0F00) >> 8;
    uint8_t nn = opcode & 0x00FF;
    V[x] = nn;
    pc += 2;
  }

  void load_IndexRegister(uint16_t opcode) // ANNN - Load index register with immediate value
  {
    I = (opcode & 0x0FFF);
    pc += 2;
    g_lastDebugString = ConsoleDebuggerStr(opcode);
  }

  void draw_sprite(uint16_t opcode) // DXNN - Draw sprite to screen (only aligned)
  {

    uint8_t x = V[(opcode & 0x0F00) >> 8] % 64;
    uint8_t y = V[(opcode & 0x00F0) >> 4] % 32;
    uint8_t height = opcode & 0x000F;
    V[0xF] = 0;
    for (int row = 0; row < height; ++row)
    {
      uint8_t sprite_byte = memory[I + row];
      for (int col = 0; col < 8; ++col)
      {
        uint8_t sprite_pixel = (sprite_byte >> (7 - col)) & 1;
        uint8_t &screen_pixel = gfx[(y + row) % 32][(x + col) % 64];
        if (sprite_pixel == 1)
        {
          if (screen_pixel == 1)
          {
            V[0xF] = 1;
          }
          screen_pixel ^= 1;
        }
      }
    }
    pc += 2;
    g_lastDebugString = ConsoleDebuggerStr(opcode);
  }

  void add(uint16_t opcode) // 7XNN - Add the value NN to VX.
  {
    uint8_t x = (opcode & 0x0F00) >> 8;
    uint8_t nn = (opcode & 0x00FF);

    V[x] = (V[x] + nn) & 0x00FF;
    pc += 2;
    g_lastDebugString = ConsoleDebuggerStr(opcode);
  }

  void skip_opcode(uint16_t opcode) // 3XNN - skip next opcode if vX == NN
  {
    uint8_t x = (opcode & 0x0F00) >> 8;
    uint8_t nn = (opcode & 0x00FF);

    if (V[x] == nn)
    {
      pc += 4; // skip sigiente intructions
    }
    else
    {
      pc += 2;
    }
    g_lastDebugString = ConsoleDebuggerStr(opcode);
  }

  void skip_opcodeEquals(uint16_t opcode) // 4XNN - skip next opcode if vX != NN
  {
    uint8_t x = (opcode & 0x0F00) >> 8;
    uint8_t nn = (opcode & 0x00FF);

    if (V[x] != nn)
    {
      pc += 4;
    }
    else
    {
      pc += 2;
    }
    g_lastDebugString = ConsoleDebuggerStr(opcode);
  }

  void skip_opcodeNextRegister(uint16_t opcode) // 5XY0 - skip next opcode if vX == vY
  {
    uint8_t x = (opcode & 0x0F00) >> 8;
    uint8_t y = (opcode & 0x00F0) >> 4;

    if (V[x] == V[y])
    {
      pc += 4;
    }
    else
    {
      pc += 2;
    }
    g_lastDebugString = ConsoleDebuggerStr(opcode);
  }

  void skip_opcodeEqualsRegister(uint16_t opcode) // 9XY0 - skip next opcode if vX != vY
  {
    uint8_t x = (opcode & 0x0F00) >> 8;
    uint8_t y = (opcode & 0x00F0) >> 4;

    if (V[x] != V[y])
    {
      pc += 4;
    }
    else
    {
      pc += 2;
    }
    g_lastDebugString = ConsoleDebuggerStr(opcode);
  }

  void call_subroutine(uint16_t opcode) // 2NNN  - push return address onto stack and call subroutine at address NNN
  {
    stack[sp] = pc;
    ++sp;
    pc = opcode & 0X0FFF;
    g_lastDebugString = ConsoleDebuggerStr(opcode);
  }

  void return_subroutine(uint16_t opcode) // 00EE  - return from subroutine to address pulled from stack
  {
    --sp;
    pc = stack[sp];
    pc += 2;
    std::cout << "call" << "\n";
    g_lastDebugString = ConsoleDebuggerStr(opcode);
  }

  void equals_register(uint16_t opcode) // 8XY0 - set vX to the value of vY
  {
    uint8_t x = (opcode & 0x0F00) >> 8;
    uint8_t y = (opcode & 0x00F0) >> 4;

    V[x] = V[y];

    pc += 2;

    g_lastDebugString = ConsoleDebuggerStr(opcode);
  }

  void or_register(uint16_t opcode) // 8XY1 -  set vX to the result of bitwise vX OR vY
  {
    uint8_t x = (opcode & 0x0F00) >> 8;
    uint8_t y = (opcode & 0x00F0) >> 4;

    V[x] |= V[y];

    pc += 2;

    g_lastDebugString = ConsoleDebuggerStr(opcode);
  }

  void and_register(uint16_t opcode) // 8XY2 -set vX to the result of bitwise vX AND vY
  {
    uint8_t x = (opcode & 0x0F00) >> 8;
    uint8_t y = (opcode & 0x00F0) >> 4;

    V[x] &= V[y];

    pc += 2;

    g_lastDebugString = ConsoleDebuggerStr(opcode);
  }

  void xor_register(uint16_t opcode) // 8XY3 -set vX to the result of bitwise vX XOR vY
  {
    uint8_t x = (opcode & 0x0F00) >> 8;
    uint8_t y = (opcode & 0x00F0) >> 4;

    V[x] ^= V[y];

    pc += 2;
    g_lastDebugString = ConsoleDebuggerStr(opcode);
  }

  void add_register(uint16_t opcode) // 8XY4 -add vY to vX, vF is set to 1 if an overflow happened, to 0 if not, even if X=F!
  {

    uint8_t x = (opcode & 0x0F00) >> 8;
    uint8_t y = (opcode & 0x00F0) >> 4;
    uint16_t sum = V[x] + V[y];

    uint8_t overflow_flag = (sum > 0xFF) ? 1 : 0; // Calcula el flag primero
    V[x] = sum & 0xFF;                            // Guarda el resultado
    V[0xF] = overflow_flag;

    pc += 2;

    g_lastDebugString = ConsoleDebuggerStr(opcode);
  }

  void subtract_register(uint16_t opcode) // 8XY5 -subtract vY from vX, vF is set to 0 if an underflow happened, to 1 if not, even if X=F!
  {
    uint8_t x = (opcode & 0x0F00) >> 8;
    uint8_t y = (opcode & 0x00F0) >> 4;
    
    uint8_t no_borrow = (V[x] >= V[y]) ? 1 : 0;  
    V[x] = (V[x] - V[y]) & 0xFF;                 
    V[0xF] = no_borrow;                          

    pc += 2;
    g_lastDebugString = ConsoleDebuggerStr(opcode);
  }

  void shift_register(uint16_t opcode) // 8XY6 - set vX to vY and shift vX one bit to the right, set vF to the bit shifted out, even if X=F!
  {
    uint8_t x = (opcode & 0x0F00) >> 8;
    uint8_t y = (opcode & 0x00F0) >> 4;

    uint8_t lsb = V[x] & 0x1; // Guarda el LSB ANTES de modificar
    V[x] >>= 1;               // Desplaza primero
    V[0xF] = lsb;             // Establece el flag después

    pc += 2;

    g_lastDebugString = ConsoleDebuggerStr(opcode);
  }

  void f_8XY7(uint16_t opcode) // // 8XY7 - 	set vX to the result of subtracting vX from vY, vF is set to 0 if an underflow happened, to 1 if not, even if X=F!
  {
    uint8_t x = (opcode & 0x0F00) >> 8;
    uint8_t y = (opcode & 0x00F0) >> 4;

    uint8_t no_borrow = (V[y] >= V[x]) ? 1 : 0;
    V[x] = (V[y] - V[x]) & 0xFF;
    V[0xF] = no_borrow;

    pc += 2;
    g_lastDebugString = ConsoleDebuggerStr(opcode);
  }

  void f_8XYE(uint16_t opcode) //  8XYE - 	set vX to vY and shift vX one bit to the left, set vF to the bit shifted out, even if X=F!
  {
    uint8_t x = (opcode & 0x0F00) >> 8;
    uint8_t y = (opcode & 0x00F0) >> 4;

    uint8_t msb = (V[x] & 0x80) >> 7;
    V[x] <<= 1;
    V[0xF] = msb;
    pc += 2;

    g_lastDebugString = ConsoleDebuggerStr(opcode);
  }

  void f_FX07(uint16_t opcode) // FX07 - Set VX to delay timer value
  {
    uint8_t x = (opcode & 0x0F00) >> 8;
    V[x] = delay_timer;
    pc = pc + 2;
  }

  void f_FX55(uint16_t opcode) // FX55 - Store registers V0 through VX in memory starting at I
  {
    uint8_t x = (opcode & 0x0F00) >> 8;
    for (uint8_t i = 0; i <= x; i++)
    {
      memory[I + i] = V[i];
    }
    pc = pc + 2;
  }

  void f_FX65(uint16_t opcode) // FX65 - Read registers V0 through VX from memory starting at I
  {
    uint8_t x = (opcode & 0x0F00) >> 8;
    for (uint8_t i = 0; i <= x; i++)
    {
      V[i] = memory[I + i]; //  Leer DE memoria A registros
    }
    pc = pc + 2;
  }

  void Fx33(uint16_t opcode) // FX33 - Store BCD representation of VX
  {
    uint8_t x = (opcode & 0x0F00) >> 8;
    uint8_t value = V[x];

    memory[I] = value / 100;           // Centenas
    memory[I + 1] = (value / 10) % 10; // Decenas
    memory[I + 2] = value % 10;        // Unidades

    pc = pc + 2;
  }

  void Fx1e(uint16_t opcode) // FX1E - Add VX to I
  {
    uint8_t x = (opcode & 0x0F00) >> 8;

    I += V[x];
    I &= 0xFFF; // Mantener dentro de 12 bits

    pc = pc + 2;
  }

  // helper funcion opcode
  void helper0x0(uint16_t opcode)
  {
    if (opcode == 0x00E0) // clear code
    {
      clear_screen(opcode);
    }
    else if (opcode == 0x00EE)
    {
      return_subroutine(opcode);
    }
    else
    {
      opcode_warning(opcode);
    }
  }
 
  void helper0x8(uint16_t opcode)
  {
    uint8_t lastNibble = (opcode & 0x000F);
    if (lastNibble == 0x0000)
    {
      equals_register(opcode);
      std::cout << "equals_register" << "\n";
    }
    else if (lastNibble == 0x0001)
    {
      or_register(opcode);
      std::cout << "or_register" << "\n";
    }
    else if (lastNibble == 0x0002)
    {
      and_register(opcode);
      std::cout << "and_register" << "\n";
    }
    else if (lastNibble == 0x0003)
    {
      xor_register(opcode);
      std::cout << "XOR_register" << "\n";
    }
    else if (lastNibble == 0x0004)
    {
      add_register(opcode);
      std::cout << "ADD_register" << "\n";
    }
    else if (lastNibble == 0x0005)
    {
      subtract_register(opcode);
      std::cout << "SUBSTRACT_register" << "\n";
    }
    else if (lastNibble == 0x0006)
    {
      shift_register(opcode);
      std::cout << "SHIFT_register" << "\n";
    }
    else if (lastNibble == 0x0007)
    {
      f_8XY7(opcode);
      std::cout << "8XY7_register" << "\n";
    }
    else if (lastNibble == 0x000E)
    {
      f_8XYE(opcode);
      std::cout << "8XYE_register" << "\n";
    }
    else
    {
      opcode_warning(opcode);
    }
  }

  void helper0xF(uint16_t opcode)
  {
    uint8_t lastNibble = (opcode & 0x00FF);
    if (lastNibble == 0x07)
    {
      f_FX07(opcode);
      std::cout << "FX07 YES" << "\n";
    }
    else if (lastNibble == 0x65)
    {
      f_FX65(opcode);
      std::cout << "FX65 YES" << "\n";
    }
    else if (lastNibble == 0x55)
    {
      f_FX55(opcode);
      std::cout << "FX65 YES" << "\n";
    }
    else if (lastNibble == 0x33)
    {
      Fx33(opcode);
      std::cout << "FX33 YES" << "\n";
    }
    else if (lastNibble == 0x1E)
    {
      Fx1e(opcode);
      std::cout << "FX1e YES" << "\n";
    }

    else
    {
      opcode_warning(opcode);
    }
  }
  // opcodes debuggers

  void opcode_warning(uint16_t opcode)
  {
    std::cout << "Opcode desconocido: 0x" << std::hex << opcode << std::dec << "\n";
    pc += 2;
  }

  std::string to_hex(uint32_t val, int width)
  {
    char buffer[20];
    // %0*X => ancho variable, mayúsculas
    std::sprintf(buffer, "%0*X", width, val);
    return std::string(buffer);
  }

  std::string ConsoleDebuggerStr(uint16_t opcode)
  {
    std::string out = "================= DEBUG CHIP-8 =================\n";

    // REGISTROS
    out += "[REGISTROS]\n";
    for (int i = 0; i < 16; i++)
    {
      out += "V[" + to_hex(i, 1) + "]: 0x" + to_hex(V[i], 2) + "  ";
      if ((i + 1) % 8 == 0)
        out += "\n";
    }

    out += "I:  0x" + to_hex(I, 4) + "    PC: 0x" + to_hex(pc, 4) + "\n";

    // OPCODE
    out += "\n[OPCODE]\n";
    out += "Actual: 0x" + to_hex(opcode, 4) + "\n";

    uint8_t x = (opcode & 0x0F00) >> 8;
    uint8_t y = (opcode & 0x00F0) >> 4;
    uint8_t kk = (opcode & 0x00FF);
    uint16_t nnn = opcode & 0x0FFF;
    uint8_t n = opcode & 0x000F;

    out += "X: " + std::to_string(x) + "  Y: " + std::to_string(y) +
           "  KK: 0x" + to_hex(kk, 2) + "  NNN: 0x" + to_hex(nnn, 3) +
           "  N: " + std::to_string(n) + "\n";

    // STACK
    out += "\n[STACK] (SP=" + std::to_string(sp) + ")\n";
    for (int i = 0; i < 16; i++)
    {
      out += "S[" + to_hex(i, 1) + "]: 0x" + to_hex(stack[i], 4) + "  ";
      if ((i + 1) % 4 == 0)
        out += "\n";
    }

    // MEMORIA
    out += "\n[MEMORY PREVIEW] desde PC\n";
    for (int i = 0; i < 10; i++)
    {
      uint16_t addr = pc + i * 2;
      if (addr >= 4096)
        break;
      uint16_t instr = (memory[addr] << 8) | memory[addr + 1];
      out += "0x" + to_hex(addr, 4) + ": 0x" + to_hex(instr, 4);
      if (i == 0)
        out += "   <-- actual";
      out += "\n";
    }

    out += "===============================================\n\n";

    return out;
  }

  void emulate_cycle()
  {
    uint16_t opcode = (memory[pc] << 8) | memory[pc + 1];
    uint8_t op = (opcode & 0xF000) >> 12;

    emscripten_set_keydown_callback(EMSCRIPTEN_EVENT_TARGET_DOCUMENT, NULL, EM_TRUE, key_callback);

    if (table_opcode[op])
    {
      (this->*table_opcode[op])(opcode);
    }
    else
    {
      // print opcodes desconocidos
      opcode_warning(opcode);
    }

    if (delay_timer > 0)
      --delay_timer;
    if (sound_timer > 0)
      --sound_timer;

    redenderizar_js(&gfx[0][0]);
  }

  // Cargar ROM desde buffer y tamaño, para llamar desde JS
  void load_rom(const uint8_t *data, size_t size)
  {
    if (size > (4096 - 0x200))
    {
      std::cerr << "ROM demasiado grande\n";
      return;
    }
    std::copy(data, data + size, memory.begin() + 0x200); // copia segura
    pc = 0x200;                                           //
  }
};

Chip8 chip8;

extern "C"
{
  EMSCRIPTEN_KEEPALIVE
  void step()
  {
    chip8.emulate_cycle();
  }

  EMSCRIPTEN_KEEPALIVE
  void load_rom(uint8_t *data, int size)
  {
    chip8.load_rom(data, size);
  }
  EMSCRIPTEN_KEEPALIVE
  const char *getDebuggerString()
  {
    return g_lastDebugString.c_str();
  }
}
