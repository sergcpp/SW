#include "test_common.h"

#include <stdlib.h>
#include <string.h>

#include "../SWtexture.h"

static SWubyte tex[] = { 0, 0, 0,        1, 0, 0,        0, 1, 0,        0, 0, 255,
                         1, 1, 0,        11, 13, 14,     190, 111, 20,   20, 20, 20,
                         10, 111, 12,    190, 111, 20,   0, 1, 0,        0, 0, 1,
                         1, 0, 0,        0, 1, 0,        0, 0, 1,        0, 0, 0
                       };

void test_texture() {

    {
        // Texture init move
        SWtexture t;
        swTexInitMove(&t, SW_RGB, SW_UNSIGNED_BYTE, 4, 4, tex, NULL);
        assert(t.pixels == tex);
        swTexDestroy(&t);
    }

    {
        // Texture init malloced
        SWtexture t;
        void *tex_data = malloc(sizeof(tex));
        memcpy(tex_data, tex, sizeof(tex));
        swTexInitMove_malloced(&t, SW_RGB, SW_UNSIGNED_BYTE, 4, 4, tex_data);
        assert(t.pixels == tex_data);
        swTexDestroy(&t);
    }

    {
        // Texture swTexGetColorFloat_RGBA
        SWtexture t_;
        swTexInit(&t_, SW_RGB, SW_UNSIGNED_BYTE, 4, 4, tex);
        assert(t_.pixels != NULL);
        assert(((SWubyte*)t_.pixels)[3] == 1);

        SWfloat rgba[4];
        swTexGetColorFloat_RGBA(&t_, 0.9f, 0.0f, rgba);
        assert(rgba[0] == 0);
        assert(rgba[1] == 0);
        assert(rgba[2] == 1);
        assert(rgba[3] == 1);

        swTexDestroy(&t_);
        assert(t_.pixels == NULL);
    }
}
