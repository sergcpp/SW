#include "Transform.h"

#include <glm/gtc/matrix_transform.hpp>

#include "../eng/sys/Json.h"

void Transform::UpdateBBox() {
    bbox_[0] = bbox_[1] = glm::vec3(matr_[3]);

    for (int j = 0; j < 3; j++) {
        for (int i = 0; i < 3; i++) {
            float a = matr_[i][j] * orig_bbox_[0][i];
            float b = matr_[i][j] * orig_bbox_[1][i];

            if (a < b) {
                bbox_[0][j] += a;
                bbox_[1][j] += b;
            } else {
                bbox_[0][j] += b;
                bbox_[1][j] += a;
            }
        }
    }
}

glm::quat Transform::rot() const {
    return glm::quat_cast(matr_);
}

glm::vec3 Transform::angles() const {
    return glm::degrees(glm::eulerAngles(rot()));
}

void Transform::SetRot(float x, float y, float z, float w) {
    SetRot(glm::quat(x, y, z, w));
}

void Transform::SetRot(const glm::quat &q) {
    glm::vec4 tr = matr_[3];
    glm::vec3 _pos = pos();
    matr_ = glm::mat4_cast(q);
    SetPos(_pos);
    UpdateBBox();
}

void Transform::SetAngles(float x, float y, float z) {
    glm::vec3 _pos = pos();
    matr_ = glm::mat4x4(1);
    matr_ = glm::rotate(matr_, glm::radians(z), glm::vec3(0.0f, 0.0f, 1.0f));
    matr_ = glm::rotate(matr_, glm::radians(x), glm::vec3(1.0f, 0.0f, 0.0f));
    matr_ = glm::rotate(matr_, glm::radians(y), glm::vec3(0.0f, 1.0f, 0.0f));
    SetPos(_pos);
    UpdateBBox();
}

void Transform::SetAngles(const glm::vec3 &v) {
    SetAngles(v.x, v.y, v.z);
}

void Transform::Move(float dx, float dy, float dz) {
    matr_ = glm::translate(matr_, glm::vec3(dx, dy, dz));
    UpdateBBox();
}

void Transform::Move(const glm::vec3 &v) {
    Move(v.x, v.y, v.z);
}

void Transform::Rotate(float dx, float dy, float dz) {
    matr_ = glm::rotate(matr_, glm::radians(dz), glm::vec3(0.0f, 0.0f, 1.0f));
    matr_ = glm::rotate(matr_, glm::radians(dx), glm::vec3(1.0f, 0.0f, 0.0f));
    matr_ = glm::rotate(matr_, glm::radians(dy), glm::vec3(0.0f, 1.0f, 0.0f));
    UpdateBBox();
}

void Transform::Rotate(const glm::vec3 &axis, float angle) {
    matr_ = glm::rotate(matr_, glm::radians(angle), axis);
    UpdateBBox();
}

bool Transform::Check(const glm::vec3 &bbox_min, const glm::vec3 &bbox_max) const {
    return (bbox_min[0] < bbox_[1][0] &&
            bbox_max[0] > bbox_[0][0] &&
            bbox_min[1] < bbox_[1][1] &&
            bbox_max[1] > bbox_[0][1] &&
            bbox_min[2] < bbox_[1][2] &&
            bbox_max[2] > bbox_[0][2]);
}

bool Transform::Check(const Transform &rhs) const {
    return Check(rhs.bbox_[0], rhs.bbox_[1]);
}

bool Transform::TraceL(const glm::vec3 &orig, const glm::vec3 &dir, float *dist) const {
    glm::vec3 inv_dir = 1.0f/dir;

    float low = inv_dir[0] * (bbox_[0][0] - orig[0]);
    float high = inv_dir[0] * (bbox_[1][0] - orig[0]);
    float tmin = glm::min(low, high);
    float tmax = glm::max(low, high);

    low = inv_dir[1] * (bbox_[0][1] - orig[1]);
    high = inv_dir[1] * (bbox_[1][1] - orig[1]);
	tmin = glm::max(tmin, glm::min(low, high));
	tmax = glm::min(tmax, glm::max(low, high));

    low = inv_dir[2] * (bbox_[0][2] - orig[2]);
    high = inv_dir[2] * (bbox_[1][2] - orig[2]);
	tmin = glm::max(tmin, glm::min(low, high));
	tmax = glm::min(tmax, glm::max(low, high));

    *dist = tmin;

    return ((tmin <= tmax) && (tmax > 0.0f));
}

bool Transform::Read(const JsObject &js_in) {
    try {
        const JsObject &js_tr = (const JsObject &) js_in.at("Transform");

		if (js_tr.Has("bbox")) {
			const JsArray &js_bbox = (const JsArray &)js_tr.at("bbox");
			orig_bbox_[0].x = (float)(const JsNumber&)(js_bbox[0]);
			orig_bbox_[0].y = (float)(const JsNumber&)(js_bbox[1]);
			orig_bbox_[0].z = (float)(const JsNumber&)(js_bbox[2]);
			orig_bbox_[1].x = (float)(const JsNumber&)(js_bbox[3]);
			orig_bbox_[1].y = (float)(const JsNumber&)(js_bbox[4]);
			orig_bbox_[1].z = (float)(const JsNumber&)(js_bbox[5]);
		}

        const JsArray &js_pos = (const JsArray &) js_tr.at("pos");
        glm::vec3 pos = { (float) (const JsNumber&) (js_pos[0]),
                          (float) (const JsNumber&) (js_pos[1]),
                          (float) (const JsNumber&) (js_pos[2]) };
        SetPos(pos);

        const JsArray &js_rot = (const JsArray &) js_tr.at("rot");
        glm::quat rot = {(float) (const JsNumber&) (js_rot[0]),
                         (float) (const JsNumber&) (js_rot[1]),
                         (float) (const JsNumber&) (js_rot[2]),
                         (float) (const JsNumber&) (js_rot[3])};
        SetRot(rot);
        return true;
    } catch (...) {
        return false;
    }
}

void Transform::Write(JsObject &js_out) {
    glm::vec3 pos = this->pos();
    glm::quat rot = this->rot();

    JsObject js_tr;

    JsArray js_bbox;
    js_bbox.Push(orig_bbox_[0].x);
    js_bbox.Push(orig_bbox_[0].y);
    js_bbox.Push(orig_bbox_[0].z);
    js_bbox.Push(orig_bbox_[1].x);
    js_bbox.Push(orig_bbox_[1].y);
    js_bbox.Push(orig_bbox_[1].z);

    js_tr["bbox"] = js_bbox;

    JsArray js_pos;
    js_pos.Push(pos.x);
    js_pos.Push(pos.y);
    js_pos.Push(pos.z);

    js_tr["pos"] = js_pos;

    JsArray js_rot;
    js_rot.Push(rot.x);
    js_rot.Push(rot.y);
    js_rot.Push(rot.z);
    js_rot.Push(rot.w);

    js_tr["rot"] = js_rot;

    js_out["Transform"] = js_tr;
}
