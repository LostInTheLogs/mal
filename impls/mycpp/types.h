#pragma once

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

class MalList : public MalType, public std::vector<MalType*> {
  public:
    explicit MalList(std::vector<MalType*>&& items)
        : std::vector<MalType*>(std::move(items)) {}
};

class MalVec : public MalType, public std::vector<MalType*> {
  public:
    explicit MalVec(std::vector<MalType*>&& items)
        : std::vector<MalType*>(std::move(items)) {}
};

class MalHashMap : public MalType, public std::vector<MalType*> {
  public:
    explicit MalHashMap(std::vector<MalType*>&& items)
        : std::vector<MalType*>(std::move(items)) {}
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
