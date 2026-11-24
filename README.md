# tty3d

A software 3D renderer that draws into the terminal using text characters. No GPU, no graphics libraries; every pixel is computed on the CPU in standard C++17.

## Status

Work in progress.

- [x] Vector and matrix math, perspective projection
- [x] Character canvas with Bresenham line drawing
- [x] Spinning wireframe cube
- [ ] Triangle rasterization with a depth buffer
- [ ] Lighting and character-based shading
- [ ] OBJ model loading
- [ ] Color output

## Build

Requires CMake 3.16+ and a C++17 compiler.

```
cmake -B build
cmake --build build
```

## Run

```
./build/tty3d
```

Press Ctrl+C to exit. Use `--frames N` to render a fixed number of frames.

## How it works

Each frame, cube vertices are transformed by a model-view-projection matrix into clip space, divided by w to get normalized device coordinates, and mapped to character cells. The horizontal axis is scaled to account for terminal cells being about twice as tall as they are wide. Edges are drawn with Bresenham's algorithm, and the finished frame is written in a single call after a cursor-home escape code to avoid flicker.
