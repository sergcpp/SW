#include "Renderer.h"

#define GLM_ENABLE_EXPERIMENTAL
#define GLM_FORCE_RADIANS
#include <glm/ext.hpp>
#include <glm/gtx/norm.hpp>

#include <SW/SW.h>

#include "../comp/Drawable.h"
#include "../comp/Transform.h"
#include "../eng/ren/Camera.h"
#include "../eng/ren/RenderState.h"
#include "../eng/sys/AssetFile.h"
#include "../eng/sys/Log.h"

namespace RendererSWConstants {
    enum { A_POS,
           A_UVS,
		   A_NORMAL,
           A_INDICES,
           A_WEIGHTS };

    enum { V_UVS };

    enum { U_MVP,
           U_AMBIENT,
           U_FLASHLIGHT_POS,
           U_MV,
           U_MAT_PALETTE};

    enum { DIFFUSEMAP_SLOT };

    const int MAT_PALETTE_SIZE = 32;
}

void Renderer::DrawLevel(const Environment &env, const Drawable *dr) {
    R::MeshRef ref = dr->mesh();

    using namespace RendererSWConstants;
    using namespace glm;

    if (ref.index == -1) return;

    R::Mesh *m			= R::GetMesh(ref);
    R::Material *mat	= R::GetMaterial(m->strips[0].mat);
    R::Program *p		= R::GetProgram(mat->program);

    swBindBuffer(SW_ARRAY_BUFFER, m->attribs_buf_id);
    swBindBuffer(SW_INDEX_BUFFER, m->indices_buf_id);

    R::UseProgram(p->prog_id);

    swEnable(SW_PERSPECTIVE_CORRECTION);
    swEnable(SW_FAST_PERSPECTIVE_CORRECTION);

    mat4 mv_mat, mvp_mat;
    mv_mat = make_mat4(R::current_cam->view_matrix());
    mvp_mat = make_mat4(R::current_cam->projection_matrix()) * mv_mat;
    swSetUniform(U_MVP, SW_MAT4, &mvp_mat[0][0]);

    vec3 ambient = { 1, 1, 1 };
    swSetUniform(U_AMBIENT, SW_VEC3, &ambient[0]);

    vec4 fl = vec4(env.flashlight_pos, 1);
    fl = mvp_mat * fl;
    fl.x /= fl.w; fl.y /= fl.w; fl.z /= fl.w;
    fl.w = env.flashlight_on;
    swSetUniform(U_FLASHLIGHT_POS, SW_VEC4, &fl[0]);

    const int stride = sizeof(float) * 8;
    swVertexAttribPointer(A_POS, 3 * sizeof(float), (SWuint)stride, (void *)0);
    swVertexAttribPointer(A_UVS, 2 * sizeof(float), (SWuint)stride, (void *)(6 * sizeof(float)));

	for (R::TriStrip *s = &m->strips[0]; s->offset != -1; ++s) {
        mat = R::GetMaterial(s->mat);
        R::BindTexture(DIFFUSEMAP_SLOT, mat->textures[0].tex_id);
        swDrawElements(SW_TRIANGLE_STRIP, (SWuint)s->num_indices, SW_UNSIGNED_SHORT, (void *)uintptr_t(s->offset));
    }
}

