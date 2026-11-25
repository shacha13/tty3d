#pragma once

#include <string>
#include <vector>

#include "math.hpp"

// A grid of characters that the renderer draws into before it is
// written to the terminal in one go.
class Canvas {
public:
    Canvas(int width, int height);

    int width() const { return width_; }
    int height() const { return height_; }

    // Resets every cell to `fill` and every depth value to "infinitely far".
    void clear(char fill = ' ');
    void plot(int x, int y, char c);
    void line(int x0, int y0, int x1, int y1, char c);

    // Writes `c` only if `depth` is nearer than what is already stored
    // at (x, y). Smaller depth means closer to the camera.
    void plotDepth(int x, int y, float depth, char c);

    // Fills a triangle given in screen space (x, y in cells, z = depth).
    // Depth is interpolated across the triangle and tested per cell.
    void fillTriangle(const Vec3& a, const Vec3& b, const Vec3& c, char ch);

    // Builds the full frame as one string, prefixed with a cursor-home
    // escape so each frame overwrites the previous one without flicker.
    std::string frame() const;

private:
    int width_;
    int height_;
    std::vector<char> cells_;
    std::vector<float> depth_;
};
