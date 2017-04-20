#include "test_common.h"

#include <ren/RenderState.h>
#include <sys/Json.h>

#include "../Renderer.h"

#ifdef USE_GL_RENDER

#include <SDL2/SDL.h>

class RendererTest {
    SDL_Window *window_;
    void *gl_ctx_;
public:
    RendererTest() {
        SDL_Init(SDL_INIT_VIDEO);

        window_ = SDL_CreateWindow("View", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, 256, 256, SDL_WINDOW_OPENGL | SDL_WINDOW_HIDDEN);
        gl_ctx_ = SDL_GL_CreateContext(window_);

        R::Init(256, 256);
#ifndef EMSCRIPTEN
        GLenum glew_err = glewInit();
        if (glew_err != GLEW_OK) {
            fprintf(stderr, "GLEW Error: %s\n", glewGetErrorString(glew_err));
        }
#endif
    }

    ~RendererTest() {
        R::Deinit();

        SDL_GL_DeleteContext(gl_ctx_);
        SDL_DestroyWindow(window_);
#ifndef EMSCRIPTEN
        SDL_Quit();
#endif
    }
};
#else

#include <ren/SW/SW.h>

class RendererTest {
	SWcontext *ctx;
public:
	RendererTest() {
		ctx = swCreateContext(1, 1);
		swMakeCurrent(ctx);
		R::Init(256, 256);
	}

	~RendererTest() {
		swDeleteContext(ctx);
	}
};
#endif

void test_renderer() {
    {   // Params test
        JsObject config;
        config[ui::GL_DEFINES_KEY] = "";

        {   // Default parameters
            RendererTest _;

            ui::Renderer r(config);

            //const auto &cur = r.GetParams();

        }
    }
}