void Renderer::DrawStatic(const Environment &env, const Transform **trs, const Drawable **drs, size_t num) {
	using namespace RendererSWConstants;
	using namespace glm;

	if (!num) return;

	swEnable(SW_PERSPECTIVE_CORRECTION);
	swEnable(SW_FAST_PERSPECTIVE_CORRECTION);

	{	// use first mesh to find program, program should be same between them
		R::Mesh *m		 = R::GetMesh(drs[0]->mesh());
		R::Material *mat = R::GetMaterial(m->strips[0].mat);
		R::Program *p	 = R::GetProgram(mat->program);

		R::UseProgram(p->prog_id);
	}

	for (size_t i = 0; i < num; i++) {
		const Transform *tr = trs[i]; const Drawable *dr = drs[i];

		R::MeshRef ref = dr->mesh();

		R::Mesh *m		 = R::GetMesh(ref);
		R::Material *mat = R::GetMaterial(m->strips[0].mat);
		R::Program *p	 = R::GetProgram(mat->program);

		swBindBuffer(SW_ARRAY_BUFFER, m->attribs_buf_id);
		swBindBuffer(SW_INDEX_BUFFER, m->indices_buf_id);

		mat4 m_mat, mv_mat, mvp_mat;
		m_mat = tr->matrix();
		mv_mat = make_mat4(R::current_cam->view_matrix()) * m_mat;
		mvp_mat = make_mat4(R::current_cam->projection_matrix()) * mv_mat;
		swSetUniform(U_MVP, SW_MAT4, &mvp_mat[0][0]);

		vec3 amb = env.ambient;
		if (env.flashlight_on) {
			float d2 = 4 / distance2(tr->pos(), vec3(env.flashlight_pos));
			amb *= clamp(d2, 1.0f, 4.0f);
		}

		swSetUniform(U_AMBIENT, SW_VEC3, &amb[0]);

		const int stride = sizeof(float) * (m->type == R::MeshSkeletal ? 16 : 8);
		swVertexAttribPointer(A_POS, 3 * sizeof(float), (SWuint)stride, (void *)0);
		swVertexAttribPointer(A_UVS, 2 * sizeof(float), (SWuint)stride, (void *)(6 * sizeof(float)));

		for (R::TriStrip *s = &m->strips[0]; s->offset != -1; ++s) {
			mat = R::GetMaterial(s->mat);
			R::BindTexture(DIFFUSEMAP_SLOT, mat->textures[0].tex_id);
			swDrawElements(SW_TRIANGLE_STRIP, (SWuint)s->num_indices, SW_UNSIGNED_SHORT, (void *)uintptr_t(s->offset));
		}
	}
}

void Renderer::DrawSkeletal(const Environment &env, const Transform *tr, Drawable *dr) {
	R::MeshRef ref = dr->mesh();

	using namespace RendererSWConstants;
	using namespace glm;

	if (ref.index == -1) return;

	R::Mesh *m		 = R::GetMesh(ref);
	R::Material *mat = R::GetMaterial(m->strips[0].mat);
	R::Program *p	 = R::GetProgram(mat->program);

	R::Skeleton *skel = &m->skel;
	int cur_anim = dr->cur_anim();
	if (!skel->anims.empty() && cur_anim != -1) {
		//skel->UpdateAnim(cur_anim, dt_s, &dr->anim_time);
		//skel->ApplyAnim(cur_anim);
        assert(cur_anim < skel->anims.size());
		skel->UpdateBones();
	}

	swBindBuffer(SW_ARRAY_BUFFER, m->attribs_buf_id);
	swBindBuffer(SW_INDEX_BUFFER, m->indices_buf_id);

	R::UseProgram(p->prog_id);

	swEnable(SW_PERSPECTIVE_CORRECTION);
	swEnable(SW_FAST_PERSPECTIVE_CORRECTION);

	mat4 m_mat, mv_mat, mvp_mat;
	m_mat = tr->matrix();
	//m_mat = glm::rotate(m_mat, angle, vec3(0, 1, 0));
	mv_mat = make_mat4(R::current_cam->view_matrix()) * m_mat;
	mvp_mat = make_mat4(R::current_cam->projection_matrix()) * mv_mat;
	swSetUniform(U_MV, SW_MAT4, &mv_mat[0][0]);
	swSetUniform(U_MVP, SW_MAT4, &mvp_mat[0][0]);

	size_t num_bones = skel->matr_palette.size();
	swSetUniformv(U_MAT_PALETTE, SW_MAT4, (SWint)num_bones, &skel->matr_palette[0][0][0]);

    swSetUniform(U_AMBIENT, SW_VEC3, &env.ambient[0]);

	int stride = sizeof(float) * 16;
	swVertexAttribPointer(A_POS, 3 * sizeof(float), (SWuint)stride, (void *)0);
	swVertexAttribPointer(A_UVS, 3 * sizeof(float), (SWuint)stride, (void *)(6 * sizeof(float)));
	swVertexAttribPointer(A_INDICES, 4 * sizeof(float), (SWuint)stride, (void *)(8 * sizeof(float)));
	swVertexAttribPointer(A_WEIGHTS, 4 * sizeof(float), (SWuint)stride, (void *)(12 * sizeof(float)));

	for (R::TriStrip *s = &m->strips[0]; s->offset != -1; ++s) {
		mat = R::GetMaterial(s->mat);
		R::BindTexture(DIFFUSEMAP_SLOT, mat->textures[0].tex_id);
		swDrawElements(SW_TRIANGLE_STRIP, (SWuint)s->num_indices, SW_UNSIGNED_SHORT, (void *)uintptr_t(s->offset));
	}
}

