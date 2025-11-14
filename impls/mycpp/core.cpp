#include "core.h"

#include <fstream>
#include <iostream>
#include <memory>
#include <sstream>
#include <stdexcept>

#include "env.h"
#include "printer.h"
#include "reader.h"
#include "types.h"
#include "utils.h"

using std::string, std::make_shared, std::initializer_list, std::pair,
    std::shared_ptr;

namespace {

auto mal_eq(const shared_ptr<MalType>& a, const shared_ptr<MalType>& b)
    -> bool {
    auto& aref = *a;
    auto& bref = *b;

    auto list_a = dyn<MalListLike>(a);
    auto list_b = dyn<MalListLike>(b);
    if (list_a && list_b) {
        if (list_a->size() != list_b->size()) {
            return false;
        }

        for (size_t i = 0; i < list_a->size(); i++) {
            if (!mal_eq(list_a->at(i), list_b->at(i))) {
                return false;
            }
        }
        return true;
    }

    if (typeid(aref) != typeid(bref)) {
        return false;
    }

    auto a_str = pr_str(a, false);
    auto b_str = pr_str(b, false);
    return a_str == b_str;
};

}  // namespace

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

    constexpr auto get_mal_bool = [](bool val) -> shared_ptr<MalType> {
        if (val) {
            return make_shared<MalTrue>();
        }
        return make_shared<MalFalse>();
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
        {"list", std::make_shared<MalFunc>([](MalFuncArgs args) {
             return make_shared<MalList>(args.begin(), args.end());
         })},
        {"list?",
         make_mal_func(  //
             1,
             [&](MalFuncArgs args) -> shared_ptr<MalType> {
                 return get_mal_bool(dyn<MalList>(args[0]) != nullptr);
             })},
        {"empty?",
         make_mal_func(  //
             1,
             [&](MalFuncArgs args) -> shared_ptr<MalType> {
                 if (auto list = dyn<MalList>(args[0])) {
                     return get_mal_bool(list->empty());
                 }
                 if (auto vec = dyn<MalVec>(args[0])) {
                     return get_mal_bool(vec->empty());
                 }

                 throw std::runtime_error("not a list");
             })},
        {"count",
         make_mal_func(  //
             1,
             [](MalFuncArgs args) -> shared_ptr<MalType> {
                 size_t len = 0;
                 if (auto list = dyn<MalList>(args[0])) {
                     len = list->size();
                 } else if (auto vec = dyn<MalVec>(args[0])) {
                     len = vec->size();
                 }
                 return make_shared<MalInt>(len);
             })},
        {"=",
         make_mal_func(  //
             2,
             [&](MalFuncArgs args) -> shared_ptr<MalType> {
                 return get_mal_bool(mal_eq(args[0], args[1]));
             })},
        {"pr-str",
         std::make_shared<MalFunc>([](MalFuncArgs args) -> shared_ptr<MalType> {
             if (args.size() == 0) {
                 return make_shared<MalString>("");
             }
             std::string ret;
             for (size_t i = 0; i < args.size(); i++) {
                 if (i != 0) {
                     ret += " ";
                 }
                 ret += pr_str(args[i], true);
             }
             return make_shared<MalString>(ret.c_str());
         })},
        {"str", std::make_shared<MalFunc>([](MalFuncArgs args) {
             std::string ret;
             for (const auto& arg : args) {
                 ret += pr_str(arg, false);
             }
             return make_shared<MalString>(ret.c_str());
         })},
        {"prn", std::make_shared<MalFunc>([](MalFuncArgs args) {
             std::string ret;
             for (size_t i = 0; i < args.size(); i++) {
                 if (i != 0) {
                     ret += " ";
                 }
                 ret += pr_str(args[i], true);
             }
             std::cout << ret << '\n';
             return make_shared<MalNil>();
         })},
        {"println", std::make_shared<MalFunc>([](MalFuncArgs args) {
             std::string ret;
             for (size_t i = 0; i < args.size(); i++) {
                 if (i != 0) {
                     ret += " ";
                 }
                 ret += pr_str(args[i], false);
             }
             std::cout << ret << '\n';
             return make_shared<MalNil>();
         })},
        {"read-string",
         make_mal_func(  //
             1,
             [](MalFuncArgs args) {
                 auto str = dyn<MalString>(args[0]);
                 return read_str(*str);
             })},
        {"slurp",
         make_mal_func(  //
             1,
             [](MalFuncArgs args) {
                 auto path = dyn<MalString>(args[0]);
                 std::ifstream file(*path);
                 if (!file) {
                     throw std::runtime_error("error reading file");
                 }
                 std::ostringstream sstr;
                 sstr << file.rdbuf();
                 return make_shared<MalString>(sstr.str());
             })},
    };
    static EvalEnv root_env(list);

    return root_env;
}
