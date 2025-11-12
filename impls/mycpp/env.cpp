#include "env.h"

#include <algorithm>
#include <format>
#include <memory>
#include <utility>

EvalEnv::EvalEnv(std::initializer_list<
                     std::pair<const std::string, std::shared_ptr<MalType>>>
                     list,
                 std::shared_ptr<EvalEnv> outer,
                 std::span<const MalSymbol> binds,
                 std::span<std::shared_ptr<MalType>> exprs)
    : outer(std::move(outer)), data(list) {
    auto vararg = std::ranges::find(binds, MalSymbol{"&"}) != binds.end();

    if ((!vararg && binds.size() != exprs.size()) ||
        binds.size() > exprs.size() + 2) {
        throw std::runtime_error(
            std::format("invalid length of exprs: {} binds {} exprs",
                        binds.size(), exprs.size()));
    }

    std::optional<MalSymbol> extra;
    auto extra_list = std::make_shared<MalList>();
    for (size_t i = 0; i < std::max(exprs.size(), binds.size()); i++) {
        if (binds[i] == "&") {
            extra = binds[i + 1];
        }

        if (i >= exprs.size()) {
            continue;
        }

        if (extra) {
            extra_list->push_back(exprs[i]);
        } else {
            set(binds[i], exprs[i]);
        }
    }
    if (extra) {
        set(*extra, extra_list);
    }
}

void EvalEnv::set(const MalSymbol& key, std::shared_ptr<MalType> value) {
    data[key] = std::move(value);
}
std::shared_ptr<MalType> EvalEnv::get(const MalSymbol& key) const {
    if (!data.contains(key)) {
        if (outer != nullptr) {
            return outer->get(key);
        }

        const std::string& key_str = key;
        throw std::runtime_error(std::format("symbol '{}' not found", key_str));
    }
    auto ret = data.at(key);
    return ret;
}

bool EvalEnv::contains(const MalSymbol& key) const {
    return data.contains(key);
}
