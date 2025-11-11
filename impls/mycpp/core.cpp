#include "core.h"

#include "env.h"
#include "types.h"
#include "utils.h"

using std::string, std::make_shared, std::initializer_list, std::pair,
    std::shared_ptr;

EvalEnv create_root_env() {
    constexpr auto make_mal_func = [](unsigned int arg_count,
                                      const std::function<MalFuncSig>& f) {
        return std::make_shared<MalFunc>(arg_count, f);
    };

    constexpr auto make_int_func = [make_mal_func](const auto op) {
        return make_mal_func(2, [op](MalFuncArgs args) {
            auto a = dyn<MalInt>(args[0])->get();
            auto b = dyn<MalInt>(args[1])->get();
            return make_shared<MalInt>(op(a, b));
        });
    };

    static initializer_list<pair<const string, shared_ptr<MalType>>> list = {
        {"+", make_int_func(std::plus<>())},
        {"-", make_int_func(std::minus<>())},
        {"*", make_int_func(std::multiplies<>())},
        {"/", make_int_func(std::divides<>())},
    };
    static EvalEnv root_env(list);

    return root_env;
}
