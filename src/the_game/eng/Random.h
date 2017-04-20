#ifndef GAME_RANDOM_H
#define GAME_RANDOM_H

#include <glm/vec3.hpp>

namespace Random {
	int GetInt(int min, int max);
	float GetFloat(float min, float max);
	float GetNormalizedFloat();
	float GetMinus1to1Float();
	glm::vec3 GetNormalizedVec3();
}

#endif