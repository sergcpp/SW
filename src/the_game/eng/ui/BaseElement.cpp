#include "BaseElement.h"

namespace ui {
    namespace BaseElementConstants {
        const unsigned default_flags = (1 << Visible) | (1 << Resizable);
    }
}

ui::BaseElement::BaseElement(const glm::vec2 &pos, const glm::vec2 &size, const BaseElement *parent)
        : flags_(BaseElementConstants::default_flags) {
    if (parent) {
        Resize(pos, size, parent);
    } else {
        rel_dims_[0] = pos;
        rel_dims_[1] = size;
    }
}

void ui::BaseElement::Resize(const BaseElement *parent) {
	//Resize(dims_[0], dims_[1], parent);

    using namespace glm;

    dims_[0] = parent->pos() + 0.5f * (rel_dims_[0] + vec2(1, 1)) * parent->size();
    dims_[1] = 0.5f * rel_dims_[1] * parent->size();

    dims_px_[0] = (ivec2) ((vec2)parent->pos_px() + 0.5f * (rel_dims_[0] + vec2(1, 1)) * (vec2)parent->size_px());
    dims_px_[1] = (ivec2) (rel_dims_[1] * (vec2) parent->size_px() * 0.5f);
}

void ui::BaseElement::Resize(const glm::vec2 &pos, const glm::vec2 &size, const BaseElement *parent) {
    rel_dims_[0] = pos;
    rel_dims_[1] = size;

    Resize(parent);

    /*using namespace glm;

	dims_px_[0] = (ivec2) ((vec2)parent->pos_px() + 0.5f * (pos + vec2(1, 1)) * (vec2)parent->size_px());
    dims_px_[1] = (ivec2) (size * (vec2) parent->size_px() * 0.5f);

    dims_[0] = parent->pos() + 0.5f * (pos + vec2(1, 1)) * parent->size();
    dims_[1] = 0.5f * size * parent->size();*/
}

bool ui::BaseElement::Check(const glm::ivec2 &p) const {
	return (p.x >= dims_px_[0].x && 
			p.y >= dims_px_[0].y &&
			p.x <= dims_px_[0].x + dims_px_[1].x &&
			p.y <= dims_px_[0].y + dims_px_[1].y);
}

bool ui::BaseElement::Check(const glm::vec2 &p) const {
	return (p.x >= dims_[0].x && 
			p.y >= dims_[0].y &&
			p.x <= dims_[0].x + dims_[1].x &&
			p.y <= dims_[0].y + dims_[1].y);
}
