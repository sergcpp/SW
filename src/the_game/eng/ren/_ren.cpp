
#include "Anim.cpp"
#include "Camera.cpp"
#include "Material.cpp"
#include "Matrices_old.cpp"
#include "Mesh.cpp"
#include "Plane.cpp"
#include "Program.cpp"
#include "RenderState.cpp"
#include "RenderThread.cpp"
#include "Texture.cpp"

/*extern "C" {
    #include "SW/_SW.c"
}*/

#if defined(USE_GL_RENDER)
	#include "FrameBufGL.cpp"
    #include "ProgramGL.cpp"
    #include "TextureGL.cpp"
#elif defined(USE_SW_RENDER)
    #include "TextureSW.cpp"
#endif
