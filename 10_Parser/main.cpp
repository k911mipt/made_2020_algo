#include <fstream>
#include <iostream>
#include <vector>

#include "./memcheck_crt.h"
#include "./lexer.hpp"
#include "./parser.hpp"
#include "./interpreter.hpp"


int main() {
    //ENABLE_CRT; // TODO Fix memory leaks!

    std::ifstream input_stream("input.txt");
    Lexer lexer(input_stream);
    Parser parser(lexer);
    Interpreter interpreter(parser);
    std::cout << "scope:" << "\n";

    interpreter.interprete();
    for (auto const& [key, val] : interpreter.scope) {
        std::cout << key << " = " << val << "\n";
    }
    return EXIT_SUCCESS;
}