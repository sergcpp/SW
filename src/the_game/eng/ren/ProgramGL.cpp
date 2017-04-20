#include "Program.h"

#include <cstdlib>
#include <cstdio>
#include <cstring>

#include <array>
#include <string>

#include "GL.h"
#include "SparseArray.h"

#ifdef _MSC_VER
#pragma warning(disable : 4996)
#endif

namespace R {
	std::string glsl_defines;

	extern SparseArray<Program> programs;
	extern SparseArray<std::array<char, 256>> name_buffers;
}

namespace {
	GLuint LoadShader(GLenum shader_type, const char *source);
}


R::ProgramRef R::LoadProgramGLSL(const char *name, const char *vs_source, const char *fs_source, eProgramLoadStatus *status) {
    auto it = programs.begin();
    for (; it != programs.end(); it++) {
        if (strcmp(name, it->name) == 0) {
            break;
        }
    }
    if (it != programs.end() &&
    		// will not be initialized
    		!(it->not_ready && vs_source && fs_source)) {
        it->counter++;

        ProgramRef ref;
        ref.index = (int) std::distance(programs.begin(), it);

		if (status) *status = ProgFound;

        return std::move(ref);
	} else {
		if (it == programs.end()) {
			// TODO: point to standart program
			Program p;
			p.counter = 0;
			p.prog_id = 0;
			p.not_ready = 1;
			strcpy(p.name, name);

			size_t index = programs.Add(p);
			it = programs.it_at(index);
		}

		it->counter++;

		if (!vs_source || !fs_source) {
			it->not_ready = 1;

			ProgramRef ref;
			ref.index = (int)it.index();

			if (status) *status = ProgSetToDefault;

			return std::move(ref);
		}

        std::string vs_source_str = glsl_defines + vs_source;
        std::string fs_source_str = glsl_defines + fs_source;

		GLuint v_shader = LoadShader(GL_VERTEX_SHADER, vs_source_str.c_str());
		if (!v_shader) {
			fprintf(stderr, "VertexShader %s error", name);
		}

		GLuint f_shader = LoadShader(GL_FRAGMENT_SHADER, fs_source_str.c_str());
		if (!f_shader) {
			fprintf(stderr, "FragmentShader %s error", name);
		}

		GLuint program = glCreateProgram();
		if (program) {
			glAttachShader(program, v_shader);
			glAttachShader(program, f_shader);
			glLinkProgram(program);
			GLint link_status = GL_FALSE;
			glGetProgramiv(program, GL_LINK_STATUS, &link_status);
			if (link_status != GL_TRUE) {
				GLint buf_len = 0;
				glGetProgramiv(program, GL_INFO_LOG_LENGTH, &buf_len);
				if (buf_len) {
					char *buf = (char *)malloc((size_t)buf_len);
					if (buf) {
						glGetProgramInfoLog(program, buf_len, NULL, buf);
						fprintf(stderr, "Could not link program: %s", buf);
						free(buf);
						throw;
					}
				}
				glDeleteProgram(program);
				program = 0;
			}
		} else {
			fprintf(stderr, "error");
			throw;
		}


		// Parse attribute and uniform bindings
		struct Binding { std::string name; int loc; };
		std::vector<Binding> attr_bindings, uniform_bindings;
		std::vector<Binding> *cur_bind_target = nullptr;

		const char *delims = " \r\n\t";
		char const* p = vs_source_str.c_str() + vs_source_str.find("/*");
		char const* q = strpbrk(p + 2, delims);
		int pass = 0;

	SECOND_PASS:
		for (; p != NULL && q != NULL; q = strpbrk(p, delims)) {
			if (p == q) {
				p = q + 1;
				continue;
			}

			std::string item(p, q);
			if (item == "/*") {
				cur_bind_target = nullptr;
			} else if (item == "*/") {
				break;
			} else if (item == "ATTRIBUTES") {
				cur_bind_target = &attr_bindings;
			} else if (item == "UNIFORMS") {
				cur_bind_target = &uniform_bindings;
			} else if (cur_bind_target) {
				p = q + 1; q = strpbrk(p, delims);
				if (*p != ':') {
					fprintf(stderr, "Error parsing material %s", name);
				}
				p = q + 1; q = strpbrk(p, delims);
				int loc = atoi(p);
				cur_bind_target->push_back({ item, loc });
			}

			if (!q) break;
			p = q + 1;
		}

		if (pass++ == 0) {
			p = fs_source_str.c_str() + fs_source_str.find("/*");
			q = strpbrk(p + 1, delims);
			cur_bind_target = nullptr;
			goto SECOND_PASS;
		}

		it->prog_id = program;
		it->not_ready = 0;

		size_t index = name_buffers.Add();
		char *p_names_buf = &(*name_buffers.Get(index))[0];

		for (auto &b : attr_bindings) {
			auto &a = it->attributes[b.loc];
			a.name = p_names_buf;
			a.loc = glGetAttribLocation(program, b.name.c_str());
			if (a.loc == -1) {
				a.name = nullptr;
				continue;
			}

			strcpy(p_names_buf, b.name.c_str());
			p_names_buf += b.name.size() + 1;
		}

		for (auto &b : uniform_bindings) {
			auto &u = it->uniforms[b.loc];
			u.name = p_names_buf;
			u.loc = glGetUniformLocation(program, b.name.c_str());
			if (u.loc == -1) {
				u.name = nullptr;
			}

			strcpy(p_names_buf, b.name.c_str());
			p_names_buf += b.name.size() + 1;
		}

		// Enumerate rest of attributes
		int num;
		glGetProgramiv(program, GL_ACTIVE_ATTRIBUTES, &num);
		for (int i = 0; i < num; i++) {
			int len;
			GLenum n;
			char name[128];
			glGetActiveAttrib(program, i, 128, &len, &len, &n, name);

			int skip = 0, free_index = -1;
			for (int j = 0; j < MAX_NUM_ATTRIBUTES; j++) {
				if (free_index == -1 && it->attributes[j].loc == -1) {
					free_index = j;
				}
				if (it->attributes[j].loc != -1 && strcmp(it->attributes[j].name, name) == 0) {
					skip = 1;
					break;
				}
			}

			if (!skip && free_index != -1) {
				it->attributes[free_index].name = p_names_buf;
				it->attributes[free_index].loc = glGetAttribLocation(program, name);

				strcpy(p_names_buf, name);
				p_names_buf += len + 1;
			}
		}

		printf("PROGRAM %s\n", name);

		// Print all attributes
		printf("\tATTRIBUTES\n");
		for (int i = 0; i < MAX_NUM_ATTRIBUTES; i++) {
			if (it->attributes[i].loc == -1) {
				continue;
			}
			printf("\t\t%s : %i\n", it->attributes[i].name, it->attributes[i].loc);
		}

		// Enumerate rest of uniforms
		glGetProgramiv(program, GL_ACTIVE_UNIFORMS, &num);
		for (int i = 0; i < num; i++) {
			int len;
			GLenum n;
			char name[128];
			glGetActiveUniform(program, i, 128, &len, &len, &n, name);

			int skip = 0, free_index = -1;
			for (int j = 0; j < MAX_NUM_UNIFORMS; j++) {
				if (free_index == -1 && it->uniforms[j].loc == -1) {
					free_index = j;
				}
				if (it->uniforms[j].loc != -1 && strcmp(it->uniforms[j].name, name) == 0) {
					skip = 1;
					break;
				}
			}

			if (!skip && free_index != -1) {
				it->uniforms[free_index].name = p_names_buf;
				it->uniforms[free_index].loc = glGetUniformLocation(program, name);

				strcpy(p_names_buf, name);
				p_names_buf += len + 1;
			}
		}
		
		// Print all uniforms
		printf("\tUNIFORMS\n");
		for (int i = 0; i < MAX_NUM_UNIFORMS; i++) {
			if (it->uniforms[i].loc == -1) {
				continue;
			}
			printf("\t\t%s : %i\n", it->uniforms[i].name, it->uniforms[i].loc);
		}

        ProgramRef ref;
        ref.index = (int) it.index();

		if (status) *status = ProgCreatedFromData;

        return std::move(ref);
    }
}

namespace {
	GLuint LoadShader(GLenum shader_type, const char *source) {
		GLuint shader = glCreateShader(shader_type);
		if (shader) {
			glShaderSource(shader, 1, &source, NULL);
			glCompileShader(shader);
			GLint compiled = 0;
			glGetShaderiv(shader, GL_COMPILE_STATUS, &compiled);
			if (!compiled) {
				GLint infoLen = 0;
				glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &infoLen);
				if (infoLen) {
					char *buf = (char *)malloc((size_t)infoLen);
					if (buf) {
						glGetShaderInfoLog(shader, infoLen, NULL, buf);
						fprintf(stderr, "Could not compile shader %d: %s", shader_type, buf);
						free(buf);
					}
					glDeleteShader(shader);
					shader = 0;
				}
				throw;
			}
		} else {
			fprintf(stderr, "error");
		}

        GLint info_len = 0;
        glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &info_len);

        if (info_len) {
            char *buf = (char *) malloc((size_t) info_len);
            glGetShaderInfoLog(shader, info_len, NULL, buf);
            fprintf(stderr, "%s", buf);
        }

		return shader;
	}
}
