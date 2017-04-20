#include "GSRunning.h"

#include <fstream>

#include "../eng/GameStateManager.h"
#include "../eng/ren/RenderState.h"
#include "../eng/sys/Json.h"
#include "../eng/sys/Log.h"

#include "GSCreate.h"
#include "../Game.h"
#include "../gui/CodeLockUI.h"
#include "../gui/FontStorage.h"
#include "../managers/SceneManager.h"

GSRunning::GSRunning(GameBase *game) : game_(game) {
    state_manager_	= game->GetComponent<GameStateManager>(STATE_MANAGER_KEY);
    ui_root_		= game->GetComponent<ui::BaseElement>(UI_ROOT_KEY);
	ui_cursor_		= game->GetComponent<GCursor>(UI_CURSOR_KEY);
    ui_fonts_		= game->GetComponent<FontStorage>(UI_FONTS_KEY);
    ui_renderer_	= game->GetComponent<ui::Renderer>(UI_RENDERER_KEY);

    scene_manager_  = game->GetComponent<SceneManager>(SCENE_MANAGER_KEY);
    renderer_       = game->GetComponent<Renderer>(RENDERER_KEY);

    scene_manager_->change_map_signal.Connect<GSRunning, &GSRunning::OnChangeMap>(this);
	scene_manager_->show_codelock_signal.Connect<GSRunning, &GSRunning::OnShowCodelock>(this);
}

GSRunning::~GSRunning() {
    scene_manager_->change_map_signal.clear();
}

void GSRunning::Enter() {
	std::string code;

	{	// create scene
		JsObject js_scene;

		auto cur_lev = game_->GetComponent<LevelInfo>(CURRENT_LEVEL_KEY);

		std::ifstream in_file("game_assets/maps/" + cur_lev->name, std::ios::binary);
		if (!js_scene.Read(in_file)) {
			LOGE("Cannot parse json file!");
		}

		const JsObject &js_lev = (const JsObject &)js_scene.at("level");
		if (js_lev.Has("codelock")) {
			const JsObject &js_codelock = (const JsObject &)js_lev.at("codelock");
			code = ((const JsString &)js_codelock.at("code")).val;
		}

		if (!scene_manager_->Read(js_scene)) {
			LOGE("Cannot load scene!");
		}

		scene_manager_->Resize(game_->width, game_->height);
	}

	{	// create ui
		auto main_font = ui_fonts_->FindFont("main_font");
		float k = float(ui_root_->size_px().x) / ui_root_->size_px().y;
		codelock_ui_.reset(new CodeLockUI(main_font, code, { -0.375f, -0.5f * k }, { 0.75f, 1.0f * k }, ui_root_.get()));
		codelock_ui_->guessed_signal.Connect<GSRunning, &GSRunning::OnCodeGuessed>(this);
		codelock_ui_->cancel_signal.Connect<GSRunning, &GSRunning::OnCodeCanceled>(this);
	}
}

void GSRunning::Exit() {
	
}

void GSRunning::Draw(float dt_s) {
	R::ClearDepth();

    scene_manager_->Draw(renderer_.get(), dt_s);

	if (show_codelock_) {
		ui_renderer_->BeginDraw();
		codelock_ui_->Draw(ui_renderer_.get());
		ui_cursor_->Draw(ui_renderer_.get());
		ui_renderer_->EndDraw();
	}
}

void GSRunning::Update(int dt_ms) {
    scene_manager_->Update(dt_ms);
}

void GSRunning::HandleInput(InputManager::Event evt) {
	if (show_codelock_) {
		codelock_ui_->HandleInput(evt, ui_root_.get());
		ui_cursor_->HandleInput(evt, ui_root_.get());
	}

	if (scene_manager_->HandleInput(evt)) {
		return;
	}

    switch (evt.type) {
		case InputManager::RAW_INPUT_RESIZE: {
			scene_manager_->Resize((int)evt.point.x, (int)evt.point.y);
			//codelock_ui_->Resize(ui_root_.get());
			float k = float(ui_root_->size_px().x) / ui_root_->size_px().y;
			codelock_ui_->Resize({ -0.375f, -0.5f * k }, { 0.75f, 1.0f * k }, ui_root_.get());
		} break;
        default:
            break;
    }
}

void GSRunning::OnChangeMap(const std::string &map_name, const std::string &door_name, bool revert) {
	LOGI("Changing map to %s", map_name.c_str());
	auto cur_level = game_->GetComponent<LevelInfo>(CURRENT_LEVEL_KEY);
	cur_level->name      = map_name;
    cur_level->door_name = door_name;
    cur_level->revert    = revert;
	state_manager_.lock()->Push(GSCreate(GS_TRANSITION, game_));
}

void GSRunning::OnShowCodelock() {
	show_codelock_ = true;
}

void GSRunning::OnCodeGuessed() {
	LOGI("GUESSED!");

	auto cur_level = game_->GetComponent<LevelInfo>(CURRENT_LEVEL_KEY);
	cur_level->name		 = "";
	cur_level->door_name = "gates.json";
	cur_level->revert	 = true;
	state_manager_.lock()->Switch(GSCreate(GS_TRANSITION, game_));

	show_codelock_ = false;
}

void GSRunning::OnCodeCanceled() {
	codelock_ui_->Reset();
	show_codelock_ = false;
}