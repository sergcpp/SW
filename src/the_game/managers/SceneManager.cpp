#include "SceneManager.h"

#include <fstream>

#include <glm/geometric.hpp>
#include <glm/gtc/type_ptr.hpp>

#include "../comp/Drawable.h"
#include "../comp/Transform.h"
#include "../eng/ren/RenderState.h"
#include "../eng/sys/Json.h"
#include "../eng/sys/Log.h"

namespace SceneManagerInternal {
    const glm::vec3 cam_center = { 0, 2.75f, 1.6f };
    const glm::vec3 cam_target = { 0, 1, -1 };
    const glm::vec3 cam_up = { 0, 1, 0 };

    const char player_def_file[] = "game_assets/models/player.json";

	float TRIGGER_RADIUS = 1;

	inline void operator<<(glm::vec2 &vec, const JsArray &arr) {
		vec = glm::vec2(((const JsNumber &)arr.at(0)).val,
						((const JsNumber &)arr.at(1)).val);
	}

	inline void operator<<(glm::ivec2 &vec, const JsArray &arr) {
		vec = glm::ivec2(((const JsNumber &)arr.at(0)).val,
						 ((const JsNumber &)arr.at(1)).val);
	}

	inline void operator<<(glm::vec3 &vec, const JsArray &arr) {
		vec = glm::vec3(((const JsNumber &)arr.at(0)).val,
						((const JsNumber &)arr.at(1)).val, 
						((const JsNumber &)arr.at(2)).val );
	}

	inline void operator<<(glm::vec4 &vec, const JsArray &arr) {
		vec = glm::vec4(((const JsNumber &)arr.at(0)).val,
						((const JsNumber &)arr.at(1)).val,
						((const JsNumber &)arr.at(2)).val,
						((const JsNumber &)arr.at(3)).val);
	}

	struct DrawList {
		std::vector<const Transform *> trs;
		std::vector<const Drawable *> drs;

		void reserve(size_t s) {
			trs.reserve(s);
			drs.reserve(s);
		}

		void clear() {
			trs.clear();
			drs.clear();
		}

		bool empty() { return trs.empty(); }
	};
}

SceneManager::SceneManager() : level_("Level"),
							   player_("Player"),
							   cam_(&SceneManagerInternal::cam_center[0],
									&SceneManagerInternal::cam_target[0],
									&SceneManagerInternal::cam_up[0]) {

    std::ifstream in_file(SceneManagerInternal::player_def_file, std::ios::binary);
    JsObject js_pl_def;
    if (!js_pl_def.Read(in_file)) {
        LOGE("Error parsing json file!");
    }

    std::unique_ptr<Drawable> dr(new Drawable);
    if (dr->Read(js_pl_def)) {
        player_.AddComponent(dr.release());
    }
	player_.AddComponent(new Transform(glm::vec3(-0.35f, 0, -0.35f), glm::vec3(0.35f, 2.0f, 0.35f)));

	player_controller_.use_signal.Connect<SceneManager, &SceneManager::OnPlayerUse>(this);

    Resize(1, 1);
}

void SceneManager::Update(int dt_ms) {
	player_controller_.Update(player_, dt_ms);

	// resolve collisions
    auto pl_tr = player_.GetComponent<Transform>();
	for (auto &m : static_meshes_) {
        auto tr = m.GetComponent<Transform>();
		if (!tr) continue;

		glm::vec3 resp;
		if (Collide(pl_tr, tr, resp)) {
			pl_tr->SetPos(pl_tr->pos() + resp);
		}
	}

	for (const auto &c : level_colliders_) {
		glm::vec3 resp;
		if (Collide(pl_tr, c.first, c.second, resp)) {
			pl_tr->SetPos(pl_tr->pos() + resp);
		}
	}
}

