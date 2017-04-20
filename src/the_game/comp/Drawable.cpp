#include "Drawable.h"

#include "../eng/sys/AssetFile.h"
#include "../eng/sys/Json.h"
#include "../eng/sys/Log.h"

void Drawable::UpdateAnim(float dt_s) {
    assert(mesh_.type == R::MeshSkeletal);
    R::Mesh *m = R::GetMesh(mesh_);

    R::Skeleton *skel = &m->skel;
    if (cur_anim_ != -1) {
        assert(cur_anim_ < skel->anims.size());
        skel->UpdateAnim(cur_anim_, dt_s, &anim_time);
        skel->ApplyAnim(cur_anim_);
    }
}

void Drawable::UpdateAnim(int anim1, int anim2, float t, float dt_s) {
    assert(mesh_.type == R::MeshSkeletal);
    R::Mesh *m = R::GetMesh(mesh_);

    R::Skeleton *skel = &m->skel;
    assert(anim1 < skel->anims.size() && anim2 < skel->anims.size());

    anim_time += dt_s;
    skel->UpdateAnim(anim1, 0, &anim_time);
    skel->UpdateAnim(anim2, 0, &anim_time);

    skel->ApplyAnim(anim1, anim2, t);
}

bool Drawable::Read(const JsObject &js_in) {
    try {
        const JsObject &js_dr = (const JsObject &) js_in.at("drawable");

        const JsString &mesh_name = (const JsString &) js_dr.at("mesh_name");
        LoadMesh(mesh_name.val);

        if (js_dr.Has("anims")) {
            const JsArray &anims = (const JsArray &) js_dr.at("anims");
            for (const auto &a : anims.elements) {
                const JsString &anim_name = (const JsString &) a;
                LoadAnim(anim_name.val);
            }
        }

        return true;
    } catch (...) {
        return false;
    }
}

void Drawable::OnTextureNeeded(const char *name) {
    using namespace std;

    sys::AssetFile in_file((string("game_assets/textures/") + name).c_str());
    size_t in_file_size = in_file.size();
    unique_ptr<char[]> in_buf(new char[in_file_size]);
    in_file.Read(&in_buf[0], in_file_size);

    R::LoadTexture2D(name, &in_buf[0]);

    LOGI("Texture %s loaded", name);
}

void Drawable::OnMaterialNeeded(const char *name) {
    using namespace std;

    sys::AssetFile in_file(string("game_assets/materials/") + name);
    if (!in_file) {
        LOGE("Error loading material %s", name);
        return;
    }

    size_t file_size = in_file.size();

    string mat_src;
    mat_src.resize(file_size);
    in_file.Read((char *)mat_src.data(), file_size);

    R::eMatLoadStatus status;
    R::MaterialRef m_ref = R::LoadMaterial(name, mat_src.data(), &status, OnProgramNeeded, OnTextureNeeded);
}

void Drawable::OnProgramNeeded(const char *name, const char *arg1, const char *arg2) {
#if defined(USE_SW_RENDER)
    void LoadSWProgram(const char *);
    LoadSWProgram(name);
#endif
}

void Drawable::LoadMesh(const std::string &name) {
	using namespace std;

	sys::AssetFile in_file((string("game_assets/models/") + name).c_str());
	size_t in_file_size = in_file.size();
	std::unique_ptr<char[]> in_buf(new char[in_file_size]);
	in_file.Read(&in_buf[0], in_file_size);
	mesh_ = R::LoadMesh(name.c_str(), &in_buf[0], OnMaterialNeeded);
}

void Drawable::LoadAnim(const std::string &name) {
    using namespace std;

    sys::AssetFile in_file((string("game_assets/models/") + name).c_str());
    size_t in_file_size = in_file.size();
    std::unique_ptr<char[]> in_buf(new char[in_file_size]);
    in_file.Read(&in_buf[0], in_file_size);

    R::Mesh *mesh = R::GetMesh(mesh_);
    mesh->skel.AddAnimSequence(name.c_str(), &in_buf[0]);
}
