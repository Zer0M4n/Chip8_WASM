# Chip8_WASM
CHIP 8 EMULATOR USINg WASM 

compile
´´´bash
 emcc chip8.cpp -o chip8.js \
  -s EXPORTED_FUNCTIONS='["_load_rom","_step","_malloc","_free"]' \
  -s EXPORTED_RUNTIME_METHODS='["ccall","cwrap","HEAPU8"]' \
  -s ALLOW_MEMORY_GROWTH=1 \

´´´´
