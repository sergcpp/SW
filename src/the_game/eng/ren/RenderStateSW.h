#ifndef RENDER_STATE_SW_H
#define RENDER_STATE_SW_H

#include <cstdint>

#include "SW/SW.h"

namespace R {
	extern uint32_t current_program;
	extern uint32_t binded_textures[16];

	inline void UseProgram(uint32_t p) {
		if (current_program != p) {
			swUseProgram(p);
			current_program = p;
		}
	}

	inline void BindTexture(int slot, uint32_t t) {
		if (binded_textures[slot] != t) {
			swActiveTexture(SW_TEXTURE0 + slot);
			swBindTexture(t);
			binded_textures[slot] = t;
		}
	}

	inline void ClearColor(float r = 0, float g = 0, float b = 0, float a = 1) {
		swClearColor(r, g, b, a);
	}

	inline void ClearDepth() {
		swClearDepth(1);
	}

	inline void ClearColorAndDepth(float r = 0, float g = 0, float b = 0, float a = 1) {
		ClearColor(r, g, b, a);
		ClearDepth();
	}

	inline void SetDepthTest(bool b) {
		b ? swEnable(SW_DEPTH_TEST) : swDisable(SW_DEPTH_TEST);
	}
}

#endif // RENDER_STATE_SW_H