#include <functional>
#include <iostream>
#include <span>
#include <stdexcept>
#include <vector>

#include "printer.h"
#include "reader.h"
#include "types.h"

using std::string, std::shared_ptr, std::dynamic_pointer_cast, std::make_shared;

using EvalEnv = std::unordered_map<string, shared_ptr<MalType>>;

namespace {

shared_ptr<MalType> eval(shared_ptr<MalType> ast, const EvalEnv& eval_env) {
    if (auto symbol = dynamic_pointer_cast<MalSymbol>(ast)) {
        auto str_sym = dynamic_pointer_cast<string>(symbol);

        if (!eval_env.contains(*str_sym)) {
            throw std::runtime_error("unknown symbol");
        }
        auto ret = eval_env.at(*str_sym);
        return ret;
    }

    if (auto list = dynamic_pointer_cast<MalList>(ast);
        (list != nullptr) and list->size() > 0) {
        std::vector<shared_ptr<MalType>> evaluated;
        for (const auto& el : *list) {
            auto evalled = eval(el, eval_env);
            evaluated.push_back(evalled);
        }

        auto fn = dynamic_pointer_cast<MalFunc>(evaluated[0]);
        if (fn == nullptr) {
            throw std::runtime_error(
                "trying to call sth that is not a function");
        }

        return (*fn)(std::span(evaluated.begin() + 1, evaluated.end()));
    }

    if (auto list = dynamic_pointer_cast<MalVec>(ast);
        (list != nullptr) and list->size() > 0) {
        std::vector<shared_ptr<MalType>> evaluated;
        for (const auto& el : *list) {
            auto evalled = eval(el, eval_env);
            evaluated.push_back(evalled);
        }

        return make_shared<MalVec>(std::move(evaluated));
    }

    if (auto list = dynamic_pointer_cast<MalHashMap>(ast);
        (list != nullptr) and list->size() > 0) {
        std::vector<shared_ptr<MalType>> evaluated;
        for (const auto& el : *list) {
            auto evalled = eval(el, eval_env);
            evaluated.push_back(evalled);
        }

        return make_shared<MalHashMap>(std::move(evaluated));
    }

    return ast;
}

void print(const string& out) {
    std::cout << out << '\n';
}

void rep(const string& str) {
    static EvalEnv eval_env = {
        {"+", make_shared<MalFunc>([](MalFuncArgs args) -> shared_ptr<MalType> {
             if (args.size() != 2) {
                 throw std::runtime_error("+ requires 2 args");
             }
             auto a = dynamic_pointer_cast<MalInt>(args[0])->get();
             auto b = dynamic_pointer_cast<MalInt>(args[1])->get();
             return make_shared<MalInt>(a + b);
         })},
        {"-", make_shared<MalFunc>([](MalFuncArgs args) -> shared_ptr<MalType> {
             if (args.size() != 2) {
                 throw std::runtime_error("- requires 2 args");
             }
             auto a = dynamic_pointer_cast<MalInt>(args[0])->get();
             auto b = dynamic_pointer_cast<MalInt>(args[1])->get();
             return make_shared<MalInt>(a - b);
         })},
        {"*", make_shared<MalFunc>([](MalFuncArgs args) -> shared_ptr<MalType> {
             if (args.size() != 2) {
                 throw std::runtime_error("* requires 2 args");
             }
             auto a = dynamic_pointer_cast<MalInt>(args[0])->get();
             auto b = dynamic_pointer_cast<MalInt>(args[1])->get();
             return make_shared<MalInt>(a * b);
         })},
        {"/", make_shared<MalFunc>([](MalFuncArgs args) -> shared_ptr<MalType> {
             if (args.size() != 2) {
                 throw std::runtime_error("/ requires 2 args");
             }
             auto a = dynamic_pointer_cast<MalInt>(args[0])->get();
             auto b = dynamic_pointer_cast<MalInt>(args[1])->get();
             return make_shared<MalInt>(a / b);
         })},
    };

    shared_ptr<MalType> out = nullptr;

    try {
        auto input = read_str(str);
        std::cout << pr_str(input) << "wow\n";
        out = eval(input, eval_env);
    } catch (std::runtime_error& e) {
        std::cout << e.what() << "\n";
    }

    print(pr_str(out));
}
}  // namespace

int main(int /*argc*/, char* /*argv*/[]) {
    while (true) {
        std::cout << "user> ";

        string str;
        std::getline(std::cin, str);
        if (std::cin.eof()) {
            return 0;
        }

        rep(str);
    }
    return 0;
}
