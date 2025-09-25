#include <fstream>
#include <iostream>
#include <print>
#include <optional>

#include "CompilerState.h"
#include "Expr.h"
#include "parser.h"
#include "Source.h"
#include "llvm/TargetParser/Host.h"

#define ARGS_NOEXCEPT
#include "third_party/args.hxx"

std::optional<std::vector<std::string>> readFile(const std::string& inputPath) {
    const char COMMENT_SEPARATOR = ';';
    std::ifstream input(inputPath);
    if (!input.good())
        return std::nullopt;
    std::vector<std::string> program;
    for (std::string line; getline(input, line);) {
        program.push_back(line.substr(0, line.find(COMMENT_SEPARATOR, 0)));
    }
    return program;
}


int main(int ac, char* av[]) {
    args::ArgumentParser argsParser("YABFPP compiler.");

    args::HelpFlag help(argsParser, "HELP", "Show this help menu.", {'h', "help"});
    args::Positional<std::string> inputPath(argsParser, "input-file", "Input file name");
    args::ValueFlag<std::string> outputPath(argsParser, "output-file", "Output file name.", {'o', "output-file"}, "a.ll");
    args::ValueFlag<int> initialTapeSize(argsParser, "tape-size", "Initial tape size.", {'t', "tape-size"}, 30000);
    args::Flag legacyModeFlag(argsParser, "legacy-mode", "Legacy mode switch.", {'l', "legacy-mode"}, false);
    args::ValueFlag<std::string> targetTriple(argsParser, "target", "The target triple is a string in the format of: CPU_TYPE-VENDOR-OPERATING_SYSTEM or CPU_TYPE-VENDOR-KERNEL-OPERATING_SYSTEM.", {'t', "target"}, llvm::sys::getDefaultTargetTriple());

    bool parseSuccess = argsParser.ParseCLI(ac, av);

    // Check if the parser ran successfully.
    if (argsParser.GetError() != args::Error::None) {
        std::cout << argsParser;
        return 0; 
    }

    if (!inputPath) {
        std::println("fatal error: no input files");
        std::println("compilation terminated.");
        return 0;
    }

    std::optional<std::vector<std::string>> program = readFile(args::get(inputPath));
    if (!program.has_value()) {
        std::println("Input file doesn't exist");
        return 1;
    }

    Source src = getSource(program.value(), get(legacyModeFlag));

    auto state = initCompilerState(get(inputPath), get(targetTriple));
    BFMachine bfMachine = createBFMachine(state.get(), initialTapeSize);
    Parser parser;
    auto expr = parser.parse(src);
    expr.generate(bfMachine);
    state->finalizeAndPrintIRtoFile(get(outputPath));
    return 0;
}
