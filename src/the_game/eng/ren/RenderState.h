#ifndef RENDER_STATE_H
#define RENDER_STATE_H

#if defined(USE_GL_RENDER)
#include "RenderStateGL.h"
#elif defined(USE_SW_RENDER)
#include "RenderStateSW.h"
#elif defined(USE_VK_RENDER)
#include "RenderStateVK.h"
#endif

class Camera;

namespace R {
	extern Camera *current_cam;
	extern int w, h;

	void Init(int w, int h);
	void Resize(int w, int h);
	void Deinit();
}

#endif // RENDER_STATE_H