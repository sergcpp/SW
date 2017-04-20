#include "test_common.h"

#include <SDL2/SDL.h>

#include "../Material.h"

#if defined(USE_GL_RENDER)
#include "../GL.h"
class MaterialTest {
    SDL_Window *window_;
    void *gl_ctx_;
public:
    MaterialTest() {
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

    ~MaterialTest() {
        SDL_GL_DeleteContext(gl_ctx_);
        SDL_DestroyWindow(window_);
#ifndef EMSCRIPTEN
        SDL_Quit();
#endif
    }
};
#elif defined(USE_SW_RENDER)
#include "../SW/SW.h"
class MaterialTest {
    SWcontext *ctx;
public:
    MaterialTest() {
        ctx = swCreateContext(1, 1);
        swMakeCurrent(ctx);
    }

    ~MaterialTest() {
        swDeleteContext(ctx);
    }
};
#endif

static void OnProgramNeeded(const char *name, const char *arg1, const char *arg2) {

}

static void OnTextureNeeded(const char *name) {

}

void test_material() {

    {   // Load material
        MaterialTest test;

        const char *mat_src =   "gl_program: constant constant.vs constant.fs\n"
                "sw_program: constant\n"
                "flag: alpha_blend\n"
                "texture: checker.tga\n"
                "texture: checker.tga\n"
                "texture: metal_01.tga\n"
                "texture: checker.tga\n"
                "param: 0 1 2 3\n"
                "param: 0.5 1.2 11 15";

        R::eMatLoadStatus status;
        R::MaterialRef m_ref = R::LoadMaterial("mat1", nullptr, &status, OnProgramNeeded, OnTextureNeeded);
        assert(status == R::MatSetToDefault);

        {
            R::Material *mat = R::GetMaterial(m_ref);

            assert(mat->not_ready == 1);
            assert(mat->counter == 1);
        }

        R::LoadMaterial("mat1", mat_src, &status, OnProgramNeeded, OnTextureNeeded);

        assert(status == R::MatCreatedFromData);
        assert(m_ref.flags() == R::AlphaBlend);

        R::Material *mat = R::GetMaterial(m_ref);

        assert(mat->not_ready == 0);
        assert(mat->counter == 1);

        assert(std::string(mat->name) == "mat1");

        R::Program *p = R::GetProgram(mat->program);

        assert(std::string(p->name) == "constant");

        assert(mat->flags == R::AlphaBlend);

        R::Texture2D *t0 = R::GetTexture(mat->textures[0]);
        R::Texture2D *t1 = R::GetTexture(mat->textures[1]);
        R::Texture2D *t2 = R::GetTexture(mat->textures[2]);
        R::Texture2D *t3 = R::GetTexture(mat->textures[3]);

        assert(mat->textures[0].index == mat->textures[1].index);
        assert(mat->textures[0].index == mat->textures[3].index);

        assert(std::string(t0->name) == "checker.tga");
        assert(std::string(t1->name) == "checker.tga");
        assert(std::string(t2->name) == "metal_01.tga");
        assert(std::string(t3->name) == "checker.tga");

        assert(mat->params[0] == glm::vec4(0, 1, 2, 3));
        assert(mat->params[1] == glm::vec4(0.5, 1.2, 11, 15));
    }
}
