#pragma once

#include <glm/mat4x4.hpp>
#include <glm/vec3.hpp>
#include <glm/gtc/quaternion.hpp>

#include "../eng/GoComponent.h"

#include "ISerializable.h"

class Transform : public GoComponent, public ISerializable {
    glm::mat4 matr_;
    glm::vec3 bbox_[2], orig_bbox_[2];

    void UpdateBBox();
public:
    OVERRIDE_NEW(Transform)
    DEF_ID("Transform")

    Transform(const glm::vec3 &bbox_min = glm::vec3(), const glm::vec3 &bbox_max = glm::vec3()) : matr_(1) {
		bbox_[0] = orig_bbox_[0] = bbox_min;
		bbox_[1] = orig_bbox_[1] = bbox_max;
	}

    const glm::vec3 &bbox_min() const { return bbox_[0]; }
    const glm::vec3 &bbox_max() const { return bbox_[1]; }

    const glm::vec3 &orig_bbox_min() const { return orig_bbox_[0]; }
    const glm::vec3 &orig_bbox_max() const { return orig_bbox_[1]; }

    const glm::mat4 &matrix() const { return matr_; }

    glm::vec3 pos() const { return glm::vec3(matr_[3]); }
    glm::quat rot() const;
    glm::vec3 angles() const;

    void SetPos(float x, float y, float z) {
        matr_[3] = glm::vec4(x, y, z, 1.0f);
        UpdateBBox();
    }

    void SetPos(const glm::vec3 &v) {
        SetPos(v.x, v.y, v.z);
    }

    void SetMatrix(const glm::mat4x4 &mat) {
        matr_ = mat;
        UpdateBBox();
    }

    void SetRot(float x, float y, float z, float w);
    void SetRot(const glm::quat &q);

    void SetAngles(float x, float y, float z);
    void SetAngles(const glm::vec3 &v);

    void Move(float dx, float dy, float dz);
    void Move(const glm::vec3 &v);

    void Rotate(float dx, float dy, float dz);
    void Rotate(const glm::vec3 &axis, float angle);

    bool Check(const glm::vec3 &bbox_min, const glm::vec3 &bbox_max) const;
    bool Check(const Transform &rhs) const;

    bool TraceL(const glm::vec3 &orig, const glm::vec3 &dir, float *dist) const;

    // ISerializable
	bool Read(const JsObject &js_in) override;
    void Write(JsObject &js_out) override;
};

