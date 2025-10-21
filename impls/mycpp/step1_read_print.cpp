#include <iostream>

#include "printer.h"
#include "reader.h"

using std::string, std::shared_ptr;

namespace {

shared_ptr<MalType> eval(shared_ptr<MalType> input) {
    return input;
}

void print(const string& out) {
    std::cout << out << '\n';
}

void rep(const string& str) {
    shared_ptr<MalType> out = nullptr;

    try {
        auto input = read_str(str);
        if (input == nullptr) {
            return;
        }
        out = eval(input);
        print(pr_str(out));
    } catch (std::runtime_error& e) {
        std::cout << e.what() << "\n";
    }
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
