#include "lexer.h"
#include "parser.h"
#include <iostream>
#include <fstream>
#include <sstream>

int main(int argc, char* argv[]) {
    if (argc < 2) {
        std::cout << "Usage: darix <file.dax>" << std::endl;
        std::cout << "       darix --repl     (interactive mode)" << std::endl;
        return 1;
    }
    
    // REPL mode
    if (std::string(argv[1]) == "--repl") {
        std::cout << "DariX JIT v2.0 - Interactive Mode" << std::endl;
        std::cout << "Type 'exit' to quit." << std::endl;
        
        std::string line;
        while (true) {
            std::cout << "> ";
            if (!std::getline(std::cin, line)) break;
            
            if (line == "exit" || line == "quit") break;
            if (line.empty()) continue;
            
            // Simple single-expression evaluation for REPL
            darix::Lexer lexer(line);
            auto tokens = lexer.scanTokens();
            
            darix::Parser parser(tokens);
            auto statements = parser.parse();
            
            if (parser.hasErrors()) {
                for (const auto& error : parser.errors()) {
                    std::cerr << error << std::endl;
                }
                continue;
            }
            
            std::cout << "Parsed " << statements.size() << " statement(s)" << std::endl;
        }
        
        return 0;
    }
    
    // File mode
    std::string filename = argv[1];
    std::ifstream file(filename);
    
    if (!file.is_open()) {
        std::cerr << "Error: Could not open file " << filename << std::endl;
        return 1;
    }
    
    std::stringstream buffer;
    buffer << file.rdbuf();
    std::string source = buffer.str();
    
    // Lexical analysis
    darix::Lexer lexer(source);
    auto tokens = lexer.scanTokens();
    
    std::cout << "Lexed " << tokens.size() << " tokens" << std::endl;
    
    // Parsing
    darix::Parser parser(tokens);
    auto statements = parser.parse();
    
    if (parser.hasErrors()) {
        for (const auto& error : parser.errors()) {
            std::cerr << error << std::endl;
        }
        return 1;
    }
    
    std::cout << "Parsed " << statements.size() << " statement(s)" << std::endl;
    
    // TODO: Add interpreter/JIT compilation here
    
    return 0;
}
