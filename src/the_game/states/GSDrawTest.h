#pragma once

#include "../eng/GameState.h"
#include "../eng/Go.h"
#include "../eng/ren/Camera.h"
#include "../eng/ren/Program.h"
#include "../eng/ren/Texture.h"

class DrawTestUI;
class GameBase;
class GameStateManager;
class GCursor;
class FontStorage;
class Renderer;

namespace ui {
	class BaseElement;
	class Renderer;
}

class GSDrawTest : public GameState {
	GameBase *game_;
	std::weak_ptr<GameStateManager> state_manager_;
	std::shared_ptr<GCursor> ui_cursor_;
	std::shared_ptr<FontStorage> ui_fonts_;
	std::shared_ptr<ui::BaseElement> ui_root_;
	std::shared_ptr<ui::Renderer> ui_renderer_;
	
	std::shared_ptr<Renderer> renderer_;

	std::unique_ptr<DrawTestUI> ui_draw_test_;

	R::ProgramRef vtx_color_prog_, constant_prog_;
	R::Texture2DRef checker_tex_;
	Camera cam_;
	GameObject head_, biped_;

    bool grabbed_;
	int scene_;
	float angle_;

	void Init();
	void DrawPrimitives();
	void DrawPlanes();
	void DrawHead();
	void DrawSkeletal(float dt_s);
public:
	GSDrawTest(GameBase *game);
	~GSDrawTest();

	void Enter() override;
	void Exit() override;

	void Draw(float dt_s) override;

	void Update(int dt_ms) override;

	void HandleInput(InputManager::Event) override;
};