#include "test_common.h"

#include "../Signal_.h"

namespace {
    double static_func(int param1, float param2) {
        return param1 + param2;
    }

    class AAA {
    public:
        double member_func(int param1, float param2) {
            return param1 - param2;
        }

        double member_const_func(int param1, float param2) const {
            return param1 - 0.5f * param2;
        }
    };
}

void test_signal() {

    {   // Delegate bind

        {   // Static func
            sys::Delegate<double(int, float)> del;
            del.Bind<::static_func>();
            assert(del(1, 2.4f) == Approx(3.4));
        }

        {   // Member func
            sys::Delegate<double(int, float)> del;
            AAA a;
            del.Bind<AAA, &AAA::member_func>(&a);
            assert(del(2, 1.4f) == Approx(0.6));
        }

        {   // Member const func
            sys::Delegate<double(int, float)> del;
            AAA a;
            const AAA &_a = a;
            del.Bind<AAA, &AAA::member_const_func>(&_a);
            assert(del(2, 1.4f) == Approx(1.3));
        }
    }

    {   // Signal connect
        sys::Signal<double(int, float)> sig;

        AAA a;

        sig.Connect<::static_func>();
        sig.Connect<AAA, &AAA::member_func>(&a);
        sig.Connect<AAA, &AAA::member_const_func>(&a);

        assert(sig.size() == 3);

        assert(sig.FireOne(0, 5, 2.2f) == Approx(7.2));
        assert(sig.FireOne(1, 5, 2.2f) == Approx(2.8));
        assert(sig.FireOne(2, 5, 2.2f) == Approx(3.9));

        assert(sig.FireL(4, 1.0f) == Approx(3.5));

        std::vector<double> result = sig.FireV(4, 0.3f);
        assert(result.size() == 3);
        assert(result[0] == Approx(4.3));
        assert(result[1] == Approx(3.7));
        assert(result[2] == Approx(3.85));
    }
}
