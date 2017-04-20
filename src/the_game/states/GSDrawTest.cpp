#include "GSDrawTest.h"

#include <fstream>

#include <glm/gtc/type_ptr.hpp>

#include "../eng/GameStateManager.h"
#include "../eng/ren/RenderState.h"
#include "../eng/sys/Json.h"
#include "../eng/sys/Log.h"
#include "../eng/ui/Renderer.h"

#include "../Game.h"
#include "../comp/Drawable.h"
#include "../comp/Transform.h"
#include "../gui/DrawTestUI.h"
#include "../gui/FontStorage.h"
#include "../gui/GCursor.h"
#include "../managers/Renderer.h"

namespace GSDrawTestInternal {
	const float CAM_CENTER[] = { 0, 0.75f, 10 },
				CAM_TARGET[] = { 0, 0.75f, 0 },
				CAM_UP[] = { 0, 1, 0 };
	const char HEAD_OBJ_NAME[] = "game_assets/models/draw_test_01.json",
               BIPED_OBJ_NAME[] = "game_assets/models/draw_test_02.json";
}

GSDrawTest::GSDrawTest(GameBase *game) : game_(game), head_("Head"), biped_("Biped"),
                                         cam_(GSDrawTestInternal::CAM_CENTER,
                                              GSDrawTestInternal::CAM_TARGET,
                                              GSDrawTestInternal::CAM_UP) {
	state_manager_	= game->GetComponent<GameStateManager>(STATE_MANAGER_KEY);

	ui_root_		= game->GetComponent<ui::BaseElement>(UI_ROOT_KEY);
	ui_cursor_		= game->GetComponent<GCursor>(UI_CURSOR_KEY);
	ui_fonts_		= game->GetComponent<FontStorage>(UI_FONTS_KEY);
	ui_renderer_	= game->GetComponent<ui::Renderer>(UI_RENDERER_KEY);

	renderer_		= game->GetComponent<Renderer>(RENDERER_KEY);

	Init();

	cam_.Perspective(8, float(game_->width) / game_->height, 0.1f, 1000.0f);

	{   std::ifstream in_file(GSDrawTestInternal::HEAD_OBJ_NAME, std::ios::binary);
		JsObject js;
		if (!js.Read(in_file)) throw std::runtime_error("Failed to parse json file!");

		std::unique_ptr<Drawable> dr(new Drawable);
		if (dr->Read(js)) {
			head_.AddComponent(dr.release());
		} else {
			throw std::runtime_error("Failed to create drawable!");
		}

		head_.AddComponent(new Transform);
	}

    {   std::ifstream in_file(GSDrawTestInternal::BIPED_OBJ_NAME, std::ios::binary);
        JsObject js;
		if (!js.Read(in_file)) throw std::runtime_error("Failed to parse json file!");

        std::unique_ptr<Drawable> dr(new Drawable);
        if (dr->Read(js)) {
            biped_.AddComponent(dr.release());
        } else {
            throw std::runtime_error("Failed to create drawable!");
        }

        biped_.AddComponent(new Transform);
    }
}

GSDrawTest::~GSDrawTest() {

}

void GSDrawTest::Enter() {
	auto main_font = ui_fonts_->FindFont("main_font");
	if (!ui_draw_test_) {
		ui_draw_test_.reset(new DrawTestUI(main_font, ui_root_.get()));
	}

	scene_   = 1;
	grabbed_ = false;
	angle_   = 90;
}

void GSDrawTest::Exit() {

}

void GSDrawTest::Draw(float dt_s) {
	R::ClearColorAndDepth();

	R::current_cam = &cam_;

	if (scene_ == 1) {
		DrawPrimitives();
	} else if (scene_ == 2) {
		DrawPlanes();
	} else if (scene_ == 3) {
		DrawHead();
	} else if (scene_ == 4) {
        DrawSkeletal(dt_s);
    }

	ui_renderer_->BeginDraw();

	ui_draw_test_->set_state(scene_);
	ui_draw_test_->Draw(ui_renderer_.get());

	ui_cursor_->Draw(ui_renderer_.get());

	ui_renderer_->EndDraw();
}

void GSDrawTest::Update(int dt_ms) {
	
}

void GSDrawTest::HandleInput(InputManager::Event evt) {
	ui_cursor_->HandleInput(evt, ui_root_.get());

	switch (evt.type) {
		case InputManager::RAW_INPUT_P1_DOWN:
			grabbed_ = true;
			break;
		case InputManager::RAW_INPUT_P1_UP:
			grabbed_ = false;
			break;
		case InputManager::RAW_INPUT_P1_MOVE:
			if (grabbed_) {
				angle_ += evt.move.dx;
			}
			break;
		case InputManager::RAW_INPUT_KEY_UP: {
			if (evt.key == InputManager::RAW_INPUT_BUTTON_OTHER) {
				if (evt.raw_key >= '1' && evt.raw_key <= '9') {
					scene_ = evt.raw_key - '0';
				} else if (evt.raw_key == ' ') {
                    if (scene_ == 4) {
                        auto dr = biped_.GetComponent<Drawable>();
                        if (dr) {
                            dr->set_cur_anim(dr->cur_anim() == -1 ? 0 : -1);
                        }
                    }
                }
			}
		} break;
		case InputManager::RAW_INPUT_RESIZE:
			cam_.Perspective(8, float(game_->width) / game_->height, 0.1f, 100.0f);
			break;
	}
}

void GSDrawTest::DrawHead() {
	angle_ = glm::clamp(angle_, 45.0f, 135.0f);

    auto tr = head_.GetComponent<Transform>();
    auto dr = head_.GetComponent<Drawable>();
	if (tr && dr) {
		tr->SetAngles(0, angle_, 0);
		renderer_->DrawMatcap(tr, dr);
	}
}

void GSDrawTest::DrawSkeletal(float dt_s) {
    auto tr = biped_.GetComponent<Transform>();
    auto dr = biped_.GetComponent<Drawable>();
    if (tr && dr) {
        tr->SetPos(0, -0.2f, -6);
        tr->SetAngles(0, angle_, 0);

		dr->UpdateAnim(dt_s);
        renderer_->DrawSkeletalMatcap(tr, dr);

        auto mesh_ref = dr->mesh();
        auto *mesh = R::GetMesh(mesh_ref);

        auto *skel = &mesh->skel;
        skel->UpdateBones();
        for (int i = 0 ; i < skel->bones.size(); i++) {
            if (skel->bones[i].parent_id == -1) continue;

            glm::vec3 p1 = skel->bone_pos(skel->bones[i].parent_id),
                      p2 = skel->bone_pos(i);

            glm::mat4 m;
            skel->bone_matrix(i, m);

            glm::mat4 m_mat, mv_mat, mvp_mat;
            m_mat = tr->matrix();

            p1 = glm::vec3(m_mat * glm::vec4(p1, 1));
            p2 = glm::vec3(m_mat * glm::vec4(p2, 1));

            renderer_->DrawLine(p1, p2, false);
        }
    }
}