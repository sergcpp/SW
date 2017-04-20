#include "test_common.h"

#include <string.h>

#include "../SWbuffer.h"

void test_buffer() {
    {   // Buffer init/destroy
        SWbuffer b;
        const char data1[] = "Data we put in buffer";
        swBufInit(&b, sizeof(data1), data1);
        uintptr_t i = ((uintptr_t)b.data) % 16;
        assert(i == 0);
        swBufDestroy(&b);
        assert(b.data == NULL);
    }

    {   // Buffer data
        SWbuffer b;
        const char data1[] = "Data we put in buffer";
        swBufInit(&b, sizeof(data1), data1);
        char data1_chk[sizeof(data1)];
        swBufGetData(&b, 0, sizeof(data1), data1_chk);
        assert(strcmp(data1, data1_chk) == 0);
        swBufDestroy(&b);
    }
}
