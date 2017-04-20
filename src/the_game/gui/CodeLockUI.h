#pragma once

#include <vector>

#include "../eng/TimedInput.h"
#include "../eng/sys/Signal_.h"
#include "../eng/ui/Image.h"

namespace ui {
	class BitmapFont;
}

class CodeLockUI : public ui::BaseElement {
	std::shared_ptr<ui::BitmapFont> font_;

	std::string unlock_seq_, entered_seq_;

	ui::Image image_;
	std::vector<glm::vec2> buttons_;
public:
	CodeLockUI(const std::shared_ptr<ui::BitmapFont> &font,
			   const std::string &unlock_seq, const glm::vec2 &pos, const glm::vec2 &size, const ui::BaseElement *parent);

	void Reset() { entered_seq_.clear(); }

	void Draw(ui::Renderer *r) override;

	void Resize(const ui::BaseElement *parent) override;
	void Resize(const glm::vec2 &pos, const glm::vec2 &size, const BaseElement *parent) override;

	void HandleInput(InputManager::Event evt, const ui::BaseElement *root);

	sys::Signal<void()> guessed_signal, cancel_signal;
};