void SceneManager::Draw(Renderer *r, float dt_s) {
    using namespace glm; using namespace SceneManagerInternal;

    auto pl_tr = player_.GetComponent<Transform>();
    auto pl_dr = player_.GetComponent<Drawable>();

	vec3 pl_center = pl_tr->pos() + vec3(0, 1, 0);
	vec3 cam_target;

	assert(!cam_points_.empty());

	glm::vec3 cam_pos;
	{	// choose closest camera
		float best_dist = std::numeric_limits<float>::max();
		for (const auto &cp : cam_points_) {
			float dist = distance(pl_center, cp.affect_point);
			if (dist < best_dist) {
				cam_pos = cp.pos;
				if (cp.follow) {
					cam_target = pl_center;
				} else {
					cam_target = cp.affect_point;
				}
				best_dist = dist;
			}
		}
	}

    env_.flashlight_on = player_controller_.flashlight_on();
    env_.flashlight_pos = pl_center + 2.0f * vec3(-pl_tr->matrix()[0]);

    auto lev_dr = level_.GetComponent<Drawable>();

	R::current_cam = &cam_;

    DrawList draw_list;
    draw_list.reserve(static_meshes_.size());

    for (auto &m : static_meshes_) {
        auto tr = m.GetComponent<Transform>();
        auto dr = m.GetComponent<Drawable>();
        if (dr && tr) {
            draw_list.trs.push_back(tr);
            draw_list.drs.push_back(dr);
        }
    }

    //pl_dr->UpdateAnim(dt_s);
    player_controller_.UpdateAnim(player_, dt_s);

	if (fr_buf_) {	// draw scene to mirror frame buffer
		glm::vec3 mirrored_cam_pos = cam_pos;
		mirrored_cam_pos -= 2 * (glm::dot(mirrored_cam_pos, glm::vec3(mirror_.plane)) - mirror_.plane.w) * glm::vec3(mirror_.plane);

		glm::vec3 mirrored_cap_target = cam_target;
		mirrored_cap_target -= 2 * (glm::dot(mirrored_cap_target, glm::vec3(mirror_.plane)) - mirror_.plane.w) * glm::vec3(mirror_.plane);

		cam_.SetupView(&mirrored_cam_pos[0], &mirrored_cap_target[0], &SceneManagerInternal::cam_up[0]);

		fr_buf_.ClearDepth();
		r->DrawToFramebuffer(fr_buf_, [&]() {
			r->DrawLevel(env_, lev_dr);

            if (!draw_list.empty()) { // msvc complains about &draw_list.trs[0] in debug mode
                r->DrawStatic(env_, &draw_list.trs[0], &draw_list.drs[0], draw_list.trs.size());
            }

            r->DrawSkeletal(env_, pl_tr, pl_dr);

            /*r->DrawBbox(pl_tr->bbox_min(), pl_tr->bbox_max());

			glm::vec3 line_start = pl_tr->pos() + glm::vec3(0, 0.1f, 0),
					  line_end = pl_tr->pos() + glm::vec3(-pl_tr->matrix()[2]) + glm::vec3(0, 0.1f, 0);
			r->DrawLine(line_start, line_end);*/
		});
	}

	cam_.SetupView(&cam_pos[0], &cam_target[0], &SceneManagerInternal::cam_up[0]);

    r->DrawSkeletal(env_, pl_tr, pl_dr);

    if (lev_dr) {
        r->DrawLevel(env_, lev_dr);
    }

	if (!draw_list.empty()) { // msvc complains about &draw_list.trs[0] in debug mode
		r->DrawStatic(env_, &draw_list.trs[0], &draw_list.drs[0], draw_list.trs.size());
	}
	
	{	// draw bounding boxes
		/*r->DrawBbox(pl_tr->bbox_min(), pl_tr->bbox_max());

		glm::vec3 line_start = pl_tr->pos() + glm::vec3(0, 0.1f, 0),
			      line_end = pl_tr->pos() + glm::vec3(-pl_tr->matrix()[0]) + glm::vec3(0, 0.1f, 0);
		r->DrawLine(line_start, line_end);

		for (auto &m : static_meshes_) {
            auto tr = m.GetComponent<Transform>();
			if (tr) {
				r->DrawBbox(tr->bbox_min(), tr->bbox_max());
			}
		}

		for (auto &c : level_colliders_) {
			r->DrawBbox(c.first, c.second);
		}*/
	}

	if (fr_buf_) {
		r->DrawMirror(fr_buf_, mirror_.plane, mirror_.dims);
	}

	//r->DebugFramebuffer(0, 0, fr_buf_);
}

