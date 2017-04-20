#ifndef CAMERA_H
#define CAMERA_H

#include "Plane.h"

enum {
	LEFT_PLANE, RIGHT_PLANE, TOP_PLANE, BOTTOM_PLANE, NEAR_PLANE, FAR_PLANE
};

class Camera {
protected:
	float view_matrix_[16];
	float projection_matrix_[16];

	float world_position_[3];

	Plane frustum_planes_[6];
	bool is_orthographic_;

public:

	Camera(const float center[3], const float target[3], const float up[3]);
	~Camera();

	const float* const view_matrix() const {
		return view_matrix_;
	}

	const Plane* const frustum_planes() const {
		return frustum_planes_;
	}

	const float* projection_matrix() const {
		return projection_matrix_;
	}

	const float* world_position() const {
		return world_position_;
	}

	const bool is_orthographic() const {
		return is_orthographic_;
	}

	void Perspective(float angle, float aspect, float near, float far);
	void Orthographic(float left, float right, float top, float down, float near, float far);

	void SetupView(const float center[3], const float target[3], const float up[3]);

	void UpdatePlanes();
	bool IsInFrustum(const float bbox[8][3]) const;

	void Move(float* v, float delta_time);
	void Rotate(float* v, float delta_time);

};

#endif
