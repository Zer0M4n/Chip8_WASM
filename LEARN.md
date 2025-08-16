# CHIP-8 Emulator Development Guide

Before starting this section, I don't plan to give an extensive guide on how to develop the emulator, as there's already a lot of content about it and I feel I can't contribute much to that. However, I would like to help with the part that many of us find difficult: the first steps.

I would like you to not use AI when making your own emulator, or if you're going to use it, specify that it should not give you code solutions and only explain topics you don't understand.

Knowing this, let's begin.

## CHIP-8 Specifications, Nibbles and Opcodes

- **Memory**: CHIP-8 has direct access to up to 4 kilobytes of RAM.
- **Display**: 64 x 32 pixels (or 128 x 64 for SUPER-CHIP) monochrome, that is, black and white.
- **Program counter** (PC): points to the current instruction in memory.
- **Index register**: a 16-bit register called "I" used to point to memory locations.
- **Stack**: for 16-bit addresses, used to call subroutines/functions and return from them. The stack helps us save instructions to call them later.
- **Delay timer**: 8-bit, decremented at 60 Hz frequency (60 times per second) until it reaches 0.
- **Sound timer**: 8-bit, works like the delay timer but also emits a beep while its value is not 0.
- **General purpose registers**: 16 8-bit registers (one byte) numbered from 0 to F in hexadecimal (0 to 15 in decimal), called V0 to VF. The registers would be like our variables but instead of storing data like string, int or float, they only store numeric data.
- **VF register**: also used as a flag register; many instructions set it to 1 or 0 according to some rule, for example, using it as a carry flag.

### Nibble
A nibble by definition is 4 consecutive bits (remember that a bit is a value of 0 or 1) for example 1111 which in hexadecimal would be 0xF. It's important to know what a nibble is because it's something we'll constantly use in opcodes.

### Opcodes

Opcodes (operation codes) are codes that tell us what to do - in simple words, functions that the processor will execute. For example, the jump opcode (1NNN) jumps to address NNN. It measures 2 bytes, that is, 4 nibbles.

An example of how CHIP-8 executes instructions can be seen with opcode 8XY4. This opcode adds register V[x] with V[y]. CHIP-8 looks at an opcode like this:

```c
// CHIP-8 opcode
8XY4   // VX = VX + VY

// Equivalent in C
V[X] = V[X] + V[Y];
```

- 8 → main opcode (arithmetic/logical operation).
- X → destination register (VX).
- Y → source register (VY).
- 4 → operation subtype (in this case, addition).

To extract the registers, we're going to perform operations with nibbles for the same addition instruction but with these values 8 3 5 4. As we can see, our registers are found in nibbles 2 and 3. To get the value of the second nibble:

- Instruction = 0x8354
- We shift 8 bits to the right → 0x83
- We do AND with 0x0F → 0x03
- Result: X = 3.

We use AND to keep only the values that interest us, in this case the second nibble.

Example:
```c
opcode = 1000 0011 0101 0100
opcode >> 8 =      1000 0011

(1000 0011) AND (0000 1111) = 0000 0011
```

Knowing this, a question arises: How do we get the opcodes?

We're going to get the opcodes from the ROMs by reading the binaries. For example, if we read the binaries of 1-chip8-logo (ROM obtained from the test suite by [Timendus](https://github.com/Timendus/chip8-test-suite?tab=readme-ov-file)), we'll get this table:

| Offset   | Bytes                                     | ASCII                        |
|----------|-------------------------------------------|-------------------------------|
| 00000000 | 00 e0 61 01 60 08 a2 50 d0 1f 60 10 a2 5f d0 1f | a P                           |
| 00000010 | 60 18 a2 6e d0 1f 60 20 a2 7d d0 1f 60 28 a2 8c | n } (                         |
| 00000020 | d0 1f 60 30 a2 9b d0 1f 61 10 60 08 a2 aa d0 1f | 0 a                           |
| 00000030 | 60 10 a2 b9 d0 1f 60 18 a2 c8 d0 1f 60 20 a2 d7 |                               |
| 00000040 | d0 1f 60 28 a2 e6 d0 1f 60 30 a2 f5 d0 1f 12 4e | ( 0 N                        |
| 00000050 | 0f 02 02 02 02 00 00 1f 3f 71 e0 e5 e0 e8 a0    | ? q                           |
| 00000060 | 0d 2a 28 28 28 00 00 18 b8 b8 38 38 3f bf 00    | *(((* 8 8 ?                   |
| 00000070 | a5 bd a1 9d 00 00 0c 1d 1d 01 0d 1d 9d 01 c7 29 | )                             |
| 00000080 | 29 29 27 00 00 f8 fc ce c6 c6 c6 00 49 4a 49    | ) ) ' I J I                   |
| 00000090 | 48 3b 00 00 00 01 03 03 03 01 f0 30 90 00 00 80 | H ; 0                         |
| 000000a0 | 00 00 00 fe c7 83 83 83 c6 fc e7 e0 e0 e0 e0 71 | q                             |
| 000000b0 | 3f 1f 00 00 07 02 02 02 02 39 38 38 38 38 b8 b8 | ? 9 8 8 8 8                   |
| 000000c0 | 38 00 00 31 4a 79 40 3b dd dd dd dd dd dd dd dd | 8 1 J y @ ;                   |
| 000000d0 | 00 00 a0 38 20 a0 18 ce fc f8 c0 d4 dc c4 c5 00 | 8                             |
| 000000e0 | 00 30 44 24 14 63 f1 03 07 07 77 17 63 71 00 00 | D $ c W c q                   |
| 000000f0 | 28 8e a8 a8 a6 ce 87 03 03 03 87 fe fc 00 00 60 | (                             |
| 00000100 | 60 90 f0 80 70                               | p                             |

Although it may seem they don't tell us anything, if we pay attention, the first value 00 e0 is the opcode designated for CLS (Clear Screen) - this clears the screen, and so on with all values.

## First Steps

After this theory, let's try to make an application that loads the ROM into memory and reads the opcodes and recognizes them.

First, before anything, we need to represent the CHIP-8 specifications:

```c++
uint16_t pc = 0x200; // program counter
uint16_t I;          // index register
uint16_t sp = 0;     // stack pointer
std::array<uint16_t, 16> stack{};
std::array<uint8_t, 4096> memory{};
std::array<uint8_t, 16> V{};                   // registers
std::array<std::array<uint8_t, 64>, 32> gfx{}; // graphics
uint8_t delay_timer = 0;
uint8_t sound_timer = 0;
```

We represent memory and registers as arrays, since these are responsible for storing values in their blocks. We also add a Stack Pointer to help us know what position the stack is in and return to different stack positions. For graphics, we represent it as a 2D matrix with pixel size. Obviously, if we show the real pixels on a modern screen, you wouldn't see anything - that's where your creativity comes in on how to implement graphics.

Now we need to load into memory (remember that CHIP-8 has a section in memory exclusive for ROMs).

We'll use this function:
```c++
void load_rom(const std::string& filename) {
    std::ifstream file(filename, std::ios::binary | std::ios::ate);
    if (!file) {
        std::cerr << "Error opening ROM\n";
        return;
    }

    std::streamsize size = file.tellg();
    file.seekg(0, std::ios::beg);

    if (size > (4096 - 0x200)) {
        std::cerr << "ROM too large for memory\n";
        return;
    }

    if (!file.read(reinterpret_cast<char*>(memory.data() + 0x200), size)) {
        std::cerr << "Error reading ROM\n";
        return;
    }

    std::cout << "ROM loaded: " << size << " bytes\n";
}
```

We load the ROM into memory and make sure it doesn't exceed the reserved memory.

Now we need our CPU cycle:
```c++
void emulate_cycle() {
    uint16_t opcode = (memory[pc] << 8) | memory[pc + 1];
    uint8_t op = (opcode & 0xF000) >> 12;

    if (table_opcode[op]) {
        (this->*table_opcode[op])(opcode);
    } else {
        opcode_warning(opcode);
    }

    // Decrement timers if > 0
    if (delay_timer > 0) --delay_timer;
    if (sound_timer > 0) --sound_timer;
}
```

Our CPU cycle is summarized as:
- First we get the opcode from memory
- We verify that the opcode exists and execute the corresponding function
- If not in the table, it will print the opcode to console
- Timers are decremented
- Repeat

To execute opcodes, SWITCH-CASE is normally used. In the case of this project, pointers that point to an opcode function were used:

```c++
// typedef for opcode function pointer
using Opcode_pointer = void (Chip8::*)(uint16_t opcode);
std::array<Opcode_pointer, 16> table_opcode{};

// Methods
void load_rom(const std::string& filename) {
    std::ifstream file(filename, std::ios::binary | std::ios::ate);
    if (!file) {
        std::cerr << "Error opening ROM\n";
        return;
    }

    std::streamsize size = file.tellg();
    file.seekg(0, std::ios::beg);

    if (size > (4096 - 0x200)) {
        std::cerr << "ROM too large for memory\n";
        return;
    }

    if (!file.read(reinterpret_cast<char*>(memory.data() + 0x200), size)) {
        std::cerr << "Error reading ROM\n";
        return;
    }

    std::cout << "ROM loaded: " << size << " bytes\n";
}

// Functions for opcodes
void clear_screen(uint16_t opcode) {
    std::cout << "code: " << opcode << "\n";

    std::cout << "code: " << opcode << "\n";
    if (opcode == 0x00E0) {
        for (auto& row : gfx) {
            row.fill(0);
        }
    } else {
        opcode_warning(opcode);
    }
    pc += 2;
}

void load_normalRegister(uint16_t opcode) { // 6xnn
    std::cout << "code: " << opcode << "\n";
    uint8_t x = (opcode & 0x0F00) >> 8;
    uint8_t nn = opcode & 0x00FF;
    V[x] = nn;
    pc += 2;
}

void load_IndexRegister(uint16_t opcode) { // Annn
    std::cout << "code: " << opcode << "\n";

    I = opcode & 0x0FFF;
    pc += 2;
}

void draw_sprite(uint16_t opcode) { // Dxyn
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
                if (screen_pixel == 1) {
                    V[0xF] = 1;
                }
                screen_pixel ^= 1;
            }
        }
    }
    pc += 2;
}
```

Now if we execute 10 cycles of our project, this should appear in the console:

<img width="1275" height="392" alt="image" src="https://github.com/user-attachments/assets/cc3b91d3-b4b6-4278-9ad3-d8a50bb212a2" />

Now our program recognizes CHIP-8 instructions [all this can be found in the old.cpp file].

Now you have a framework for how to start making your project.
