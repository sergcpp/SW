#include "Mesh.h"

#include <cstdio>
#include <cstdlib>
#include <ctime>

#include <algorithm>
#include <array>

#define GLM_ENABLE_EXPERIMENTAL
#define GLM_FORCE_RADIANS
#include <glm/gtx/quaternion.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include "SparseArray.h"

#if defined(USE_GL_RENDER)
	#include "GL.h"
#elif defined(USE_SW_RENDER)
	#include "SW/SW.h"
#endif

#ifdef _MSC_VER
#pragma warning(disable : 4996)
#endif

namespace R {
	SparseArray<Mesh> meshes(64);

	int max_uniform_vec4	= 0;
	int max_gpu_bones		= 0;
}

R::MeshRef R::LoadMesh(const char *name, void *data, material_load_callback on_mat_load) {
	auto it = meshes.begin();
	for (; it != meshes.end(); it++) {
		if (strcmp(name, it->name) == 0) {
			break;
		}
	}
	if (it != meshes.end()) {
		it->counter++;

		MeshRef ref;
		ref.index = (int) it.index();
        ref.type = it->type;
		ref.flags = it->flags;

		return ref;
	} else {
		Mesh m;
		m.counter = 1;
		strcpy(m.name, name);

		char mesh_type_str[12];
		memcpy(mesh_type_str, data, 12);

		if (strcmp(mesh_type_str, "STATIC_MESH\0") == 0) {
			InitMeshSimple(m, data, on_mat_load);
		} else if (strcmp(mesh_type_str, "TERRAI_MESH\0") == 0) {
			InitMeshTerrain(m, data, on_mat_load);
		} else if (strcmp(mesh_type_str, "SKELET_MESH\0") == 0) {
			InitMeshSkeletal(m, data, on_mat_load);
		}

		MeshRef ref;
        ref.type = m.type;
		ref.flags = m.flags;
		ref.index = (int)meshes.Add(m);

		return ref;
	}
}

R::Mesh *R::GetMesh(const struct MeshRef &ref) {
    assert(ref.index != -1);
	return meshes.Get((size_t)ref.index);
}

void R::ReleaseMesh(Mesh &m) {
	if (m.type != MeshUndefined) {
#if defined(USE_GL_RENDER)
		GLuint buf = m.attribs_buf_id;
		glDeleteBuffers(1, &buf);
		buf = m.indices_buf_id;
		glDeleteBuffers(1, &buf);
#elif defined(USE_SW_RENDER)
        SWint buf = (SWint)m.attribs_buf_id;
        swDeleteBuffer(buf);
        buf = (SWint)m.indices_buf_id;
        swDeleteBuffer(buf);
#endif
	}

	free(m.attribs);
	m.attribs = nullptr;
	free(m.indices);
	m.indices = nullptr;

	m.type = MeshUndefined;
	m.name[0] = '\0';
}

void R::ReleaseMesh(MeshRef &ref) {
	if (ref.index == -1) {
		return;
	}
	Mesh *m = meshes.Get((size_t)ref.index);
	if (!--m->counter) {
		ReleaseMesh(*m);
		meshes.Remove((size_t)ref.index);
	}
	ref.index = -1;
    ref.type = MeshUndefined;
	ref.flags = 0;
}

void R::ReleaseAllMeshes() {
    if (!meshes.Size()) return;
    fprintf(stderr, "----------REMAINING MESHES---------\n");
    for (auto &m : meshes) {
        fprintf(stderr, "%s (%i)\n", m.name, m.counter);
    }
    fprintf(stderr, "-----------------------------------\n");
    meshes.Clear();
}

#define READ_ADVANCE(dest, p, size) memcpy(dest, p, size); p += size;

