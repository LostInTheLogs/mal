#pragma once
#include <memory>
#include <string>
#include <unordered_map>

#include "types.h"

// using EvalEnv = std::unordered_map<std::string, std::shared_ptr<MalType>>;

class EvalEnv {
  public:
    void set(const MalSymbol& key, std::shared_ptr<MalType> value);
    std::shared_ptr<MalType> get(const MalSymbol& key) const;

  private:
    std::shared_ptr<EvalEnv> outer = nullptr;
    std::unordered_map<std::string, std::shared_ptr<MalType>> data;
};
