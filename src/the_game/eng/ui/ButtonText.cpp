#include "ButtonText.h"

#include "BitmapFont.h"
#include "Renderer.h"

ui::ButtonText::ButtonText(const std::string &text, BitmapFont *font, const glm::vec2 &pos, const BaseElement *parent)
        : ButtonBase(pos, {0, 0}, parent),
          type_mesh_(text, font, pos, parent) {}

bool ui::ButtonText::Check(const glm::vec2 &p) const {
	return type_mesh_.Check(p);
}

bool ui::ButtonText::Check(const glm::ivec2 &p) const {
	return type_mesh_.Check(p);
}

void ui::ButtonText::Move(const glm::vec2 &pos, const BaseElement *parent) {
	type_mesh_.Move(pos, parent);
}

void ui::ButtonText::Draw(Renderer *r) {
    using namespace glm;

	const auto &cur = r->GetParams();

    if (state_ == ST_NORMAL) {
        r->EmplaceParams(vec3(0.9f, 0.9f, 0.9f), cur.z_val(), cur.blend_mode(), cur.scissor_test());
    } else if (state_ == ST_FOCUSED) {
		r->EmplaceParams(vec3(1, 1, 1), cur.z_val(), cur.blend_mode(), cur.scissor_test());
    } else { // state_ == ST_PRESSED
		r->EmplaceParams(vec3(0.5f, 0.5f, 0.5f), cur.z_val(), cur.blend_mode(), cur.scissor_test());
    }
    type_mesh_.Draw(r);

    r->PopParams();
}

