#include "test_common.h"

#include <SDL2/SDL.h>

#include "../RenderState.h"
#include "../Texture.h"

#ifdef USE_GL_RENDER

#include "../GL.h"

class RenderStateTest {
	SDL_Window *window_;
	void *gl_ctx_;
public:
	RenderStateTest() {
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

	~RenderStateTest() {
		SDL_GL_DeleteContext(gl_ctx_);
		SDL_DestroyWindow(window_);
#ifndef EMSCRIPTEN
		SDL_Quit();
#endif
	}
};
#else

#include "../SW/SW.h"

class RenderStateTest {
    SWcontext *ctx;
    SWint tex, prog;
public:
    RenderStateTest() {
        ctx = swCreateContext(1, 1);
        swMakeCurrent(ctx);
    }

    ~RenderStateTest() {
        swDeleteContext(ctx);
    }
};
#endif

static unsigned char test_tga_img[] = "\x00\x00\x02\x00\x00\x00\x00\x00\x00\x00\x00\x00\x02\x00\x02\x00" \
									  "\x18\x00\x00\x00\x00\xFF\xFF\xFF\xFF\xFF\xFF\x00\x00\x00\x00\x00" \
									  "\x00\x00\x00\x00\x00\x00\x54\x52\x55\x45\x56\x49\x53\x49\x4F\x4E" \
									  "\x2D\x58\x46\x49\x4C\x45\x2E\x00";

void test_render_state() {

    {   // Bind tex and prog
        RenderStateTest test;

        R::Texture2DRef t_ref = R::LoadTexture2D("checker.tga", test_tga_img, nullptr);
        R::BindTexture(0, t_ref.tex_id);
        R::BindTexture(2, t_ref.tex_id);
        R::BindTexture(4, t_ref.tex_id);

#if defined(USE_GL_RENDER) || defined(USE_SW_RENDER)
        assert(R::binded_textures[0] == t_ref.tex_id);
        assert(R::binded_textures[2] == t_ref.tex_id);
        assert(R::binded_textures[4] == t_ref.tex_id);
#endif
    }
}

