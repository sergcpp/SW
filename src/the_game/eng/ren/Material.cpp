#include "Material.h"

#include <cstdio>
#include <cstdlib>
#include <cstring>

#include <string>

#include "SparseArray.h"

#ifdef _MSC_VER
#pragma warning(disable : 4996)
#endif

namespace R {
    SparseArray<Material> materials;
}

R::MaterialRef R::LoadMaterial(const char *name, const char *mat_src, eMatLoadStatus *status, program_load_callback on_program_load, texture_load_callback on_tex_load) {
	auto it = materials.begin();
	for (; it != materials.end(); it++) {
		if (strcmp(name, it->name) == 0) {
			break;
		}
	}
	if (it != materials.end() &&
		//will not be initialized
		!(it->not_ready && mat_src)) {
		it->counter++;

		if (status) *status = MatFound;

		MaterialRef ref;
		ref.index = (int) it.index();

		return ref;
	} else {
		if (it == materials.end()) {
			// TODO: point to standart mat
			Material m;
			strcpy(m.name, name);
			m.counter = 0;
			m.flags = 0;

			size_t index = materials.Add(m);
			it = materials.it_at(index);
		}

		it->counter++;

		if (!mat_src) {
			it->not_ready = 1;

			MaterialRef ref;
			ref.index = (int) it.index();

			if (status) *status = MatSetToDefault;

			return ref;
		}

		// Parse material
		const char *delims = " \r\n";
		const char *p = mat_src;
		const char *q = strpbrk(p + 1, delims);

		int num_textures = 0;
		int num_params = 0;

		for (; p != NULL && q != NULL; q = strpbrk(p, delims)) {
			if (p == q) {
				p = q + 1;
				continue;
			}
			std::string item(p, q);

			if (item == "gl_program:") {
#ifdef USE_GL_RENDER
                p = q + 1; q = strpbrk(p, delims);
                std::string program_name = std::string(p, q);
                p = q + 1; q = strpbrk(p, delims);
                std::string v_shader_name = std::string(p, q);
                p = q + 1; q = strpbrk(p, delims);
                std::string f_shader_name = std::string(p, q);

                eProgramLoadStatus st;
                it->program = LoadProgramGLSL(program_name.c_str(), nullptr, nullptr, &st);
                if (st == ProgSetToDefault) {
                    on_program_load(program_name.c_str(), v_shader_name.c_str(), f_shader_name.c_str());
                }
#endif
			} else if (item == "sw_program:") {
#ifdef USE_SW_RENDER
                p = q + 1; q = strpbrk(p, delims);
                std::string program_name = std::string(p, q);

                eProgramLoadStatus st;
                it->program = CreateProgramSW(program_name.c_str(), nullptr, nullptr, 0, &st);
                if (st == ProgSetToDefault) {
                    on_program_load(program_name.c_str(), nullptr, nullptr);
                }
#endif
			} else if (item == "flag:") {
				p = q + 1; q = strpbrk(p, delims);
				std::string flag = std::string(p, q);

				if (flag == "alpha_blend") {
					it->flags |= AlphaBlend;
				} else if (flag == "doublesided") {
					it->flags |= DoubleSided;
				} else {
					fprintf(stderr, "Unknown flag %s", flag.c_str());
				}
			} else if (item == "texture:") {
				p = q + 1; q = strpbrk(p, delims);
				std::string texture_name = std::string(p, q);

				eTexLoadStatus st;
				it->textures[num_textures] = LoadTexture2D(texture_name.c_str(), nullptr, &st, Trilinear, Repeat);
				if (st == TexCreatedDefault) {
					on_tex_load(texture_name.c_str());
				}
				num_textures++;
			} else if (item == "param:") {
				glm::vec4 &par = it->params[num_params++];
				p = q + 1; q = strpbrk(p, delims);
				par[0] = (float) atof(p);
				p = q + 1; q = strpbrk(p, delims);
				par[1] = (float) atof(p);
				p = q + 1; q = strpbrk(p, delims);
				par[2] = (float) atof(p);
				p = q + 1; q = strpbrk(p, delims);
				par[3] = (float)atof(p);
			}

			if (!q) break;
			p = q + 1;
		}

		it->not_ready = 0;
		if (status) *status = MatCreatedFromData;

		MaterialRef ref;
		ref.index = (int)it.index();

		return ref;
	}
}

R::Material *R::GetMaterial(const MaterialRef &ref) {
	return materials.Get((size_t)ref.index);
}

void R::ReleaseMaterial(MaterialRef &ref) {
	if (ref.index == -1) {
		return;
	}
	Material *m = materials.Get((size_t)ref.index);
	if (!--m->counter) {
		materials.Remove((size_t)ref.index);
	}
	ref.index = -1;
}

int R::NumMaterialsNotReady() {
	int num = 0;
	for (auto &m : materials) {
		if (m.not_ready) {
			num++;
		}
	}
	return num;
}

void R::ReleaseAllMaterials() {
	if (!materials.Size()) return;
	fprintf(stderr, "---------REMAINING MATERIALS--------\n");
	for (auto &m : materials) {
		fprintf(stderr, "%s (%i)\n", m.name, m.counter);
	}
	fprintf(stderr, "-----------------------------------\n");
	materials.Clear();
}