void R::InitMeshSimple(Mesh &m, void *data, material_load_callback on_mat_load) {
	char *p = (char *)data;

	char mesh_type_str[12];
	READ_ADVANCE(mesh_type_str, p, 12);
	assert(strcmp(mesh_type_str, "STATIC_MESH\0") == 0);

	m.type = MeshSimple;

	enum {	MESH_INFO_CHUNK = 0,
			VTX_ATTR_CHUNK,
			VTX_NDX_CHUNK,
			MATERIALS_CHUNK,
			STRIPS_CHUNK };

	struct ChunkPos {
		int offset;
		int length;
	};

	struct Header {
		int num_chunks;
		ChunkPos p[5];
	} file_header;

	READ_ADVANCE(&file_header, p, sizeof(file_header));

	// Skip name, cant remember why i put it there
	p += 32;

	READ_ADVANCE(&m.bbox_min[0], p, sizeof(glm::vec3));
	READ_ADVANCE(&m.bbox_max[0], p, sizeof(glm::vec3));

	m.attribs_size = (size_t) file_header.p[VTX_ATTR_CHUNK].length;
	m.attribs = malloc(m.attribs_size);
	READ_ADVANCE(m.attribs, p, m.attribs_size);

	m.indices_size = (size_t) file_header.p[VTX_NDX_CHUNK].length;
	m.indices = malloc(m.indices_size);
	READ_ADVANCE(m.indices, p, m.indices_size);

	std::vector<std::array<char, 64>> material_names((size_t) file_header.p[MATERIALS_CHUNK].length/64);
	for (auto &n : material_names) {
		READ_ADVANCE(&n[0], p, 64);
	}

	m.flags = 0;

	int num_strips = file_header.p[STRIPS_CHUNK].length / 12;
	for (int i = 0; i < num_strips; i++) {
		int index, num_indices, alpha;
		READ_ADVANCE(&index, p, 4);
		READ_ADVANCE(&num_indices, p, 4);
		READ_ADVANCE(&alpha, p, 4);

		m.strips[i].offset = (int)(index * sizeof(unsigned short));
		m.strips[i].num_indices = (int)num_indices;
		m.strips[i].flags = 0;

		if (alpha) {
			m.strips[i].flags |= MeshHasAlpha;
			m.flags |= MeshHasAlpha;
		}
		R::eMatLoadStatus status;
		m.strips[i].mat = R::LoadMaterial(&material_names[i][0], nullptr, &status, nullptr, nullptr);
		if (status == R::MatSetToDefault) {
			on_mat_load(&material_names[i][0]);
		}
	}

	if (num_strips < sizeof(m.strips) / sizeof(TriStrip)) {
		m.strips[num_strips].offset = -1;
	}

#if defined(USE_GL_RENDER)
	glGenBuffers(1, &m.attribs_buf_id);
	glBindBuffer(GL_ARRAY_BUFFER, m.attribs_buf_id);
	glBufferData(GL_ARRAY_BUFFER, m.attribs_size, m.attribs, GL_STATIC_DRAW);

	glGenBuffers(1, &m.indices_buf_id);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m.indices_buf_id);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, m.indices_size, m.indices, GL_STATIC_DRAW);
#elif defined(USE_SW_RENDER)
    m.attribs_buf_id = (uint32_t)swCreateBuffer();
    swBindBuffer(SW_ARRAY_BUFFER, m.attribs_buf_id);
    swBufferData(SW_ARRAY_BUFFER, (SWuint)m.attribs_size, m.attribs);

    m.indices_buf_id = (uint32_t)swCreateBuffer();
    swBindBuffer(SW_INDEX_BUFFER, m.indices_buf_id);
    swBufferData(SW_INDEX_BUFFER, (SWuint)m.indices_size, m.indices);
#endif

}

