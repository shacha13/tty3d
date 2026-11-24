#include "canvas.hpp"

#include <algorithm>
#include <cstdlib>

Canvas::Canvas(int width, int height)
    : width_(width), height_(height), cells_(static_cast<size_t>(width) * height, ' ') {}

void Canvas::clear(char fill) {
    std::fill(cells_.begin(), cells_.end(), fill);
}

void Canvas::plot(int x, int y, char c) {
    if (x < 0 || y < 0 || x >= width_ || y >= height_) return;
    cells_[static_cast<size_t>(y) * width_ + x] = c;
}

// Bresenham's line algorithm, integer-only and valid for every octant.
void Canvas::line(int x0, int y0, int x1, int y1, char c) {
    int dx = std::abs(x1 - x0);
    int dy = -std::abs(y1 - y0);
    int sx = x0 < x1 ? 1 : -1;
    int sy = y0 < y1 ? 1 : -1;
    int err = dx + dy;

    while (true) {
        plot(x0, y0, c);
        if (x0 == x1 && y0 == y1) break;
        int e2 = 2 * err;
        if (e2 >= dy) { err += dy; x0 += sx; }
        if (e2 <= dx) { err += dx; y0 += sy; }
    }
}

std::string Canvas::frame() const {
    std::string out = "\x1b[H";
    out.reserve(out.size() + cells_.size() + height_);
    for (int y = 0; y < height_; ++y) {
        out.append(&cells_[static_cast<size_t>(y) * width_], width_);
        out.push_back('\n');
    }
    return out;
}