void SceneManager::Resize(int w, int h) {
    float aspect = float(w) / h;
    cam_.Perspective(75, aspect, 0.1f, 20.0f);
}

bool SceneManager::HandleInput(InputManager::Event evt) {
	return player_controller_.HandleInput(evt);
}

bool SceneManager::Read(const JsObject &js_in) {
	using namespace SceneManagerInternal;

    try {
        const JsObject &js_lev = (const JsObject &) js_in.at("level");

        level_.ClearComponents();

        std::unique_ptr<Drawable> lev_dr(new Drawable);
        if (lev_dr->Read(js_lev)) {
            level_.AddComponent(lev_dr.release());
        } else {
            return false;
        }

        if (js_lev.Has("ambient")) {
            const JsArray &js_ambient = (const JsArray &) js_lev.at("ambient");
			env_.ambient << js_ambient;
        } else {
            env_.ambient = { 1, 1, 1 };
        }

		cam_points_.clear();
        if (js_lev.Has("cam_points")) {
            const JsArray &js_cam_points = (const JsArray &) js_lev.at("cam_points");
            for (size_t i = 0; i < js_cam_points.Size(); i++) {
				const JsObject &js_cp = (const JsObject &)js_cam_points.at(i);
				const JsArray &js_cp_pos = (const JsArray &)js_cp.at("pos");
				const JsArray &js_cp_aff = (const JsArray &)js_cp.at("aff");

				CamPoint cp;
				cp.pos << js_cp_pos;
				if (js_cp_aff.Size()) {
					cp.affect_point << js_cp_aff;
				} else {
					cp.affect_point = cp.pos;
				}

				if (js_cp.Has("follow_player")) {
					auto v = ((const JsLiteral &)js_cp.at("follow_player")).val;
					if (v == JS_TRUE) {
						cp.follow = true;
					} else if (v == JS_FALSE) {
						cp.follow = false;
					} else {
						return false;
					}
				} else {
					cp.follow = true;
				}

                cam_points_.push_back(cp);
            }
        }

		if (js_lev.Has("player_start")) {
			const JsArray &js_pos = (const JsArray &)js_lev.at("player_start");
			glm::vec3 pos;
			pos << js_pos;

            auto pl_tr = player_.GetComponent<Transform>();
			if (glm::length(pl_tr->pos()) < 0.01f) {
				pl_tr->SetPos(pos);
			}
		}
		player_controller_.Reset();

		static_meshes_.clear();
		if (js_lev.Has("static_meshes")) {
			const JsArray &js_static_meshes = (const JsArray &)js_lev.at("static_meshes");
			for (size_t i = 0; i < js_static_meshes.Size(); i++) {
				const JsObject &m = (const JsObject &)js_static_meshes.at(i);
				static_meshes_.emplace_back("static_mesh");
				auto &mesh = static_meshes_.back();

				glm::vec3 bbox_min, bbox_max;

				std::unique_ptr<Drawable> mesh_dr(new Drawable);
				if (mesh_dr->Read(m)) {
					R::Mesh *m = R::GetMesh(mesh_dr->mesh());
					bbox_min = m->bbox_min;
					bbox_max = m->bbox_max;
					mesh.AddComponent(mesh_dr.release());
				} else {
					return false;
				}

				std::unique_ptr<Transform> mesh_tr(new Transform(bbox_min, bbox_max));
				if (mesh_tr->Read(m)) {
					mesh.AddComponent(mesh_tr.release());
				} else {
					return false;
				}
			}
		}

		level_colliders_.clear();
		if (js_lev.Has("colliders")) {
			const JsArray &js_lev_colliders = (const JsArray &)js_lev.at("colliders");
			for (size_t i = 0; i < js_lev_colliders.Size(); i++) {
				const JsArray &min = (const JsArray &)((const JsArray &)js_lev_colliders.at(i)).at(0),
							  &max = (const JsArray &)((const JsArray &)js_lev_colliders.at(i)).at(1);

				level_colliders_.emplace_back();
				level_colliders_.back().first << min;
				level_colliders_.back().second << max;
			}
		}

		transitions_.clear();
		if (js_lev.Has("transitions")) {
			const JsArray &js_transitions = (const JsArray &)js_lev.at("transitions");
			for (size_t i = 0; i < js_transitions.Size(); i++) {
				const JsObject &js_t = (const JsObject &)js_transitions.at(i);
				const JsArray &js_pos = (const JsArray &)js_t.at("pos");
				const JsString &js_map = (const JsString &)js_t.at("map");
                const JsString &js_door = (const JsString &)js_t.at("door");

				Transition t;
				t.pos << js_pos;
				t.map = js_map.val;
                t.door = js_door.val;

                if (js_t.Has("revert")) {
                    const JsLiteral &js_revert = (const JsLiteral &)js_t.at("revert");
                    t.revert = js_revert.val == JS_TRUE;
                }

				if (js_t.Has("player_pos")) {
					t.pl_pos << (const JsArray &)js_t.at("player_pos");
				}

				transitions_.push_back(t);
			}
		}

		if (js_lev.Has("mirror")) {
			const JsObject &js_m = (const JsObject &)js_lev.at("mirror");
			const JsArray &js_m_plane = (const JsArray &)js_m.at("plane");
			const JsArray &js_m_dims = (const JsArray &)js_m.at("dims");

			glm::ivec2 res = { 256, 128 };
			if (js_m.Has("res")) {
				res << (const JsArray &)js_m.at("res");
			}
			fr_buf_ = { res.x, res.y };

			mirror_.plane << js_m_plane;
			mirror_.dims[0] << (const JsArray &)js_m_dims.at(0);
			mirror_.dims[1] << (const JsArray &)js_m_dims.at(1);
		} else {
			fr_buf_ = {};
		}

		codelock_pos_ = {};
		if (js_lev.Has("codelock")) {
			const JsObject &js_codelock = (const JsObject &)js_lev.at("codelock");
			codelock_pos_ << (const JsArray &)js_codelock.at("pos");
		}

        return true;
    } catch (...) {
        return false;
    }
}

