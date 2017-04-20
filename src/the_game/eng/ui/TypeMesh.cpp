#include "TypeMesh.h"

#include "BitmapFont.h"
#include "Renderer.h"

ui::TypeMesh::TypeMesh(const std::string &text, BitmapFont *font, const glm::vec2 &pos, const BaseElement *parent)
        : BaseElement(pos, {0, 0}, parent),
          text_(text), font_(font) {
    Move(pos, parent);
}

void ui::TypeMesh::Centrate() {
    using namespace glm;

    vec2 delta = dims_[0] - center_;

    center_ += delta;
    dims_[0] += delta;

    ivec2 delta_px = delta * ((vec2)dims_px_[1] / dims_[1]);
    dims_px_[0] += delta_px;

    for (auto point = pos_.begin(); point != pos_.end(); point += 3) {
        point[0] += delta.x;
        point[1] += delta.y;
    }
}

void ui::TypeMesh::Move(const glm::vec2 &pos, const BaseElement *parent) {
    using namespace glm;

    float w = font_->GetTriangles(text_.c_str(), pos_, uvs_, indices_, pos, parent);
    vec2 size = { w, font_->height(parent) };

	
	dims_[0] = parent->pos() + 0.5f * (pos + vec2(1, 1)) * parent->size();
	dims_[1] = size; //0.5f * rel_dims_[1] * parent->size();

	rel_dims_[0] = pos;
	rel_dims_[1] = 2.0f * dims_[1] / parent->size();

    dims_px_[0] = (ivec2) ((vec2)parent->pos_px() + 0.5f * (pos + vec2(1, 1)) * (vec2)parent->size_px());
    dims_px_[1] = (ivec2) (size * (vec2) parent->size_px() * 0.5f);

    //dims_[0] = parent->pos() + 0.5f * (pos + vec2(1, 1)) * parent->size();
    //dims_[1] = size;

    center_ = dims_[0] + 0.5f * dims_[1];
}

void ui::TypeMesh::Resize(const BaseElement *parent) {
    this->Move(rel_dims_[0], parent);
}

/*void ui::TypeMesh::Resize(const glm::vec2 &pos, const glm::vec2 &size, const BaseElement *parent) {
    this->Move(pos, parent);
}*/

void ui::TypeMesh::Draw(Renderer *r) {
    const auto &cur = r->GetParams();

    r->EmplaceParams(cur.col(), cur.z_val(), (eBlendMode)font_->blend_mode(), cur.scissor_test());
    r->DrawUIElement(font_->tex(), ui::PRIM_TRIANGLE, pos_, uvs_, indices_);
    r->PopParams();
}