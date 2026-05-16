# wasm-client-demo
A thin JS shim layer to provide interactivity and drawing for a C program compiled to WASM.

### Features
- No NPM
- No Emscripten
- One C source file, one HTML file

### Limitations
- No C standard library
- No heap allocations xD

### Dependencies
- Clang and LLD
- Something to serve the page with (e.g. Python -> `uv run python -m http.server 8027`)