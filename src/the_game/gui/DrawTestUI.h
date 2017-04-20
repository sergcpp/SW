#pragma once

#include "../eng/TimedInput.h"
#include "../eng/ui/BaseElement.h"

namespace ui {
	class BitmapFont;
}

class GCursor;

class DrawTestUI : public ui::BaseElement {
	std::shared_ptr<ui::BitmapFont> font_;
	int state_;
public:
	DrawTestUI(const std::shared_ptr<ui::BitmapFont> &font, const ui::BaseElement *root);

	void set_state(int s) { state_ = s; }

	void Draw(ui::Renderer *r) override;

	void Resize(const ui::BaseElement *parent) override;

	void Update(int dt_ms);
};
