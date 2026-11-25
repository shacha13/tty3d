# tty3d

A software 3D renderer that draws into the terminal using text characters. No GPU, no graphics libraries; every pixel is computed on the CPU in standard C++17.

## Status

Work in progress.

- [x] Vector and matrix math, perspective projection
- [x] Character canvas with Bresenham line drawing
- [x] Spinning wireframe cube
- [x] Triangle rasterization with a depth buffer
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

Press Ctrl+C to exit. Use `--frames N` to render a fixed number of frames, and `--wireframe` to draw edges only.

## How it works

Each frame, cube vertices are transformed by a model-view-projection matrix into clip space, divided by w to get normalized device coordinates, and mapped to character cells. The horizontal axis is scaled to account for terminal cells being about twice as tall as they are wide.

Each cube face is split into two triangles and filled with a bounding-box rasterizer: edge functions decide which cells lie inside, and the same values give barycentric weights for interpolating depth. A per-cell depth buffer keeps only the nearest surface, so hidden faces are removed regardless of draw order. In wireframe mode, edges are drawn with Bresenham's algorithm instead.

The finished frame is written in a single call after a cursor-home escape code to avoid flicker.
