#include "Texture.h"

#include <cstdint>
#include <cstring>
#include <cstdio>

#include <limits>

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
	SparseArray<Texture2D> textures(256);
    float anisotropy = 0.0f;


#if defined(USE_GL_RENDER) || defined(USE_SW_RENDER)
    extern uint32_t binded_textures[16];
#endif
}

R::Texture2DRef R::LoadTexture2D(const char *name, const void *data, eTexFilter f, eTexRepeat r) {
	return R::LoadTexture2D(name, data, nullptr, 0, 0, Undefined, f, r);
}

R::Texture2DRef R::LoadTexture2D(const char *name, const void *data, eTexLoadStatus *load_status,
                                 int w, int h, eTex2DFormat format, eTexFilter f, eTexRepeat r) {
	auto it = textures.begin();
	for (; it != textures.end(); ++it) {
		if (strcmp(name, it->name) == 0) {
			break;
		}
	}
	if (it != textures.end() &&
			// will not be initialized
			!(it->not_ready && data)) {
		it->counter++;

		Texture2DRef ref;
		ref.index = (int) it.index();
#if defined(USE_GL_RENDER) || defined(USE_SW_RENDER)
		ref.tex_id = it->tex_id;
#endif

		if (load_status) *load_status = TexFound;

		return ref;
	} else {
		if (it == textures.end()) {
			Texture2D t;
			t.counter = 0;
			strcpy(t.name, name);
			int index = (int)textures.Add(t);
			it = textures.it_at(index);
		}
		
        if (!data) {
            unsigned char cyan[3] = {0, 255, 255};
            InitTex2DFromRAWData(*it, cyan, 1, 1, RawRGB888, NoFilter, Repeat);
            // mark it as not ready
            it->not_ready = 1;
			if (load_status) *load_status = TexCreatedDefault;
        } else {
            if (strstr(it->name, ".tga") != 0 || strstr(it->name, ".TGA") != 0) {
                InitTex2DFromTGAFile(*it, data, f, r);
            } else if (strstr(it->name, ".tex") != 0 || strstr(it->name, ".TEX") != 0) {
                InitTex2DFromTEXFile(*it, data, f, r);
            } else {
				InitTex2DFromRAWData(*it, data, w, h, format, f, r);
			}
            it->not_ready = 0;
			if (load_status) *load_status = TexCreatedFromData;
        }

		it->counter++;

		Texture2DRef ref;
#if defined(USE_GL_RENDER) || defined(USE_SW_RENDER)
		ref.tex_id	= it->tex_id;
#endif
		ref.index	= (int)it.index();

		return ref;
	}
}

R::Texture2DRef R::LoadTextureCube(const char *name, const void *data[6], eTexFilter f) {
    return R::LoadTextureCube(name, data, nullptr, 0, 0, Undefined, f);
}

R::Texture2DRef R::LoadTextureCube(const char *name, const void *data[6], eTexLoadStatus *load_status,
                                   int w, int h, eTex2DFormat format, eTexFilter f) {
    auto it = textures.begin();
    for (; it != textures.end(); ++it) {
        if (strcmp(name, it->name) == 0) {
            break;
        }
    }
    if (it != textures.end() &&
        // will not be initialized
        !(it->not_ready && data)) {
        it->counter++;

        Texture2DRef ref;
        ref.index = (int) it.index();
#if defined(USE_GL_RENDER) || defined(USE_SW_RENDER)
        ref.tex_id = it->tex_id;
#endif

        if (load_status) *load_status = TexFound;

        return ref;
    } else {
        if (it == textures.end()) {
            Texture2D t;
            t.counter = 0;
            t.not_ready = 0;
            strcpy(t.name, name);
            int index = (int)textures.Add(t);
            it = textures.it_at(index);
        }

        if (!data) {
            const unsigned char cyan[3] = { 0, 255, 255 };
            const void *data[6] = { cyan, cyan, cyan, cyan, cyan, cyan };
            InitTexCubeFromRAWData(*it, data, 1, 1, RawRGB888, NoFilter);
            // mark it as not ready
            it->not_ready = 1;
            if (load_status) *load_status = TexCreatedDefault;
        } else {
            if (strstr(it->name, ".tga") != 0 || strstr(it->name, ".TGA") != 0) {
                InitTexCubeFromTGAFile(*it, data, f);
            } else {
                InitTexCubeFromRAWData(*it, data, w, h, format, f);
            }
            if (load_status) *load_status = TexCreatedFromData;
        }

        it->counter++;

        Texture2DRef ref;
#if defined(USE_GL_RENDER) || defined(USE_SW_RENDER)
        ref.tex_id	= it->tex_id;
#endif
        ref.index	= (int)it.index();

        return ref;
    }
}

void R::ReleaseTexture(Texture2D &t) {
#if defined(USE_GL_RENDER)
	if (t.tex_id != -1) {
		GLuint gl_tex = (GLuint)t.tex_id;
		glDeleteTextures(1, &gl_tex);
        for (int i = 0; i < 16; i++) {
            if (binded_textures[i] == t.tex_id) {
                binded_textures[i] = std::numeric_limits<uint32_t>::max();
            }
        }
		t.tex_id = 0;
	}
#elif defined(USE_SW_RENDER)
    if (t.tex_id != -1) {
        SWint sw_tex = (SWint)t.tex_id;
        swDeleteTexture(sw_tex);
        for (int i = 0; i < 16; i++) {
            if (binded_textures[i] == t.tex_id) {
                binded_textures[i] = std::numeric_limits<uint32_t>::max();
            }
        }
        t.tex_id = 0;
    }
#endif
	t.format = Undefined;
	t.name[0] = '\0';
}

