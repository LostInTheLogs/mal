#pragma once

#include <memory>
#include <string>
#include <vector>

#include "types.h"

class Reader {
  public:
    explicit Reader(const std::vector<std::string>& tokens);
    Reader(Reader&&) = default;
    Reader(const Reader&) = default;
    Reader& operator=(Reader&&) = default;
    Reader& operator=(const Reader&) = default;
    ~Reader() = default;

    std::string next();
    std::string peek();

  private:
    std::vector<std::string> tokens;
    unsigned int position = 0;
};

std::shared_ptr<MalType> read_form(Reader& reader);
std::vector<std::string> tokenize(const std::string& str);
std::shared_ptr<MalType> read_str(const std::string& str);
