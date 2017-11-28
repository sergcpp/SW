#include "test_common.h"

#include "../SWprogram.h"

void test_program() {

    {
        // Program swProgRegUniform
        SWprogram p_;
        SWubyte uniform_buf[128];
        swProgInit(&p_, uniform_buf, NULL, NULL, 0);

        swProgRegUniform(&p_, 0, SW_VEC2);
        swProgRegUniform(&p_, 1, SW_FLOAT);
        swProgRegUniform(&p_, 2, SW_VEC3);
        assert((uintptr_t)p_.uniforms[0].data == (uintptr_t)p_.uniform_buf);
        assert((uintptr_t)p_.uniforms[1].data == (uintptr_t)p_.uniform_buf + 2 * sizeof(SWfloat));
        assert((uintptr_t)p_.uniforms[2].data == (uintptr_t)p_.uniform_buf + 3 * sizeof(SWfloat));
        assert(p_.unifrom_buf_size == 6 * sizeof(SWfloat));

        swProgDestroy(&p_);
    }

    {
        // Program swProgSetProgramUniform
        SWprogram p_;
        SWubyte uniform_buf[128];
        swProgInit(&p_, uniform_buf, NULL, NULL, 0);

        swProgRegUniform(&p_, 0, SW_VEC2);
        swProgRegUniform(&p_, 1, SW_FLOAT);
        swProgRegUniform(&p_, 2, SW_VEC3);
        SWfloat uv_scale[] = { 2, 4, 1 };
        swProgSetProgramUniform(&p_, 2, SW_VEC3, uv_scale);
        SWfloat *f = (SWfloat*)p_.uniforms[2].data;
        assert(f[0] == 2);
        assert(f[1] == 4);
        assert(f[2] == 1);

        swProgDestroy(&p_);
    }
}
