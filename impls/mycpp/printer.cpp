#include "printer.h"

#include <memory>
#include <regex>
#include <string>
#include <utility>

#include "types.h"
#include "utils.h"

using std::string, std::shared_ptr;

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

std::string pr_str(const shared_ptr<MalType>& mal_type, bool readably) {
    if (auto str = dyn<MalString>(mal_type)) {
        auto ret_str = std::string(str->c_str());
        if (readably) {
            ret_str = std::regex_replace(ret_str, std::regex(R"(\\)"), "\\\\");
            ret_str = std::regex_replace(ret_str, std::regex(R"(")"), "\\\"");
            ret_str = std::regex_replace(ret_str, std::regex("\n"), "\\n");
            return "\"" + ret_str + "\"";
        }
        return ret_str;
    }
    if (auto keyword = dyn<MalKeyword>(mal_type)) {
        return {keyword->c_str()};
    }
    if (auto symbol = dyn<MalSymbol>(mal_type)) {
        return {symbol->c_str()};
    }
    if (auto integer = dyn<MalInt>(mal_type)) {
        return std::to_string(integer->get());
    }
    if (auto vec = dyn<MalVec>(mal_type)) {
        return pr_seq(std::span(*vec), readably, "[", "]");
    }
    if (auto list = dyn<MalList>(mal_type)) {
        return pr_seq(std::span(*list), readably, "(", ")");
    }
    if (auto map = dyn<MalHashMap>(mal_type)) {
        return pr_seq(std::span(*map), readably, "{", "}");
    }
    if (dyn<MalFunc>(mal_type) || dyn<MalFnFunc>(mal_type)) {
        return {"#<function>"};
    }
    if (dyn<MalNil>(mal_type)) {
        return {"nil"};
    }
    if (dyn<MalTrue>(mal_type)) {
        return {"true"};
    }
    if (dyn<MalFalse>(mal_type)) {
        return {"false"};
    }
    if (dyn<MalEmpty>(mal_type)) {
        return {""};
    }

    return "[?]";
}
