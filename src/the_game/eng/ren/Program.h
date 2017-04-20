#ifndef PROGRAM_H
#define PROGRAM_H

#include <cstdint>
#include <cstring>

namespace R {
#if defined(USE_GL_RENDER) || defined(USE_SW_RENDER)
	const int MAX_NUM_ATTRIBUTES = 16;
	const int MAX_NUM_UNIFORMS = 16;
	struct Descr {
		const char *name;
		int loc;
	};
	typedef Descr Uniform;
	typedef Descr Attribute;
#endif
    struct Program {
#if defined(USE_GL_RENDER) || defined(USE_SW_RENDER)
        uint32_t	prog_id;
		Attribute	attributes[MAX_NUM_ATTRIBUTES];
		Uniform		uniforms[MAX_NUM_UNIFORMS];
#endif
		int			counter;
		int			not_ready;
        char		name[32];

#if defined(USE_GL_RENDER) || defined(USE_SW_RENDER)
		Program() : prog_id(0), counter(0), not_ready(1) {
			for (int i = 0; i < MAX_NUM_ATTRIBUTES; i++) {
				attributes[i].name = uniforms[i].name = nullptr;
				attributes[i].loc = uniforms[i].loc = -1;
			}
			for (int i = 0; i < MAX_NUM_UNIFORMS; i++) {
				uniforms[i].name = nullptr;
				uniforms[i].loc = -1;
			}
        }

		int attribute(int i) {
			return attributes[i].loc;
		}

		int attribute(const char *name) {
			for (int i = 0; i < MAX_NUM_ATTRIBUTES; i++) {
				if (strcmp(attributes[i].name, name) == 0) {
					return attributes[i].loc;
				}
			}
			return -1;
		}

		int uniform(int i) {
			return uniforms[i].loc;
		}

		int uniform(const char *name) {
			for (int i = 0; i < MAX_NUM_UNIFORMS; i++) {
				if (strcmp(uniforms[i].name, name) == 0) {
					return uniforms[i].loc;
				}
			}
			return -1;
		}
#endif
    };

    Program *GetProgram(const struct ProgramRef &ref);
	void ReleaseProgram(Program &p);
    void ReleaseProgram(ProgramRef &ref);

    struct ProgramRef {
        int index;

        ProgramRef() : index(-1) {}
        ~ProgramRef() {
            this->Release();
        }

        ProgramRef(const ProgramRef &ref) {
			index = ref.index;
            if (index != -1) {
                GetProgram(ref)->counter++;
            }
        }
        ProgramRef(ProgramRef &&ref) {
            index = ref.index;
			ref.index = -1;
        }
        ProgramRef& operator=(const ProgramRef &ref) {
			this->Release();
			index = ref.index;
            if (index != -1) {
                GetProgram(ref)->counter++;
            }
            return *this;
        }
        ProgramRef& operator=(ProgramRef &&ref) {
			this->Release();
            index = ref.index;
			ref.index = -1;
            return *this;
        }

        void Release() {
            ReleaseProgram(*this);
        }

        bool loaded() const {
#if defined(USE_GL_RENDER) || defined(USE_SW_RENDER)
			return !GetProgram(*this)->not_ready;
#endif
        }
    };

	enum eProgramLoadStatus { ProgFound, ProgSetToDefault, ProgCreatedFromData };
#if defined(USE_GL_RENDER)
	ProgramRef LoadProgramGLSL(const char *name, const char *vs_source, const char *fs_source, eProgramLoadStatus *status = nullptr);
#elif defined(USE_SW_RENDER)
	ProgramRef CreateProgramSW(const char *name, void *vshader, void *fshader, int num_fvars, eProgramLoadStatus *status = nullptr);

    struct AttrUnifArg {
		const char *name; int index, type, size;

        AttrUnifArg() : name(nullptr) {}
		AttrUnifArg(const char *_name, int _index, int _type, int _size) : name(_name), index(_index), type(_type), size(_size) {}
		AttrUnifArg(const char *_name, int _index, int _type) : name(_name), index(_index), type(_type), size(1) {}
	}; // list should end with { nullptr, #, # }
    void RegisterUnifAttrs(ProgramRef &ref, const AttrUnifArg *unifs, const AttrUnifArg *attrs);
#endif

	int NumProgramsNotReady();
    void ReleaseAllPrograms();
}

#endif // PROGRAM_H
