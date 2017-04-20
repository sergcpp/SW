#ifndef TEXTURE_H
#define TEXTURE_H

#include <cstdint>

#include <memory>

namespace R {
    enum eTex2DFormat { Undefined, RawRGB888, RawRGBA8888, RawLUM8, RawR32F, Compressed };
    enum eTexFilter { NoFilter, Bilinear, Trilinear };
    enum eTexRepeat { Repeat, ClampToEdge };

    struct Texture2D {
#if defined(USE_GL_RENDER) || defined(USE_SW_RENDER)
        uint32_t	tex_id;
#endif
        int         counter, w, h;
        eTex2DFormat format;
        int	        not_ready;
        char        name[64];

        Texture2D() : counter(0), w(0), h(0), format(Undefined), not_ready(0) {
            name[0] = '\0';
#if defined(USE_GL_RENDER) || defined(USE_SW_RENDER)
            tex_id = 0;
#endif
        }
    };

    Texture2D *GetTexture(const struct Texture2DRef &ref);
    void ReleaseTexture(Texture2D &t);
    void ReleaseTexture(Texture2DRef &ref);

    struct Texture2DRef {
        int			index;
#if defined(USE_GL_RENDER) || defined(USE_SW_RENDER)
        uint32_t	tex_id;
#endif
        Texture2DRef() : index(-1) {
#if defined(USE_GL_RENDER) || defined(USE_SW_RENDER)
            tex_id = 0;
#endif
        }

        ~Texture2DRef() {
            this->Release();
        }

        Texture2DRef(const Texture2DRef &ref) {
            index = ref.index;
            if (index != -1) {
                GetTexture(ref)->counter++;
            }
#if defined(USE_GL_RENDER) || defined(USE_SW_RENDER)
            tex_id = ref.tex_id;
#endif
        }
        Texture2DRef(Texture2DRef &&ref) {
            index = ref.index;
            ref.index = -1;
#if defined(USE_GL_RENDER) || defined(USE_SW_RENDER)
            tex_id = ref.tex_id;
            ref.tex_id = 0;
#endif
        }

        Texture2DRef& operator=(const Texture2DRef &ref) {
            this->Release();
            index = ref.index;
            if (index != -1) {
                GetTexture(ref)->counter++;
            }
#if defined(USE_GL_RENDER) || defined(USE_SW_RENDER)
            tex_id = ref.tex_id;
#endif
            return *this;
        }
        
        Texture2DRef& operator=(Texture2DRef &&ref) {
                this->Release();
                index = ref.index;
                ref.index = -1;
#if defined(USE_GL_RENDER) || defined(USE_SW_RENDER)
                tex_id = ref.tex_id;
                ref.tex_id = 0;
#endif
                return *this;
            }

        void Release() {
            ReleaseTexture(*this);
        }

        int loaded() const {
            return !R::GetTexture(*this)->not_ready;
        }
    };

    enum eTexLoadStatus { TexFound, TexCreatedDefault, TexCreatedFromData };

    Texture2DRef LoadTexture2D(const char *name, const void *data, eTexFilter f = Trilinear, eTexRepeat r = Repeat);
    Texture2DRef LoadTexture2D(const char *name, const void *data, eTexLoadStatus *load_status,
                               int w = 0, int h = 0,
                               eTex2DFormat format = Undefined, eTexFilter f = Trilinear, eTexRepeat r = Repeat);

    Texture2DRef LoadTextureCube(const char *name, const void *data[6], eTexFilter f = Trilinear);
    Texture2DRef LoadTextureCube(const char *name, const void *data[6], eTexLoadStatus *load_status,
                                 int w = 0, int h = 0, eTex2DFormat format = Undefined, eTexFilter f = Trilinear);

    int NumTexturesNotReady();
    void ReleaseAllTextures();

    std::unique_ptr<uint8_t[]> ReadTGAFile(const void *data, int &w, int &h, eTex2DFormat &format);

    void InitTex2DFromTGAFile(Texture2D &t, const void *data, eTexFilter f = Trilinear, eTexRepeat r = Repeat);
    void InitTex2DFromTEXFile(Texture2D &t, const void *data, eTexFilter f = Trilinear, eTexRepeat r = Repeat);
    void InitTex2DFromRAWData(Texture2D &t, const void *data,
                              int w, int h, eTex2DFormat format, eTexFilter f = Trilinear, eTexRepeat r = Repeat);

    void InitTexCubeFromTGAFile(Texture2D &t, const void *data[6], eTexFilter f = Bilinear);
    void InitTexCubeFromRAWData(Texture2D &t, const void *data[6],
                                int w, int h, eTex2DFormat format, eTexFilter f = Bilinear);

    void ChangeFilter(Texture2D &t, eTexFilter f, eTexRepeat r);
}

#endif // TEXTURE_H
