#include "test_runner.h"

#include <functional>
#include <string>
#include <optional>

using namespace std;

template <typename T>
class LazyValue {
public:
    explicit LazyValue(std::function<T()> init)
        : initFunc_(move(init)), val_() {}

    bool HasValue() const {
        return val_.has_value();
    }
    const T& Get() const {
        if(!val_) {
            val_ = move(initFunc_());
        }
        return *val_;
    }

private:
    function<T()> initFunc_;
    mutable optional<T> val_;
};

void UseExample() {
    const string big_string = "Giant amounts of memory";

    LazyValue<string> lazy_string([&big_string] { return big_string; });

    ASSERT(!lazy_string.HasValue());
    ASSERT_EQUAL(lazy_string.Get(), big_string);
    ASSERT_EQUAL(lazy_string.Get(), big_string);
}

void TestInitializerIsntCalled() {
    bool called = false;

    {
        LazyValue<int> lazy_int([&called] {
            called = true;
            return 0;
        });
    }
    ASSERT(!called);
}

int main() {
    TestRunner tr;
    RUN_TEST(tr, UseExample);
    RUN_TEST(tr, TestInitializerIsntCalled);
    return 0;
}