void SceneManager::OnPlayerUse() {
	using namespace SceneManagerInternal;

    auto pl_tr = player_.GetComponent<Transform>();
	for (const auto &t : transitions_) {
		if (glm::distance(pl_tr->pos(), t.pos) <= TRIGGER_RADIUS) {
			change_map_signal.FireN(t.map, t.door, t.revert);
			// move player to other side of door
			pl_tr->SetPos(t.pl_pos);
			return;
		}
	}

	if (glm::length(codelock_pos_) > 0.1f) {
		if (glm::distance(pl_tr->pos(), codelock_pos_) <= TRIGGER_RADIUS) {
			show_codelock_signal.FireN();
			return;
		}
	}
}

bool SceneManager::Collide(const Transform *tr1, const Transform *tr2, glm::vec3 &response) {
	return Collide(tr1, tr2->bbox_min(), tr2->bbox_max(), response);
}

bool SceneManager::Collide(const Transform *tr1, const glm::vec3 &b_min, const glm::vec3 &b_max, glm::vec3 &response) {
	const glm::vec3 &a_min = tr1->bbox_min(), &a_max = tr1->bbox_max();

	float distances[] = { b_max.x - a_min.x,
						  a_max.x - b_min.x,
						  b_max.y - a_min.y,
						  a_max.y - b_min.y,
						  b_max.z - a_min.z,
						  a_max.z - b_min.z };

	float min_dist = std::numeric_limits<float>::max();
	int min_index = -1;

	for (int i = 0; i < 6; i++) {
		if (distances[i] < 0) {
			return false;
		}

		if (distances[i] < min_dist) {
			min_index = i;
			min_dist = distances[i];
		}
	}

	static const glm::vec3 normals[6] = { { -1, 0, 0 },
										  { 1, 0, 0 },
										  { 0, -1, 0 },
										  { 0, 1, 0 },
										  { 0, 0, -1 },
										  { 0, 0, 1 } };

	response = -normals[min_index] * min_dist;

	return true;
}