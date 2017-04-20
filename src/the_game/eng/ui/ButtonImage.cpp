#include "ButtonImage.h"

ui::ButtonImage::ButtonImage(const char *tex_normal, const glm::vec2 uvs_normal[2],
                             const char *tex_focused, const glm::vec2 uvs_focused[2],
                             const char *tex_pressed, const glm::vec2 uvs_pressed[2],
                             const glm::vec2 &pos, const glm::vec2 &size, const BaseElement *parent)
        : ButtonBase(pos, size, parent),
          image_normal_{tex_normal, uvs_normal, {-1, -1}, {2, 2}, this},
          image_focused_{tex_focused, uvs_focused, {-1, -1}, {2, 2}, this},
          image_pressed_{tex_pressed, uvs_pressed, {-1, -1}, {2, 2}, this} {
}

void ui::ButtonImage::Resize(const BaseElement *parent) {
    BaseElement::Resize(parent);

	image_normal_.Resize(this);
	image_focused_.Resize(this);
	image_pressed_.Resize(this);

    if (additional_element_) {
        additional_element_->Resize(this);
    }
}

/*void ui::ButtonImage::Resize(const glm::vec2 &pos, const glm::vec2 &size, const BaseElement *parent) {
    BaseElement::Resize(pos, size, parent);

	image_normal_.Resize(pos, size, this);
	image_focused_.Resize(pos, size, this);
	image_pressed_.Resize(pos, size, this);

    if (additional_element_) {
        additional_element_->Resize(pos, size, this);
    }
}*/

void ui::ButtonImage::Draw(Renderer *r) {
	if (state_ == ST_NORMAL) {
		image_normal_.Draw(r);
	} else if (state_ == ST_FOCUSED) {
		image_focused_.Draw(r);
	} else if (state_ == ST_PRESSED) {
		image_pressed_.Draw(r);
	}

    if (additional_element_) {
        additional_element_->Draw(r);
    }
}