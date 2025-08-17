# 🎮 Chip8_WASM  
**CHIP-8 Emulator using WebAssembly (WASM)**  

This project implements a **CHIP-8** emulator compiled to **WebAssembly** with Emscripten, offering a high-performance emulation experience in web browsers.

---
![Grabación 2025-08-16 193637](https://github.com/user-attachments/assets/72b4c9d3-7a81-412d-b6dc-cb38e01c9247)

## 📋 Overview

CHIP-8 is an interpreted programming language developed by Joseph Weisbecker in the mid-1970s. This emulator allows running classic CHIP-8 ROMs directly in the browser using WebAssembly for optimal performance.

### ✨ Features

- 🚀 **High performance** with WebAssembly
- 🎯 **Full compatibility** with CHIP-8 ROMs
- 🔧 **Integrated debugging API**  
- 🌐 **Browser execution** without plugins
- 📱 **Responsive** and cross-platform

---

## ⚡ Compilation  

### Prerequisites

- **Emscripten SDK** installed and configured
- **Source file** `chip8.cpp` 
- `.ch8` ROMs for testing

### Compilation Command

Use `emcc` to compile `chip8.cpp` into a JavaScript + WASM module:  

```bash
emcc chip8.cpp -o chip8.js \
  -s EXPORTED_FUNCTIONS='["_load_rom","_getDebuggerString","_step","_malloc","_free"]' \
  -s EXPORTED_RUNTIME_METHODS='["ccall","cwrap","HEAPU8","UTF8ToString"]' \
  -s ALLOW_MEMORY_GROWTH=1 \
  -s TOTAL_MEMORY=67108864
```

### Compilation Parameters

| Parameter | Description |
|-----------|-------------|
| `EXPORTED_FUNCTIONS` | C++ functions exposed to JavaScript |
| `EXPORTED_RUNTIME_METHODS` | Runtime methods for interoperability |
| `ALLOW_MEMORY_GROWTH` | Allows dynamic memory growth |
| `TOTAL_MEMORY` | Initial allocated memory (64MB) |

---

## 🏗️ System Architecture

### Flow Diagram

```
          ┌────────────┐
          │   ROM .ch8 │
          └──────┬─────┘
                 │
                 ▼
        ┌─────────────────┐
        │  C++ / Chip-8   │
        │ (emulator core) │
        └──────┬──────────┘
               │ Compiled with
               │ Emscripten
               ▼
        ┌─────────────────┐
        │   WebAssembly   │
        │   (chip8.wasm)  │
        └──────┬──────────┘
               │
               ▼
        ┌─────────────────┐
        │  JavaScript API │
        │ (ccall, cwrap)  │
        └──────┬──────────┘
               │
       ┌───────▼─────────┐
       │   render() JS   │
       │ (CHIP8 graphics)│
       └─────────────────┘
```

### Main Components

1. **C++ Core**: Main CHIP-8 emulator logic
2. **WebAssembly Module**: Optimized compiled code
3. **JavaScript Bridge**: API for browser communication
4. **Rendering System**: Graphics rendering system

---

## 🔧 API Reference

### Exported Functions

#### `load_rom(romData, size)`
Loads a CHIP-8 ROM into memory.
- **Parameters**: 
  - `romData`: Pointer to ROM data
  - `size`: ROM size in bytes
- **Returns**: `true` if loading was successful

#### `step()`
Executes one instruction cycle of the emulator.
- **Returns**: Current emulator state

#### `getDebuggerString()`
Gets debug information of the current state.
- **Returns**: String with register and memory information

---

## 🚀 Basic Usage

### Change rom

```javascript
 async function loadRom(path) {
      const response = await fetch(path);
      if (!response.ok) {
        throw new Error(`Error cargando ROM: ${response.statusText}`);
      }
      const buffer = await response.arrayBuffer();
      const bytes = new Uint8Array(buffer);

      const ptr = Module._malloc(bytes.length);
      if (!ptr) throw new Error('No memory load');

      Module.HEAPU8.set(bytes, ptr);
      Module.ccall('load_rom', null, ['number', 'number'], [ptr, bytes.length]);
      Module._free(ptr);

      console.log('ROM load !!!!:', path);
    }

    Module.onRuntimeInitialized = async function () {
  console.log('WASM runtime listo');
  try {
    await loadRom('rom/1dcell.ch8');

    const cyclesPerFrame = 10; 
    const frameMs = 1000 / 60; 

    setInterval(() => {
      for (let i = 0; i < cyclesPerFrame; i++) {
        Module.ccall('step', null, [], []);
      }
      updateDebugger(); // actualizar debugger
    }, frameMs);

  } catch (e) {
    console.error('Error load rom fail:', e);
  }
};
    function updateDebugger() {
      const ptr = Module.ccall('getDebuggerString', 'number', [], []);
      const dbgStr = Module.UTF8ToString(ptr);
      const html = dbgStr.replace(/\n/g, "<br>");
      document.getElementById('screen_debugger').innerHTML = html;
    }

```


---

## 🎮 Controls

### CHIP-8 Keyboard Mapping

```
CHIP-8 Keypad    Keyboard
┌─┬─┬─┬─┐       ┌─┬─┬─┬─┐
│1│2│3│C│       │1│2│3│4│
├─┼─┼─┼─┤  -->  ├─┼─┼─┼─┤
│4│5│6│D│       │Q│W│E│R│
├─┼─┼─┼─┤       ├─┼─┼─┼─┤
│7│8│9│E│       │A│S│D│F│
├─┼─┼─┼─┤       ├─┼─┼─┼─┤
│A│0│B│F│       │Z│X│C│V│
└─┴─┴─┴─┘       └─┴─┴─┴─┘
```

---

## 🛠️ Development and Debug

### Debug Flags

For debug compilation:

```bash
emcc chip8.cpp -o chip8.js \
  -s EXPORTED_FUNCTIONS='["_load_rom","_getDebuggerString","_step","_malloc","_free"]' \
  -s EXPORTED_RUNTIME_METHODS='["ccall","cwrap","HEAPU8","UTF8ToString"]' \
  -s ALLOW_MEMORY_GROWTH=1 \
  -s TOTAL_MEMORY=67108864 \
  -s ASSERTIONS=1 \
  -O0 -g
```

---

## 📦 Project Structure

```
chip8_wasm/
├── rom/
├── index.html
├── chip8.cpp
├── chip8.wasm #compiler output
|── chip8.js #compiler output
└── README.md
```

---

## 🤝 Contributing

1. Fork the project
2. Create a feature branch (`git checkout -b feature/new-feature`)
3. Commit your changes (`git commit -am 'Add new feature'`)
4. Push to the branch (`git push origin feature/new-feature`)
5. Open a Pull Request

---


## 🔗 References

- [CHIP-8 Technical Reference](http://devernay.free.fr/hacks/chip8/C8TECH10.HTM)
- [Emscripten Documentation](https://emscripten.org/docs/)
- [WebAssembly Specification](https://webassembly.github.io/spec/)

---


**Enjoy coding and playing with CHIP-8!** 🎮✨
