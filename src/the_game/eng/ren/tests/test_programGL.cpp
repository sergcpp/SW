#include "test_common.h"

#include <SDL2/SDL.h>

#include "../GL.h"
#include "../Material.h"

class ProgramTest {
    SDL_Window *window_;
    void *gl_ctx_;
public:
    ProgramTest() {
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

    ~ProgramTest() {
        SDL_GL_DeleteContext(gl_ctx_);
        SDL_DestroyWindow(window_);
#ifndef EMSCRIPTEN
        SDL_Quit();
#endif
    }
};

void test_program() {
    {   // Load program
        ProgramTest test;

        const char vs_src[] =	\
            " \
            /*\n \
            ATTRIBUTES\n \
                aVertexPosition : 0\n \
                aVertexPosition1 : 1\n \
            UNIFORMS\n \
                uMVPMatrix : 0\n \
            */\n \
            \n \
            attribute vec3 aVertexPosition;\n \
            uniform mat4 uMVPMatrix;\n \
            \n \
            void main(void) {\n \
                gl_Position = uMVPMatrix * vec4(aVertexPosition, 1.0);\n \
            } ";

        const char fs_src[] =
                " \
            precision mediump float;\n \
            /*\n \
            UNIFORMS\n \
                asdasd : 1\n \
            */\n \
            uniform vec3 col;\n \
            \n\
            void main(void) {\n \
                gl_FragColor = vec4(col, 1.0);\n \
            }";

        R::eProgramLoadStatus status;
        R::ProgramRef p = R::LoadProgramGLSL("constant", nullptr, nullptr, &status);

        assert(p.index == 0);
        assert(status == R::ProgSetToDefault);

        {
            R::Program *pp = R::GetProgram(p);

            assert(pp != nullptr);
            assert(std::string(pp->name) == "constant");
            assert(pp->prog_id == 0); // not initialized
            assert(pp->counter == 1);
            assert(pp->not_ready == 1);
        }

        R::LoadProgramGLSL("constant", vs_src, fs_src, &status);

        assert(status == R::ProgCreatedFromData);

        R::Program *pp = R::GetProgram(p);

        assert(pp != nullptr);
        assert(std::string(pp->name) == "constant");

        assert(pp->not_ready != 1);

        assert(std::string(pp->attributes[0].name) == "aVertexPosition");
        assert(pp->attributes[0].loc != -1);
        assert(pp->attributes[1].name == nullptr);
        assert(pp->attributes[1].loc == -1);

        assert(std::string(pp->uniforms[0].name) == "uMVPMatrix");
        assert(pp->uniforms[0].loc != -1);
        assert(std::string(pp->uniforms[1].name) == "col");
        assert(pp->uniforms[1].loc != -1);
    }
}
