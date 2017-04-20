#include "Program.h"

#include <cstdio>

#include <array>
#include <limits>

#include "RenderState.h"
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
	SparseArray<Program> programs(16);
	SparseArray<std::array<char, 256>> name_buffers(16);

	extern uint32_t current_program;
}

void R::ReleaseProgram(Program &p) {
#if defined(USE_GL_RENDER)
	if (!p.not_ready) {
		GLuint gl_prog = p.prog_id;
		glDeleteProgram(gl_prog);
        if (current_program == p.prog_id) {
            current_program = std::numeric_limits<uint32_t>::max();
        }
		p.prog_id = 0;
	}
#elif defined(USE_SW_RENDER)
    SWint sw_prog = p.prog_id;
    swDeleteProgram(sw_prog);
    if (current_program == p.prog_id) {
        current_program = std::numeric_limits<uint32_t>::max();
    }
    p.prog_id = 0;
#endif
}

void R::ReleaseProgram(ProgramRef &ref) {
	if (ref.index == -1) {
		return;
	}
	Program *p = programs.Get((size_t)ref.index);
	if (!--p->counter) {
		ReleaseProgram(*p);
		programs.Remove((size_t)ref.index);
		name_buffers.Remove((size_t)ref.index);
	}
	ref.index = -1;
}

R::Program *R::GetProgram(const ProgramRef &ref) {
    assert(ref.index >= 0);
	return programs.Get((size_t)ref.index);
}

int R::NumProgramsNotReady() {
	int num = 0;
	for (auto &p : programs) {
		if (p.not_ready) {
			num++;
		}
	}
	return num;
}

void R::ReleaseAllPrograms() {
	if (!programs.Size()) return;
	fprintf(stderr, "---------REMAINING PROGRAMS--------\n");
	for (auto &p : programs) {
#if defined(USE_GL_RENDER) || defined(USE_SW_RENDER)
		fprintf(stderr, "%s %i (%i)\n", p.name, p.prog_id, p.counter);
#endif
        ReleaseProgram(p);

	}
	fprintf(stderr, "-----------------------------------\n");

	programs.Clear();
	name_buffers.Clear();
}


#ifdef USE_SW_RENDER
R::ProgramRef R::CreateProgramSW(const char *name, void *vshader, void *fshader, int num_fvars, eProgramLoadStatus *status) {
    auto it = programs.begin();
    for (; it != programs.end(); it++) {
        if (strcmp(name, it->name) == 0) {
            break;
        }
    }
    if (it != programs.end() &&
        // will not be initialized
        !(it->not_ready && vshader && fshader)) {
        it->counter++;

        ProgramRef ref;
        ref.index = (int) it.index();

        if (status) *status = ProgFound;

        return ref;
    } else {
        if (it == programs.end()) {
            Program p;
            p.counter = 0;
            p.prog_id = 0;
            p.not_ready = 1;
            strcpy(p.name, name);

            name_buffers.Add();

            int index = (int)programs.Add(p);
            it = programs.it_at((size_t)index);
        }

        it->counter++;

        if (!vshader || !fshader) {
            it->not_ready = 1;

            ProgramRef ref;
            ref.index = (int)it.index();

            if (status) *status = ProgSetToDefault;

            return ref;
        }

        SWint program = swCreateProgram();
        R::UseProgram(program);
        swInitProgram((vtx_shader_proc)vshader, (frag_shader_proc)fshader, num_fvars);

        it->prog_id = (uint32_t)program;
        it->not_ready = 0;

        ProgramRef ref;
        ref.index = (int) it.index();

        if (status) *status = ProgCreatedFromData;

        return ref;
    }
}

namespace {
    bool UniformUnique(R::Program *p, const char *name) {
        for (int i = 0; i < R::MAX_NUM_UNIFORMS; i++) {
            if (p->uniforms[i].name && strcmp(p->uniforms[i].name, name) == 0) {
                return false;
            }
        }
        return true;
    }
    bool AttributeUnique(R::Program *p, const char *name) {
        for (int i = 0; i < R::MAX_NUM_ATTRIBUTES; i++) {
            if (p->attributes[i].name && strcmp(p->attributes[i].name, name) == 0) {
                return false;
            }
        }
        return true;
    }
}

void R::RegisterUnifAttrs(ProgramRef &ref, const AttrUnifArg *unifs, const AttrUnifArg *attrs) {
    Program *p = R::GetProgram(ref);
    R::UseProgram(p->prog_id);

    char *p_names_buf = &(*name_buffers.Get((size_t)ref.index))[0];

    const AttrUnifArg *arg = unifs;
    while (arg && arg->name) {
        assert(UniformUnique(p, arg->name));

        strcpy(p_names_buf, arg->name);

        p->uniforms[arg->index].name = p_names_buf;
        p->uniforms[arg->index].loc = arg->index;

        p_names_buf += strlen(arg->name) + 1;

        swRegisterUniformv(arg->index, (SWenum) arg->type, arg->size);

        arg++;
    }

    arg = attrs;
    while (arg && arg->name) {
        assert(AttributeUnique(p, arg->name));

        strcpy(p_names_buf, arg->name);

        p->attributes[arg->index].name = p_names_buf;
        p->attributes[arg->index].loc = arg->index;

        p_names_buf += strlen(arg->name) + 1;

        arg++;
    }
}


#endif