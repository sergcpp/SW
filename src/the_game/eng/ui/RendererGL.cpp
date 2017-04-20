#include "Renderer.h"

#include <cassert>

#include <ren/RenderState.h>
#include <sys/Json.h>

namespace UIRendererConstants {
    enum { U_COL,
           U_TEXTURE,
           U_Z_OFFSET };

	const char vs_source[] = 
		"/*\n"
		"ATTRIBUTES\n"
		"	aVertexPosition : 0\n"
		"	aVertexUVs : 1\n"
		"UNIFORMS\n"
		"	z_offset : 2\n"
		"*/\n"
		"\n"
		"uniform float z_offset;\n"
		"\n"
		"attribute vec3 aVertexPosition;\n"
		"attribute vec2 aVertexUVs;\n"
		"\n"
		"varying vec2 aVertexUVs_;\n"
		"\n"
		"void main(void) {\n"
		"    gl_Position = vec4(aVertexPosition + vec3(0.0, 0.0, z_offset), 1.0);\n"
		"    aVertexUVs_ = aVertexUVs;\n"
		"}";

	const char fs_source[] =
		"#ifdef GL_ES\n"
		"	precision mediump float;\n"
		"#else\n"
		"	#define lowp\n"
		"	#define mediump\n"
		"	#define highp\n"
		"#endif\n"
		"\n"
		"/*\n"
		"UNIFORMS\n"
		"	col : 0\n"
		"	s_texture : 1\n"
		"*/\n"
		"\n"
		"uniform vec3 col;\n"
		"uniform sampler2D s_texture;\n"
		"\n"
		"varying vec2 aVertexUVs_;\n"
		"\n"
		"void main(void) {\n"
		"	gl_FragColor = vec4(col, 1.0) * texture2D(s_texture, aVertexUVs_);\n"
		"}";
}

ui::Renderer::Renderer(const JsObject &config) {
	using namespace UIRendererConstants;
	using namespace glm;

    const JsString &js_gl_defines = (const JsString &) config.at(GL_DEFINES_KEY);

    {	// Load main shader
        R::eProgramLoadStatus status;
        ui_program_ = R::LoadProgramGLSL(UI_PROGRAM_NAME, vs_source, fs_source, &status);
        assert(status == R::ProgCreatedFromData);
    }

    glGenBuffers(1, &attribs_buf_id_);
    glBindBuffer(GL_ARRAY_BUFFER, attribs_buf_id_);
    glBufferData(GL_ARRAY_BUFFER, 0xff * 5 * sizeof(GLfloat), nullptr, GL_DYNAMIC_DRAW);

    glGenBuffers(1, &indices_buf_id_);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, indices_buf_id_);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, 0xff * sizeof(GLubyte), nullptr, GL_DYNAMIC_DRAW);
}

ui::Renderer::~Renderer() {
    glDeleteBuffers(1, &attribs_buf_id_);
    glDeleteBuffers(1, &indices_buf_id_);
}

void ui::Renderer::BeginDraw() {
	using namespace glm;

    R::Program *p = R::GetProgram(ui_program_);

    R::UseProgram(p->prog_id);

    glEnableVertexAttribArray((GLuint)p->attribute(0));
    glEnableVertexAttribArray((GLuint)p->attribute(1));

    glDisable(GL_DEPTH_TEST);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	glEnable(GL_SCISSOR_TEST);

	ivec2 scissor_test[2] = { { 0, 0 }, { R::w, R::h } };
	this->EmplaceParams(vec3(1, 1, 1), 0.0f, BL_ALPHA, scissor_test);

    glBindBuffer(GL_ARRAY_BUFFER, attribs_buf_id_);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, indices_buf_id_);
}

void ui::Renderer::EndDraw() {
    R::PrintGLError();

    glEnable(GL_DEPTH_TEST);
    glDisable(GL_BLEND);
	glDisable(GL_SCISSOR_TEST);

	this->PopParams();

    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
}

