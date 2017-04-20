#include "test_common.h"

#include <SDL2/SDL.h>

#include "../Texture.h"

#ifdef USE_GL_RENDER

#include "../GL.h"

class TextureTest {
	SDL_Window *window_;
	void *gl_ctx_;
public:
	TextureTest() {
		SDL_Init(SDL_INIT_VIDEO);

		window_ = SDL_CreateWindow("View", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, 256, 256, SDL_WINDOW_OPENGL | SDL_WINDOW_HIDDEN);
		gl_ctx_ = SDL_GL_CreateContext(window_);
#ifndef EMSCRIPTEN
		GLenum glew_err = glewInit();
		if (glew_err != GLEW_OK) {
			fprintf(stderr, "GLEW Error: %s\n", glewGetErrorString(glew_err));
		}
#endif
	}

	~TextureTest() {
		SDL_GL_DeleteContext(gl_ctx_);
		SDL_DestroyWindow(window_);
#ifndef EMSCRIPTEN
		SDL_Quit();
#endif
	}
};
#else

#include "../SW/SW.h"

class TextureTest {
    SWcontext *ctx;
public:
    TextureTest() {
        ctx = swCreateContext(1, 1);
        swMakeCurrent(ctx);
    }

    ~TextureTest() {
        swDeleteContext(ctx);
    }
};
#endif


static void OnTextureNeeded(const char *name) {

}

static unsigned char test_tga_img[] =	"\x00\x00\x02\x00\x00\x00\x00\x00\x00\x00\x00\x00\x02\x00\x02\x00" \
										"\x18\x00\x00\x00\x00\xFF\xFF\xFF\xFF\xFF\xFF\x00\x00\x00\x00\x00" \
										"\x00\x00\x00\x00\x00\x00\x54\x52\x55\x45\x56\x49\x53\x49\x4F\x4E" \
										"\x2D\x58\x46\x49\x4C\x45\x2E\x00";

void test_texture() {
    {   // TGA load
        TextureTest test;

        R::eTexLoadStatus status;
        R::Texture2DRef t_ref = R::LoadTexture2D("checker.tga", nullptr, &status);
        assert(status == R::TexCreatedDefault);
        assert(t_ref.index == 0);

        R::Texture2D *p_t = R::GetTexture(t_ref);
        assert(p_t != nullptr);
        assert(std::string(p_t->name) == "checker.tga");
        assert(p_t->counter == 1);
#if defined(USE_GL_RENDER) || defined(USE_SW_RENDER)
        assert(p_t->tex_id == t_ref.tex_id);
#endif
        assert(p_t->w == 1);
        assert(p_t->h == 1);
        assert(p_t->format == R::RawRGB888);
        assert(p_t->not_ready == 1);

        {
            R::Texture2DRef t_ref2 = R::LoadTexture2D("checker.tga", nullptr, &status);
            assert(status == R::TexFound);
            assert(t_ref2.index == 0);

            p_t = R::GetTexture(t_ref2);
            assert(p_t != nullptr);
            assert(p_t->counter == 2);
#if defined(USE_GL_RENDER) || defined(USE_SW_RENDER)
            assert(p_t->tex_id == t_ref2.tex_id);
#endif
            assert(p_t->not_ready == 1);
        }

        p_t = R::GetTexture(t_ref);
        assert(p_t != nullptr);
        assert(p_t->counter == 1);

        {
            R::Texture2DRef t_ref3 = R::LoadTexture2D("checker.tga", test_tga_img, &status);
            assert(status == R::TexCreatedFromData);
            assert(t_ref3.index == 0);
#if defined(USE_GL_RENDER) || defined(USE_SW_RENDER)
            assert(t_ref3.tex_id == t_ref.tex_id);
#endif

            p_t = R::GetTexture(t_ref3);
            assert(p_t != nullptr);
            assert(p_t->counter == 2);
#if defined(USE_GL_RENDER) || defined(USE_SW_RENDER)
            assert(p_t->tex_id == t_ref3.tex_id);
#endif
            assert(p_t->w == 2);
            assert(p_t->h == 2);
            assert(p_t->format == R::RawRGB888);
            assert(p_t->not_ready == 0);
        }

        p_t = R::GetTexture(t_ref);
        assert(p_t != nullptr);
        assert(p_t->counter == 1);
    }
}
