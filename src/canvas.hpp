#pragma once

#include <string>
#include <vector>

// A grid of characters that the renderer draws into before it is
// written to the terminal in one go.
class Canvas {
public:
    Canvas(int width, int height);

    int width() const { return width_; }
    int height() const { return height_; }

    void clear(char fill = ' ');
    void plot(int x, int y, char c);
    void line(int x0, int y0, int x1, int y1, char c);

    // Builds the full frame as one string, prefixed with a cursor-home
    // escape so each frame overwrites the previous one without flicker.
    std::string frame() const;

private:
    int width_;
    int height_;
    std::vector<char> cells_;
};
