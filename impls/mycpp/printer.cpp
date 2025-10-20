#include "printer.h"

#include <regex>
#include <string>

#include "types.h"

std::string pr_str(MalType* mal_type, bool readably) {
    if (auto* str = dynamic_cast<MalString*>(mal_type)) {
        auto ret_str = std::string(str->c_str());
        if (readably) {
            ret_str = std::regex_replace(ret_str, std::regex(R"(\\)"), "\\\\");
            ret_str = std::regex_replace(ret_str, std::regex(R"(")"), "\\\"");
            ret_str = std::regex_replace(ret_str, std::regex("\n"), "\\n");
        }
        return {"\"" + ret_str + "\""};
    }
    if (auto* symbol = dynamic_cast<MalSymbol*>(mal_type)) {
        return {symbol->c_str()};
    }
    if (auto* integer = dynamic_cast<MalInt*>(mal_type)) {
        return std::to_string(integer->get());
    }
    if (auto* list = dynamic_cast<MalList*>(mal_type)) {
        std::string ret = "(";
        for (size_t i = 0; i < list->size(); i++) {
            if (i != 0) {
                ret += " ";
            }
            ret += pr_str(list->at(i));
        }
        ret += ")";
        return ret;
    }
    if (dynamic_cast<MalNil*>(mal_type) != nullptr) {
        return {"nil"};
    }
    if (dynamic_cast<MalTrue*>(mal_type) != nullptr) {
        return {"true"};
    }
    if (dynamic_cast<MalFalse*>(mal_type) != nullptr) {
        return {"false"};
    }

    return "[?]";
}
