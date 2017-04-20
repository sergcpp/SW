#ifndef RENDER_STATE_GL_H
#define RENDER_STATE_GL_H

#include <cstdint>
#ifndef NDEBUG
#include <cstdio>
#endif

#include "GL.h"

namespace R {
    const int NUM_TEXTURE_SLOTS = 16;

	extern uint32_t current_program;
    extern uint32_t binded_textures[NUM_TEXTURE_SLOTS];
    extern uint32_t invalid_texture;

	inline void UseProgram(uint32_t p) {
		if (current_program != p) {
			glUseProgram(p);
			current_program = p;
		}
	}

	inline void BindTexture(int slot, uint32_t t) {
		if (binded_textures[slot] != t) {
			glActiveTexture((GLenum)(GL_TEXTURE0 + slot));
			glBindTexture(GL_TEXTURE_2D, t);
			binded_textures[slot] = t;
		}
	}

	inline void BindTextureCube(int slot, uint32_t t) {
		if (binded_textures[slot] != t) {
			glActiveTexture((GLenum)(GL_TEXTURE0 + slot));
            glBindTexture(GL_TEXTURE_CUBE_MAP, t);
            binded_textures[slot] = t;
		}
	}

    inline void UnBindTexture(uint32_t t) {
        for (int i = 0; i < NUM_TEXTURE_SLOTS; i++) {
            if (binded_textures[i] == t) {
                binded_textures[i] = invalid_texture;
            }
        }
    }

	inline void ClearColor(float r = 0, float g = 0, float b = 0, float a = 1) {
		glClearColor(r, g, b, a);
		glClear(GL_COLOR_BUFFER_BIT);
	}

	inline void ClearColorAndDepth(float r = 0, float g = 0, float b = 0, float a = 1) {
		glClearColor(r, g, b, a);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	}

	inline void SetDepthTest(bool b) {
		b ? glEnable(GL_DEPTH_TEST) : glDisable(GL_DEPTH_TEST);
	}

	inline void SetBlend(bool b) {
		b ? glEnable(GL_BLEND) : glDisable(GL_BLEND);
	}

    inline void SetCullFace(bool b) {
        b ? glEnable(GL_CULL_FACE) : glDisable(GL_CULL_FACE);
    }

#ifndef NDEBUG
	inline void PrintGLError(const char *op = "undefined") {
		for (GLint error = glGetError(); error; error = glGetError()) {
			fprintf(stderr, "after %s glError (0x%x)\n", op, error);
		}
	}
#else
	inline void PrintGLError(const char *op = "undefined") {}
#endif

	int IsExtensionSupported(const char *ext);
}

#endif // RENDER_STATE_GL_H