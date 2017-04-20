#include "GSMenu.h"

#include "../eng/GameStateManager.h"
#include "../eng/ren/RenderState.h"
#include "../eng/sys/Json.h"
#include "../eng/sys/Log.h"
#include "../eng/ui/BitmapFont.h"
#include "../eng/ui/LinearLayout.h"
#include "../eng/ui/Renderer.h"
#include "../eng/ui/Utils.h"

#include "../Game.h"
#include "../gui/FontStorage.h"
#include "../gui/GBackground.h"
#include "../gui/GCursor.h"
#include "../gui/MenuUI.h"

#include "GSCreate.h"

namespace GSMenuConstants {
    
}

GSMenu::GSMenu(GameBase *game) : game_(game) {
    state_manager_ = game->GetComponent<GameStateManager>(STATE_MANAGER_KEY);
    ui_root_       = game->GetComponent<ui::BaseElement>(UI_ROOT_KEY);
    ui_background_ = game->GetComponent<GBackground>(UI_BACKGROUND_KEY);
    ui_cursor_     = game->GetComponent<GCursor>(UI_CURSOR_KEY);
    ui_fonts_      = game->GetComponent<FontStorage>(UI_FONTS_KEY);
    ui_renderer_   = game->GetComponent<ui::Renderer>(UI_RENDERER_KEY);
}

GSMenu::~GSMenu() {

}

void GSMenu::Enter() {
	auto main_font = ui_fonts_->FindFont("main_font");
	if (!ui_menu_) {
		ui_menu_.reset(new MenuUI(main_font, ui_cursor_, ui_root_.get()));
		ui_menu_->new_game_signal().Connect<GSMenu, &GSMenu::OnStart>(this);
		ui_menu_->exit_signal().Connect<GSMenu, &GSMenu::OnExit>(this);
	}
}

void GSMenu::Exit() {
    
}

void GSMenu::Draw(float dt_s) {
    R::ClearColorAndDepth(0, 0, 0, 1);
    
    ui_renderer_->BeginDraw();
    ui_background_->Draw(ui_renderer_.get());

	ui_menu_->Draw(ui_renderer_.get());
    
    ui_cursor_->Draw(ui_renderer_.get());
    ui_renderer_->EndDraw();
}

void GSMenu::Update(int dt_ms) {
    ui_background_->Update(dt_ms);
    ui_menu_->Update(dt_ms);
}

void GSMenu::HandleInput(InputManager::Event evt) {
    ui_cursor_->HandleInput(evt, ui_root_.get());
	ui_menu_->HandleInput(evt, ui_root_.get());

    switch (evt.type) {
		case InputManager::RAW_INPUT_P1_DOWN:
		case InputManager::RAW_INPUT_P2_DOWN: {

		} break;
		case InputManager::RAW_INPUT_P1_UP:
		case InputManager::RAW_INPUT_P2_UP: {

		} break;
		case InputManager::RAW_INPUT_P1_MOVE:
		case InputManager::RAW_INPUT_P2_MOVE: {

		} break;
		case InputManager::RAW_INPUT_KEY_DOWN:
			if (evt.key == InputManager::RAW_INPUT_BUTTON_RETURN) {
				//state_manager_.lock()->Pop();
			}
			break;
		case InputManager::RAW_INPUT_RESIZE:
			ui_menu_->Resize(ui_root_.get());
			break;
		default:
			break;
    }
}

void GSMenu::OnStart() {
	if (auto state_manager = state_manager_.lock()) {
		state_manager->Push(GSCreate(GS_LOADING, game_));
	}
}

void GSMenu::OnExit() {
	game_->Quit();
}