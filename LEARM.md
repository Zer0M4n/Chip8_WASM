# Chip-8 Emulator Guide

Before starting this section, I do not plan to give an extensive guide on how to develop the emulator, as there is already a lot of content on that, and I feel I cannot contribute much. However, I would like to help with the part that many of us find difficult: the first steps.

I would like that when making your own emulator, you avoid using AI, or if you do use it, specify that it should not provide code solutions and only explain topics you do not understand.

Knowing this, let's start.

## Chip-8 Specifications, Nibble, and Opcodes

* **Memory**: CHIP-8 has direct access to up to 4 kilobytes of RAM.
* **Display**: 64 x 32 pixels (or 128 x 64 for SUPER-CHIP), monochrome (black and white).
* **Program Counter (PC)**: points to the current instruction in memory.
* **Index Register**: a 16-bit register called "I" used to point to memory locations.
* **Stack**: for 16-bit addresses, used to call subroutines/functions and return from them. The stack helps us store instructions to call them later.
* **Delay Timer**: 8-bit, decremented at 60 Hz until reaching 0.
* **Sound Timer**: 8-bit, works like the delay timer but also emits a beep while its value is not 0.
* **General-Purpose Registers**: 16 registers of 8 bits (1 byte) numbered 0 to F in hexadecimal (0 to 15 decimal), called V0 to VF. Registers are like variables, but instead of storing strings, ints, or floats, they only store numeric data.
* **VF Register**: also used as a flag register; many instructions set it to 1 or 0 according to some rule, for example, using it as a carry flag.

### Nibble

A nibble is 4 consecutive bits (0 or 1). Example: `1111` in hexadecimal is `0xF`. Nibbles are commonly used in opcodes.

### Opcodes

Opcodes (operation codes) tell the CPU what to do. Each opcode is 2 bytes long (4 nibbles). Example:

```c
// CHIP-8 opcode
8XY4   // VX = VX + VY

// Equivalent in C
V[X] = V[X] + V[Y];
```

* `8` → main opcode (arithmetic/logical)
* `X` → destination register (VX)
* `Y` → source register (VY)
* `4` → addition subtype

To extract registers using nibble operations for instruction `0x8354`:

* Shift 8 bits right → 0x83
* AND with 0x0F → 0x03
* Result: X = 3

```c
opcode = 1000 0011 0101 0100
opcode >> 8 =      1000 0011
(1000 0011) AND (0000 1111) = 0000 0011
```

Opcodes are obtained by reading ROM binaries, e.g., the `1-chip8-logo` ROM ([Temendus test suite](https://github.com/Timendus/chip8-test-suite?tab=readme-ov-file)):

| Offset   | Bytes                                           | ASCII |
| -------- | ----------------------------------------------- | ----- |
| 00000000 | 00 e0 61 01 60 08 a2 50 d0 1f 60 10 a2 5f d0 1f | a P   |
| 00000010 | 60 18 a2 6e d0 1f 60 20 a2 7d d0 1f 60 28 a2 8c | n } ( |
| ...      | ...                                             | ...   |

`00 e0` is the opcode for CLS (Clear Screen).

## First Steps

### Representing Chip-8 specifications

```c++
uint16_t pc = 0x200;  // program counter
uint16_t I;           // index register
uint16_t sp = 0;      // stack pointer
std::array<uint16_t, 16> stack{};
std::array<uint8_t, 4096> memory{};
std::array<uint8_t, 16> V{};
std::array<std::array<uint8_t, 64>, 32> gfx{};
uint8_t delay_timer = 0;
uint8_t sound_timer = 0;
```

### Loading the ROM

```c++
void load_rom(const std::string& filename) {
    std::ifstream file(filename, std::ios::binary | std::ios::ate);
    if (!file) { std::cerr << "Error opening ROM\n"; return; }

    std::streamsize size = file.tellg();
    file.seekg(0, std::ios::beg);

    if (size > (4096 - 0x200)) { std::cerr << "ROM too large for memory\n"; return; }

    if (!file.read(reinterpret_cast<char*>(memory.data() + 0x200), size)) {
        std::cerr << "Error reading ROM\n"; return;
    }

    std::cout << "ROM loaded: " << size << " bytes\n";
}
```

### CPU Cycle

```c++
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
}
```

### Opcodes via Function Pointers

```c++
using Opcode_pointer = void (Chip8::*)(uint16_t opcode);
std::array<Opcode_pointer, 16> table_opcode{};

void clear_screen(uint16_t opcode) {
    std::cout << "code: " << opcode << "\n";
    if (opcode == 0x00E0) { for (auto& row : gfx) row.fill(0); } 
    else opcode_warning(opcode);
    pc += 2;
}

void load_normalRegister(uint16_t opcode) {
    std::cout << "code: " << opcode << "\n";
    uint8_t x = (opcode & 0x0F00) >> 8;
    uint8_t nn = opcode & 0x00FF;
    V[x] = nn;
    pc += 2;
}

void load_IndexRegister(uint16_t opcode) {
    std::cout << "code: " << opcode << "\n";
    I = opcode & 0x0FFF;
    pc += 2;
}

void draw_sprite(uint16_t opcode) {
    std::cout << "code: " << opcode << "\n";
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
                if (screen_pixel == 1) V[0xF] = 1;
                screen_pixel ^= 1;
            }
        }
    }
    pc += 2;
}
```

Running 10 CPU cycles should produce console output similar to the image provided.

This provides a basic framework to start your own Chip-8 emulator.
