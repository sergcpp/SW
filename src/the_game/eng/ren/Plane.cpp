#include "Plane.h"

#include <glm/geometric.hpp>

bool Plane::operator==(const Plane &rhs) const {
    if (fabs(rhs.d - d) < epsilon) {
        if (fabs(rhs.n[0] - n[0]) < epsilon) if (fabs(rhs.n[1] - n[1]) < epsilon) if (fabs(rhs.n[2] - n[2]) < epsilon)
            return true;
    }
    return false;
}

int Plane::ClassifyPoint(const float point[3]) const {
    float result = glm::dot(*(glm::vec3 *)point, n) + d;
    if (result > epsilon) {
        return FRONT;
    } else if (result < -epsilon) {
        return BACK;
    }
    return ONPLANE;
}

bool Plane::GetIntersection(const Plane &pl1, const Plane &pl2, const Plane &pl3, float* p) {
    glm::vec3 cross_2_3 = glm::cross(pl2.n, pl3.n);

    float denom = glm::dot(pl1.n, cross_2_3);

    if (glm::abs(denom) < epsilon) {
        return false;
    }
    denom = 1.0f / denom;

    glm::vec3 cross_3_1 = glm::cross(pl3.n, pl1.n);
    glm::vec3 cross_1_2 = glm::cross(pl1.n, pl2.n);

    *(glm::vec3 *) p = (-pl1.d * cross_2_3 - pl2.d * cross_3_1 - pl3.d * cross_1_2) * denom;
    return true;
}
