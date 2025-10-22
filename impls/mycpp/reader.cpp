#include "reader.h"

#include <algorithm>
#include <cassert>
#include <iostream>
#include <memory>
#include <regex>
#include <stdexcept>
#include <string>
#include <vector>

#include "types.h"

using std::string, std::vector, std::shared_ptr, std::make_shared;

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

shared_ptr<MalType> read_atom(Reader& reader) {
    string token = reader.next();

    if (token == "nil") {
        return make_shared<MalNil>();
    }
    if (token == "true") {
        return make_shared<MalTrue>();
    }
    if (token == "false") {
        return make_shared<MalFalse>();
    }

    if (token.at(0) == ':') {
        return make_shared<MalKeyword>(std::move(token));
    }

    if (token.at(0) == ';') {
        return nullptr;
    }

    try {
        int integer = std::stoi(token);
        return make_shared<MalInt>(integer);

    } catch (...) {
    }

    return make_shared<MalSymbol>(std::move(token));
}

vector<shared_ptr<MalType>> read_sequence(Reader& reader, const string& start,
                                          const string& end) {
    vector<shared_ptr<MalType>> items;
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
            throw std::runtime_error("unbalanced parenthesis");
        }

        auto form = read_form(reader);
        assert(form);
        items.push_back(form);
    }
}

shared_ptr<MalList> read_list(Reader& reader) {
    auto items = read_sequence(reader, "(", ")");
    return make_shared<MalList>(std::move(items));
}

shared_ptr<MalVec> read_vector(Reader& reader) {
    auto items = read_sequence(reader, "[", "]");
    return make_shared<MalVec>(std::move(items));
}

shared_ptr<MalHashMap> read_hashmap(Reader& reader) {
    auto items = read_sequence(reader, "{", "}");
    return make_shared<MalHashMap>(std::move(items));
}

shared_ptr<MalString> read_string(Reader& reader) {
    auto str = reader.next();
    if (str.length() < 2 or str[0] != '"' or str[str.length() - 1] != '"') {
        throw std::runtime_error("unbalanced quotes");
    }

    string out;

    bool escape = false;
    for (size_t i = 1; i < str.length() - 1; i++) {
        auto in_char = str[i];
        if (escape) {
            if (in_char == 'n') {
                out += '\n';
            } else if (in_char == '\\') {
                out += '\\';
            } else if (in_char == '"') {
                out += '"';
            } else {
                throw std::runtime_error("unknown escape sequence");
            }
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

    return make_shared<MalString>(std::move(out));
}

shared_ptr<MalList> read_quote(Reader& reader, const string& prefix,
                               string symbol) {
    auto quote = reader.next();
    assert(quote == prefix);

    auto element = read_form(reader);

    vector<std::shared_ptr<MalType>> vec{
        make_shared<MalSymbol>(std::move(symbol)),
        element,
    };

    return make_shared<MalList>(std::move(vec));
}

shared_ptr<MalList> read_meta(Reader& reader) {
    auto quote = reader.next();
    assert(quote == "^");

    auto meta = read_form(reader);
    auto element = read_form(reader);

    vector<std::shared_ptr<MalType>> vec{
        make_shared<MalSymbol>("with-meta"),
        element,
        meta,
    };

    return make_shared<MalList>(std::move(vec));
}

}  // namespace

shared_ptr<MalType> read_form(Reader& reader) {
    string token;
    try {
        token = reader.peek();
    } catch (std::out_of_range&) {
        return nullptr;
    }

    switch (token[0]) {
        case '(':
            return read_list(reader);
        case '[':
            return read_vector(reader);
        case '{':
            return read_hashmap(reader);
        case '"':
            return read_string(reader);
        case '\'':
            return read_quote(reader, "\'", "quote");
        case '`':
            return read_quote(reader, "`", "quasiquote");
        case '@':
            return read_quote(reader, "@", "deref");
        case '~':
            if (token == "~@") {
                return read_quote(reader, "~@", "splice-unquote");
            } else {
                return read_quote(reader, "~", "unquote");
            }
        case '^':
            return read_meta(reader);
        default:
            return read_atom(reader);
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

shared_ptr<MalType> read_str(const string& str) {
    auto tokens = tokenize(str);
    Reader reader{tokens};

    return read_form(reader);
}
