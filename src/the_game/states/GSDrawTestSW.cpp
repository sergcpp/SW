#include "GSDrawTest.h"

#include <glm/matrix.hpp>
#include <glm/gtc/type_ptr.hpp>

#include <SW/SW.h>

#include "../eng/ren/RenderState.h"
#include "../eng/sys/AssetFile.h"

namespace GSDrawTestInternal {
	enum { A_POS, A_COL, A_UVS };
	enum { V_COL = 0, V_UVS = 0 }; // offset in floats
	enum { U_MVP };
	enum { DIFFUSEMAP_SLOT };

	const char CHECKER_TEX_NAME[] = "game_assets/textures/checker.tga";
}

extern "C" {
	VSHADER vtx_color_vs(VS_IN, VS_OUT) {
		using namespace glm; using namespace GSDrawTestInternal;

		*(vec3 *)V_FVARYING(V_COL) = make_vec3(V_FATTR(A_COL));
		*(vec4 *)V_POS_OUT = make_mat4(F_UNIFORM(U_MVP)) * vec4(make_vec3(V_FATTR(A_POS)), 1);
	}

	FSHADER vtx_color_fs(FS_IN, FS_OUT) {
		using namespace glm; using namespace GSDrawTestInternal;

		*(vec4 *)F_COL_OUT = vec4(make_vec3(F_FVARYING_IN(V_COL)), 1);
	}

	///////////////////////////////////////////

	VSHADER constant_vs(VS_IN, VS_OUT) {
		using namespace glm; using namespace GSDrawTestInternal;

		*(vec2 *)V_FVARYING(V_UVS) = make_vec2(V_FATTR(A_UVS));
		*(vec4 *)V_POS_OUT = make_mat4(F_UNIFORM(U_MVP)) * vec4(make_vec3(V_FATTR(A_POS)), 1);
	}

	FSHADER constant_fs(FS_IN, FS_OUT) {
		using namespace glm; using namespace GSDrawTestInternal;

		TEXTURE(DIFFUSEMAP_SLOT, F_FVARYING_IN(V_UVS), F_COL_OUT);
	}
}

void GSDrawTest::Init() {
	using namespace GSDrawTestInternal;

	{	vtx_color_prog_ = R::CreateProgramSW("vtx_color", (void *)vtx_color_vs, (void *)vtx_color_fs, 3);
		R::AttrUnifArg	unifs[] = { { "mvp", U_MVP, SW_MAT4 }, {} },
						attribs[] = { { "pos", A_POS, SW_VEC3 }, { "col", A_COL, SW_VEC3 }, {} };
		R::RegisterUnifAttrs(vtx_color_prog_, unifs, attribs);
	}
	
	{	constant_prog_ = R::CreateProgramSW("constant", (void *)constant_vs, (void *)constant_fs, 2);
		R::AttrUnifArg	unifs[] = { { "mvp", U_MVP, SW_MAT4 }, {} },
						attribs[] = { { "pos", A_POS, SW_VEC3 }, { "uvs", A_UVS, SW_VEC2 }, {} };
		R::RegisterUnifAttrs(constant_prog_, unifs, attribs);
	}

	{	sys::AssetFile in_file(CHECKER_TEX_NAME);
		size_t sz = in_file.size();
		std::unique_ptr<char[]> in_buf(new char[sz]);
		in_file.Read(&in_buf[0], sz);
		R::eTexLoadStatus status;
		checker_tex_ = R::LoadTexture2D(CHECKER_TEX_NAME, &in_buf[0], &status);
		assert(status == R::TexCreatedFromData);
	}
}

