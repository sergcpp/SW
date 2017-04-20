#include "Camera.h"

#include <cmath>

#include "Matrices_old.h"

Camera::Camera(const float center[3], const float target[3], const float up[3]) {
	SetupView(center, target, up);
}

Camera::~Camera() {}

void Camera::SetupView(const float center[3], const float target[3], const float up[3]) {
	matrixSetIdentityM(view_matrix_);

	matrixLookAtM(view_matrix_, center[0], center[1], center[2], target[0], target[1], target[2], up[0], up[1], up[2]);

	world_position_[0] = -(view_matrix_[0] * view_matrix_[12] + view_matrix_[1] * view_matrix_[13]
			+ view_matrix_[2] * view_matrix_[14]);
	world_position_[1] = -(view_matrix_[4] * view_matrix_[12] + view_matrix_[5] * view_matrix_[13]
			+ view_matrix_[6] * view_matrix_[14]);
	world_position_[2] = -(view_matrix_[8] * view_matrix_[12] + view_matrix_[9] * view_matrix_[13]
			+ view_matrix_[10] * view_matrix_[14]);
}

void Camera::Perspective(float angle, float aspect, float nearr, float farr) {
	is_orthographic_ = false;
	BuildPerspProjMat(projection_matrix_, angle, aspect, nearr, farr);
}

void Camera::Orthographic(float left, float right, float down, float top, float nearr, float farr) {
	is_orthographic_ = true;
	matrixFrustumM2(projection_matrix_, left, right, down, top, nearr, farr);
}

void Camera::Move(float* v, float delta_time) {
	view_matrix_[12] -= v[0] * delta_time;
	view_matrix_[13] -= v[1] * delta_time;
	view_matrix_[14] -= v[2] * delta_time;

	//world_position_[0] = -(view_matrix_[0]*view_matrix_[12] + view_matrix_[1]*view_matrix_[13] + view_matrix_[2]*view_matrix_[14]);
	//world_position_[1] = -(view_matrix_[4]*view_matrix_[12] + view_matrix_[5]*view_matrix_[13] + view_matrix_[6]*view_matrix_[14]);
	//world_position_[2] = -(view_matrix_[8]*view_matrix_[12] + view_matrix_[9]*view_matrix_[13] + view_matrix_[10]*view_matrix_[14]);
}

void Camera::Rotate(float* v, float delta_time) {

	float front[3];
	front[0] = -view_matrix_[2];
	front[1] = -view_matrix_[6];
	front[2] = -view_matrix_[10];

	float rot_matrix[16];
	matrixSetIdentityM(rot_matrix);

	matrixRotateM(rot_matrix, v[0] * delta_time, view_matrix_[0], view_matrix_[4], view_matrix_[8]);
	matrixRotateM(rot_matrix, v[1] * delta_time, view_matrix_[1], view_matrix_[5], view_matrix_[9]);

	float tr_front[3];

	tr_front[0] = front[0] * rot_matrix[0] + front[1] * rot_matrix[1] + front[2] * rot_matrix[2];
	tr_front[1] = front[0] * rot_matrix[4] + front[1] * rot_matrix[5] + front[2] * rot_matrix[6];
	tr_front[2] = front[0] * rot_matrix[8] + front[1] * rot_matrix[9] + front[2] * rot_matrix[10];

	matrixSetIdentityM(view_matrix_);

	matrixLookAtM(view_matrix_, world_position_[0], world_position_[1], world_position_[2],
			world_position_[0] + tr_front[0], world_position_[1] + tr_front[1], world_position_[2] + tr_front[2], 0.0,
			1.0, 0.0);
}