void R::ReleaseTexture(Texture2DRef &ref) {
    if (ref.index == -1) {
        return;
    }
	Texture2D *t = textures.Get((size_t) ref.index);
	if (!--t->counter) {
		ReleaseTexture(*t);
		textures.Remove((size_t) ref.index);
	}
	ref.index = -1;
#if defined(USE_GL_RENDER) || defined(USE_SW_RENDER)
	ref.tex_id = 0;
#endif
}

int R::NumTexturesNotReady() {
	int num = 0;
	for (auto &t : textures) {
		if (t.not_ready) {
			num++;
		}
	}
	return num;
}

void R::ReleaseAllTextures() {
	if (!textures.Size()) return;
	fprintf(stderr, "---------REMAINING TEXTURES--------\n");
	for (auto &t : textures) {
		fprintf(stderr, "%s (%i)\n", t.name, t.counter);
	}
	fprintf(stderr, "-----------------------------------\n");

	textures.Clear();
}

R::Texture2D *R::GetTexture(const Texture2DRef &ref) {
    return textures.Get(ref.index);
}

std::unique_ptr<uint8_t[]> R::ReadTGAFile(const void *data, int &w, int &h, eTex2DFormat &format) {
    uint8_t tga_header[12] = {0, 0, 2, 0, 0, 0, 0, 0, 0, 0, 0, 0};
    const uint8_t *tga_compare = (const uint8_t *) data;
    const uint8_t *img_header = (const uint8_t *) data + sizeof(tga_header);
    uint32_t img_size;
    bool compressed = false;

    if (memcmp(tga_header, tga_compare, sizeof(tga_header)) != 0) {
        if (tga_compare[2] == 1) {
            fprintf(stderr, "Image cannot be indexed color.");
        }
        if (tga_compare[2] == 3) {
            fprintf(stderr, "Image cannot be greyscale color.");
        }
        if (tga_compare[2] == 9 || tga_compare[2] == 10) {
            //fprintf(stderr, "Image cannot be compressed.");
            compressed = true;
        }
    }

    w = img_header[1] * 256u + img_header[0];
    h = img_header[3] * 256u + img_header[2];

    if (w <= 0 || h <= 0 ||
        (img_header[4] != 24 && img_header[4] != 32)) {
        if (w <= 0 || h <= 0) {
            fprintf(stderr, "Image must have a width and height greater than 0");
        }
        if (img_header[4] != 24 && img_header[4] != 32) {
            fprintf(stderr, "Image must be 24 or 32 bit");
        }
        return nullptr;
    }

    uint32_t bpp = img_header[4];
    uint32_t bytes_per_pixel = bpp / 8; 
    img_size = w * h * bytes_per_pixel;
    const uint8_t *image_data = (const uint8_t *)data + 18;

    std::unique_ptr<uint8_t[]> image_ret(new uint8_t[img_size]);

    if (!compressed) {
        for (unsigned i = 0; i < img_size; i += bytes_per_pixel) {
            image_ret[i] = image_data[i + 2];
            image_ret[i + 1] = image_data[i + 1];
            image_ret[i + 2] = image_data[i];
            if (bytes_per_pixel == 4) {
                image_ret[i + 3] = image_data[i + 3];
            }
        }
    } else {
        for (unsigned num = 0; num < img_size; ) {
            uint8_t packet_header = *image_data++;
            if (packet_header & (1 << 7)) {
                uint8_t color[4];
                unsigned size = (packet_header & ~(1 << 7)) + 1;
                size *= bytes_per_pixel;
                for (unsigned i = 0; i < bytes_per_pixel; i++) {
                    color[i] = *image_data++;
                }
                for (unsigned i = 0; i < size; i += bytes_per_pixel, num += bytes_per_pixel) {
                    image_ret[num] = color[2];
                    image_ret[num + 1] = color[1];
                    image_ret[num + 2] = color[0];
                    if (bytes_per_pixel == 4) {
                        image_ret[num + 3] = color[3];
                    }
                }
            } else {
                unsigned size = (packet_header & ~(1 << 7)) + 1;
                size *= bytes_per_pixel;
                for (unsigned i = 0; i < size; i += bytes_per_pixel, num += bytes_per_pixel) {
                    image_ret[num] = image_data[i + 2];
                    image_ret[num + 1] = image_data[i + 1];
                    image_ret[num + 2] = image_data[i];
                    if (bytes_per_pixel == 4) {
                        image_ret[num + 3] = image_data[i + 3];
                    }
                }
                image_data += size;
            }
        }
    }

    if (bpp == 32) {
        format = RawRGBA8888;
    } else if (bpp == 24) {
        format = RawRGB888;
    }

    return image_ret;
}

void R::InitTex2DFromTGAFile(Texture2D &t, const void *data, eTexFilter f, eTexRepeat r) {
    int w = 0, h = 0;
    eTex2DFormat format = Undefined;
    auto image_data = ReadTGAFile(data, w, h, format);

    InitTex2DFromRAWData(t, image_data.get(), w, h, format, f, r);
}

void R::InitTexCubeFromTGAFile(Texture2D &t, const void *data[6], eTexFilter f) {
    std::unique_ptr<uint8_t[]> image_data[6];
    const void *_image_data[6] = {};
    int w = 0, h = 0;
    eTex2DFormat format = Undefined;
    for (int i = 0; i < 6; i++) {
        if (data[i]) {
            image_data[i] = ReadTGAFile(data[i], w, h, format);
            _image_data[i] = image_data[i].get();
        }
    }
    InitTexCubeFromRAWData(t, _image_data, w, h, format, f);
}