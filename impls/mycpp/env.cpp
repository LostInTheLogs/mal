#include "env.h"

#include <format>
#include <memory>
#include <utility>

void EvalEnv::set(const MalSymbol& key, std::shared_ptr<MalType> value) {
    data[key] = std::move(value);
}
std::shared_ptr<MalType> EvalEnv::get(const MalSymbol& key) const {
    if (!data.contains(key)) {
        if (outer != nullptr) {
            return outer->get(key);
        }

        const std::string& key_str = key;
        throw std::runtime_error(std::format("symbl '{}' not found", key_str));
    }
    auto ret = data.at(key);
    return ret;
}

bool EvalEnv::contains(const MalSymbol& key) const {
    return data.contains(key);
}
