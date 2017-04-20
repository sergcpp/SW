#include "test_common.h"

#include "../SparseArray.h"

namespace {
    std::vector<int> GenerateTestData(int size) {
        std::vector<int> vec(size);
        for (int i = 0; i < size; i++) {
            vec[i] = i;
        }
        return vec;
    }
}

void test_sparse_array() {

    {   // Resize method
        SparseArray<int> s1;
        assert(s1.Size() == 0);
        s1.Resize(128);
        assert(s1.Size() == 0);
        assert(s1.Capacity() == 128);
    }

    {   // Adding elements to array
        SparseArray<int> s1;
        size_t i1 = s1.Add(1);
        size_t i2 = s1.Add(12);
        size_t i3 = s1.Add(45);

        assert(i1 == 0);
        assert(i2 == 1);
        assert(i3 == 2);

        assert(*s1.Get(0) == 1);
        assert(*s1.Get(1) == 12);
        assert(*s1.Get(2) == 45);

        s1.Remove(1);

        assert(*s1.Get(0) == 1);
        assert(*s1.Get(2) == 45);

        size_t i4 = s1.Add(32);
        size_t i5 = s1.Add(78);

        assert(i4 == 1);
        assert(i5 == 3);

        assert(*s1.Get(1) == 32);
        assert(*s1.Get(3) == 78);

        s1.Resize(2);

        assert(*s1.Get(0) == 1);
        assert(*s1.Get(1) == 32);

        auto it = s1.begin();
        assert(*it == 1);
        it++;
        assert(*it == 32);
    }

    {   // Iteration test
        auto data = GenerateTestData(100);
        SparseArray<int> s1;
        s1.Resize(100);
        for (auto v : data) {
            s1.Add(v);
        }

        auto it = s1.begin();
        for (int i = 0; i < 100; i++) {
            assert(*it == data[i]);
            ++it;
        }

        for (size_t i = 0; i < 100; i += 2) {
            s1.Remove(i);
        }

        it = s1.begin();
        for (int i = 1; i < 100; i += 2) {
            assert(*it == data[i]);
            ++it;
        }
    }
}