void ui::Renderer::DrawImageQuad(const R::Texture2DRef &tex, const glm::vec2 dims[2], const glm::vec2 uvs[2]) {
    using namespace UIRendererConstants;
    
    float vertices[] = {dims[0].x, dims[0].y, 0,
                        uvs[0].x, uvs[0].y,

                        dims[0].x, dims[0].y + dims[1].y, 0,
                        uvs[0].x, uvs[1].y,

                        dims[0].x + dims[1].x, dims[0].y + dims[1].y, 0,
                        uvs[1].x, uvs[1].y,

                        dims[0].x + dims[1].x, dims[0].y, 0,
                        uvs[1].x, uvs[0].y};

    unsigned char indices[] = {2, 1, 0,
                               3, 2, 0};

    glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(vertices), vertices);
    glBufferSubData(GL_ELEMENT_ARRAY_BUFFER, 0, sizeof(indices), indices);

    R::Program *p = R::GetProgram(ui_program_);

    const DrawParams &cur_params = params_.back();
    this->ApplyParams(p, cur_params);

    R::BindTexture(0, tex.tex_id);
    glUniform1i(p->uniform(U_TEXTURE), 0);

    glVertexAttribPointer((GLuint)p->attribute(0), 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void *) 0);
    glVertexAttribPointer((GLuint)p->attribute(1), 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void *) (3 * sizeof(float)));

    glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_BYTE, 0);
}

void ui::Renderer::DrawUIElement(const R::Texture2DRef &tex, ePrimitiveType prim_type,
                                 const std::vector<float> &pos, const std::vector<float> &uvs,
                                 const std::vector<unsigned char> &indices) {
    using namespace UIRendererConstants;

    assert(pos.size() / 5 < 0xff);
    if (pos.empty()) return;

    R::Program *p = R::GetProgram(ui_program_);

    const DrawParams &cur_params = params_.back();
    this->ApplyParams(p, cur_params);

    R::BindTexture(0, tex.tex_id);
    glUniform1i(p->uniform(U_TEXTURE), 0);

    glBufferSubData(GL_ARRAY_BUFFER, 0, pos.size() * sizeof(GLfloat), &pos[0]);
    glBufferSubData(GL_ARRAY_BUFFER, pos.size() * sizeof(GLfloat), uvs.size() * sizeof(GLfloat), &uvs[0]);

    glBufferSubData(GL_ELEMENT_ARRAY_BUFFER, 0, indices.size() * sizeof(GLubyte), &indices[0]);

    glVertexAttribPointer((GLuint)p->attribute(0), 3, GL_FLOAT, GL_FALSE, 0, (void *)0);
    glVertexAttribPointer((GLuint)p->attribute(1), 2, GL_FLOAT, GL_FALSE, 0, (void *)((uintptr_t)pos.size() * sizeof(GLfloat)));

    if (prim_type == PRIM_TRIANGLE) {
        glDrawElements(GL_TRIANGLES, (GLsizei)indices.size(), GL_UNSIGNED_BYTE, 0);
    }
}

void ui::Renderer::ApplyParams(R::Program *p, const DrawParams &params) {
    using namespace UIRendererConstants;

    glUniform3f(p->uniform(U_COL), params.col_[0], params.col_[1], params.col_[2]);
    glUniform1f(p->uniform(U_Z_OFFSET), params.z_val_);
    if (params.blend_mode_ == BL_ALPHA) {
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    } else if (params.blend_mode_ == BL_COLOR) {
        glBlendFunc(GL_ONE, GL_ONE_MINUS_SRC_COLOR);
    }

	if (params.scissor_test_[1].x > 0 && params.scissor_test_[1].y > 0) {
		glScissor(params.scissor_test_[0].x, params.scissor_test_[0].y,
				  params.scissor_test_[1].x, params.scissor_test_[1].y);
	}
}