void Renderer::DrawBbox(const glm::vec3 &min, const glm::vec3 &max) {
	using namespace RendererSWConstants;
	using namespace glm;

	R::Program *p = R::GetProgram(line_program_);

	swBindBuffer(SW_ARRAY_BUFFER, 0);
	swBindBuffer(SW_INDEX_BUFFER, 0);

	R::UseProgram(p->prog_id);

	mat4 mv_mat, mvp_mat;
	mv_mat = make_mat4(R::current_cam->view_matrix());
	mvp_mat = make_mat4(R::current_cam->projection_matrix()) * mv_mat;
	swSetUniform(U_MVP, SW_MAT4, &mvp_mat[0][0]);

	const vec3 points[] = { min,
						    { max.x, min.y, min.z },
							{ max.x, min.y, max.z },
							{ min.x, min.y, max.z },

							{ min.x, max.y, min.z },
							{ max.x, max.y, min.z },
							max,
							{ min.x, max.y, max.z } };

	unsigned char indices[] = { 0, 1,	1, 2,	2, 3,	3, 0,
								0, 4,	1, 5,	2, 6,	3, 7,
								4, 5,	5, 6,	6, 7,	7, 4 };

	swVertexAttribPointer(A_POS, 3 * sizeof(float), 0, (void *)&points[0][0]);
	swDrawElements(SW_LINES, 24, SW_UNSIGNED_BYTE, &indices[0]);
}

void Renderer::DrawLine(const glm::vec3 &start, const glm::vec3 &end, bool depth_test) {
	using namespace RendererSWConstants;
	using namespace glm;

    if (!depth_test) {
        swDisable(SW_DEPTH_TEST);
    }

	R::Program *p = R::GetProgram(line_program_);

	swBindBuffer(SW_ARRAY_BUFFER, 0);
	swBindBuffer(SW_INDEX_BUFFER, 0);

	R::UseProgram(p->prog_id);

	mat4 m_mat, mv_mat, mvp_mat;
	mv_mat = make_mat4(R::current_cam->view_matrix()) * m_mat;
	mvp_mat = make_mat4(R::current_cam->projection_matrix()) * mv_mat;
	swSetUniform(U_MVP, SW_MAT4, &mvp_mat[0][0]);

	const vec3 points[] = { start, end };

	swVertexAttribPointer(A_POS, 3 * sizeof(float), 0, (void *)&points[0][0]);
	swDrawArrays(SW_LINES, 0, 2);

    if (!depth_test) {
        swEnable(SW_DEPTH_TEST);
    }
}

