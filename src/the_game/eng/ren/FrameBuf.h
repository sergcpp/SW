#ifndef FRAMEBUF_H
#define FRAMEBUF_H

#include <ren/Texture.h>
#include <sys/Optional.h>

namespace R {
    struct FrameBuf {
        R::eTex2DFormat format;
        int w = -1, h = -1;

#if defined(USE_GL_RENDER)
        uint32_t fb, col_tex;
        sys::Optional<uint32_t> depth_rb;
#endif
        FrameBuf() : format(Undefined), w(-1), h(-1) {}
        FrameBuf(int w, int h, R::eTex2DFormat format, R::eTexFilter filter,
                 R::eTexRepeat  repeat, bool with_depth = true);
        ~FrameBuf();

        FrameBuf(const FrameBuf &rhs) = delete;
        FrameBuf &operator=(const FrameBuf &rhs) = delete;
        FrameBuf(FrameBuf &&rhs);
        FrameBuf &operator=(FrameBuf &&rhs);
    };
}

#endif // FRAMEBUF_H