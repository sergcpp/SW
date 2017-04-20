#pragma once

#include "../eng/GameState.h"

class FontStorage;
class GameBase;
class GameStateManager;
class GCursor;

namespace ui {
	class BaseElement;
	class Renderer;
}

class LoadingUI;

class GSLoading : public GameState {
	GameBase *game_;
	std::weak_ptr<GameStateManager> state_manager_;
	std::shared_ptr<ui::BaseElement> ui_root_;
	std::shared_ptr<ui::Renderer> ui_renderer_;

	std::unique_ptr<LoadingUI> ui_loading_;

	unsigned loading_timer_ = 0;
    bool started_ = false;
public:
	GSLoading(GameBase *game);
	~GSLoading();

	void Enter() override;
	void Exit() override;

	void Draw(float dt_s) override;

	void Update(int dt_ms) override;

	void HandleInput(InputManager::Event evt) override;
};