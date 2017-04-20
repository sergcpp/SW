#pragma once

#include <glm/vec3.hpp>

#include "../eng/TimedInput.h"
#include "../eng/sys/Signal_.h"

class GameObject;

class PlayerController {
	float angle_ = 0,
		  angular_vel_ = 0,
		  forward_vel_ = 0,
          anim_transition_acc_ = 0;
	bool running_ = false,
		 flashlight_on_ = false;
	int anim_before_ = -1;

	enum eState { Idle, Walking, Running } state_;

	glm::vec3 last_pos_; // needed to calc real speed after collision response
public:
	PlayerController();

	float forward_vel() const { return forward_vel_; }
    bool flashlight_on() const { return flashlight_on_; }

	eState state() const { return state_; }

	void Update(GameObject &player, int dt_ms);

    void UpdateAnim(GameObject &player, float dt_s);

	bool HandleInput(InputManager::Event evt);

	void Reset();

	sys::Signal<void()> use_signal;
};
