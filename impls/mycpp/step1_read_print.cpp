#include <iostream>

#include "printer.h"
#include "reader.h"

using std::string;

namespace {

MalType* eval(MalType* input) {
    return input;
}

void print(const string& out) {
    std::cout << out << '\n';
}

void rep(const string& str) {
    MalType* out = nullptr;

    try {
        auto* input = read_str(str);
        out = eval(input);
    } catch (std::runtime_error& e) {
        std::cout << e.what() << "\n";
    }

    print(pr_str(out));
}
}  // namespace

int main(int /*argc*/, char* /*argv*/[]) {
    while (true) {
        std::cout << "user> ";

        string str;
        std::getline(std::cin, str);
        if (std::cin.eof()) {
            return 0;
        }

        rep(str);
    }
    return 0;
}
