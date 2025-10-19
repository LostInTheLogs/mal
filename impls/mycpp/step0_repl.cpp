#include <iostream>

using std::string;

namespace {
string read(string str) {
    return str;
}

string eval(string input) {
    return input;
}

void print(string out) {
    std::cout << out << '\n';
}

void rep(string str) {
    auto input = read(str);
    auto out = eval(input);
    print(out);
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
