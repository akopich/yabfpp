#include <boost/program_options.hpp>
#include <fstream>
#include <iostream>
#include <optional>

#include "CompilerState.h"
#include "Expr.h"
#include "parser.h"
#include "Source.h"
#include "llvm/TargetParser/Host.h"

namespace po = boost::program_options;

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
    std::string inputPath;
    std::string outPath;
    std::string targetTriple;
    int initialTapeSize;
    bool legacyMode;

    po::options_description desc("CLI options");

    desc.add_options()
            ("help", "produce help message")
            ("input-file", po::value<std::string>(&inputPath), "input file")
            ("output-file,o", po::value<std::string>(&outPath)->default_value("a.ll"), "Output file name.")
            ("tape-size,t", po::value<int>(&initialTapeSize)->default_value(30000), "Initial tape size.")
            ("legacy-mode,l", po::bool_switch(&legacyMode)->default_value(false), "Legacy mode switch.")
            ("target",
             po::value<std::string>(&targetTriple)->default_value(llvm::sys::getDefaultTargetTriple()),
             "The target triple is a string in the format of: CPU_TYPE-VENDOR-OPERATING_SYSTEM or CPU_TYPE-VENDOR-KERNEL-OPERATING_SYSTEM.");

    po::positional_options_description p;
    p.add("input-file", -1);
    po::variables_map vm;
    po::store(po::command_line_parser(ac, av).options(desc).positional(p).run(), vm);
    po::notify(vm);

    if (vm.count("help") > 0) {
        std::cout << "Usage: yabfpp [options] file" << std::endl;
        std::cout << desc;
        return 0;
    }

    if (vm.count("input-file") == 0) {
        std::cout << "fatal error: no input files" << std::endl;
        std::cout << "compilation terminated." << std::endl;
        return 1;
    }

    std::optional<std::vector<std::string>> program = readFile(inputPath);
    if (!program.has_value()) {
        std::cout << "Input file doesn't exist" << std::endl;
        return 1;
    }
    Source src = getSource(program.value(), legacyMode);

    auto state = initCompilerState(inputPath, targetTriple);
    BFMachine bfMachine = createBFMachine(state.get(), initialTapeSize);
    Parser parser;
    auto expr = parser.parse(src);
    expr->generate(bfMachine);
    state->finalizeAndPrintIRtoFile(outPath);
    return 0;
}
