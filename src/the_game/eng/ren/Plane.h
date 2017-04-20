#ifndef POLYPLANE_H
#define POLYPLANE_H

#include <glm/vec3.hpp>

const float epsilon = 0.002f;
const float epsilon_sqr = epsilon * epsilon;
const float inv_epsilon = 1.0f / epsilon;
enum {
    FRONT, BACK, ONPLANE, SPANNING, TOUCH_FRONT, TOUCH_BACK
};

struct Plane {
    glm::vec3	n;
    float		d;

    bool operator==(const Plane &rhs) const;

    int ClassifyPoint(const float point[3]) const;

    static bool GetIntersection(const Plane &pl1, const Plane &pl2, const Plane &pl3, float* p);
};


#endif