void R::InitMeshTerrain(Mesh &m, void *data, material_load_callback on_mat_load) {
	char *p = (char *)data;

	char mesh_type_str[12];
	READ_ADVANCE(mesh_type_str, p, 12);
	assert(strcmp(mesh_type_str, "TERRAI_MESH\0") == 0);

	m.type = MeshTerrain;

	enum {	MESH_INFO_CHUNK = 0,
			VTX_ATTR_CHUNK,
			VTX_NDX_CHUNK,
			MATERIALS_CHUNK,
			STRIPS_CHUNK };

	struct ChunkPos {
		int offset;
		int length;
	};

	struct Header {
		int num_chunks;
		ChunkPos p[5];
	} file_header;

	READ_ADVANCE(&file_header, p, sizeof(file_header));

	// Skip name, cant remember why i put it there
	p += 32;

	READ_ADVANCE(&m.bbox_min[0], p, sizeof(glm::vec3));
	READ_ADVANCE(&m.bbox_max[0], p, sizeof(glm::vec3));

	m.attribs_size = file_header.p[VTX_ATTR_CHUNK].length + file_header.p[VTX_NDX_CHUNK].length * sizeof(float);
	m.attribs = malloc(m.attribs_size);

	float *p_fattrs = (float *) m.attribs;
	for (int i = 0; i < file_header.p[VTX_NDX_CHUNK].length; i++) {
		READ_ADVANCE(&p_fattrs[i * 9], p, 8 * sizeof(float));
	}

	for (int i = 0; i < file_header.p[VTX_NDX_CHUNK].length; i++) {
		unsigned char c;
		READ_ADVANCE(&c, p, 1);
		p_fattrs[i * 9 + 8] = float(c);
	}

	m.indices_size = (size_t) file_header.p[VTX_NDX_CHUNK].length;
	m.indices = malloc(m.indices_size);
	READ_ADVANCE(m.indices, p, m.indices_size);

	std::vector<std::array<char, 64>> material_names((size_t) file_header.p[MATERIALS_CHUNK].length / 64);
	for (auto &n : material_names) {
		READ_ADVANCE(&n[0], p, 64);
	}

	m.flags = 0;

	int num_strips = file_header.p[STRIPS_CHUNK].length / 12;
	for (int i = 0; i < num_strips; i++) {
		int index, num_indices, alpha;
		READ_ADVANCE(&index, p, 4);
		READ_ADVANCE(&num_indices, p, 4);
		READ_ADVANCE(&alpha, p, 4);

		m.strips[i].offset = (int)index * sizeof(unsigned short);
		m.strips[i].num_indices = (int)num_indices;
		m.strips[i].flags = 0;

		if (alpha) {
			m.strips[i].flags |= MeshHasAlpha;
			m.flags |= MeshHasAlpha;
		}
		R::eMatLoadStatus status;
		m.strips[i].mat = R::LoadMaterial(&material_names[i][0], nullptr, &status, nullptr, nullptr);
		if (status == R::MatSetToDefault) {
			on_mat_load(&material_names[i][0]);
		}
	}

	if (num_strips < sizeof(m.strips) / sizeof(TriStrip)) {
		m.strips[num_strips].offset = -1;
	}

#if defined(USE_GL_RENDER)
	glGenBuffers(1, &m.attribs_buf_id);
	glBindBuffer(GL_ARRAY_BUFFER, m.attribs_buf_id);
	glBufferData(GL_ARRAY_BUFFER, m.attribs_size, m.attribs, GL_STATIC_DRAW);

	glGenBuffers(1, &m.indices_buf_id);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m.indices_buf_id);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, m.indices_size, m.indices, GL_STATIC_DRAW);
#elif defined(USE_SW_RENDER)
    m.attribs_buf_id = (uint32_t)swCreateBuffer();
    swBindBuffer(SW_ARRAY_BUFFER, m.attribs_buf_id);
    swBufferData(SW_ARRAY_BUFFER, (SWuint)m.attribs_size, m.attribs);

    m.indices_buf_id = (uint32_t)swCreateBuffer();
    swBindBuffer(SW_INDEX_BUFFER, m.indices_buf_id);
    swBufferData(SW_INDEX_BUFFER, (SWuint)m.indices_size, m.indices);
#endif

}

