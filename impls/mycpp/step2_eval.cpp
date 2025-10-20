#include <functional>
#include <iostream>
#include <map>
#include <span>
#include <stdexcept>
#include <vector>

#include "printer.h"
#include "reader.h"
#include "types.h"

using std::string;

namespace {

MalType* eval(MalType* ast) {
    std::map<string, std::function<MalType*(std::span<MalType*>)>> eval_env = {
        {"+",
         [](std::span<MalType*> args) -> MalType* {
             if (args.size() != 2) {
                 throw std::runtime_error("+ requires 2 args");
             }
             auto a = dynamic_cast<MalInt*>(args[0])->get();
             auto b = dynamic_cast<MalInt*>(args[1])->get();
             return new MalInt(a + b);
         }},
        {"-",
         [](std::span<MalType*> args) -> MalType* {
             if (args.size() != 2) {
                 throw std::runtime_error("- requires 2 args");
             }
             auto a = dynamic_cast<MalInt*>(args[0])->get();
             auto b = dynamic_cast<MalInt*>(args[1])->get();
             return new MalInt(a - b);
         }},
        {"*",
         [](std::span<MalType*> args) -> MalType* {
             if (args.size() != 2) {
                 throw std::runtime_error("* requires 2 args");
             }
             auto a = dynamic_cast<MalInt*>(args[0])->get();
             auto b = dynamic_cast<MalInt*>(args[1])->get();
             return new MalInt(a * b);
         }},
        {"/",
         [](std::span<MalType*> args) -> MalType* {
             if (args.size() != 2) {
                 throw std::runtime_error("/ requires 2 args");
             }
             auto a = dynamic_cast<MalInt*>(args[0])->get();
             auto b = dynamic_cast<MalInt*>(args[1])->get();
             return new MalInt(a / b);
         }},
    };
    return ast;
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
