#include "LoadingIcon.h"

#include <glm/gtc/type_ptr.hpp>

#include <SW/SW.h>

#include "../eng/ren/RenderState.h"

namespace LoadingIconConstants {
	enum { A_POS, A_U };

    enum { V_U };

    enum { U_TIME };
}

extern "C" {
	VSHADER curve_vs(VS_IN, VS_OUT) {
		using namespace glm; using namespace LoadingIconConstants;

        *V_FVARYING(V_U) = *V_FATTR(A_U);
		*(vec4 *)V_POS_OUT = vec4(make_vec2(V_FATTR(A_POS)), 0.5f, 1);
	}

	FSHADER curve_fs(FS_IN, FS_OUT) {
		using namespace glm; using namespace LoadingIconConstants;

        float r = *F_FVARYING_IN(V_U) + *F_UNIFORM(U_TIME);
        r = glm::abs(glm::fract(r) - 0.5f);

		*(vec4 *)F_COL_OUT = vec4(r, 0, 0, 1);
	}
}

void LoadingIcon::Init() {
    using namespace LoadingIconConstants;

	curve_program_ = R::CreateProgramSW("curve", (void *)curve_vs, (void *)curve_fs, 1);

    R::AttrUnifArg unifs[] = { { "time", U_TIME, SW_FLOAT }, { nullptr, 0, 0 } };
    R::RegisterUnifAttrs(curve_program_, unifs, nullptr);
}

void LoadingIcon::DrawCurve(const glm::vec2 &pos, const glm::vec2 &size, float t) {
	using namespace LoadingIconConstants;

    swBindBuffer(SW_ARRAY_BUFFER, 0);
    swBindBuffer(SW_INDEX_BUFFER, 0);

	R::Program *p = R::GetProgram(curve_program_);

	R::UseProgram(p->prog_id);

	glm::vec2 sz1 = { (1.0f/6) * size.x, 0.5f * size.y };
	glm::vec2 p0 = { pos.x, pos.y + sz1.y };

	const float vertices[] = {	p0.x, p0.y,                                         0,
								p0.x, p0.y + 0.5523f * sz1.y,                       0.1f,
								p0.x + (1 - 0.5523f) * sz1.x, p0.y + sz1.y,         0.2f,
                                p0.x + sz1.x, p0.y + sz1.y,                         0.3f,
                                p0.x + (1 + 0.75f) * sz1.x, p0.y + sz1.y,           0.4f,
                                p0.x + (3 - 0.5523f) * sz1.x, p0.y + 0.4f * sz1.y,  0.5f,
								p0.x + 3 * sz1.x, p0.y,                             0.6f,
                                p0.x + (3 + 0.5523f) * sz1.x, p0.y - 0.4f * sz1.y,  0.7f,
                                p0.x + (5 - 0.75f) * sz1.x, p0.y - sz1.y,           0.8f,
                                p0.x + 5 * sz1.x, p0.y - sz1.y,                     0.9f,
                                p0.x + (5 + 0.5523f) * sz1.x, p0.y - sz1.y,         1.0f,
                                p0.x + 6 * sz1.x, p0.y - 0.5523f * sz1.y,           1.0f,
                                p0.x + 6 * sz1.x, p0.y,                             1.0f,
                                p0.x + 6 * sz1.x, p0.y + 0.5523f * sz1.y,           1.0f,
                                p0.x + (5 + 0.5523f) * sz1.x, p0.y + sz1.y,         1.0f,
                                p0.x + 5 * sz1.x, p0.y + sz1.y,                     1.0f,
                                p0.x + (5 - 0.75f) * sz1.x, p0.y + sz1.y,           1.0f,
                                p0.x + (3 + 0.5523f) * sz1.x, p0.y + 0.4f * sz1.y,  1.0f,
                                p0.x + 3 * sz1.x, p0.y,                             1.0f,
                                p0.x + (3 - 0.5523f) * sz1.x, p0.y - 0.4f * sz1.y,  1.0f,
                                p0.x + (1 + 0.75f) * sz1.x, p0.y - sz1.y,           1.0f,
                                p0.x + sz1.x, p0.y - sz1.y,                         1.0f,
                                p0.x + (1 - 0.5523f) * sz1.x, p0.y - sz1.y,         1.0f,
                                p0.x, p0.y - 0.5523f * sz1.y,                       1.0f,
                                p0.x, p0.y,                                         1.0f,
                             };

    swSetFloat(SW_CURVE_TOLERANCE, 2.0f);

    swSetUniform(U_TIME, SW_FLOAT, &t);

	swVertexAttribPointer(A_POS, sizeof(float) * 2, sizeof(float) * 3, &vertices[0]);
    swVertexAttribPointer(A_U, sizeof(float), sizeof(float) * 3, &vertices[2]);
    swDrawArrays(SW_CURVE_STRIP, 0, 25);
}