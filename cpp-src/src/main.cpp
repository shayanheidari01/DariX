#include "darix/ast.hpp"
#include "darix/compiler.hpp"
#include "darix/interpreter.hpp"
#include "darix/lexer.hpp"
#include "darix/object.hpp"
#include "darix/parser.hpp"
#include "darix/version.hpp"
#include "darix/vm.hpp"
#include <cstdio>
#include <cstdlib>
#include <fstream>
#include <iostream>
#include <sstream>
#include <string>
#include <vector>

using namespace darix;

static std::string readFile(const std::string& filename) {
    std::ifstream file(filename);
    if (!file.is_open()) {
        std::cerr << "Error reading file: " << filename << "\n";
        std::exit(1);
    }
    std::stringstream buffer;
    buffer << file.rdbuf();
    return buffer.str();
}

static void printHelp() {
    std::cout << "DariX command line (C++)\n\n";
    std::cout << "Usage:\n";
    std::cout << "  darix run <file.dax|->        Run a script (use '-' for stdin)\n";
    std::cout << "  darix repl                    Start interactive REPL\n";
    std::cout << "  darix eval \"<code>\"            Evaluate a code snippet\n";
    std::cout << "  darix disasm <file.dax>       Disassemble bytecode\n";
    std::cout << "  darix version                 Show version info\n";
    std::cout << "  darix help                    Show this help\n";
}

static std::pair<std::shared_ptr<Program>, std::vector<std::string>> parseCode(const std::string& code, const std::string& filename) {
    Lexer lexer(code, filename);
    Parser parser(lexer);
    auto program = parser.parseProgram();
    return {program, parser.errors()};
}

static void handleParseErrors(const std::vector<std::string>& errors) {
    std::cerr << "Parse Errors Detected:\n";
    std::cerr << "========================\n";
    for (size_t i = 0; i < errors.size(); i++) {
        std::cerr << (i + 1) << ". " << errors[i] << "\n";
    }
    std::cerr << "\nSuggestion: Check your syntax.\n";
    std::exit(1);
}

static void handleRuntimeResult(ObjectPtr result) {
    if (!result) return;
    if (result->type() == ObjectType::ERROR) {
        std::cout << result->inspect() << "\n";
        std::exit(1);
    }
    if (result->type() == ObjectType::EXCEPTION_SIGNAL) {
        std::cout << "Unhandled exception:\n" << result->inspect() << "\n";
        std::exit(1);
    }
}

static ObjectPtr runInterpreter(Program* program) {
    Interpreter interp;
    return interp.interpret(program);
}

static ObjectPtr runVM(Program* program) {
    try {
        Compiler compiler;
        compiler.compile(program);
        auto bc = compiler.bytecode();
        VM machine(bc);
        return machine.run();
    } catch (const std::exception&) {
        return newError("VM compilation failed");
    }
}

static void runAuto(Program* program) {
    auto result = runVM(program);
    if (result && result->type() == ObjectType::ERROR) {
        // VM failed, fall back to interpreter
        handleRuntimeResult(runInterpreter(program));
        return;
    }
    if (result && result->type() == ObjectType::EXCEPTION_SIGNAL) {
        handleRuntimeResult(result);
        return;
    }
    handleRuntimeResult(result);
}

static void runFile(const std::string& filename) {
    auto content = (filename == "-") ? [] {
        std::stringstream buf;
        buf << std::cin.rdbuf();
        return buf.str();
    }() : readFile(filename);

    auto [program, errors] = parseCode(content, filename);
    if (!errors.empty()) handleParseErrors(errors);
    runAuto(program.get());
}

static void runCode(const std::string& code) {
    auto [program, errors] = parseCode(code, "<eval>");
    if (!errors.empty()) handleParseErrors(errors);
    runAuto(program.get());
}

static void disasmFile(const std::string& filename) {
    auto content = readFile(filename);
    auto [program, errors] = parseCode(content, filename);
    if (!errors.empty()) handleParseErrors(errors);
    Compiler compiler;
    compiler.compile(program.get());
    auto bc = compiler.bytecode();
    std::cout << "# Bytecode Instructions:\n";
    std::cout << Disassemble(bc->instructions);
}

int main(int argc, char* argv[]) {
    if (argc <= 1) {
        // REPL mode
        std::cout << "DariX " << versionString() << "\n";
        std::cout << "Type 'exit' to quit.\n";

        Interpreter interp;
        std::string line;
        while (true) {
            std::cout << ">> ";
            if (!std::getline(std::cin, line)) break;
            if (line == "exit" || line == "quit") break;
            if (line.empty()) continue;

            auto [program, errors] = parseCode(line, "<repl>");
            if (!errors.empty()) {
                for (auto& e : errors) std::cerr << e << "\n";
                continue;
            }
            auto result = interp.interpret(program.get());
            if (result && result->type() != ObjectType::NULL_OBJ) {
                std::cout << result->inspect() << "\n";
            }
        }
        return 0;
    }

    std::string command = argv[1];

    if (command == "run") {
        if (argc < 3) {
            std::cerr << "Usage: darix run <file.dax|->\n";
            return 1;
        }
        runFile(argv[2]);
    } else if (command == "eval") {
        if (argc < 3) {
            std::cerr << "Usage: darix eval \"<code>\"\n";
            return 1;
        }
        runCode(argv[2]);
    } else if (command == "disasm") {
        if (argc < 3) {
            std::cerr << "Usage: darix disasm <file.dax>\n";
            return 1;
        }
        disasmFile(argv[2]);
    } else if (command == "version" || command == "-v" || command == "--version") {
        std::cout << versionString() << "\n";
    } else if (command == "help" || command == "-h" || command == "--help") {
        printHelp();
    } else if (command == "repl") {
        // Same as no-args mode
        return main(0, nullptr);
    } else {
        // Try as file
        std::ifstream test(command);
        if (test.good()) {
            runFile(command);
        } else {
            std::cerr << "Unknown command or file: " << command << "\n\n";
            printHelp();
            return 1;
        }
    }

    return 0;
}