void Renderer::DrawMirror(const FrameBuf &f, const glm::vec4 &plane, const glm::vec2 dims[2]) {
	using namespace RendererSWConstants;
	using namespace glm;

	swDisable(SW_PERSPECTIVE_CORRECTION);

	R::Program *p = R::GetProgram(mirror_program_);

	swBindBuffer(SW_ARRAY_BUFFER, 0);
	swBindBuffer(SW_INDEX_BUFFER, 0);

	R::UseProgram(p->prog_id);

	mat4 m_mat, mv_mat, mvp_mat;
	mv_mat = make_mat4(R::current_cam->view_matrix()) * m_mat;
	mvp_mat = make_mat4(R::current_cam->projection_matrix()) * mv_mat;
	swSetUniform(U_MVP, SW_MAT4, &mvp_mat[0][0]);

	const void *bgra8888_pixels = swGetPixelDataRef(f.fb);
	SWint temp_tex = swCreateTexture();
	R::BindTexture(DIFFUSEMAP_SLOT, temp_tex);
	swTexImage2DConst(SW_RGBA, SW_UNSIGNED_BYTE, f.w, f.h, (SWubyte *)bgra8888_pixels);

	R::BindTexture(DIFFUSEMAP_SLOT, temp_tex);

	const vec3 up = vec3(0, 1, 0);
	const vec3 side = normalize(cross(up, vec3(plane)));
	const vec3 start = vec3(plane) * plane.w + dims[0].x * side + dims[0].y * up;
	const vec3 end = start + dims[1].x * side + dims[1].y * up;

	const vec3 points[] = { { start }, { end }, { start.x, end.y, start.z },
	                        { start }, { end.x, start.y, start.z }, { end } };

	swVertexAttribPointer(A_POS, 3 * sizeof(float), 0, (void *)&points[0][0]);
	swDrawArrays(SW_TRIANGLES, 0, 6);

	swDeleteTexture(temp_tex);
	R::binded_textures[DIFFUSEMAP_SLOT] = -1;
}

void Renderer::DrawMatcap(const Transform *tr, const Drawable *dr) {
	R::MeshRef ref = dr->mesh();

	using namespace RendererSWConstants;
	using namespace glm;

	if (ref.index == -1) return;

	R::Mesh *m		 = R::GetMesh(ref);
	R::Material *mat = R::GetMaterial(m->strips[0].mat);
	R::Program *p	 = R::GetProgram(mat->program);

	swBindBuffer(SW_ARRAY_BUFFER, m->attribs_buf_id);
	swBindBuffer(SW_INDEX_BUFFER, m->indices_buf_id);

	R::UseProgram(p->prog_id);

	swEnable(SW_PERSPECTIVE_CORRECTION);
	swEnable(SW_FAST_PERSPECTIVE_CORRECTION);

	mat4 m_mat, mv_mat, mvp_mat;
	m_mat = tr->matrix();
	//m_mat = glm::rotate(m_mat, angle, vec3(0, 1, 0));
	mv_mat = make_mat4(R::current_cam->view_matrix()) * m_mat;
	mvp_mat = make_mat4(R::current_cam->projection_matrix()) * mv_mat;
	swSetUniform(U_MV, SW_MAT4, &mv_mat[0][0]);
	swSetUniform(U_MVP, SW_MAT4, &mvp_mat[0][0]);

	int stride = sizeof(float) * 8;
	swVertexAttribPointer(A_POS, 3 * sizeof(float), (SWuint)stride, (void *)0);
	swVertexAttribPointer(A_NORMAL, 3 * sizeof(float), (SWuint)stride, (void *)(3 * sizeof(float)));

	for (R::TriStrip *s = &m->strips[0]; s->offset != -1; ++s) {
		mat = R::GetMaterial(s->mat);
		R::BindTexture(DIFFUSEMAP_SLOT, mat->textures[0].tex_id);
		swDrawElements(SW_TRIANGLE_STRIP, (SWuint)s->num_indices, SW_UNSIGNED_SHORT, (void *)uintptr_t(s->offset));
	}
}

