#pragma once

namespace terminal {

// Enables ANSI escape handling (needed on Windows), clears the screen
// and hides the cursor. restore() undoes the visible changes.
void init();
void restore();

// Current terminal size in character cells. Falls back to 80x24.
void size(int& cols, int& rows);

}  // namespace terminal
