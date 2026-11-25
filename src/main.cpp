#include <array>
#include <chrono>
#include <csignal>
#include <cmath>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <thread>

#include "canvas.hpp"
#include "math.hpp"
#include "terminal.hpp"

namespace {

volatile std::sig_atomic_t running = 1;

void onSignal(int) { running = 0; }

const std::array<Vec3, 8> kCubeVertices = {{
    {-1, -1, -1}, {1, -1, -1}, {1, 1, -1}, {-1, 1, -1},
    {-1, -1, 1},  {1, -1, 1},  {1, 1, 1},  {-1, 1, 1},
}};

const std::array<std::array<int, 2>, 12> kCubeEdges = {{
    {0, 1}, {1, 2}, {2, 3}, {3, 0},
    {4, 5}, {5, 6}, {6, 7}, {7, 4},
    {0, 4}, {1, 5}, {2, 6}, {3, 7},
}};

// Each face is a quad split into two triangles. Faces get their own
// character so the depth buffer's work is visible until lighting exists.
struct Face {
    std::array<int, 4> v;
    char ch;
};

const std::array<Face, 6> kCubeFaces = {{
    {{0, 1, 2, 3}, '#'},  // back   (-z)
    {{4, 5, 6, 7}, '@'},  // front  (+z)
    {{0, 4, 7, 3}, '%'},  // left   (-x)
    {{1, 5, 6, 2}, '*'},  // right  (+x)
    {{0, 1, 5, 4}, '+'},  // bottom (-y)
    {{3, 2, 6, 7}, '='},  // top    (+y)
}};

// Terminal cells are roughly twice as tall as they are wide.
constexpr float kCellAspect = 2.0f;

struct ScreenPoint {
    Vec3 pos;  // x, y in cells; z is NDC depth
    bool visible;
};

ScreenPoint project(const Mat4& mvp, const Vec3& p, int width, int height) {
    Vec4 clip = mvp * Vec4{p.x, p.y, p.z, 1};
    if (clip.w <= 0) return {{}, false};

    float ndcX = clip.x / clip.w;
    float ndcY = clip.y / clip.w;
    float ndcZ = clip.z / clip.w;

    float sx = (ndcX + 1) * 0.5f * width;
    float sy = (1 - ndcY) * 0.5f * height;
    return {{sx, sy, ndcZ}, true};
}

std::array<ScreenPoint, 8> projectCube(const Canvas& canvas, const Mat4& mvp) {
    std::array<ScreenPoint, 8> pts;
    for (size_t i = 0; i < kCubeVertices.size(); ++i) {
        pts[i] = project(mvp, kCubeVertices[i], canvas.width(), canvas.height());
    }
    return pts;
}

void drawWireframe(Canvas& canvas, const std::array<ScreenPoint, 8>& pts) {
    auto cell = [](float v) { return static_cast<int>(std::floor(v)); };
    for (const auto& e : kCubeEdges) {
        const auto& a = pts[e[0]];
        const auto& b = pts[e[1]];
        if (a.visible && b.visible) {
            canvas.line(cell(a.pos.x), cell(a.pos.y), cell(b.pos.x), cell(b.pos.y), '#');
        }
    }
}

void drawSolid(Canvas& canvas, const std::array<ScreenPoint, 8>& pts) {
    for (const auto& f : kCubeFaces) {
        const auto& a = pts[f.v[0]];
        const auto& b = pts[f.v[1]];
        const auto& c = pts[f.v[2]];
        const auto& d = pts[f.v[3]];
        if (!(a.visible && b.visible && c.visible && d.visible)) continue;
        canvas.fillTriangle(a.pos, b.pos, c.pos, f.ch);
        canvas.fillTriangle(a.pos, c.pos, d.pos, f.ch);
    }
}

}  // namespace

int main(int argc, char** argv) {
    // --frames N renders a fixed number of frames and exits (useful for testing).
    // --wireframe draws edges only instead of filled faces.
    long maxFrames = -1;
    bool wireframe = false;
    for (int i = 1; i < argc; ++i) {
        if (std::strcmp(argv[i], "--frames") == 0 && i + 1 < argc) {
            maxFrames = std::strtol(argv[++i], nullptr, 10);
        } else if (std::strcmp(argv[i], "--wireframe") == 0) {
            wireframe = true;
        }
    }

    std::signal(SIGINT, onSignal);

    int cols, rows;
    terminal::size(cols, rows);
    Canvas canvas(cols, rows - 1);

    float aspect = static_cast<float>(canvas.width()) / (canvas.height() * kCellAspect);
    Mat4 projection = Mat4::perspective(1.0f, aspect, 0.1f, 100.0f);
    Mat4 view = Mat4::translate(0, 0, -4.5f);

    terminal::init();

    using clock = std::chrono::steady_clock;
    const auto frameTime = std::chrono::milliseconds(33);
    auto start = clock::now();

    for (long frame = 0; running && frame != maxFrames; ++frame) {
        auto frameStart = clock::now();
        float t = std::chrono::duration<float>(frameStart - start).count();

        Mat4 model = Mat4::rotateY(t * 0.9f) * Mat4::rotateX(t * 0.6f) * Mat4::rotateZ(t * 0.3f);
        Mat4 mvp = projection * view * model;

        canvas.clear();
        auto pts = projectCube(canvas, mvp);
        if (wireframe) {
            drawWireframe(canvas, pts);
        } else {
            drawSolid(canvas, pts);
        }
        std::fputs(canvas.frame().c_str(), stdout);
        std::fflush(stdout);

        std::this_thread::sleep_until(frameStart + frameTime);
    }

    terminal::restore();
    return 0;
}
