#pragma once

#include "../eng/GameState.h"

class FontStorage;
class GameBase;
class GameStateManager;
class GBackground;
class GCursor;

namespace ui {
    class BaseElement;
    class Renderer;
}

class MenuUI;

class GSMenu : public GameState {
    GameBase *game_;
    std::weak_ptr<GameStateManager> state_manager_;
    std::shared_ptr<GBackground> ui_background_;
    std::shared_ptr<GCursor> ui_cursor_;
    std::shared_ptr<FontStorage> ui_fonts_;
    std::shared_ptr<ui::BaseElement> ui_root_;
    std::shared_ptr<ui::Renderer> ui_renderer_;

	std::unique_ptr<MenuUI> ui_menu_;

	void OnStart();
	void OnExit();
 public:
	GSMenu(GameBase *game);
	~GSMenu();

    void Enter() override;
    void Exit() override;

    void Draw(float dt_s) override;

    void Update(int dt_ms) override;

    void HandleInput(InputManager::Event evt) override;
};
