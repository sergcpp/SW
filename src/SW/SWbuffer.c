#include "SWbuffer.h"

#include <assert.h>
#include <stdlib.h>
#include <string.h>

void swBufInit(SWbuffer *b, SWuint size, const void *data) {
    assert(data);
    void *aligned_data = sw_aligned_alloc(size, 16);
    assert(aligned_data);
    memcpy(aligned_data, data, size);
    b->size = size;
    b->data = aligned_data;
}

void swBufDestroy(SWbuffer *b) {
    sw_aligned_free(b->data);
    memset(b, 0, sizeof(SWbuffer));
}

void swBufGetData(SWbuffer *b, SWuint offset, SWuint size, void *data) {
    assert(b->data);
    memcpy(data, (char*)b->data + offset, size);
}