void Renderer::DrawSkeletalMatcap(const Transform *tr, const Drawable *dr) {
    R::MeshRef ref = dr->mesh();

    using namespace RendererSWConstants;
    using namespace glm;

    if (ref.index == -1) return;

    R::Mesh *m		 = R::GetMesh(ref);
    R::Material *mat = R::GetMaterial(m->strips[0].mat);
    R::Program *p	 = R::GetProgram(matcap_skel_program_);

    R::Skeleton *skel = &m->skel;
    int cur_anim = dr->cur_anim();
    if (!skel->anims.empty() && cur_anim != -1) {
        assert(cur_anim < skel->anims.size());
        skel->UpdateBones();
    }
    
    swBindBuffer(SW_ARRAY_BUFFER, m->attribs_buf_id);
    swBindBuffer(SW_INDEX_BUFFER, m->indices_buf_id);

    R::UseProgram(p->prog_id);

    swEnable(SW_PERSPECTIVE_CORRECTION);
    swEnable(SW_FAST_PERSPECTIVE_CORRECTION);

    mat4 m_mat, mv_mat, mvp_mat;
    m_mat = tr->matrix();
    //m_mat = glm::rotate(m_mat, angle, vec3(0, 1, 0));
    mv_mat = make_mat4(R::current_cam->view_matrix()) * m_mat;
    mvp_mat = make_mat4(R::current_cam->projection_matrix()) * mv_mat;
    swSetUniform(U_MV, SW_MAT4, &mv_mat[0][0]);
    swSetUniform(U_MVP, SW_MAT4, &mvp_mat[0][0]);

    size_t num_bones = skel->matr_palette.size();
    swSetUniformv(U_MAT_PALETTE, SW_MAT4, (SWint)num_bones, &skel->matr_palette[0][0][0]);

    int stride = sizeof(float) * 16;
    swVertexAttribPointer(A_POS, 3 * sizeof(float), (SWuint)stride, (void *)0);
    swVertexAttribPointer(A_NORMAL, 3 * sizeof(float), (SWuint)stride, (void *)(3 * sizeof(float)));
    swVertexAttribPointer(A_INDICES, 4 * sizeof(float), (SWuint)stride, (void *)(8 * sizeof(float)));
    swVertexAttribPointer(A_WEIGHTS, 4 * sizeof(float), (SWuint)stride, (void *)(12 * sizeof(float)));

    for (R::TriStrip *s = &m->strips[0]; s->offset != -1; ++s) {
        mat = R::GetMaterial(s->mat);
        R::BindTexture(DIFFUSEMAP_SLOT, mat->textures[0].tex_id);
        swDrawElements(SW_TRIANGLE_STRIP, (SWuint)s->num_indices, SW_UNSIGNED_SHORT, (void *)uintptr_t(s->offset));
    }
}

extern "C" {
    #include <SW/SWcompress.h>
}

void Renderer::Temp() {
    sys::AssetFile in_file("game_assets/textures/hair.tga");
    size_t in_file_size = in_file.size();
    std::unique_ptr<char[]> in_buf(new char[in_file_size]);
    in_file.Read(&in_buf[0], in_file_size);

    int w, h;
    R::eTex2DFormat format;
    auto image_data =  R::ReadTGAFile(&in_buf[0], w, h, format);

    void *out_data = nullptr;
    SWint out_size;
    swTexCompress(&image_data[0], SW_RGB, w, h, &out_data, &out_size);

    sys::AssetFile out_file("game_assets/textures/hair.tex", sys::AssetFile::OUT);
    unsigned short res = (unsigned short)w;
    out_file.Write((char *)&res, 2);
    res = (unsigned short)h;
    out_file.Write((char *)&res, 2);
    out_file.Write((char *)out_data, (size_t)out_size);

    free(out_data);
}

