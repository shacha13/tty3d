#include <array>
#include <chrono>
#include <csignal>
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

// Terminal cells are roughly twice as tall as they are wide.
constexpr float kCellAspect = 2.0f;

struct ScreenPoint {
    int x, y;
    bool visible;
};

ScreenPoint project(const Mat4& mvp, const Vec3& p, int width, int height) {
    Vec4 clip = mvp * Vec4{p.x, p.y, p.z, 1};
    if (clip.w <= 0) return {0, 0, false};

    float ndcX = clip.x / clip.w;
    float ndcY = clip.y / clip.w;

    int sx = static_cast<int>((ndcX + 1) * 0.5f * (width - 1) + 0.5f);
    int sy = static_cast<int>((1 - ndcY) * 0.5f * (height - 1) + 0.5f);
    return {sx, sy, true};
}

void drawCube(Canvas& canvas, const Mat4& mvp) {
    std::array<ScreenPoint, 8> pts;
    for (size_t i = 0; i < kCubeVertices.size(); ++i) {
        pts[i] = project(mvp, kCubeVertices[i], canvas.width(), canvas.height());
    }
    for (const auto& e : kCubeEdges) {
        const auto& a = pts[e[0]];
        const auto& b = pts[e[1]];
        if (a.visible && b.visible) canvas.line(a.x, a.y, b.x, b.y, '#');
    }
    for (const auto& p : pts) {
        if (p.visible) canvas.plot(p.x, p.y, '@');
    }
}

}  // namespace

int main(int argc, char** argv) {
    // --frames N renders a fixed number of frames and exits (useful for testing).
    long maxFrames = -1;
    for (int i = 1; i < argc; ++i) {
        if (std::strcmp(argv[i], "--frames") == 0 && i + 1 < argc) {
            maxFrames = std::strtol(argv[++i], nullptr, 10);
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
        drawCube(canvas, mvp);
        std::fputs(canvas.frame().c_str(), stdout);
        std::fflush(stdout);

        std::this_thread::sleep_until(frameStart + frameTime);
    }

    terminal::restore();
    return 0;
}
