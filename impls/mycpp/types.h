#pragma once

#include <format>
#include <functional>
#include <memory>
#include <span>
#include <string>
#include <vector>

class MalType {
  public:
    MalType() = default;
    MalType(MalType&&) = default;
    MalType(const MalType&) = default;
    MalType& operator=(MalType&&) = default;
    MalType& operator=(const MalType&) = default;
    virtual ~MalType() = default;

  private:
};

class MalList : public MalType, public std::vector<std::shared_ptr<MalType>> {
  public:
    explicit MalList(std::vector<std::shared_ptr<MalType>>&& items)
        : std::vector<std::shared_ptr<MalType>>(std::move(items)) {}
};

class MalVec : public MalType, public std::vector<std::shared_ptr<MalType>> {
  public:
    explicit MalVec(std::vector<std::shared_ptr<MalType>>&& items)
        : std::vector<std::shared_ptr<MalType>>(std::move(items)) {}
};

class MalHashMap : public MalType,
                   public std::vector<std::shared_ptr<MalType>> {
  public:
    explicit MalHashMap(std::vector<std::shared_ptr<MalType>>&& items)
        : std::vector<std::shared_ptr<MalType>>(std::move(items)) {}
};

class MalSymbol : public MalType, public std::string {
  public:
    explicit MalSymbol(std::string&& content)
        : std::string(std::move(content)) {}
};

class MalString : public MalType, public std::string {
  public:
    explicit MalString(std::string&& content)
        : std::string(std::move(content)) {}
};

class MalKeyword : public MalType, public std::string {
  public:
    explicit MalKeyword(std::string&& content)
        : std::string(std::move(content)) {}
};

class MalInt : public MalType {
  public:
    explicit MalInt(int integer) : integer(integer) {};

    [[nodiscard]] int get() const {
        return integer;
    }

  private:
    int integer;
};

class MalNil : public MalType {};
class MalTrue : public MalType {};
class MalFalse : public MalType {};

using MalFuncArgs = std::span<std::shared_ptr<MalType>>;
using MalFuncSig = std::shared_ptr<MalType>(MalFuncArgs);
class MalFunc : public MalType, public std::function<MalFuncSig> {
  public:
    explicit MalFunc(const unsigned int arg_count,
                     std::function<std::shared_ptr<MalType>(MalFuncArgs)> f)
        : std::function<MalFuncSig>(
              [arg_count, f = std::forward<decltype(f)>(f)](
                  MalFuncArgs args) -> std::shared_ptr<MalType> {
                  if (args.size() != arg_count) {
                      throw std::runtime_error(std::format(
                          "fn requires 2 args {} provided", args.size()));
                  }
                  return f(args);
              }) {};

    using std::function<std::shared_ptr<MalType>(
        std::span<std::shared_ptr<MalType>>)>::function;
};
