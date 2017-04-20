#include "test_common.h"

#include <string>

#include "../Program.h"
#include "../SW/SW.h"

class ProgramTest {
    SWcontext *ctx;
public:
    ProgramTest() {
        ctx = swCreateContext(1, 1);
    }

    ~ProgramTest() {
        swDeleteContext(ctx);
    }
};

VSHADER vshader(VS_IN, VS_OUT) { }

FSHADER fshader(FS_IN, FS_OUT) { }

void test_program() {
    {   // Create program
        ProgramTest test;

        R::eProgramLoadStatus status;
        R::ProgramRef p = R::CreateProgramSW("constant", nullptr, nullptr, 0, &status);

        assert(p.index == 0);
        assert(status == R::ProgSetToDefault);

        {
            R::Program *pp = R::GetProgram(p);

            assert(pp != nullptr);
            assert(std::string(pp->name) == "constant");
            assert(pp->counter == 1);
            assert(pp->prog_id == 0); // default value
            assert(pp->not_ready == 1);
        }

        R::CreateProgramSW("constant", (void*)vshader, (void*)fshader, 0, &status);

        assert(status == R::ProgCreatedFromData);

        R::Program *pp = R::GetProgram(p);

        R::AttrUnifArg unifs[] = {{"unif1", 0, SW_FLOAT}, {"unif2", 1, SW_VEC3}, {nullptr, 0, 0}};
        R::AttrUnifArg attrs[] = {{"attr1", 0, -1}, {"attr2", 1, -1}, {nullptr, 0, 0}};
        R::RegisterUnifAttrs(p, unifs, attrs);

        assert(pp != nullptr);
        assert(std::string(pp->name) == "constant");

        assert(std::string(pp->uniforms[0].name) == "unif1");
        assert(pp->uniforms[0].loc == 0);
        assert(std::string(pp->uniforms[1].name) == "unif2");
        assert(pp->uniforms[1].loc == 1);

        assert(std::string(pp->attributes[0].name) == "attr1");
        assert(pp->attributes[0].loc == 0);
        assert(std::string(pp->attributes[1].name) == "attr2");
        assert(pp->attributes[1].loc == 1);

        assert_nothrow(R::RegisterUnifAttrs(p, nullptr, nullptr));

        assert(pp->not_ready != 1);
    }
}
