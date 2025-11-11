#include "core.h"

#include <iostream>
#include <memory>

#include "env.h"
#include "printer.h"
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
            auto a = dyn<MalInt>(args[0]);
            auto b = dyn<MalInt>(args[1]);
            if (!a || !b) {
                throw std::runtime_error("function arguments aren't ints");
            }
            return make_shared<MalInt>(op(a->get(), b->get()));
        });
    };

    constexpr auto make_bool_func = [make_mal_func](const auto check) {
        return make_mal_func(  //
            2, [check](MalFuncArgs args) -> shared_ptr<MalType> {
                auto a = dyn<MalInt>(args[0])->get();
                auto b = dyn<MalInt>(args[1])->get();
                if (check(a, b)) {
                    return make_shared<MalTrue>();
                }
                return make_shared<MalFalse>();
            });
    };

    static initializer_list<pair<const string, shared_ptr<MalType>>> list = {
        {"+", make_int_func(std::plus<>())},
        {"-", make_int_func(std::minus<>())},
        {"*", make_int_func(std::multiplies<>())},
        {"/", make_int_func(std::divides<>())},
        {"<", make_bool_func(std::less<>())},
        {"<=", make_bool_func(std::less_equal<>())},
        {">", make_bool_func(std::greater<>())},
        {">=", make_bool_func(std::greater_equal<>())},
        {"prn",
         make_mal_func(  //
             1,
             [](MalFuncArgs args) {
                 std::cout << pr_str(args[0], true) << '\n';
                 return make_shared<MalNil>();
             })},
        {"list", std::make_shared<MalFunc>([](MalFuncArgs args) {
             return make_shared<MalList>(args.begin(), args.end());
         })},
        {"list?",
         make_mal_func(  //
             1,
             [](MalFuncArgs args) -> shared_ptr<MalType> {
                 if (dyn<MalList>(args[0])) {
                     return make_shared<MalTrue>();
                 }
                 return make_shared<MalFalse>();
             })},
        {"empty?",
         make_mal_func(  //
             1,
             [](MalFuncArgs args) -> shared_ptr<MalType> {
                 auto list = dyn<MalList>(args[0]);
                 if (!list) {
                     throw std::runtime_error("not a list");
                 }
                 if (list->empty()) {
                     return make_shared<MalTrue>();
                 }
                 return make_shared<MalFalse>();
             })},
        {"count",
         make_mal_func(  //
             1,
             [](MalFuncArgs args) -> shared_ptr<MalType> {
                 size_t len = 0;
                 if (auto list = dyn<MalList>(args[0])) {
                     len = list->size();
                 }
                 return make_shared<MalInt>(len);
             })},

        {"=",
         make_mal_func(  //
             2,
             [](MalFuncArgs args) -> shared_ptr<MalType> {
                 auto a = pr_str(args[0], false);
                 auto b = pr_str(args[1], false);
                 if (a == b) {
                     return make_shared<MalTrue>();
                 }
                 return make_shared<MalFalse>();
             })},
    };
    static EvalEnv root_env(list);

    return root_env;
}
