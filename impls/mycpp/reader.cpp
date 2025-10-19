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

MalType* read_atom(Reader& reader) {
    string token = reader.next();

    try {
        int integer = std::stoi(token);
        return new MalInt(integer);

    } catch (...) {
    }

    return new MalSymbol(std::move(token));
}

MalList* read_list(Reader& reader) {
    vector<MalType*> items;
    reader.next();  // opening (

    while (true) {
        try {
            string next_str = reader.peek();
            if (next_str == ")") {
                reader.next();  // closing )
                return new MalList(std::move(items));
            }
        } catch (std::out_of_range&) {
            fflush(stdout);
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
