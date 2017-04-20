#ifndef MESH_H
#define MESH_H

#include <array>

#include <glm/vec3.hpp>

#include "Anim.h"
#include "Material.h"

namespace R {
	enum eMeshFlags { MeshHasAlpha = 1 };

	struct TriStrip {
		int			offset;
		int			num_indices;
		MaterialRef mat;
		uint32_t	flags;
	};

	enum eMeshType { MeshUndefined, MeshSimple, MeshTerrain, MeshSkeletal };

	struct Mesh {
		int				type;
		uint32_t		flags;
#if defined(USE_GL_RENDER) || defined(USE_SW_RENDER)
		uint32_t		attribs_buf_id;
		uint32_t		indices_buf_id;
#endif
		int 			counter;
		void 			*attribs;
		size_t			attribs_size;
		void 			*indices;
		size_t			indices_size;
        std::array<TriStrip, 16>    strips;
		glm::vec3 		bbox_min, bbox_max;
		char 			name[32];

        Skeleton        skel;

		Mesh() : type(MeshUndefined), counter(0), attribs(nullptr), indices(nullptr) {
#if defined(USE_GL_RENDER) || defined(USE_SW_RENDER)
			attribs_buf_id = indices_buf_id = 0;
#endif
		}
	};

	Mesh *GetMesh(const struct MeshRef &ref);
	void ReleaseMesh(Mesh &m);
	void ReleaseMesh(MeshRef &ref);

	struct MeshRef {
		int				index;
        int             type;
		unsigned int	flags;

		MeshRef() : index(-1) {}
		~MeshRef() {
			this->Release();
		}

		MeshRef(const MeshRef &ref) {
			index = ref.index;
            type = ref.type;
			flags = ref.flags;
			if (index != -1) {
				GetMesh(ref)->counter++;
			}
		}

		MeshRef(MeshRef &&ref) {
			index = ref.index;
			ref.index = -1;
            type = ref.type;
            ref.type = MeshUndefined;
			flags = ref.flags;
			ref.flags = 0;
		}

		MeshRef& operator=(const MeshRef &ref) {
			if (&ref != this) {
				this->Release();
				index = ref.index;
				if (index != -1) {
					GetMesh(ref)->counter++;
				}
				type = ref.type;
				flags = ref.flags;
			}
			return *this;
		}

		MeshRef& operator=(MeshRef &&ref) {
			if (&ref != this) {
				this->Release();
				index = ref.index;
				ref.index = -1;
				type = ref.type;
				ref.type = MeshUndefined;
				flags = ref.flags;
				ref.flags = 0;
			}
			return *this;
		}

		void Release() {
			ReleaseMesh(*this);
		}
	};

	typedef void (*material_load_callback) (const char *name);

	MeshRef LoadMesh(const char *name, void *data, material_load_callback on_mat_load);

    void ReleaseAllMeshes();

    // simple static mesh with normals
	void InitMeshSimple(Mesh &mesh, void *data, material_load_callback 	on_mat_load);

    // simple mesh with tex index per vertex
	void InitMeshTerrain(Mesh &mesh, void *data, material_load_callback on_mat_load);

    // mesh with 4 bone weights per vertex
	void InitMeshSkeletal(Mesh &mesh, void *data, material_load_callback on_mat_load);

    // split skeletal mesh into chunks to fit uniforms limit in shader
    void SplitMesh(Mesh &m, int bones_limit);
}

#endif // MESH_H
