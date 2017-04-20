#include "GSTransition.h"

#include <fstream>

#include "../eng/GameStateManager.h"
#include "../eng/ren/RenderState.h"
#include "../eng/sys/Json.h"
#include "../eng/sys/Log.h"

#include "../Game.h"
#include "../comp/Drawable.h"
#include "../comp/Transform.h"
#include "../managers/Renderer.h"

namespace GSTransitionInternal {
    const float CAM_CENTER[] = { 0, 1.5f, 5 },
                CAM_TARGET[] = { 0, 1.5f, 0 },
                CAM_UP[] = { 0, 1, 0 };
}


GSTransition::GSTransition(GameBase *game) : game_(game), door_("Door"),
                                             cam_(GSTransitionInternal::CAM_CENTER,
                                                  GSTransitionInternal::CAM_TARGET,
                                                  GSTransitionInternal::CAM_UP) {
	state_manager_ = game->GetComponent<GameStateManager>(STATE_MANAGER_KEY);
	renderer_	   = game->GetComponent<Renderer>(RENDERER_KEY);
    lev_info_      = game->GetComponent<LevelInfo>(CURRENT_LEVEL_KEY);

    cam_.Perspective(45, float(game_->width) / game_->height, 0.1f, 100.0f);

    door_.AddComponent(new Transform);

    JsObject js;

    {   std::ifstream in_file("game_assets/models/" + lev_info_->door_name, std::ios::binary);
        if (!js.Read(in_file)) {
            throw std::runtime_error("Cannot parse json file!");
        }
    }

    std::unique_ptr<Drawable> dr(new Drawable);
    if (dr->Read(js)) {
        dr->set_cur_anim(0);
        door_.AddComponent(dr.release());
    } else {
        throw std::runtime_error("Cannot parse file!");
    }
}

GSTransition::~GSTransition() {

}

void GSTransition::Enter() {
	timer_ = 0;

    R::current_cam = &cam_;

	auto dr = door_.GetComponent<Drawable>();
	if (dr) {
		dr->anim_time = 0;
	}
}

void GSTransition::Exit() {

}

void GSTransition::Draw(float dt_s) {
	R::ClearColorAndDepth();

    auto tr = door_.GetComponent<Transform>();
    auto dr = door_.GetComponent<Drawable>();
    if (tr && dr) {
        tr->SetPos(0, 0, float(timer_) / 400);
        tr->SetAngles(0, -90.0f + (lev_info_->revert ? 180.0f : 0), 0);
        dr->UpdateAnim(dt_s);
		renderer_->DrawSkeletal({{ 1, 1, 1 }}, tr, dr);
    }
}

void GSTransition::Update(int dt_ms) {
	timer_ += dt_ms;
	if (timer_ > 2000) {
		state_manager_.lock()->Pop();
	}
}
