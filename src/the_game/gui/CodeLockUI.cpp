#include "CodeLockUI.h"

#include <glm/geometric.hpp>

#include "../eng/sys/Log.h"
#include "../eng/ui/BitmapFont.h"
#include "../eng/ui/Utils.h"

namespace CodeLockUIInternal {
	const char TEX_NAME[] = "game_assets/textures/codelock.tga";
	const glm::vec2 TEX_UVS[] = { { 0, 0 }, { 1, 1 } };
}

CodeLockUI::CodeLockUI(const std::shared_ptr<ui::BitmapFont> &font,
					   const std::string &unlock_seq, const glm::vec2 &pos, const glm::vec2 &size, const ui::BaseElement *parent)
	: ui::BaseElement(pos, size, parent),
	font_(font),
	image_(CodeLockUIInternal::TEX_NAME, CodeLockUIInternal::TEX_UVS, { -1, -1 }, { 2, 2 }, this), 
	unlock_seq_(unlock_seq) {

	buttons_ = { { 0, -0.35f },
				 { -0.29f, 0.25f },
				 { 0, 0.25f },
				 { 0.29f, 0.25f },
				 { -0.29f, 0.05f },
				 { 0, 0.05f },
				 { 0.29f, 0.05f },
				 { -0.29f, -0.15f },
				 { 0, -0.15f },
				 { 0.29f, -0.15f } };
}

void CodeLockUI::Draw(ui::Renderer *r) {
	image_.Draw(r);

	float w = font_->GetWidth(entered_seq_.c_str(), this);
	font_->DrawText(r, entered_seq_.c_str(), { -w * 0.5f, 0.8f }, this);

	/*for (const auto &b : buttons_) {
		font_->DrawText(r, "+", b, this);
	}*/
}

void CodeLockUI::Resize(const ui::BaseElement *parent) {
	BaseElement::Resize(parent);

	image_.Resize(this);
}

void CodeLockUI::Resize(const glm::vec2 &pos, const glm::vec2 &size, const ui::BaseElement *parent) {
	BaseElement::Resize(pos, size, parent);

	image_.Resize({ -1, -1 }, { 2, 2 }, this);
}

void CodeLockUI::HandleInput(InputManager::Event evt, const ui::BaseElement *root) {
	switch (evt.type) {
		case InputManager::RAW_INPUT_P1_UP:
		case InputManager::RAW_INPUT_P2_UP: {
			glm::vec2 p1 = ui::MapPointToScreen({ evt.point.x, evt.point.y }, root->size_px());

			if (!Check(p1)) {
				cancel_signal.FireN();
				return;
			}

			for (size_t i = 0; i < buttons_.size(); i++) {
				glm::vec2 p2 = pos() + 0.5f * (buttons_[i] + glm::vec2(1, 1)) * size();
				if (glm::distance(p1, p2) < 0.1f * size().x) {
					entered_seq_.push_back('0' + i);

					if (entered_seq_.find(unlock_seq_) != std::string::npos) {
						LOGI("%s %s", entered_seq_.c_str(), unlock_seq_.c_str());
						guessed_signal.FireN();
					}
				}
			}
		} break;
	}
}