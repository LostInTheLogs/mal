#include "core.h"

#include "env.h"
#include "types.h"
#include "utils.h"

using std::string, std::make_shared, std::shared_ptr;

EvalEnv create_root_env() {
    static EvalEnv root_env{};

    root_env.set(
        MalSymbol("+"),
        make_shared<MalFunc>([](MalFuncArgs args) -> shared_ptr<MalType> {
            if (args.size() != 2) {
                throw std::runtime_error("+ requires 2 args");
            }
            auto a = dyn<MalInt>(args[0])->get();
            auto b = dyn<MalInt>(args[1])->get();
            return make_shared<MalInt>(a + b);
        }));
    root_env.set(
        MalSymbol("-"),
        make_shared<MalFunc>([](MalFuncArgs args) -> shared_ptr<MalType> {
            if (args.size() != 2) {
                throw std::runtime_error("- requires 2 args");
            }
            auto a = dyn<MalInt>(args[0])->get();
            auto b = dyn<MalInt>(args[1])->get();
            return make_shared<MalInt>(a - b);
        }));
    root_env.set(
        MalSymbol("*"),
        make_shared<MalFunc>([](MalFuncArgs args) -> shared_ptr<MalType> {
            if (args.size() != 2) {
                throw std::runtime_error("* requires 2 args");
            }
            auto a = dyn<MalInt>(args[0])->get();
            auto b = dyn<MalInt>(args[1])->get();
            return make_shared<MalInt>(a * b);
        }));
    root_env.set(
        MalSymbol("/"),
        make_shared<MalFunc>([](MalFuncArgs args) -> shared_ptr<MalType> {
            if (args.size() != 2) {
                throw std::runtime_error("/ requires 2 args");
            }
            auto a = dyn<MalInt>(args[0])->get();
            auto b = dyn<MalInt>(args[1])->get();
            return make_shared<MalInt>(a / b);
        }));
    return root_env;
}
