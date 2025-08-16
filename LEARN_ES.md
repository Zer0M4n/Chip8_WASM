Antes de empezar este apartado no planeo dar una guia extenza de como desarrolar el emulador , ya hay mucho contenido de ello y 
siento que no pueda aportar mucho de ello a un asi me gustaria como ayudar en la parte que a mucos de nosotros se nos dificulta , los primeros paso.

Me gustaria que al momento de hacer su propio emulador  no usara IA o si la va usar especicarle que no le de solucion de codigo y que solo le explique 
los temas que uste noe entiende .

Sabiendo esto empezemos

## Chip 8 especificaciones , Nibble y Opcodes

- **Memoria**: CHIP-8 tiene acceso directo a hasta 4 kilobytes de RAM.
- **Pantalla**: 64 x 32 píxeles (o 128 x 64 para SUPER-CHIP) monocromática, es decir, en blanco y negro.
- **Contador de programa** (program counter, “PC”): apunta a la instrucción actual en la memoria.
- **Registro índice**: un registro de 16 bits llamado “I” que se usa para apuntar a ubicaciones en la memoria.
- **Pila**: para direcciones de 16 bits, usada para llamar subrutinas/funciones y retornar de ellas, la pila nos ayudara a guardar intrucciones ara luego llamarlas despues.
- **Temporizador de retardo**: de 8 bits, decrementado a una frecuencia de 60 Hz (60 veces por segundo) hasta llegar a 0.
- **Temporizador de sonido**: de 8 bits, funciona como el temporizador de retardo pero además emite un pitido mientras su valor no sea 0.
- **Registros de propósito general**: 16 registros de 8 bits (un byte) numerados del 0 al F en hexadecimal (0 a 15 en decimal), llamados V0 a VF , los registros
  serian como nuestras variables pero en vez de guardar un datos como string , int o float , guardan solo datos numericos como .
- **Registro VF**: también se utiliza como registro de bandera; muchas instrucciones lo configuran en 1 o 0 según alguna regla, por ejemplo,
    usándolo como bandera de acarreo (carry flag).

### Nibble
Un nibble con por definicion son 4 bits consecutivos (recuerda que un bit es valor de 0 o 1) por ejemplo 1111 que en hexadcimal seria 0xF es importante saber 
que es un nibble por que algo que constante mente ultilizaremos en los opcodes.

### Opcodes

Los opcodes (operation code) son codigos que nos dicen que que hacer en pocas palabras fucniones que el procesar hara  por ejemplo el opcode jump (1NNN) salta 
a la direccion nnnn . Mide 2 bytes osea 4 nibbles.

Un ejemplo de Chip8 ejecutas las intrucciones lo vamos a ver con el opcode 8XY4 , este opcode suma un registro V[x] con V[y], un opcode lo mira asi chip 8
```c
// CHIP-8 opcode
8XY4   // VX = VX + VY

// Equivalente en C
V[X] = V[X] + V[Y];
```
- 8 → opcode principal (operación aritmética/lógica).
- X → registro destino (VX).
- Y → registro fuente (VY).
- 4 → subtipo de operación (en este caso, suma).

para sacar los registros vamos hacer operaciones con nibbes para la misma intruccion de suma ero con estos valores 8 3 5 4, como vemo snuestros registros 
se encuentrna en los nibbles 2 y 3 , para obtener su valor del segundo nibble seria .


- Instrucción = 0x8354
- Desplazamos 8 bits a la derecha → 0x83
- Hacemos AND con 0x0F → 0x03
- Resultado: X = 3.

Usamos AND para quedarnos nosotros con los valores que nos interesan , en este caso el segundo nibble

ejemplo
```c
opcode = 1000 0011 0101 0100
opcode >> 8 =      1000 0011

(1000 0011) AND (0000 1111) = 0000 0011

```
Sabiendo estos surge una pregunta ,Como obtenemos los opcodes?.

Los opcodes lo vamos a obtener de las ROM leyendo los binarios por ejemplo si leemos los binarios de 1-chip8-logo (ROM obtetenida de la tes suite de [Temendus](https://github.com/Timendus/chip8-test-suite?tab=readme-ov-file))
vamos a obtner esta tabla
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
| 00000100 | 90 f0 80 70                               | p                             |

aunque no pareca que no nos dicen nada si ponmeos tencion el primer valor 00 e0 , este es opcode designado para CLS (Clear Screen) esta limpia la pantalla y asi qcon todos los valores

## Primeros pasos

Despues de esta teoria, vamos intentar hacer una aplicacion que carge en memoria la rom y lea los opcodes y lo reconozca.

primero antes de todo ocupamos representar las especificaciones de la chip 8

```c++
  uint16_t pc = 0x200; // progrma counter
  uint16_t I;          // register specialy
  uint16_t sp = 0;     // stack pointer
  std::array<uint16_t, 16> stack{};
  std::array<uint8_t, 4096> memory{};
  std::array<uint8_t, 16> V{};                   // register
  std::array<std::array<uint8_t, 64>, 32> gfx{}; // graphics
  uint8_t delay_timer = 0;
  uint8_t sound_timer = 0;
```

reprentamos la memoria y los registros como vectores , ya que estos se encargan guardar valores en sus bloques, tambien agregamos un Stack Pointer 
esto nos ayuda en saber en que posicion esta el el satck y regresar a a diferentes posiciones del stack. sobre los graficos lo representamos como una matriz 2D con 
el tamaño de los pixeles , obvio que si mostramos los pixeles reales en una pantalla moderna no mirarias nada , ahi esta en su creatividad en como implementar los graficos

ahora necesitamos cargar en memoria (recordar que la chip 8 tiene una apartado en la memoria exclusiva para las roms)

usaremos esta funcion 
```c++
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
```
cargammos la rom en memoria y nos aseguramoas que esta no se exceda de la memoria reservada.

Ahora necesitamos nuestro ciclo de CPU
```c++
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
```

nuestro ciclo de cpu se resume en: 
- primero obtenemos el opcode de la memoria
- Verificamos que el opcode exista y ejecutamos la funcion correspondiente
- En caso de no estar en la tbla imprimira el opcode en consola
- Se disminuyen los timmers
- Repite

Para ejecutar los opcodes normalmente se usan SWITCH-CASE en el caso de este proyecto se usaron punteros que apuntan a una funcion opcode

```c++
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
```

Ahora si ejecutamos 10 ciclos de nuestro proyeccto deberia a parecer esto en consola

<img width="1275" height="392" alt="image" src="https://github.com/user-attachments/assets/cc3b91d3-b4b6-4278-9ad3-d8a50bb212a2" />
ahora nueesto programa recconoce intrucciones de la chip 8 [todo esto se enecuentra en el archivo old.cpp]

ahora ya tienes un esquema de como empezar hacer tus proyecto .







