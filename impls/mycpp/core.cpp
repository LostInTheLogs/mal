#include "core.h"

#include "env.h"
#include "types.h"
#include "utils.h"

using std::string, std::make_shared, std::shared_ptr;

EvalEnv create_root_env() {
    static EvalEnv root_env{};

    const auto register_func = [](const char* name, unsigned int arg_count,
                                  const auto&& f) {
        const auto abcd = [name, arg_count, f = std::forward<decltype(f)>(f)](
                              MalFuncArgs args) -> shared_ptr<MalType> {
            if (args.size() != arg_count) {
                throw std::runtime_error(string(name) + " requires 2 args");
            }
            return f(args);
        };
        root_env.set(MalSymbol(name), std::make_shared<MalFunc>(abcd));
    };

    const auto register_int_func = [register_func](const char* name, auto op) {
        register_func(name, 2, [op](MalFuncArgs args) {
            auto a = dyn<MalInt>(args[0])->get();
            auto b = dyn<MalInt>(args[1])->get();
            return make_shared<MalInt>(op(a, b));
        });
    };

    register_int_func("+", std::plus<>());
    register_int_func("-", std::minus<>());
    register_int_func("*", std::multiplies<>());
    register_int_func("/", std::divides<>());
    return root_env;
}
