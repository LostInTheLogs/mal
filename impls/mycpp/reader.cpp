#include "reader.h"

#include <cassert>
#include <iostream>
#include <regex>
#include <string>
#include <vector>

#include "types.h"

using std::string, std::vector;

Reader::Reader(const std::vector<std::string>& tokens) : tokens(tokens) {}

string Reader::peek() {
    return this->tokens.at(this->position);
}

string Reader::next() {
    auto ret = peek();
    this->position++;
    return ret;
}

namespace {

MalType* read_atom(Reader& reader) {
    string token = reader.next();

    try {
        int integer = std::stoi(token);
        return new MalInt(integer);

    } catch (...) {
    }

    return new MalSymbol(std::move(token));
}

vector<MalType*> read_mal_types_between(Reader& reader, string start,
                                        string end) {
    vector<MalType*> items;
    if (reader.peek() != start) {
        throw std::runtime_error("this is not a list");
    }
    reader.next();  // `start`

    while (true) {
        try {
            string next_str = reader.peek();
            if (next_str == end) {
                reader.next();  // 'end'
                return items;
            }
        } catch (std::out_of_range&) {
            for (auto* item : items) {
                delete item;
            }
            throw std::runtime_error("unbalanced parenthesis");
        }

        auto* form = read_form(reader);
        assert(form);
        items.push_back(form);
    }
}

MalList* read_list(Reader& reader) {
    auto items = read_mal_types_between(reader, "(", ")");
    return new MalList(std::move(items));
}

MalVec* read_vector(Reader& reader) {
    auto items = read_mal_types_between(reader, "[", "]");
    return new MalVec(std::move(items));
}

MalHashMap* read_hashmap(Reader& reader) {
    auto items = read_mal_types_between(reader, "{", "}");
    return new MalHashMap(std::move(items));
}

}  // namespace

MalType* read_form(Reader& reader) {
    string token;
    try {
        token = reader.peek();
    } catch (std::out_of_range&) {
        return nullptr;
    }

    switch (token[0]) {
        case '(':
            return read_list(reader);
            break;
        case '[':
            return read_vector(reader);
            break;
        case '{':
            return read_hashmap(reader);
            break;
        default:
            return read_atom(reader);
            break;
    }
}

std::vector<string> tokenize(const string& str) {
    std::vector<string> tokens;
    std::regex tokens_regex(
        R"__([\s,]*(~@|[\[\]{}()'`~^@]|"(?:\\.|[^\\"])*"?|;.*|[^\s\[\]{}('"`,;)]*))__");

    auto begin = std::sregex_iterator(str.begin(), str.end(), tokens_regex);
    auto end = std::sregex_iterator();

    for (auto it = begin; it != end; ++it) {
        std::string match = (*it)[1].str();
        if (!match.empty()) {
            tokens.push_back(match);
        }
    }

    return tokens;
}

MalType* read_str(const string& str) {
    auto tokens = tokenize(str);
    Reader reader{tokens};

    return read_form(reader);
}
