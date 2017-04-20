#include "Utils.h"

glm::vec2 ui::MapPointToScreen(const glm::ivec2 &p, const glm::ivec2 &res) {
    return (2.0f * glm::vec2(p.x, res.y - p.y)) / (glm::vec2)res + glm::vec2(-1, -1);
}

