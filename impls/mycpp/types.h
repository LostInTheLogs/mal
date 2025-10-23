#pragma once

#include <functional>
#include <memory>
#include <span>
#include <string>
#include <vector>

class MalType {};

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
class MalFunc : public MalType,
                public std::function<std::shared_ptr<MalType>(
                    std::span<std::shared_ptr<MalType>>)> {
  public:
    using std::function<std::shared_ptr<MalType>(
        std::span<std::shared_ptr<MalType>>)>::function;
};
