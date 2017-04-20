#include "test_common.h"

#include <cstdio>

#include <utility>

#include "../Optional.h"

void test_optional() {
    {   // Create Optional
        class MyObj {
            bool b;
        public:
            MyObj(int) : b(true) {
                printf("constructed\n");
            }
            MyObj(const MyObj &rhs) {
                b = rhs.b;
                printf("copied\n");
            }
            MyObj(MyObj &&rhs) {
                printf("moved\n");
                b = rhs.b;
                rhs.b = false;
            }
            ~MyObj() {
                if (b) {
                    printf("destroyed\n");
                }
            }
        };

        sys::Optional<MyObj> v1;
        assert_false(v1.initialized());
        sys::Optional<MyObj> v2(MyObj(11));
        assert(v2.initialized());
        v1 = v2;
        assert(v1.initialized());
        assert(v2.initialized());
        v1 = v1;
        assert(v1.initialized());
        v1 = std::move(v2);
        assert(v1.initialized());
        assert_false(v2.initialized());
    }
}

