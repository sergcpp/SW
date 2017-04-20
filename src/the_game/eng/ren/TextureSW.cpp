#include "Texture.h"

#include "RenderState.h"
#include "SW/SW.h"

void R::InitTex2DFromRAWData(Texture2D &t, const void *data,
                             int w, int h, eTex2DFormat format, eTexFilter f, eTexRepeat r) {
    swActiveTexture(SW_TEXTURE0);

    SWint tex_id;
    if (t.format == Undefined) {
        tex_id = swCreateTexture();

        t.tex_id = (uint32_t)tex_id;
    } else {
        tex_id = (SWuint) t.tex_id;
    }

    R::BindTexture(0, (uint32_t)tex_id);
    swActiveTexture(SW_TEXTURE0);

    t.format	= format;
    t.w			= w;
    t.h			= h;

    if (data) {
        if (format == RawRGBA8888) {
            swTexImage2D(SW_RGBA, SW_UNSIGNED_BYTE, w, h, data);
        } else if (format == RawRGB888) {
            swTexImage2D(SW_RGB, SW_UNSIGNED_BYTE, w, h, data);
        }
    }
}

void R::InitTex2DFromTEXFile(Texture2D &t, const void *data, eTexFilter f, eTexRepeat r) {
    swActiveTexture(SW_TEXTURE0);

    SWint tex_id;
    if (t.format == Undefined) {
        tex_id = swCreateTexture();

        t.tex_id = (uint32_t)tex_id;
    } else {
        tex_id = (SWuint) t.tex_id;
    }

    R::BindTexture(0, (uint32_t)tex_id);
    swActiveTexture(SW_TEXTURE0);

    unsigned short *res = (unsigned short *)data;

    t.format	= Compressed;
    t.w			= res[0];
    t.h			= res[1];

    if (data) {
        swTexImage2D(SW_RGBA, SW_COMPRESSED, t.w, t.h, (char *)data + 4);
    }
}


void R::InitTexCubeFromRAWData(Texture2D &t, const void *data[6], int w, int h,
                               eTex2DFormat format, eTexFilter f) {

}

void R::ChangeFilter(Texture2D &t, eTexFilter f, eTexRepeat r) {}
