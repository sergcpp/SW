#ifndef UTILS_H
#define UTILS_H

#include <glm/vec2.hpp>

namespace ui {
    glm::vec2 MapPointToScreen(const glm::ivec2 &p, const glm::ivec2 &res);
}

#endif // UTILS_H