void GSDrawTest::DrawPrimitives() {
	using namespace GSDrawTestInternal;

	R::Program *p = R::GetProgram(vtx_color_prog_);

	swBindBuffer(SW_ARRAY_BUFFER, 0);
	swBindBuffer(SW_INDEX_BUFFER, 0);

	R::UseProgram(p->prog_id);

	glm::mat4 mvp_mat;
	swSetUniform(U_MVP, SW_MAT4, &mvp_mat[0][0]);


	{	// draw line

		const glm::vec3 lines_pos[] = { { -0.75f, -0.5f, 0.5f }, 
										{ -0.5f, -0.25f, 0.5f }, 
										{ -0.75f, 0, 0.5f },
										{ -0.5f, 0.25f, 0.5f },
										{ -0.75f, 0.5f, 0.5f },
										{ -0.5f, 0.75f, 0.5f }, };
		const glm::vec3 lines_col[] = { { 1, 0, 0 }, 
										{ 1, 1, 0 }, 
										{ 0, 1, 0 },
										{ 0, 1, 1 },
										{ 1, 0, 1 },
										{ 1, 0, 0 } };

		swVertexAttribPointer(A_POS, 3 * sizeof(float), 0, (void *)&lines_pos[0][0]);
		swVertexAttribPointer(A_COL, 3 * sizeof(float), 0, (void *)&lines_col[0][0]);
		swDrawArrays(SW_LINE_STRIP, 0, 6);
	}

	{	// draw triangle

		const glm::vec3 tri_pos[] = { { -0.25f, -0.25f, 0.5f }, { 0.25f, -0.25f, 0.5f }, { 0, 0.5f, 0.5f } };
		const glm::vec3 tri_col[] = { { 1, 0, 0 }, { 0, 1, 0 }, { 0, 0, 1 } };

		swVertexAttribPointer(A_POS, 3 * sizeof(float), 0, (void *)&tri_pos[0][0]);
		swVertexAttribPointer(A_COL, 3 * sizeof(float), 0, (void *)&tri_col[0][0]);
		swDrawArrays(SW_TRIANGLES, 0, 3);
	}

	{	// draw curve

		const glm::vec3 curve_pos[] = { { 0.75f, -0.5f, 0.5f },
										{ 0.25f, -0.25f, 0.5f },
										{ 1.0f, 0, 0.5f },
										{ 0.5f, 0.25f, 0.5f }, 
										{ 0.25f, 0.5f, 0.5f },
										{ 0.5f, 0.75f, 0.5f },
										{ 0.75f, 0.75f, 0.5f } };
		const glm::vec3 curve_col[] = { { 1, 0, 0 },
										{ 1, 1, 0 },
										{ 0, 1, 0 },
										{ 0, 1, 1 },
										{ 0, 0, 1 },
										{ 1, 0, 1 },
										{ 0, 1, 1 } };

		swVertexAttribPointer(A_POS, 3 * sizeof(float), 0, (void *)&curve_pos[0][0]);
		swVertexAttribPointer(A_COL, 3 * sizeof(float), 0, (void *)&curve_col[0][0]);
		swDrawArrays(SW_CURVE_STRIP, 0, 7);
	}
}

void GSDrawTest::DrawPlanes() {
	using namespace GSDrawTestInternal;

	R::ProgramRef p_ref = R::CreateProgramSW("constant", nullptr, nullptr, 0);
	R::Program *p = R::GetProgram(p_ref);

	swBindBuffer(SW_ARRAY_BUFFER, 0);
	swBindBuffer(SW_INDEX_BUFFER, 0);

	R::UseProgram(p->prog_id);

	R::BindTexture(DIFFUSEMAP_SLOT, checker_tex_.tex_id);

	glm::mat4 m_mat, mv_mat, mvp_mat;
	mv_mat = glm::make_mat4(R::current_cam->view_matrix()) * m_mat;
	mvp_mat = glm::make_mat4(R::current_cam->projection_matrix()) * mv_mat;
	swSetUniform(U_MVP, SW_MAT4, &mvp_mat[0][0]);

	glm::vec3 tri_pos[] = { { -0.7f, 0, -8.0f }, { 0.7f, 0, -8.0f }, { -0.7f, 1, -100.0f }, { 0.7f, 1, -100.0f } };
	const glm::vec2 tri_uvs[] = { { 0, 0 }, { 1, 0 }, { 0, 1 }, { 1, 1 } };

	{	// plane with perspective correction
		swEnable(SW_PERSPECTIVE_CORRECTION);
		swDisable(SW_FAST_PERSPECTIVE_CORRECTION);

		swVertexAttribPointer(A_POS, 3 * sizeof(float), 0, (void *)&tri_pos[0][0]);
		swVertexAttribPointer(A_UVS, 2 * sizeof(float), 0, (void *)&tri_uvs[0][0]);
		swDrawArrays(SW_TRIANGLE_STRIP, 0, 4);
	}

	{	// plane without perspective correction
		swDisable(SW_PERSPECTIVE_CORRECTION);

		for (auto &p : tri_pos) p.x -= 1.5f;
		swDrawArrays(SW_TRIANGLE_STRIP, 0, 4);
	}

	{	// plane with fast perspective correction
		swEnable(SW_PERSPECTIVE_CORRECTION);
		swEnable(SW_FAST_PERSPECTIVE_CORRECTION);

		for (auto &p : tri_pos) p.x += 3.0f;
		swDrawArrays(SW_TRIANGLE_STRIP, 0, 4);
	}
}
