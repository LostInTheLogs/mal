#include "reader.h"

#include <algorithm>
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

    if (token == "nil") {
        return new MalNil();
    }
    if (token == "true") {
        return new MalTrue();
    }
    if (token == "false") {
        return new MalFalse();
    }

    if (token.at(0) == '0') {
        return new MalKeyword(std::move(token));
    }

    if (token.at(0) == ';') {
        return nullptr;
    }

    try {
        int integer = std::stoi(token);
        return new MalInt(integer);

    } catch (...) {
    }

    return new MalSymbol(std::move(token));
}

vector<MalType*> read_sequence(Reader& reader, const string& start,
                               const string& end) {
    vector<MalType*> items;
    if (reader.next() != start) {
        throw std::runtime_error("this is not a list");
    }

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
    auto items = read_sequence(reader, "(", ")");
    return new MalList(std::move(items));
}

MalVec* read_vector(Reader& reader) {
    auto items = read_sequence(reader, "[", "]");
    return new MalVec(std::move(items));
}

MalHashMap* read_hashmap(Reader& reader) {
    auto items = read_sequence(reader, "{", "}");
    return new MalHashMap(std::move(items));
}

MalString* read_string(Reader& reader) {
    auto str = reader.next();
    if (str.length() < 2 or str[0] != '"' or str[str.length() - 1] != '"') {
        throw std::runtime_error("unbalanced quotes");
    }

    string out;

    bool escape = false;
    for (size_t i = 1; i < str.length() - 1; i++) {
        auto in_char = str[i];
        if (escape) {
            out += in_char;
            escape = false;
            continue;
        }
        if (in_char == '\\') {
            escape = true;
            continue;
        }
        out += in_char;
    }
    if (escape) {
        throw std::runtime_error("incomplete escape / unbalanced quotes");
    }

    return new MalString(std::move(out));
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
        case '"':
            return read_string(reader);
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
