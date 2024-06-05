#ifndef __DAEMON_CORE_UTILITIES_DEFER__
#define __DAEMON_CORE_UTILITIES_DEFER__

#include <functional>

#define var_defer__(x) defer__ ## x
#define var_defer_(x) var_defer__(x)

#define ref_defer(ops) defer var_defer_(__COUNTER__)([&]{ ops; }) // Capture all by ref
#define val_defer(ops) defer var_defer_(__COUNTER__)([=]{ ops; }) // Capture all by val
#define none_defer(ops) defer var_defer_(__COUNTER__)([ ]{ ops; }) // Capture nothing


namespace core::utilities
{
    class defer
    {
    public:
        using action = std::function<void(void)>;

    public:
        defer(const action &act)
            : _action(act) {}
        defer(action &&act)
            : _action(std::move(act)) {}

        defer(const defer &act) = delete;
        defer &operator=(const defer &act) = delete;

        defer(defer &&act) = delete;
        defer &operator=(defer &&act) = delete;

        ~defer()
        {
            _action();
        }

    private:
        action _action;
    };
}

#endif // __DAEMON_CORE_UTILITIES_DEFER__