#include "Renderer.h"

#include <glm/gtc/type_ptr.hpp>

#include <SW/SW.h>
#include <sys/Json.h>
#include <ren/RenderState.h>

namespace UIRendererConstants {
    enum { A_POS,
           A_UV };

    enum { V_UV };

	enum { U_COL };

    enum { DIFFUSEMAP_SLOT };
}

extern "C" {
	VSHADER ui_program_vs(VS_IN, VS_OUT) {
		using namespace glm; using namespace UIRendererConstants;

		*(vec2 *)V_FVARYING(V_UV) = make_vec2(V_FATTR(A_UV));
		*(vec4 *)V_POS_OUT = vec4(make_vec3(V_FATTR(A_POS)), 1);
	}

	FSHADER ui_program_fs(FS_IN, FS_OUT) {
		using namespace glm; using namespace UIRendererConstants;

		vec4 rgba;
		TEXTURE(DIFFUSEMAP_SLOT, F_FVARYING_IN(V_UV), &rgba[0]);

		*(vec4 *)F_COL_OUT = rgba * vec4(make_vec3(F_UNIFORM(U_COL)), 1);
	}
}

ui::Renderer::Renderer(const JsObject &config) {
	using namespace UIRendererConstants;

	ui_program_ = R::CreateProgramSW(UI_PROGRAM_NAME, (void *)ui_program_vs, (void *)ui_program_fs, 2);

	R::AttrUnifArg unifs[] = { { "col", U_COL, SW_VEC3 }, {} },
				   attrs[] = { { "pos", A_POS, SW_VEC3 }, { "uvs", A_UV, SW_VEC2 }, {} };
	R::RegisterUnifAttrs(ui_program_, unifs, attrs);
}

ui::Renderer::~Renderer() {
    
}

void ui::Renderer::BeginDraw() {
	using namespace UIRendererConstants;

	R::Program *p = R::GetProgram(ui_program_);

	R::UseProgram(p->prog_id);
	const glm::vec3 white = { 1, 1, 1 };
	swSetUniform(U_COL, SW_VEC3, &white[0]);

	swBindBuffer(SW_ARRAY_BUFFER, 0);
	swBindBuffer(SW_INDEX_BUFFER, 0);

	swDisable(SW_PERSPECTIVE_CORRECTION);
	swDisable(SW_FAST_PERSPECTIVE_CORRECTION);
	swDisable(SW_DEPTH_TEST);
    swEnable(SW_BLEND);

	glm::ivec2 scissor_test[2] = { { 0, 0 }, { R::w, R::h } };
	this->EmplaceParams(glm::vec3(1, 1, 1), 0.0f, BL_ALPHA, scissor_test);
}

void ui::Renderer::EndDraw() {
	swEnable(SW_FAST_PERSPECTIVE_CORRECTION);
	swEnable(SW_DEPTH_TEST);
    swDisable(SW_BLEND);

	this->PopParams();
}

void ui::Renderer::DrawImageQuad(const R::Texture2DRef &tex, const glm::vec2 dims[2], const glm::vec2 uvs[2]) {
    using namespace UIRendererConstants;

    const float vertices[] = {dims[0].x, dims[0].y, 0,
							  uvs[0].x, uvs[0].y,

							  dims[0].x, dims[0].y + dims[1].y, 0,
							  uvs[0].x, uvs[1].y,

							  dims[0].x + dims[1].x, dims[0].y + dims[1].y, 0,
							  uvs[1].x, uvs[1].y,

							  dims[0].x + dims[1].x, dims[0].y, 0,
							  uvs[1].x, uvs[0].y};

    const unsigned char indices[] = { 2, 1, 0,
									  3, 2, 0 };

    R::BindTexture(DIFFUSEMAP_SLOT, tex.tex_id);

	swVertexAttribPointer(A_POS, sizeof(float) * 3, sizeof(float) * 5, &vertices[0]);
	swVertexAttribPointer(A_UV, sizeof(float) * 2, sizeof(float) * 5, &vertices[3]);
	swDrawElements(SW_TRIANGLES, 6, SW_UNSIGNED_BYTE, indices);
}

void ui::Renderer::DrawUIElement(const R::Texture2DRef &tex, ePrimitiveType prim_type,
								 const std::vector<float> &pos, const std::vector<float> &uvs,
								 const std::vector<unsigned char> &indices) {
    using namespace UIRendererConstants;

    if (pos.empty()) return;

    assert(pos.size() / 5 < 0xff);

    R::Program *p = R::GetProgram(ui_program_);

    const DrawParams &cur_params = params_.back();
    this->ApplyParams(p, cur_params);

    R::BindTexture(DIFFUSEMAP_SLOT, tex.tex_id);

    swVertexAttribPointer(A_POS, sizeof(float) * 3, 0, (void *)&pos[0]);
    swVertexAttribPointer(A_UV, sizeof(float) * 2, 0, (void *)&uvs[0]);

    if (prim_type == PRIM_TRIANGLE) {
        swDrawElements(SW_TRIANGLES, (SWuint)indices.size(), SW_UNSIGNED_BYTE, &indices[0]);
    }
}

void ui::Renderer::ApplyParams(R::Program *p, const DrawParams &params) {
	using namespace UIRendererConstants;

	swSetUniform(U_COL, SW_VEC3, &params.col()[0]);
}