extern "C" {
    VSHADER environment_vs(VS_IN, VS_OUT) {
        using namespace glm; using namespace RendererSWConstants;

        *(vec2 *)V_FVARYING(V_UVS) = make_vec2(V_FATTR(A_UVS));
        *(vec4 *)V_POS_OUT = make_mat4(F_UNIFORM(U_MVP)) * vec4(make_vec3(V_FATTR(A_POS)), 1);
    }

    FSHADER environment_fs(FS_IN, FS_OUT) {
        using namespace glm; using namespace RendererSWConstants;

        const vec4 &fl = make_vec4(F_UNIFORM(U_FLASHLIGHT_POS));

        TEXTURE(DIFFUSEMAP_SLOT, F_FVARYING_IN(V_UVS), F_COL_OUT);
        *(vec3 *) F_COL_OUT *= make_vec3(F_UNIFORM(U_AMBIENT));

		if (fl.w > 0.5f) {
			float d2 = 4 / distance2(make_vec3(F_POS_IN), vec3(fl));
            d2 = clamp(d2, 1.0f, 4.0f);
            *(vec3 *) F_COL_OUT = clamp(make_vec3(F_COL_OUT) * d2, vec3(0, 0, 0), vec3(1, 1, 1));
        }
    }

	///////////////////////////////////////////

	VSHADER static_vs(VS_IN, VS_OUT) {
		using namespace glm; using namespace RendererSWConstants;

		*(vec2 *)V_FVARYING(V_UVS) = make_vec2(V_FATTR(A_UVS));
		*(vec4 *)V_POS_OUT = make_mat4(F_UNIFORM(U_MVP)) * vec4(make_vec3(V_FATTR(A_POS)), 1);
	}

	FSHADER static_fs(FS_IN, FS_OUT) {
		using namespace glm; using namespace RendererSWConstants;

		TEXTURE(DIFFUSEMAP_SLOT, F_FVARYING_IN(V_UVS), F_COL_OUT);
		*(vec3 *)F_COL_OUT *= make_vec3(F_UNIFORM(U_AMBIENT));
	}

	///////////////////////////////////////////

	VSHADER skeletal_vs(VS_IN, VS_OUT) {
		using namespace glm; using namespace RendererSWConstants;

		mat4 mat = *(((mat4 *)F_UNIFORM(U_MAT_PALETTE) + (int)V_FATTR(A_INDICES)[0]));
		*(vec4*)V_POS_OUT = (make_mat4(F_UNIFORM(U_MVP)) * mat) * vec4(make_vec3(V_FATTR(A_POS)), 1);

		*(vec2 *)V_FVARYING(V_UVS) = make_vec2(V_FATTR(A_UVS));
	}

	FSHADER skeletal_fs(FS_IN, FS_OUT) {
		using namespace glm; using namespace RendererSWConstants;

		TEXTURE(DIFFUSEMAP_SLOT, F_FVARYING_IN(V_UVS), F_COL_OUT);
        *(vec3 *)F_COL_OUT *= make_vec3(F_UNIFORM(U_AMBIENT));
	}

	///////////////////////////////////////////

	VSHADER line_vs(VS_IN, VS_OUT) {
		using namespace glm; using namespace RendererSWConstants;

		*(vec4*)V_POS_OUT = make_mat4(F_UNIFORM(U_MVP)) * vec4(make_vec3(V_FATTR(A_POS)), 1);
	}

	FSHADER line_fs(FS_IN, FS_OUT) {
		using namespace glm; using namespace RendererSWConstants;

		*(vec3 *)F_COL_OUT = vec3(0, 1, 1);
	}

	///////////////////////////////////////////

	VSHADER mirror_vs(VS_IN, VS_OUT) {
		using namespace glm; using namespace RendererSWConstants;

		*(vec4 *)V_POS_OUT = make_mat4(F_UNIFORM(U_MVP)) * vec4(make_vec3(V_FATTR(A_POS)), 1);

		if (V_POS_OUT[3] > 0.001f) {
			V_POS_OUT[0] /= V_POS_OUT[3];
			V_POS_OUT[1] /= V_POS_OUT[3];
			V_POS_OUT[2] /= V_POS_OUT[3];
		}
		V_POS_OUT[3] = 1;

		*(vec2 *)V_FVARYING(V_UVS) = make_vec2(V_POS_OUT) * vec2(-0.5f, -0.5f) + vec2(0.5f, 0.5f);
	}

	FSHADER mirror_fs(FS_IN, FS_OUT) {
		using namespace glm; using namespace RendererSWConstants;

		TEXTURE(DIFFUSEMAP_SLOT, F_FVARYING_IN(V_UVS), F_COL_OUT);
		std::swap(F_COL_OUT[0], F_COL_OUT[2]); // framebuffer is in bgra format
	}

	///////////////////////////////////////////

	VSHADER matcap_vs(VS_IN, VS_OUT) {
		using namespace glm; using namespace RendererSWConstants;

		*(vec4 *)V_POS_OUT = make_mat4(F_UNIFORM(U_MVP)) * vec4(make_vec3(V_FATTR(A_POS)), 1);

		vec3 n = vec3(make_mat4(F_UNIFORM(U_MV)) * vec4(make_vec3(V_FATTR(A_NORMAL)), 0));
		*(vec2 *)V_FVARYING(V_UVS) = vec2(n) * 0.5f + vec2(0.5f, 0.5f);
	}

	FSHADER matcap_fs(FS_IN, FS_OUT) {
		using namespace glm; using namespace RendererSWConstants;

		TEXTURE(DIFFUSEMAP_SLOT, F_FVARYING_IN(V_UVS), F_COL_OUT);
	}

    ///////////////////////////////////////////

    VSHADER matcap_skel_vs(VS_IN, VS_OUT) {
        using namespace glm; using namespace RendererSWConstants;

        //*(vec4 *)V_POS_OUT = make_mat4(F_UNIFORM(U_MVP)) * vec4(make_vec3(V_FATTR(A_POS)), 1);

        //mat4 mat = *(((mat4 *)F_UNIFORM(U_MAT_PALETTE) + (int)V_FATTR(A_INDICES)[0]));

        mat4 mat = ((mat4 *) F_UNIFORM(U_MAT_PALETTE))[(int) V_FATTR(A_INDICES)[0]] * V_FATTR(A_WEIGHTS)[0];
        for (int i = 1; i < 4; i++) {
            if (V_FATTR(A_WEIGHTS)[i] > 0)
                mat += ((mat4 *) F_UNIFORM(U_MAT_PALETTE))[(int) V_FATTR(A_INDICES)[i]] * V_FATTR(A_WEIGHTS)[i];
        }

        *(vec4*)V_POS_OUT =
                (make_mat4(F_UNIFORM(U_MVP)) * mat) * vec4(make_vec3(V_FATTR(A_POS)), 1);


        vec3 n = vec3(make_mat4(F_UNIFORM(U_MV)) * mat * vec4(make_vec3(V_FATTR(A_NORMAL)), 0));
        *(vec2 *)V_FVARYING(V_UVS) = vec2(n) * 0.5f + vec2(0.5f, 0.5f);
    }

    FSHADER matcap_skel_fs(FS_IN, FS_OUT) {
        using namespace glm; using namespace RendererSWConstants;

        TEXTURE(DIFFUSEMAP_SLOT, F_FVARYING_IN(V_UVS), F_COL_OUT);
    }
}

