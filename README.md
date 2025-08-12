# Chip8_WASM
CHIP 8 EMULATOR USINg WASM 

compile
´´´bash
emcc chip8.cpp -o chip8.js   -s EXPORTED_FUNCTIONS='["_load_rom","_getDebuggerString","_step","_malloc","_free"]'   -s EXPORTED_RUNTIME_METHODS='["ccall","cwrap","HEAPU8","UTF8ToString"]'   -s ALLOW_MEMORY_GROWTH=1   -s TOTAL_MEMORY=67108864
´´´´
