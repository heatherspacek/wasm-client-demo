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
- clang
- LLD
- Something to serve the page with (locally, I reach for Python -> `python -m http.server`)
- *(optional)* Something to bake resources into headers; I use Python (see `/scripts`).
