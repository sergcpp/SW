#pragma once

#include "../eng/GoComponent.h"
#include "../eng/GoID.h"
#include "../eng/ren/Mesh.h"

#include "ISerializable.h"

class Drawable : public GoComponent, public ISerializable {
    R::MeshRef mesh_;
    int cur_anim_ = -1;

    void LoadMesh(const std::string &name);
    void LoadAnim(const std::string &name);

    static void OnTextureNeeded(const char *name);
    static void OnMaterialNeeded(const char *name);
    static void OnProgramNeeded(const char *name, const char *arg1, const char *arg2);
public:
    OVERRIDE_NEW(Drawable)
    DEF_ID("Drawable")

    const R::MeshRef &mesh() const { return mesh_; }
    int cur_anim() const { return cur_anim_; }

    void set_cur_anim(int i) { cur_anim_ = i; }

    void UpdateAnim(float dt_s);
    void UpdateAnim(int anim1, int anim2, float t, float dt_s);

    // ISerializable
    bool Read(const JsObject &js_in) override;
    void Write(JsObject &js_out) override {}

	float anim_time = 0;
};