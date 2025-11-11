#pragma once
#include <memory>
#include <optional>
#include <string>
#include <unordered_map>
#include <utility>

#include "types.h"

// using EvalEnv = std::unordered_map<std::string, std::shared_ptr<MalType>>;

class EvalEnv {
  public:
    EvalEnv(std::initializer_list<
                std::pair<const std::string, std::shared_ptr<MalType>>>
                list = {},
            std::shared_ptr<EvalEnv> outer = {},
            std::span<const MalSymbol> binds = {},
            std::span<std::shared_ptr<MalType>> exprs = {})
        : outer(std::move(outer)), data(list) {
        if (binds.size() != exprs.size()) {
            throw std::runtime_error("invalid length of exprs");
        }

        for (size_t i = 0; i < binds.size(); i++) {
            set(binds[i], exprs[i]);
        }
    }

    void set(const MalSymbol& key, std::shared_ptr<MalType> value);
    std::shared_ptr<MalType> get(const MalSymbol& key) const;
    bool contains(const MalSymbol& key) const;

  private:
    std::shared_ptr<EvalEnv> outer = nullptr;
    std::unordered_map<std::string, std::shared_ptr<MalType>> data;
};
