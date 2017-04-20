#include "PlayerController.h"

#include "../comp/Drawable.h"
#include "../comp/Transform.h"
#include "../eng/Go.h"
#include "../eng/sys/Log.h"

namespace PlayerControllerInternal {
	const char *state_names[] = { "Idle", "Walking", "Running" };
}

PlayerController::PlayerController() : state_(Idle) {

}

void PlayerController::Update(GameObject &player, int dt_ms) {
	float dt_s = 0.001f * dt_ms;

    auto tr = player.GetComponent<Transform>();
	if (tr) {
		{	// update state
			float l = glm::length(tr->pos() - last_pos_);
			eState new_state;
			if (l < 0.01f) {
				new_state = Idle;
			} else if (l < 0.04f) {
				new_state = Walking;
			} else {
				new_state = Running;
			}

			if (new_state != state_) {
				LOGI("%s", PlayerControllerInternal::state_names[new_state]);
				state_ = new_state;
			}

			last_pos_ = tr->pos();
		}

		{	// move
			glm::vec3 fwd = -tr->matrix()[0];

			glm::vec3 new_pos = tr->pos();
			float k = running_ ? 2 : 1;
			new_pos += k * fwd * forward_vel_ * dt_s;
			tr->SetPos(new_pos);

			angle_ += angular_vel_ * dt_s;
			tr->SetAngles(0, angle_, 0);
		}
	}

    auto dr = player.GetComponent<Drawable>();
	if (dr) {
        int anim = 0;
        if (state_ == Walking) anim = 1;
        else if (state_ == Running) anim = 2;

        if (dr->cur_anim() != anim) {
            anim_before_ = dr->cur_anim();
            anim_transition_acc_ = 0;
            dr->set_cur_anim(anim);
            dr->anim_time = 0;
        }
    }

	/*if (state_ == Walking) {

	}*/
}

void PlayerController::UpdateAnim(GameObject &player, float dt_s) {
    auto dr = player.GetComponent<Drawable>();
    if (!dr) return;

    if (anim_before_ != -1) {
        anim_transition_acc_ += dt_s;
        float t = anim_transition_acc_ * 4.0f;
        if (t < 1) {
            dr->UpdateAnim(anim_before_, dr->cur_anim(), t, dt_s);
        } else {
            anim_before_ = -1;
            dr->UpdateAnim(dt_s);
        }
    } else {
        dr->UpdateAnim(dt_s);
    }
}

bool PlayerController::HandleInput(InputManager::Event evt) {
	switch (evt.type) {
		case InputManager::RAW_INPUT_KEY_DOWN:
			if (evt.key == InputManager::RAW_INPUT_BUTTON_SHIFT) {
				running_ = true;
			} else if (evt.key == InputManager::RAW_INPUT_BUTTON_OTHER) {
				if (evt.raw_key == 'w') {
					forward_vel_ = 1.4f;
				} else if (evt.raw_key == 's') {
					forward_vel_ = -1.4f;
				} else if (evt.raw_key == 'a') {
					angular_vel_ = 100.0f;
				} else if (evt.raw_key == 'd') {
					angular_vel_ = -100.0f;
				}
			}
			break;
		case InputManager::RAW_INPUT_KEY_UP:
			if (evt.key == InputManager::RAW_INPUT_BUTTON_SHIFT) {
				running_ = false;
			} else if (evt.key == InputManager::RAW_INPUT_BUTTON_OTHER) {
				if (evt.raw_key == 'w' || evt.raw_key == 's') {
					forward_vel_ = 0;
				} else if (evt.raw_key == 'a' || evt.raw_key == 'd') {
					angular_vel_ = 0;
				} else if (evt.raw_key == 'f') {
					flashlight_on_ = !flashlight_on_;
				} else if (evt.raw_key == 'e') {
					use_signal.FireN();
				}
			}
			break;
		default:
			break;
	}
	return false;
}

void PlayerController::Reset() {
	forward_vel_ = 0;
	running_ = false;
}