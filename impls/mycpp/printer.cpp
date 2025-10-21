#include "printer.h"

#include <memory>
#include <regex>
#include <string>
#include <utility>

#include "types.h"

using std::string, std::shared_ptr, std::dynamic_pointer_cast;

namespace {
string pr_seq(std::span<shared_ptr<MalType>> seq, bool readably, string start,
              const string& end) {
    string ret = std::move(start);
    for (size_t i = 0; i < seq.size(); i++) {
        if (i != 0) {
            ret += " ";
        }
        ret += pr_str(seq[i], readably);
    }
    ret += end;
    return ret;
}
}  // namespace

std::string pr_str(shared_ptr<MalType> mal_type, bool readably) {
    if (auto str = dynamic_pointer_cast<MalString>(mal_type)) {
        auto ret_str = std::string(str->c_str());
        if (readably) {
            ret_str = std::regex_replace(ret_str, std::regex(R"(\\)"), "\\\\");
            ret_str = std::regex_replace(ret_str, std::regex(R"(")"), "\\\"");
            ret_str = std::regex_replace(ret_str, std::regex("\n"), "\\n");
        }
        return {"\"" + ret_str + "\""};
    }
    if (auto keyword = dynamic_pointer_cast<MalKeyword>(mal_type)) {
        return {keyword->c_str()};
    }
    if (auto symbol = dynamic_pointer_cast<MalSymbol>(mal_type)) {
        return {symbol->c_str()};
    }
    if (auto integer = dynamic_pointer_cast<MalInt>(mal_type)) {
        return std::to_string(integer->get());
    }
    if (auto vec = dynamic_pointer_cast<MalVec>(mal_type)) {
        return pr_seq(std::span(*vec), readably, "[", "]");
    }
    if (auto list = dynamic_pointer_cast<MalList>(mal_type)) {
        return pr_seq(std::span(*list), readably, "(", ")");
    }
    if (auto map = dynamic_pointer_cast<MalHashMap>(mal_type)) {
        return pr_seq(std::span(*map), readably, "{", "}");
    }
    if (dynamic_pointer_cast<MalNil>(mal_type) != nullptr) {
        return {"nil"};
    }
    if (dynamic_pointer_cast<MalTrue>(mal_type) != nullptr) {
        return {"true"};
    }
    if (dynamic_pointer_cast<MalFalse>(mal_type) != nullptr) {
        return {"false"};
    }

    return "[?]";
}