void R::InitMeshSkeletal(Mesh &m, void *data, material_load_callback on_mat_load) {
    char *p = (char *)data;

    char mesh_type_str[12];
    READ_ADVANCE(mesh_type_str, p, 12);
    assert(strcmp(mesh_type_str, "SKELET_MESH\0") == 0);

    m.type = MeshSkeletal;

    enum {  MESH_INFO_CHUNK = 0,
            VTX_ATTR_CHUNK,
            VTX_NDX_CHUNK,
            MATERIALS_CHUNK,
            STRIPS_CHUNK,
            BONES_CHUNK  };

    struct ChunkPos {
        int offset;
        int length;
    };

    struct Header {
        int num_chunks;
        ChunkPos p[6];
    } file_header;

    READ_ADVANCE(&file_header, p, sizeof(file_header));

    // Skip name, cant remember why i put it there
    p += 32;

    READ_ADVANCE(&m.bbox_min[0], p, sizeof(glm::vec3));
    READ_ADVANCE(&m.bbox_max[0], p, sizeof(glm::vec3));

    m.attribs_size = (size_t) file_header.p[VTX_ATTR_CHUNK].length;
    m.attribs = malloc(m.attribs_size);
    READ_ADVANCE(m.attribs, p, m.attribs_size);

    m.indices_size = (size_t) file_header.p[VTX_NDX_CHUNK].length;
    m.indices = malloc(m.indices_size);
    READ_ADVANCE(m.indices, p, m.indices_size);

    std::vector<std::array<char, 64>> material_names((size_t) file_header.p[MATERIALS_CHUNK].length/64);
    for (auto &n : material_names) {
        READ_ADVANCE(&n[0], p, 64);
    }

    m.flags = 0;

    int num_strips = file_header.p[STRIPS_CHUNK].length / 12;
    for (int i = 0; i < num_strips; i++) {
        int index, num_indices, alpha;
        READ_ADVANCE(&index, p, 4);
        READ_ADVANCE(&num_indices, p, 4);
        READ_ADVANCE(&alpha, p, 4);

        m.strips[i].offset = (int)index * sizeof(unsigned short);
        m.strips[i].num_indices = (int)num_indices;
        m.strips[i].flags = 0;

        if (alpha) {
            m.strips[i].flags |= MeshHasAlpha;
            m.flags |= MeshHasAlpha;
        }
        R::eMatLoadStatus status;
        m.strips[i].mat = R::LoadMaterial(&material_names[i][0], nullptr, &status, nullptr, nullptr);
        if (status == R::MatSetToDefault) {
            on_mat_load(&material_names[i][0]);
        }
    }

    if (num_strips < sizeof(m.strips) / sizeof(TriStrip)) {
        m.strips[num_strips].offset = -1;
    }

    auto &bones = m.skel.bones;

    int num_bones = file_header.p[BONES_CHUNK].length / (64 + 8 + 12 + 16);
    bones.resize((size_t) num_bones);
    for (int i = 0; i < num_bones; i++) {
        glm::vec3 temp_v;
        glm::quat temp_q;
        READ_ADVANCE(bones[i].name, p, 64);
        const char *cc = bones[i].name;
        READ_ADVANCE(&bones[i].id, p, sizeof(int));
        READ_ADVANCE(&bones[i].parent_id, p, sizeof(int));

        READ_ADVANCE(&temp_v[0], p, sizeof(glm::vec3));
        bones[i].bind_matrix = glm::translate(bones[i].bind_matrix, temp_v);
        READ_ADVANCE(&temp_q[0], p, sizeof(glm::quat));
        bones[i].bind_matrix *= glm::toMat4(temp_q);
        bones[i].inv_bind_matrix = glm::inverse(bones[i].bind_matrix);

        if (bones[i].parent_id != -1) {
            bones[i].cur_matrix = bones[bones[i].parent_id].inv_bind_matrix * bones[i].bind_matrix;
            bones[i].head_pos = glm::vec3(bones[bones[i].parent_id].inv_bind_matrix * bones[i].bind_matrix[3]);
        } else {
            bones[i].cur_matrix = bones[i].bind_matrix;
            bones[i].head_pos = glm::vec3(bones[i].bind_matrix[3]);
        }
        bones[i].cur_comb_matrix = bones[i].cur_matrix;
        bones[i].dirty = true;
    }

    assert(max_gpu_bones);
    if (bones.size() <= (size_t) R::max_gpu_bones) {
        for (size_t s = 0; s < m.strips.size(); s++) {
            if (m.strips[s].offset == -1) break;
            R::BoneGroup grp;
            for (size_t i = 0; i < bones.size(); i++) {
                grp.bone_ids.push_back(i);
            }
            grp.strip_ids.push_back(s);
            grp.strip_ids.push_back(m.strips[s].offset / 2);
            grp.strip_ids.push_back(m.strips[s].num_indices);
            m.skel.bone_groups.push_back(grp);
        }
    } else {
        R::SplitMesh(m, R::max_gpu_bones);
    }

    m.skel.matr_palette.resize(m.skel.bones.size());

#if defined(USE_GL_RENDER)
    glGenBuffers(1, &m.attribs_buf_id);
    glBindBuffer(GL_ARRAY_BUFFER, m.attribs_buf_id);
    glBufferData(GL_ARRAY_BUFFER, m.attribs_size, m.attribs, GL_STATIC_DRAW);

    glGenBuffers(1, &m.indices_buf_id);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m.indices_buf_id);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, m.indices_size, m.indices, GL_STATIC_DRAW);
