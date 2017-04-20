#include "Cursor.h"

#include "Renderer.h"

ui::Cursor::Cursor(const R::Texture2DRef &tex, const glm::vec2 uvs[2], const glm::vec2 &size, const BaseElement *parent)
		: BaseElement(glm::vec2(0, 0), size, parent),
          img_(tex, uvs, glm::vec2(-1, -1), glm::vec2(2, 2), this), clicked_(false) { }

ui::Cursor::Cursor(const char *tex_name, const glm::vec2 uvs[2], const glm::vec2 &size, const BaseElement *parent)
		: BaseElement(glm::vec2(0, 0), size, parent),
          img_(tex_name, uvs, glm::vec2(-1, -1), glm::vec2(2, 2), this), clicked_(false) { }

void ui::Cursor::SetPos(const glm::vec2 &pos, const BaseElement *parent) {
    using namespace glm;

    Resize(pos, dims_[1], parent);
    if (clicked_) {
        img_.Resize(vec2(-1, -1) + offset_ * 0.8f, vec2(2, 2) * 0.8f, this);
    } else {
        img_.Resize(vec2(-1, -1) + offset_, vec2(2, 2), this);
    }
}

void ui::Cursor::Draw(Renderer *r) {
	const auto &cur = r->GetParams();

    r->EmplaceParams(clicked_ ? glm::vec3(0.8f, 0.8f, 0.8f) : glm::vec3(1, 1, 1),
		cur.z_val(), cur.blend_mode(), cur.scissor_test());
    img_.Draw(r);
    r->PopParams();
}
