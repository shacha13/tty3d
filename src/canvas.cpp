#include "canvas.hpp"

#include <algorithm>
#include <cmath>
#include <cstdlib>
#include <limits>

Canvas::Canvas(int width, int height)
    : width_(width), height_(height), cells_(static_cast<size_t>(width) * height, ' '),
      depth_(cells_.size(), std::numeric_limits<float>::infinity()) {}

void Canvas::clear(char fill) {
    std::fill(cells_.begin(), cells_.end(), fill);
    std::fill(depth_.begin(), depth_.end(), std::numeric_limits<float>::infinity());
}

void Canvas::plot(int x, int y, char c) {
    if (x < 0 || y < 0 || x >= width_ || y >= height_) return;
    cells_[static_cast<size_t>(y) * width_ + x] = c;
}

void Canvas::plotDepth(int x, int y, float depth, char c) {
    if (x < 0 || y < 0 || x >= width_ || y >= height_) return;
    size_t i = static_cast<size_t>(y) * width_ + x;
    if (depth < depth_[i]) {
        depth_[i] = depth;
        cells_[i] = c;
    }
}

namespace {

// Twice the signed area of triangle (a, b, p). Its sign tells which side
// of edge a->b the point p lies on.
float edge(const Vec3& a, const Vec3& b, float px, float py) {
    return (b.x - a.x) * (py - a.y) - (b.y - a.y) * (px - a.x);
}

}  // namespace

// Bounding-box rasterizer using edge functions. Each cell is sampled at its
// center; the three edge values, divided by the total area, are the
// barycentric weights used to interpolate depth. Screen-space depth (NDC z)
// is affine across the triangle, so linear interpolation is exact here.
void Canvas::fillTriangle(const Vec3& a, const Vec3& b, const Vec3& c, char ch) {
    float area = edge(a, b, c.x, c.y);
    if (std::fabs(area) < 1e-6f) return;

    int minX = std::max(0, static_cast<int>(std::floor(std::min({a.x, b.x, c.x}))));
    int maxX = std::min(width_ - 1, static_cast<int>(std::ceil(std::max({a.x, b.x, c.x}))));
    int minY = std::max(0, static_cast<int>(std::floor(std::min({a.y, b.y, c.y}))));
    int maxY = std::min(height_ - 1, static_cast<int>(std::ceil(std::max({a.y, b.y, c.y}))));

    float invArea = 1.0f / area;
    for (int y = minY; y <= maxY; ++y) {
        float py = y + 0.5f;
        for (int x = minX; x <= maxX; ++x) {
            float px = x + 0.5f;
            // Normalizing by area makes the weights positive inside the
            // triangle regardless of winding order.
            float w0 = edge(b, c, px, py) * invArea;
            float w1 = edge(c, a, px, py) * invArea;
            float w2 = edge(a, b, px, py) * invArea;
            if (w0 < 0 || w1 < 0 || w2 < 0) continue;

            float z = w0 * a.z + w1 * b.z + w2 * c.z;
            plotDepth(x, y, z, ch);
        }
    }
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
