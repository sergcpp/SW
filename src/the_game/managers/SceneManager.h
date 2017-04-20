#pragma once

#include <string>
#include <vector>

#include <glm/vec3.hpp>

#include "PlayerController.h"
#include "Renderer.h"
#include "../comp/ISerializable.h"
#include "../eng/Go.h"
#include "../eng/ren/Camera.h"
#include "../eng/ren/Mesh.h"

class SceneManager : public ISerializable {
    GameObject level_, player_;
	PlayerController player_controller_;

    Camera cam_;
    Renderer::Environment env_;
	Renderer::FrameBuf fr_buf_;

	struct CamPoint { glm::vec3 pos, affect_point; bool follow; };
	std::vector<CamPoint> cam_points_;
    std::vector<GameObject> static_meshes_;
	std::vector<std::pair<glm::vec3, glm::vec3>> level_colliders_;

    struct Transition { glm::vec3 pos, pl_pos; std::string map, door; bool revert; };
	std::vector<Transition> transitions_;

	struct Mirror { glm::vec4 plane; glm::vec2 dims[2]; };
	Mirror mirror_;

	glm::vec3 codelock_pos_;

	void OnPlayerUse();

	static bool Collide(const Transform *tr1, const Transform *tr2, glm::vec3 &response);
	static bool Collide(const Transform *tr1, const glm::vec3 &b_min, const glm::vec3 &b_max, glm::vec3 &response);
public:
    SceneManager();

    void Update(int dt_ms);

    void Draw(Renderer *r, float dt_s);

    void Resize(int w, int h);

	bool HandleInput(InputManager::Event evt);

    // ISerializable
    bool Read(const JsObject &js_in) override;
    void Write(JsObject &js_out) override {}

	sys::Signal<void(const std::string &, const std::string &, bool)> change_map_signal;
	sys::Signal<void()> show_codelock_signal;
};