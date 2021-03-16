#include "parser.hpp"
#include <iostream>

int main() {
    auto short_welcome = "Welcome to the recursive decent AST parser.";
    auto long_welcome = 
        "Input a line of code, and the parser will return the AST. "
        "Exit by closing input stream e.g. ctrl+d (unix) or ctrl+z (win).";
    auto prompt = "parser> ";
    
    std::cout << short_welcome << std::endl;
    std::cout << long_welcome << std::endl;
    std::cout << std::endl << prompt << std::flush;


    for ( // Infinite REPL loop
        std::string line;
        std::getline(std::cin, line);
        std::cout << prompt << std::flush
    ) {

        auto expression = parser::from_string(line).expression();
        std::cout << expression << std::endl;
    }

    std::cout << "Exiting REPL..." << std::endl;

    return EXIT_SUCCESS;
}

