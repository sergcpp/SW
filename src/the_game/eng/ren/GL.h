#ifndef GL_H
#define GL_H

#if defined(__ANDROID__) || defined(__native_client__) || defined(EMSCRIPTEN)
    #include <GLES2/gl2.h>
    #include <GLES2/gl2ext.h>
#else
    #include <GL/glew.h>
#endif

#endif //GL_H
