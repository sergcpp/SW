#pragma once

#include "../eng/GameState.h"

class CodeLockUI;
class FontStorage;
class GameBase;
class GameStateManager;
class GCursor;
class Renderer;
class SceneManager;

namespace ui {
    class BaseElement;
    class Renderer;
}

class GSRunning : public GameState {
    GameBase *game_;
    std::weak_ptr<GameStateManager> state_manager_;
	std::shared_ptr<GCursor> ui_cursor_;
    std::shared_ptr<FontStorage> ui_fonts_;
    std::shared_ptr<ui::BaseElement> ui_root_;
    std::shared_ptr<ui::Renderer> ui_renderer_;

    std::shared_ptr<SceneManager> scene_manager_;
    std::shared_ptr<Renderer> renderer_;

	bool show_codelock_ = false;
	std::unique_ptr<CodeLockUI> codelock_ui_;

	void OnChangeMap(const std::string &map_name, const std::string &door_name, bool revert);
	void OnShowCodelock();
	void OnCodeGuessed();
	void OnCodeCanceled();
public:
    GSRunning(GameBase *game);
    ~GSRunning();

    void Enter() override;
    void Exit() override;

    void Draw(float dt_s) override;

    void Update(int dt_ms) override;

    void HandleInput(InputManager::Event evt) override;
};