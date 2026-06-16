#include "lexer/lexer.hpp"
#include "parser/parser.hpp"
#include <iostream>
#include <fstream>
#include <sstream>
#include <cstring>

namespace darix {

void printUsage(const char* program) {
    std::cout << "DariX Programming Language v1.0.0\n\n";
    std::cout << "Usage: " << program << " [options] [script.drx]\n\n";
    std::cout << "Options:\n";
    std::cout << "  --help, -h     Show this help message\n";
    std::cout << "  --version, -v  Show version information\n";
    std::cout << "  --debug        Enable debug output (show tokens and AST)\n";
    std::cout << "  --check        Syntax check only (don't execute)\n";
    std::cout << "  --repl         Start interactive REPL\n";
    std::cout << "\nIf no script is provided, starts REPL mode.\n";
}

void printVersion() {
    std::cout << "DariX 1.0.0\n";
    std::cout << "A hybrid Python/Dart inspired programming language\n";
    std::cout << "Built with C++23\n";
}

void printTokens(const std::vector<Token>& tokens) {
    std::cout << "\n=== Tokens ===\n";
    for (const auto& token : tokens) {
        if (token.type == TokenType::EOF_TOKEN) {
            std::cout << "[EOF]\n";
            break;
        }
        std::cout << token.toString() << "\n";
    }
}

std::string readFile(const std::string& path) {
    std::ifstream file(path);
    if (!file.is_open()) {
        throw std::runtime_error("Could not open file: " + path);
    }
    std::stringstream buffer;
    buffer << file.rdbuf();
    return buffer.str();
}

int runFile(const std::string& path, bool debug, bool checkOnly) {
    try {
        std::string source = readFile(path);
        
        // Lexing
        Lexer lexer(source);
        auto tokens = lexer.tokenize();
        
        if (lexer.hasErrors()) {
            std::cerr << "Lexer errors:\n";
            for (const auto& error : lexer.getErrors()) {
                std::cerr << "  " << error.toString() << "\n";
            }
            return 1;
        }
        
        if (debug) {
            printTokens(tokens);
        }
        
        if (checkOnly) {
            // Parsing
            Parser parser(tokens);
            auto program = parser.parse();
            
            if (parser.hasErrors()) {
                std::cerr << "Parser errors:\n";
                for (const auto& error : parser.getErrors()) {
                    std::cerr << "  " << error.toString() << "\n";
                }
                return 1;
            }
            
            std::cout << "Syntax check passed!\n";
            return 0;
        }
        
        // Parsing
        Parser parser(tokens);
        auto program = parser.parse();
        
        if (parser.hasErrors()) {
            std::cerr << "Parser errors:\n";
            for (const auto& error : parser.getErrors()) {
                std::cerr << "  " << error.toString() << "\n";
            }
            return 1;
        }
        
        // Interpretation would go here
        std::cout << "Program parsed successfully (" 
                  << program.statements.size() << " statements)\n";
        std::cout << "Interpreter not yet implemented - Phase 1 complete\n";
        
        return 0;
        
    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << "\n";
        return 1;
    }
}

void repl() {
    std::cout << "DariX REPL v1.0.0\n";
    std::cout << "Type 'exit' or Ctrl+D to quit\n\n";
    
    while (true) {
        std::cout << "> ";
        std::string line;
        if (!std::getline(std::cin, line)) {
            std::cout << "\n";
            break;
        }
        
        if (line == "exit" || line == "quit") {
            break;
        }
        
        if (line.empty()) {
            continue;
        }
        
        // Simple expression evaluation
        Lexer lexer(line);
        auto tokens = lexer.tokenize();
        
        if (lexer.hasErrors()) {
            for (const auto& error : lexer.getErrors()) {
                std::cerr << error.toString() << "\n";
            }
            continue;
        }
        
        Parser parser(tokens);
        auto program = parser.parse();
        
        if (parser.hasErrors()) {
            for (const auto& error : parser.getErrors()) {
                std::cerr << error.toString() << "\n";
            }
            continue;
        }
        
        std::cout << "[Parsed " << program.statements.size() << " statement(s)]\n";
    }
}

} // namespace darix

int main(int argc, char* argv[]) {
    using namespace darix;
    
    std::string filename;
    bool debug = false;
    bool checkOnly = false;
    bool replMode = false;
    
    // Parse arguments
    for (int i = 1; i < argc; ++i) {
        const char* arg = argv[i];
        
        if (strcmp(arg, "--help") == 0 || strcmp(arg, "-h") == 0) {
            printUsage(argv[0]);
            return 0;
        }
        
        if (strcmp(arg, "--version") == 0 || strcmp(arg, "-v") == 0) {
            printVersion();
            return 0;
        }
        
        if (strcmp(arg, "--debug") == 0) {
            debug = true;
        } else if (strcmp(arg, "--check") == 0) {
            checkOnly = true;
        } else if (strcmp(arg, "--repl") == 0) {
            replMode = true;
        } else if (arg[0] == '-') {
            std::cerr << "Unknown option: " << arg << "\n";
            printUsage(argv[0]);
            return 1;
        } else {
            filename = arg;
        }
    }
    
    try {
        if (replMode || filename.empty()) {
            repl();
        } else {
            return runFile(filename, debug, checkOnly);
        }
    } catch (const std::exception& e) {
        std::cerr << "Fatal error: " << e.what() << "\n";
        return 1;
    }
    
    return 0;
}