void Camera::UpdatePlanes() {

	float combo_matrix[16];
	matrixMultiplyMM(combo_matrix, projection_matrix_, view_matrix_);

	frustum_planes_[LEFT_PLANE].n[0] = combo_matrix[4 * 0 + 3] + combo_matrix[4 * 0 + 0];
	frustum_planes_[LEFT_PLANE].n[1] = combo_matrix[4 * 1 + 3] + combo_matrix[4 * 1 + 0];
	frustum_planes_[LEFT_PLANE].n[2] = combo_matrix[4 * 2 + 3] + combo_matrix[4 * 2 + 0];
	frustum_planes_[LEFT_PLANE].d = combo_matrix[4 * 3 + 3] + combo_matrix[4 * 3 + 0];

	frustum_planes_[RIGHT_PLANE].n[0] = combo_matrix[4 * 0 + 3] - combo_matrix[4 * 0 + 0];
	frustum_planes_[RIGHT_PLANE].n[1] = combo_matrix[4 * 1 + 3] - combo_matrix[4 * 1 + 0];
	frustum_planes_[RIGHT_PLANE].n[2] = combo_matrix[4 * 2 + 3] - combo_matrix[4 * 2 + 0];
	frustum_planes_[RIGHT_PLANE].d = combo_matrix[4 * 3 + 3] - combo_matrix[4 * 3 + 0];

	frustum_planes_[TOP_PLANE].n[0] = combo_matrix[4 * 0 + 3] - combo_matrix[4 * 0 + 1];
	frustum_planes_[TOP_PLANE].n[1] = combo_matrix[4 * 1 + 3] - combo_matrix[4 * 1 + 1];
	frustum_planes_[TOP_PLANE].n[2] = combo_matrix[4 * 2 + 3] - combo_matrix[4 * 2 + 1];
	frustum_planes_[TOP_PLANE].d = combo_matrix[4 * 3 + 3] - combo_matrix[4 * 3 + 1];

	frustum_planes_[BOTTOM_PLANE].n[0] = combo_matrix[4 * 0 + 3] + combo_matrix[4 * 0 + 1];
	frustum_planes_[BOTTOM_PLANE].n[1] = combo_matrix[4 * 1 + 3] + combo_matrix[4 * 1 + 1];
	frustum_planes_[BOTTOM_PLANE].n[2] = combo_matrix[4 * 2 + 3] + combo_matrix[4 * 2 + 1];
	frustum_planes_[BOTTOM_PLANE].d = combo_matrix[4 * 3 + 3] + combo_matrix[4 * 3 + 1];

	frustum_planes_[NEAR_PLANE].n[0] = combo_matrix[4 * 0 + 3] + combo_matrix[4 * 0 + 2];
	frustum_planes_[NEAR_PLANE].n[1] = combo_matrix[4 * 1 + 3] + combo_matrix[4 * 1 + 2];
	frustum_planes_[NEAR_PLANE].n[2] = combo_matrix[4 * 2 + 3] + combo_matrix[4 * 2 + 2];
	frustum_planes_[NEAR_PLANE].d = combo_matrix[4 * 3 + 3] + combo_matrix[4 * 3 + 2];

	frustum_planes_[FAR_PLANE].n[0] = combo_matrix[4 * 0 + 3] - combo_matrix[4 * 0 + 2];
	frustum_planes_[FAR_PLANE].n[1] = combo_matrix[4 * 1 + 3] - combo_matrix[4 * 1 + 2];
	frustum_planes_[FAR_PLANE].n[2] = combo_matrix[4 * 2 + 3] - combo_matrix[4 * 2 + 2];
	frustum_planes_[FAR_PLANE].d = combo_matrix[4 * 3 + 3] - combo_matrix[4 * 3 + 2];

	for (int plane = LEFT_PLANE; plane <= FAR_PLANE; plane++) {
		float inv_l = 1.0f
				/ (float) sqrt(
						frustum_planes_[plane].n[0] * frustum_planes_[plane].n[0]
								+ frustum_planes_[plane].n[1] * frustum_planes_[plane].n[1]
								+ frustum_planes_[plane].n[2] * frustum_planes_[plane].n[2]);
		frustum_planes_[plane].n[0] *= inv_l;
		frustum_planes_[plane].n[1] *= inv_l;
		frustum_planes_[plane].n[2] *= inv_l;
		frustum_planes_[plane].d *= inv_l;
	}

	world_position_[0] = -(view_matrix_[0] * view_matrix_[12] + view_matrix_[1] * view_matrix_[13]
			+ view_matrix_[2] * view_matrix_[14]);
	world_position_[1] = -(view_matrix_[4] * view_matrix_[12] + view_matrix_[5] * view_matrix_[13]
			+ view_matrix_[6] * view_matrix_[14]);
	world_position_[2] = -(view_matrix_[8] * view_matrix_[12] + view_matrix_[9] * view_matrix_[13]
			+ view_matrix_[10] * view_matrix_[14]);
}

bool Camera::IsInFrustum(const float bbox[8][3]) const {
	int in_count;

	for (int plane = LEFT_PLANE; plane <= FAR_PLANE; plane++) {
		in_count = 8;

		for (int i = 0; i < 8; i++) {
			//switch (Plane::ClassifyPoint(frustum_planes_[plane], &bbox[i][0])) {
		    switch (frustum_planes_[plane].ClassifyPoint(&bbox[i][0])) {
			case BACK:
				in_count--;
				break;
			}
		}
		if (in_count == 0) {
			return false;
		}
	}
	return true;
}
