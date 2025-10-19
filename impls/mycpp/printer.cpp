#include "printer.h"

#include <string>

#include "types.h"

std::string pr_str(MalType* mal_type) {
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

    return "[?]";
}
