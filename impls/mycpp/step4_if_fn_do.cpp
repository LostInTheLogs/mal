#include <functional>
#include <iostream>
#include <span>
#include <stdexcept>
#include <vector>

#include "core.h"
#include "env.h"
#include "printer.h"
#include "reader.h"
#include "types.h"
#include "utils.h"

using std::string, std::shared_ptr, std::make_shared;

namespace {

shared_ptr<MalType> eval(shared_ptr<MalType> ast, EvalEnv& eval_env);

shared_ptr<MalType> eval_list(const shared_ptr<MalList>& list,
                              EvalEnv& eval_env) {
    const auto& first_symbol = *dyn<MalSymbol>(list->at(0));

    if (first_symbol == "def!") {
        auto key = *dyn<MalSymbol>(list->at(1));
        auto val = eval(list->at(2), eval_env);
        eval_env.set(key, val);
        return val;
    }

    if (first_symbol == "let*") {
        auto def_env = EvalEnv(eval_env);

        std::span<shared_ptr<MalType>> env_kv_pairs;
        if (auto env_list = dyn<MalList>(list->at(1))) {
            env_kv_pairs = *env_list;
        } else if (auto env_vec = dyn<MalVec>(list->at(1))) {
            env_kv_pairs = *env_vec;
        } else {
            throw std::runtime_error("incorrect 2nd arg to let*");
        }

        for (size_t i = 0; i < env_kv_pairs.size(); i += 2) {
            auto key = *dyn<MalSymbol>(env_kv_pairs[i]);
            auto val = eval(env_kv_pairs[i + 1], def_env);
            def_env.set(key, val);
        }
        return eval(list->at(2), def_env);
    }

    std::vector<shared_ptr<MalType>> evaluated;
    for (const auto& el : *list) {
        auto evalled = eval(el, eval_env);
        evaluated.push_back(evalled);
    }

    auto fn = dyn<MalFunc>(evaluated[0]);
    if (fn == nullptr) {
        throw std::runtime_error("trying to call sth that is not a function");
    }

    return (*fn)(std::span(evaluated.begin() + 1, evaluated.end()));
}

shared_ptr<MalType> eval(shared_ptr<MalType> ast, EvalEnv& eval_env) {
    if (eval_env.contains(MalSymbol("DEBUG-EVAL"))) {
        auto debug_eval = eval_env.get(MalSymbol("DEBUG-EVAL"));
        if (dyn<MalNil>(debug_eval) == nullptr and
            dyn<MalFalse>(debug_eval) == nullptr) {
            std::cout << "EVAL: " << pr_str(ast, true) << "\n";
        }
    }

    if (auto symbol = dyn<MalSymbol>(ast)) {
        return eval_env.get(*symbol);
    }

    if (auto list = dyn<MalList>(ast); (list != nullptr) and list->size() > 0) {
        return eval_list(list, eval_env);
    }

    if (auto list = dyn<MalVec>(ast); (list != nullptr) and list->size() > 0) {
        std::vector<shared_ptr<MalType>> evaluated;
        for (const auto& el : *list) {
            auto evalled = eval(el, eval_env);
            evaluated.push_back(evalled);
        }

        return make_shared<MalVec>(std::move(evaluated));
    }

    if (auto list = dyn<MalHashMap>(ast);
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
    static EvalEnv root_env = create_root_env();

    shared_ptr<MalType> out = nullptr;

    try {
        auto input = read_str(str);
        // std::cout << pr_str(input) << "\n";
        out = eval(input, root_env);
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
