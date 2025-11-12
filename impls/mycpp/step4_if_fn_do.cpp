#include <algorithm>
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

shared_ptr<MalType> eval(shared_ptr<MalType> ast,
                         const shared_ptr<EvalEnv>& eval_env);

shared_ptr<MalType> eval_def(const shared_ptr<MalList>& list,
                             const shared_ptr<EvalEnv>& eval_env) {
    auto key = *dyn<MalSymbol>(list->at(1));
    auto val = eval(list->at(2), eval_env);

    eval_env->set(key, val);
    return val;
}

shared_ptr<MalType> eval_let(const shared_ptr<MalList>& list,
                             const shared_ptr<EvalEnv>& eval_env) {
    auto def_env = make_shared<EvalEnv>(*eval_env);

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
        def_env->set(key, val);
    }
    return eval(list->at(2), def_env);
}

shared_ptr<MalType> eval_do(const shared_ptr<MalList>& list,
                            const shared_ptr<EvalEnv>& eval_env) {
    std::vector<shared_ptr<MalType>> evaluated;
    for (const auto& el : std::span(*list).subspan(1)) {
        auto evalled = eval(el, eval_env);
        evaluated.push_back(evalled);
    }
    return evaluated.at(evaluated.size() - 1);
}
shared_ptr<MalType> eval_if(const shared_ptr<MalList>& list,
                            const shared_ptr<EvalEnv>& eval_env) {
    auto condition = eval(list->at(1), eval_env);
    auto if_true = list->at(2);

    if (dyn<MalFalse>(condition) || dyn<MalNil>(condition)) {
        if (list->size() < 4) {
            return make_shared<MalNil>();
        }
        auto if_false = list->at(3);
        return eval(if_false, eval_env);
    }
    return eval(if_true, eval_env);
}
shared_ptr<MalType> eval_fn(const shared_ptr<MalList>& list,
                            const shared_ptr<EvalEnv>& eval_env) {
    std::span<shared_ptr<MalType>> binds_span;
    if (auto binds_list = dyn<MalList>(list->at(1))) {
        binds_span = *binds_list;
    } else {
        auto binds_vec = dyn<MalVec>(list->at(1));
        binds_span = *binds_vec;
    }

    auto body = list->at(2);

    std::vector<MalSymbol> binds{};
    for (const auto& bind : binds_span) {
        binds.push_back(*dyn<MalSymbol>(bind));
    }

    const auto fn = [eval_env, binds, body](MalFuncArgs args) {
        auto env = make_shared<EvalEnv>(
            std::initializer_list<
                std::pair<const std::string, std::shared_ptr<MalType>>>{},
            eval_env, std::span(binds), args);
        return eval(body, env);
    };
    shared_ptr<MalType> ret;
    return make_shared<MalFunc>(fn);
}

shared_ptr<MalType> eval_list(const shared_ptr<MalList>& list,
                              const shared_ptr<EvalEnv>& eval_env) {
    if (auto first_symbol_ptr = dyn<MalSymbol>(list->at(0))) {
        const auto& first_symbol = *first_symbol_ptr;

        if (first_symbol == "def!") {
            return eval_def(list, eval_env);
        }
        if (first_symbol == "let*") {
            return eval_let(list, eval_env);
        }
        if (first_symbol == "do") {
            return eval_do(list, eval_env);
        }
        if (first_symbol == "if") {
            return eval_if(list, eval_env);
        }
        if (first_symbol == "fn*") {
            return eval_fn(list, eval_env);
        }
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

shared_ptr<MalType> eval(shared_ptr<MalType> ast,
                         const shared_ptr<EvalEnv>& eval_env) {
    if (auto debug_eval_symbol = MalSymbol("DEBUG-EVAL");
        eval_env->contains(debug_eval_symbol)) {
        auto debug_eval = eval_env->get(debug_eval_symbol);
        if (dyn<MalNil>(debug_eval) == nullptr and
            dyn<MalFalse>(debug_eval) == nullptr) {
            std::cout << "EVAL: " << pr_str(ast, true) << "\n";
        }
    }

    if (auto symbol = dyn<MalSymbol>(ast)) {
        return eval_env->get(*symbol);
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

void rep(const string& str, bool quiet = false) {
    static auto root_env = make_shared<EvalEnv>(create_root_env());

    shared_ptr<MalType> out = nullptr;

    try {
        auto input = read_str(str);
        // std::cout << pr_str(input) << "\n";
        out = eval(input, root_env);
    } catch (std::runtime_error& e) {
        std::cout << e.what() << "\n";
    }

    if (!quiet) {
        print(pr_str(out));
    }
}
}  // namespace

int main(int /*argc*/, char* /*argv*/[]) {
    rep("(def! not (fn* (a) (if a false true)))", true);

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
