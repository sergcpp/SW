#include "LinearLayout.h"

void ui::LinearLayout::Resize(const BaseElement *parent) {
	BaseElement::Resize(parent);

	glm::vec2 _start = { -1, -1 };
	glm::vec2 _size = { 2, 2 };

	float spacing;
	float filled_space = 0.0f;
	float l;
	if (vertical_) {
		spacing = 8.0f / parent->size_px().y;
		l = _size.y - spacing * (elements_.size() + 1);
		for (BaseElement *el : elements_) {
			if (el->resizable()) {
				filled_space += el->rel_size().y;
			} else {
				l -= el->rel_size().y;
			}
		}
	} else {
		spacing = 8.0f / parent->size_px().x;
		l = _size.x - spacing * (elements_.size() + 1);
		for (BaseElement *el : elements_) {
			if (el->resizable()) {
				filled_space += el->rel_size().x;
			} else {
				l -= el->rel_size().x;
			}
		}
	}

	float mult = 1;
	if (filled_space > 0) {
		mult = l / filled_space;
	}
	float pad = 0;

	if (vertical_) {
		pad = _start.y + _size.y - spacing;

		for (BaseElement *el : elements_) {
			el->Resize({ _start.x, 1 }, { _size.x, el->rel_size().y * mult }, this);
			pad -= (el->rel_size().y + spacing);
			el->Resize({ _start.x, pad }, { _size.x, el->rel_size().y }, this);
		}
	} else {
		pad = _start.x + spacing;
		for (BaseElement *el : elements_) {
			el->Resize({ pad, _start.y }, { el->rel_size().x * mult, _size.y }, this);
			pad += el->rel_size().x + spacing;
		}
	}
}

void ui::LinearLayout::Resize(const glm::vec2 &start, const glm::vec2 &size, const BaseElement *parent) {
	BaseElement::Resize(start, size, parent);
}

bool ui::LinearLayout::Check(const glm::ivec2 &p) const {
    for (BaseElement *el : elements_) {
        if (el->Check(p)) {
            return true;
        }
    }
    return false;
}

bool ui::LinearLayout::Check(const glm::vec2 &p) const {
    for (BaseElement *el : elements_) {
        if (el->Check(p)) {
            return true;
        }
    }
    return false;
}

void ui::LinearLayout::Focus(const glm::ivec2 &p) {
    for (BaseElement *el : elements_) {
        el->Focus(p);
    }
}
void ui::LinearLayout::Focus(const glm::vec2 &p) {
    for (BaseElement *el : elements_) {
        el->Focus(p);
    }
}

void ui::LinearLayout::Press(const glm::ivec2 &p, bool push) {
    for (BaseElement *el : elements_) {
        el->Press(p, push);
    }
}

void ui::LinearLayout::Press(const glm::vec2 &p, bool push) {
    for (BaseElement *el : elements_) {
        el->Press(p, push);
    }
}

void ui::LinearLayout::Draw(Renderer *r) {
    for (BaseElement *el : elements_) {
        el->Draw(r);
    }
}