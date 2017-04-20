#include "GSLoading.h"

#include "../eng/GameStateManager.h"
#include "../eng/ren/RenderState.h"

#include "../Game.h"
#include "../gui/FontStorage.h"
#include "../gui/GBackground.h"
#include "../gui/LoadingUI.h"
#include "../managers/Renderer.h"
#include "../managers/SceneManager.h"
#include "../states/GSCreate.h"

GSLoading::GSLoading(GameBase *game) : game_(game) {
	state_manager_	= game->GetComponent<GameStateManager>(STATE_MANAGER_KEY);
	ui_root_		= game->GetComponent<ui::BaseElement>(UI_ROOT_KEY);
	ui_renderer_	= game->GetComponent<ui::Renderer>(UI_RENDERER_KEY);
}

GSLoading::~GSLoading() {

}

void GSLoading::Enter() {
	if (!ui_loading_) {
		ui_loading_.reset(new LoadingUI(ui_root_.get()));
	}

	auto scene_manager = game_->GetComponent<SceneManager>(SCENE_MANAGER_KEY);
    if (!scene_manager) {
        scene_manager = std::make_shared<SceneManager>();
        game_->SetComponent(SCENE_MANAGER_KEY, scene_manager);
    }

	auto cur_lev = game_->GetComponent<LevelInfo>(CURRENT_LEVEL_KEY);
	if (!cur_lev) {
		cur_lev = std::make_shared<LevelInfo>();
		game_->SetComponent(CURRENT_LEVEL_KEY, cur_lev);
	}

	cur_lev->name = "room_01.json";
}

void GSLoading::Exit() {

}

void GSLoading::Draw(float dt_s) {
	R::ClearColorAndDepth(0, 0, 0, 1);

	ui_loading_->Draw(ui_renderer_.get());
}

void GSLoading::Update(int dt_ms) {
    loading_timer_ += dt_ms;
    if (loading_timer_ > 2000 && !started_) {
        auto gs_running = GSCreate(GS_RUNNING, game_);
        state_manager_.lock()->Switch(gs_running);
        started_ = true;
    }
	ui_loading_->Update(dt_ms);
}

void GSLoading::HandleInput(InputManager::Event evt) {
	switch (evt.type) {
		case InputManager::RAW_INPUT_RESIZE:
            ui_loading_->Resize(ui_root_.get());
			break;
        default:
            break;
	}
}