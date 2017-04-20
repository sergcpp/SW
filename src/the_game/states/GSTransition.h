#pragma once

#include "../eng/GameState.h"
#include "../eng/Go.h"
#include "../eng/ren/Camera.h"

class GameBase;
class GameStateManager;
struct LevelInfo;
class Renderer;

class GSTransition : public GameState {
	GameBase *game_;
	std::weak_ptr<GameStateManager> state_manager_;
	std::shared_ptr<Renderer> renderer_;
    std::shared_ptr<LevelInfo> lev_info_;

    Camera cam_;
    GameObject door_;

	unsigned timer_ = 0;
public:
	GSTransition(GameBase *game);
	~GSTransition();

	void Enter() override;
	void Exit() override;

	void Draw(float dt_s) override;

	void Update(int dt_ms) override;
};