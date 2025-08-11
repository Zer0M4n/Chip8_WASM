#include <iostream>
#include <array>
#include <fstream>

class Chip8
{
public:
    uint16_t pc = 0x200; // program counter empieza en 0x200
    uint16_t I = 0;      // Index Register
    uint8_t sp = 0;      // stack pointer
    
    std::array<uint8_t, 4096> memory{};
    std::array<uint8_t, 16> V{};           // Registros V0 a VF
    std::array<uint16_t, 16> stack{};      // Stack para llamadas
    std::array<std::array<uint8_t, 64>, 32> gfx{}; // pantalla 64x32 pixeles

    uint8_t delay_timer = 0;
    uint8_t sound_timer = 0;

    // typedef para puntero a función opcode
    using Opcode_pointer = void (Chip8::*)(uint16_t opcode);
    std::array<Opcode_pointer, 16> table_opcode{};

    // Métodos
    void load_rom(const std::string& filename) {
        std::ifstream file(filename, std::ios::binary | std::ios::ate);
        if (!file) {
            std::cerr << "Error abriendo ROM\n";
            return;
        }

        std::streamsize size = file.tellg();
        file.seekg(0, std::ios::beg);

        if (size > (4096 - 0x200)) {
            std::cerr << "ROM demasiado grande para la memoria\n";
            return;
        }

        if (!file.read(reinterpret_cast<char*>(memory.data() + 0x200), size)) {
            std::cerr << "Error leyendo ROM\n";
            return;
        }

        std::cout << "ROM cargada: " << size << " bytes\n";
    }

    // Funciones para opcodes
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

    void opcode_warning(uint16_t opcode) {
        
        std::cout << "Opcode desconocido: 0x" << std::hex << opcode << std::dec << "\n";
        pc += 2;
    }

    // Método para ejecutar un ciclo (fetch-decode-execute)
    void emulate_cycle() {
        uint16_t opcode = (memory[pc] << 8) | memory[pc + 1];
        uint8_t op = (opcode & 0xF000) >> 12;

        if (table_opcode[op]) {
            (this->*table_opcode[op])(opcode);
        } else {
            opcode_warning(opcode);
        }

        // Decrementar timers si es > 0
        if (delay_timer > 0) --delay_timer;
        if (sound_timer > 0) --sound_timer;
    }

    Chip8();
    ~Chip8();
};

Chip8::Chip8() {
    table_opcode.fill(nullptr);
    table_opcode[0x0] = &Chip8::clear_screen;
    table_opcode[0x6] = &Chip8::load_normalRegister;
    table_opcode[0xA] = &Chip8::load_IndexRegister;
    table_opcode[0xD] = &Chip8::draw_sprite;
}

Chip8::~Chip8() {}

int main() {
    Chip8 chip8;
    chip8.load_rom("rom/1-chip8-logo (1).ch8");

    // Ejemplo de bucle principal simple
    for (int i = 0; i < 10; ++i) {
        chip8.emulate_cycle();
    }

    std::cout << "Emulación terminada\n";
    return 0;
}
