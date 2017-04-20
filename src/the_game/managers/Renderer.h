#pragma once

#include <glm/vec3.hpp>

#include "../eng/ren/Program.h"
#include "../eng/ren/Texture.h"

struct JsObject;
class Drawable;
class PlayerDrawable;
class Transform;

class Renderer {
	R::ProgramRef line_program_, mirror_program_, matcap_program_, matcap_skel_program_;

	void Init();
public:
    Renderer(const JsObject &config);

    struct Environment {
        glm::vec3 ambient;
        bool flashlight_on;
        glm::vec3 flashlight_pos;
    };

#if defined(USE_SW_RENDER)
	struct FrameBuf {
		int w, h, fb;

		FrameBuf() : w(0), h(0), fb(-1) {}
		FrameBuf(int w, int h);
		~FrameBuf();

		FrameBuf(const FrameBuf &rhs) = delete;
		FrameBuf &operator=(const FrameBuf &rhs) = delete;
		FrameBuf(FrameBuf &&rhs);
		FrameBuf &operator=(FrameBuf &&rhs);

		operator bool() const { return fb != -1; }

		void ClearDepth();

		struct MakeCurrent {
			int prev;

			MakeCurrent(const FrameBuf &f);
			~MakeCurrent();
		};
	};
#endif

    void Temp();

    void DrawLevel(const Environment &env, const Drawable *dr);
    void DrawStatic(const Environment &env, const Transform **trs, const Drawable **drs, size_t num = 1);
	void DrawSkeletal(const Environment &env, const Transform *tr, Drawable *dr);
	void DrawBbox(const glm::vec3 &min, const glm::vec3 &max);
	void DrawLine(const glm::vec3 &start, const glm::vec3 &end, bool depth_test = true);
	void DrawMirror(const FrameBuf &f, const glm::vec4 &plane, const glm::vec2 dims[2]);
	void DrawMatcap(const Transform *tr, const Drawable *dr);
	void DrawSkeletalMatcap(const Transform *tr, const Drawable *dr);

	template<typename T>
	void DrawToFramebuffer(const FrameBuf &f, T &&t) {
		FrameBuf::MakeCurrent _(f);
		t();
	}

	void DebugFramebuffer(int x, int y, const FrameBuf &f);
};