#elif defined(USE_SW_RENDER)
    m.attribs_buf_id = (uint32_t)swCreateBuffer();
    swBindBuffer(SW_ARRAY_BUFFER, m.attribs_buf_id);
    swBufferData(SW_ARRAY_BUFFER, (SWuint)m.attribs_size, m.attribs);

    m.indices_buf_id = (uint32_t)swCreateBuffer();
    swBindBuffer(SW_INDEX_BUFFER, m.indices_buf_id);
    swBufferData(SW_INDEX_BUFFER, (SWuint)m.indices_size, m.indices);
#endif

}

void R::SplitMesh(Mesh &m, int bones_limit) {
    assert(m.type == MeshSkeletal);

    std::vector<int> bone_ids;
    bone_ids.reserve(12);

    float *vtx_attribs          = (float *) m.attribs;
    size_t num_vtx_attribs      = m.attribs_size / sizeof(float);
    unsigned short *vtx_indices = (unsigned short *) m.indices;
    size_t num_vtx_indices      = m.indices_size / sizeof(unsigned short);

    auto t1 = clock();

    for (size_t s = 0; s < m.strips.size(); s++) {
        if (m.strips[s].offset == -1) break;
        for (int i = (int) m.strips[s].offset / 2; i < (int) (m.strips[s].offset / 2 + m.strips[s].num_indices - 2); i += 1) {
            bone_ids.clear();
            if (vtx_indices[i] == vtx_indices[i + 1] || vtx_indices[i + 1] == vtx_indices[i + 2]) {
                continue;
            }
            for (int j = i; j < i + 3; j++) {
                for (int k = 8; k < 12; k += 1) {
                    if (vtx_attribs[vtx_indices[j] * 16 + k + 4] > 0.00001f) {
                        if (std::find(bone_ids.begin(), bone_ids.end(), (int) vtx_attribs[vtx_indices[j] * 16 + k]) ==
                            bone_ids.end()) {
                            bone_ids.push_back((int) vtx_attribs[vtx_indices[j] * 16 + k]);
                        }
                    }
                }
            }
            R::BoneGroup *best_fit = nullptr;
            int best_k = std::numeric_limits<int>::max();
            for (auto &g : m.skel.bone_groups) {
                bool b = true;
                int k = 0;
                for (size_t j = 0; j < bone_ids.size(); j++) {
                    if (std::find(g.bone_ids.begin(), g.bone_ids.end(), bone_ids[j]) == g.bone_ids.end()) {
                        k++;
                        if (g.bone_ids.size() + k > (size_t) bones_limit) {
                            b = false;
                            break;
                        }
                    }
                }
                if (b && k < best_k) {
                    best_k = k;
                    best_fit = &g;
                }
            }

            if (!best_fit) {
                m.skel.bone_groups.emplace_back();
                best_fit = &m.skel.bone_groups[m.skel.bone_groups.size() - 1];
            }
            for (size_t j = 0; j < bone_ids.size(); j++) {
                if (std::find(best_fit->bone_ids.begin(), best_fit->bone_ids.end(), bone_ids[j]) ==
                    best_fit->bone_ids.end()) {
                    best_fit->bone_ids.push_back(bone_ids[j]);
                }
            }
            if (!best_fit->strip_ids.empty() && s == best_fit->strip_ids[best_fit->strip_ids.size() - 3] &&
                best_fit->strip_ids[best_fit->strip_ids.size() - 2] +
                best_fit->strip_ids[best_fit->strip_ids.size() - 1] - 0 == i) {
                best_fit->strip_ids[best_fit->strip_ids.size() - 1]++;
            } else {
                best_fit->strip_ids.push_back(s);
                if ((i - m.strips[s].offset / 2) % 2 == 0) {
                    best_fit->strip_ids.push_back(i);
                } else {
                    best_fit->strip_ids.push_back(-i);
                }
                best_fit->strip_ids.push_back(1);
            }
        }
    }

    printf("%li\n", clock() - t1);
    t1 = clock();

    std::vector<unsigned short> new_indices;
    new_indices.reserve((size_t) (num_vtx_indices * 1.2f));
    for (auto &g : m.skel.bone_groups) {
        std::sort(g.bone_ids.begin(), g.bone_ids.end());
        int cur_s = g.strip_ids[0];

        for (size_t i = 0; i < g.strip_ids.size(); i += 3) {
            bool sign = g.strip_ids[i + 1] >= 0;
            if (!sign) {
                g.strip_ids[i + 1] = -g.strip_ids[i + 1];
            }
            int beg = g.strip_ids[i + 1];
            int end = g.strip_ids[i + 1] + g.strip_ids[i + 2] + 2;

            if (g.strip_ids[i] == cur_s && i > 0) {
                new_indices.push_back(new_indices.back());
                new_indices.push_back(vtx_indices[beg]);
                g.strip_ids[i + 2 - 3] += 2;
                if ((!sign && (g.strip_ids[i + 2 - 3] % 2 == 0)) || (sign && (g.strip_ids[i + 2 - 3] % 2 != 0))) {
                    g.strip_ids[i + 2 - 3] += 1;
                    new_indices.push_back(vtx_indices[beg]);

                }
                g.strip_ids[i + 2 - 3] += g.strip_ids[i + 2] + 2;
                g.strip_ids.erase(g.strip_ids.begin() + i, g.strip_ids.begin() + i + 3);
                i -= 3;
            } else {
                cur_s = g.strip_ids[i];
                g.strip_ids[i + 2] += 2;
                g.strip_ids[i + 1] = (int) new_indices.size();
                if (!sign) {
                    g.strip_ids[i + 2] += 1;
                    new_indices.push_back(vtx_indices[beg]);
                }
            }
            new_indices.insert(new_indices.end(), &vtx_indices[beg], &vtx_indices[end]);
        }

    }
    new_indices.shrink_to_fit();
    printf("%li\n", clock() - t1);
    t1 = clock();

    clock_t find_time = 0;

    std::vector<bool> done_bools(num_vtx_attribs);
    std::vector<float> new_attribs(vtx_attribs, vtx_attribs + num_vtx_attribs);
    for (auto &g : m.skel.bone_groups) {
        std::vector<int> moved_points;
        for (size_t i = 0; i < g.strip_ids.size(); i += 3) {
            for (int j = g.strip_ids[i + 1]; j < g.strip_ids[i + 1] + g.strip_ids[i + 2]; j++) {
                if (done_bools[new_indices[j]]) {
                    if (&g - &m.skel.bone_groups[0] != 0) {
                        bool b = false;
                        for (size_t k = 0; k < moved_points.size(); k += 2) {
                            if (new_indices[j] == moved_points[k]) {
                                new_indices[j] = (unsigned short) moved_points[k + 1];
                                b = true;
                                break;
                            }
                        }
                        if (!b) {
                            new_attribs.insert(new_attribs.end(), &vtx_attribs[new_indices[j] * 16],
                                                &vtx_attribs[new_indices[j] * 16] + 16);
                            moved_points.push_back(new_indices[j]);
                            moved_points.push_back((int)new_attribs.size() / 16 - 1);
                            new_indices[j] = (unsigned short) (new_attribs.size() / 16 - 1);
                        } else {
                            continue;
                        }
                    } else {
                        continue;
                    }
                } else {
                    done_bools[new_indices[j]] = true;
                }
                for (int k = 8; k < 12; k += 1) {
                    if (new_attribs[new_indices[j] * 16 + k + 4] > 0.0f) {
                        int bone_ndx = (int)(std::find(g.bone_ids.begin(), g.bone_ids.end(),
                                            (int) new_attribs[new_indices[j] * 16 + k]) - g.bone_ids.begin());
                        new_attribs[new_indices[j] * 16 + k] = (float) bone_ndx;
                    }
                }
            }
        }
    }

    printf("---------------------------\n");
    for (auto &g : m.skel.bone_groups) {
        printf("%u\n", (unsigned int) g.strip_ids.size() / 3);
    }
    printf("---------------------------\n");

    printf("%li\n", clock() - t1);
    printf("find_time = %li\n", find_time);
    printf("after bone broups2\n");

    free(m.indices);
    m.indices_size  = new_indices.size() * sizeof(unsigned short);
    m.indices       = malloc(m.indices_size);
    memcpy(m.indices, &new_indices[0], m.indices_size);

    free(m.attribs);
    m.attribs_size  = new_attribs.size() * sizeof(float);
    m.attribs       = malloc(m.attribs_size);
    memcpy(m.attribs, &new_attribs[0], m.attribs_size);
}
