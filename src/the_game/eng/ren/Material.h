#ifndef MATERIAL_H
#define MATERIAL_H

#include <glm/vec4.hpp>

#include "Texture.h"
#include "Program.h"

namespace R {
    enum eMaterialFlags { AlphaBlend = 1, DoubleSided = 2 };

    struct Material {
        int				counter;
        uint32_t		flags;
        int				not_ready;
        char			name[32];
        ProgramRef		program;
        Texture2DRef	textures[4];
        glm::vec4		params[8];
    };
    
	Material *GetMaterial(const struct MaterialRef &ref);
	void ReleaseMaterial(MaterialRef &ref);

    struct MaterialRef {
        int index;

		MaterialRef() : index(-1) {}
		~MaterialRef() {
			this->Release();
		}

		MaterialRef(const MaterialRef &ref) : index(ref.index) {
			if (index != -1) {
				GetMaterial(ref)->counter++;
			}
		}
		MaterialRef(MaterialRef &&ref) : index(ref.index) {
			ref.index = -1;
		}
		MaterialRef& operator=(const MaterialRef &ref) {
			this->Release();
			index = ref.index;
			if (index != -1) {
				GetMaterial(ref)->counter++;
			}
			return *this;
		}
		MaterialRef& operator=(MaterialRef &&ref) {
			this->Release();
			index = ref.index;
			ref.index = -1;
			return *this;
		}

		bool operator==(const MaterialRef &rhs) {
			return index == rhs.index;
		}

		void Release() {
			ReleaseMaterial(*this);
		}

		int flags() {
			return GetMaterial(*this)->flags;
		}
    };

    typedef void (*texture_load_callback) (const char *name);
    typedef void (*program_load_callback) (const char *name, const char *arg1, const char *arg2);

	enum eMatLoadStatus { MatFound, MatSetToDefault, MatCreatedFromData };

	MaterialRef LoadMaterial(const char *name, const char *mat_src, eMatLoadStatus *status,
							 program_load_callback on_program_load,
							 texture_load_callback on_tex_load);

	int NumMaterialsNotReady();
	void ReleaseAllMaterials();
}

#endif // MATERIAL_H
