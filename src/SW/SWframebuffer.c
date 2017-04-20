#include "SWframebuffer.h"

#include <assert.h>
#include <stdlib.h>
#include <string.h>

#include "SWtexture.h"
#include "SWzbuffer.h"

void swFbufInit(SWframebuffer *f, SWenum type, SWint w, SWint h, SWint with_depth) {
    SWuint num_bytes = 0;
    f->type = type; f->w = w; f->h = h; f->pixels = f->zbuf = /*f->depth =*/ NULL;
	if (type == SW_BGRA8888) {
        num_bytes = (SWuint) w * h * 4;
    } else if (type == SW_FRGBA) {
        num_bytes = (SWuint) w * h * 4 * sizeof(SWfloat);
    }
    f->pixels = calloc(num_bytes, 1);
    if (with_depth) {
        f->zbuf = (SWzbuffer*)malloc(sizeof(SWzbuffer));
        swZbufInit(f->zbuf, w, h);
        swFbufClearDepth(f, (SWfloat) 1);
    }
}

void swFbufDestroy(SWframebuffer *f) {
    free(f->pixels);
    if (f->zbuf) {
        swZbufDestroy(f->zbuf);
        free(f->zbuf);
    }
    memset(f, 0, sizeof(SWframebuffer));
}

void swFbufClearColor_RGBA(SWframebuffer *f, SWubyte *rgba) {
    SWint x, y, span_size = 0;
	if (f->type == SW_BGRA8888) {
		for (x = 0; x < f->w; x++) {
			swPx_BGRA8888_SetColor_RGBA8888(f->w, f->h, f->pixels, x, 0, rgba);
		}
		span_size = f->w * 4;
    }

	for (y = 1; y < f->h; y++) {
		memcpy(((char*)f->pixels) + y * span_size, f->pixels, (size_t)span_size);
	}
}

void swFbufClearColorFloat(SWframebuffer *f, SWfloat r, SWfloat g, SWfloat b, SWfloat a) {
	if (f->type == SW_BGRA8888) {
        SWubyte rgba[4];
		_swPx_RGBA8888_SetColor_FRGBA_(rgba, r, g, b, a);
        swFbufClearColor_RGBA(f, rgba);
    } else if (f->type == SW_FRGBA) {
        SWfloat rgba[4] = { r, g, b, a };
        SWint x, y, span_size = f->w * 4 * sizeof(SWfloat);
        for (x = 0; x < f->w; x++) {
            swPx_FRGBA_SetColor_FRGBA(f->w, f->h, f->pixels, x, 0, rgba);
        }
        
        for (y = 1; y < f->h; y++) {
            memcpy(((char*)f->pixels) + y * span_size, f->pixels, (size_t)span_size);
        }
    }
}

void swFbufBlitPixels(SWframebuffer *f, SWint x, SWint y, SWenum type, SWenum mode, SWint w, SWint h, void *pixels, SWfloat scale) {
    SWint	beg_x = sw_max(x, 0),
            beg_y = sw_max(y, 0),
            end_x = sw_min(f->w, (SWint)(x + scale * w)),
            end_y = sw_min(f->h, (SWint)(y + scale * h));

    SWfloat u_step = (SWfloat)1.0 / (w * scale),
            v_step = (SWfloat)1.0 / (h * scale);

    SWfloat v = 0;
    SWint i, j;

#define LOOP(__fun__) \
    for (j = beg_y; j < end_y; j++) {                                   \
        SWfloat u = 0;                                                  \
        for (i = beg_x; i < end_x; i++) {                               \
            SWubyte *p = (SWubyte *) f->pixels + (j * f->w + i) * 4;    \
            __fun__(w, h, pixels, u, v, p);                             \
            u += u_step;                                                \
        }                                                               \
        v += v_step;                                                    \
    }

    if (type == SW_UNSIGNED_BYTE) {
        if (mode == SW_RGB) {
			if (f->type == SW_BGRA8888) {
                LOOP(swPx_RGB888_GetColor_BGRA8888_UV)
            }
        } else if (mode == SW_RGBA) {
			if (f->type == SW_BGRA8888) {
                LOOP(swPx_RGBA8888_GetColor_BGRA8888_UV)
            }
        }
    } else if (type == SW_COMPRESSED) {
        assert(0);
    }

#undef LOOP
}

void swFbufBlitTexture(SWframebuffer *f, SWint x, SWint y, SWtexture *t, SWfloat scale) {
    swFbufBlitPixels(f, x, y, t->type, t->mode, t->w, t->h, t->pixels, scale);
}
