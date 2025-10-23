#include "env.h"

#include <utility>

void EvalEnv::set(const MalSymbol& key, std::shared_ptr<MalType> value) {
    data[key] = std::move(value);
}
std::shared_ptr<MalType> EvalEnv::get(const MalSymbol& key) const {
    if (!data.contains(key)) {
        throw std::runtime_error("unknown symbol");
    }
    auto ret = data.at(key);
    return ret;
}
