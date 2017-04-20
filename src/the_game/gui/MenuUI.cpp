#include "MenuUI.h"

#include <glm/common.hpp>

#include "../eng/ui/BitmapFont.h"
#include "../eng/ui/Utils.h"

#include "GCursor.h"

namespace MenuUIConstants {
    const char frame_filename[] = "game_assets/textures/ui/frame_01.tga";
    const char btn_image_filename[] = "game_assets/textures/ui/btn_02.tga";
    const glm::vec2 uvs1[2] = { { 0.0f, 0.72f }, { 1.0f, 1.0f } },
                    uvs2[2] = { { 0.0f, 0.36f }, { 1.0f, 0.64f } },
                    uvs3[2] = { { 0.0f, 0.0f }, { 1.0f, 0.28f } };

    const glm::vec2 title_pos = { -0.5f, 0.45f };
	const glm::vec2 start_pos = { 0, -0.35f };
	const glm::vec2 newgame_pos = { -0.75f, 0 };
	const glm::vec2 exit_pos = { -0.75f, -0.15f };
}

MenuUI::MenuUI(const std::shared_ptr<ui::BitmapFont> &font,
               const std::shared_ptr<GCursor> cursor, const ui::BaseElement *root)
        : BaseElement({ -1, -1 }, { 2, 2 }, root),
		state_(Intro),
        font_(font),
		start_text_("< start >", font_.get(), MenuUIConstants::start_pos, this),
		newgame_text_("New Game", font_.get(), MenuUIConstants::newgame_pos, this),
		exit_text_("Exit", font_.get(), MenuUIConstants::exit_pos, this) {

	start_text_.Centrate();

    Resize(root);
}

void MenuUI::Draw(ui::Renderer *r) {
	if (state_ == Intro) {
		start_text_.Draw(r);
	} else {
		newgame_text_.Draw(r);
		exit_text_.Draw(r);
	}
}

void MenuUI::Resize(const ui::BaseElement *parent) {
	using namespace MenuUIConstants;

    BaseElement::Resize(parent);

    if (parent->size_px().x > 768 && parent->size_px().y > 300) {
        font_->set_sharp(true);
        font_->set_scale(1);
    } else {
        font_->set_sharp(true);
        font_->set_scale(glm::min(float(parent->size_px().x) / 768, float(parent->size_px().y) / 300));
    }

    start_text_.Move(start_pos, this); start_text_.Centrate();

	newgame_text_.Move(MenuUIConstants::newgame_pos, this);
	exit_text_.Move(MenuUIConstants::exit_pos, this);
}

void MenuUI::HandleInput(InputManager::Event evt, const ui::BaseElement *root) {
	switch (evt.type) {
		case InputManager::RAW_INPUT_P1_DOWN:
		case InputManager::RAW_INPUT_P2_DOWN: {
			glm::vec2 p = ui::MapPointToScreen({ evt.point.x, evt.point.y }, root->size_px());
			if (state_ == Intro) {
				state_ = Main;
			} else if (state_ == Main) {
				newgame_text_.Press(p, true);
				exit_text_.Press(p, true);
			}
		} break;
		case InputManager::RAW_INPUT_P1_UP:
		case InputManager::RAW_INPUT_P2_UP: {
			glm::vec2 p = ui::MapPointToScreen({ evt.point.x, evt.point.y }, root->size_px());
			if (state_ == Main) {
				newgame_text_.Press(p, false);
				exit_text_.Press(p, false);
			}
		} break;
		case InputManager::RAW_INPUT_P1_MOVE:
		case InputManager::RAW_INPUT_P2_MOVE: {
			glm::vec2 p = ui::MapPointToScreen({ evt.point.x, evt.point.y }, root->size_px());
			if (state_ == Main) {
				newgame_text_.Focus(p);
				exit_text_.Focus(p);
			}
		} break;
		case InputManager::RAW_INPUT_KEY_DOWN:
			break;
		case InputManager::RAW_INPUT_KEY_UP: {
		} break;
	}
}

void MenuUI::Update(int dt_ms) {
    
}