void Renderer::Init() {
	using namespace RendererSWConstants;

	{	line_program_ = R::CreateProgramSW("line", (void *)line_vs, (void *)line_fs, 0);
		R::AttrUnifArg unifs[] = { { "mvp", U_MVP, SW_MAT4 }, {} },
					   attribs[] = { { "pos", A_POS, SW_VEC3 }, {} };
		R::RegisterUnifAttrs(line_program_, unifs, attribs);
	}

	{	mirror_program_ = R::CreateProgramSW("mirror", (void *)mirror_vs, (void *)mirror_fs, 2);
		R::AttrUnifArg unifs[] = { { "mvp", U_MVP, SW_MAT4 }, {} },
					   attribs[] = { { "pos", A_POS, SW_VEC3 }, { "uvs", A_UVS, SW_VEC2 }, {} };
		R::RegisterUnifAttrs(mirror_program_, unifs, attribs);
	}

    {	matcap_program_ = R::CreateProgramSW("matcap", (void *)matcap_vs, (void *)matcap_fs, 2);
        R::AttrUnifArg unifs[] = { { "mvp", U_MVP, SW_MAT4 }, { "mvm", U_MV, SW_MAT4 }, {} },
					   attribs[] = { { "pos", A_POS, SW_VEC3 }, { "n", A_NORMAL, SW_VEC3 }, {} };
        R::RegisterUnifAttrs(matcap_program_, unifs, attribs);
    }

    {	matcap_skel_program_ = R::CreateProgramSW("matcap_skel", (void *)matcap_skel_vs, (void *)matcap_skel_fs, 2);
        R::AttrUnifArg unifs[] = { { "mvp", U_MVP, SW_MAT4 }, { "mvm", U_MV, SW_MAT4 }, { "mat_palette", U_MAT_PALETTE, SW_MAT4, MAT_PALETTE_SIZE}, {} },
                       attribs[] = { { "pos", A_POS, SW_VEC3 }, { "n", A_NORMAL, SW_VEC3 }, {} };
        R::RegisterUnifAttrs(matcap_skel_program_, unifs, attribs);
    }
}

Renderer::FrameBuf::FrameBuf(int _w, int _h) : w(_w), h(_h) {
	fb = swCreateFramebuffer(SW_BGRA8888, w, h, 1);
	LOGI("Framebuffer created (%ix%i)", w, h);
}

Renderer::FrameBuf::~FrameBuf() {
	if (fb != -1) {
		swDeleteFramebuffer(fb);
	}
}

Renderer::FrameBuf::FrameBuf(FrameBuf &&rhs) {
	*this = std::move(rhs);
}

Renderer::FrameBuf &Renderer::FrameBuf::operator=(FrameBuf &&rhs) {
	if (fb != -1) {
		swDeleteFramebuffer(fb);
	}
	w = rhs.w;		rhs.w = 0;
	h = rhs.h;		rhs.h = 0;
	fb = rhs.fb;	rhs.fb = -1;

	return *this;
}

void Renderer::FrameBuf::ClearDepth() {
	MakeCurrent _(*this);
	swClearDepth(1);
}

Renderer::FrameBuf::MakeCurrent::MakeCurrent(const FrameBuf &f) {
	prev = swGetCurFramebuffer();
	swBindFramebuffer(f.fb);
}

Renderer::FrameBuf::MakeCurrent::~MakeCurrent() {
	swBindFramebuffer(prev);
}

void Renderer::DebugFramebuffer(int x, int y, const FrameBuf &f) {
	const SWubyte *pixels = (const SWubyte *)swGetPixelDataRef(f.fb);
	
	std::vector<SWubyte> rgb_pixels(f.w * f.h * 3);
	for (int i = 0; i < f.w * f.h; i++) {
		rgb_pixels[i * 3] = pixels[i * 4 + 2];
		rgb_pixels[i * 3 + 1] = pixels[i * 4 + 1];
		rgb_pixels[i * 3 + 2] = pixels[i * 4];
	}

	SWint tex = swCreateTexture();
	R::BindTexture(0, tex);
	swTexImage2D(SW_RGB, SW_UNSIGNED_BYTE, f.w, f.h, &rgb_pixels[0]);

	swBlitTexture(x, y, 1);

	swDeleteTexture(tex);
}

// global
void LoadSWProgram(const char *name) {
	using namespace RendererSWConstants;

	if (strcmp(name, "environment") == 0) {
		R::ProgramRef p_ref = R::CreateProgramSW("environment", (void *)environment_vs, (void *)environment_fs, 2);

		R::AttrUnifArg unifs[] = { { "mvp", U_MVP, SW_MAT4 },
                                   { "ambient", U_AMBIENT, SW_VEC3 },
                                   { "flashlight_pos", U_FLASHLIGHT_POS, SW_VEC4 }, {} },
					   attribs[] = { { "pos", A_POS, SW_VEC3 }, { "uvs", A_UVS, SW_VEC3 }, {} };
		R::RegisterUnifAttrs(p_ref, unifs, attribs);
	} else if (strcmp(name, "static") == 0) {
		R::ProgramRef p_ref = R::CreateProgramSW("static", (void *)static_vs, (void *)static_fs, 2);

		R::AttrUnifArg unifs[] = { { "mvp", U_MVP, SW_MAT4 },
								   { "ambient", U_AMBIENT, SW_VEC3 }, {} },
					   attribs[] = { { "pos", A_POS, SW_VEC3 }, {} };
		R::RegisterUnifAttrs(p_ref, unifs, attribs);
	} else if (strcmp(name, "skeletal") == 0) {
		R::ProgramRef p_ref = R::CreateProgramSW("skeletal", (void *)skeletal_vs, (void *)skeletal_fs, 2);

		R::AttrUnifArg unifs[] = { { "mvp", U_MVP, SW_MAT4 },
								   { "mvm", U_MV, SW_MAT4 },
								   { "mat_palette", U_MAT_PALETTE, SW_MAT4, MAT_PALETTE_SIZE },
                                   { "ambient", U_AMBIENT, SW_VEC3 }, {} },
					   attribs[] = { { "pos", A_POS, SW_VEC3 },
									 { "uvs", A_UVS, SW_VEC2 }, {} };
		R::RegisterUnifAttrs(p_ref, unifs, attribs);
	}
}