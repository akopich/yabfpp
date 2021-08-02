#include <boost/program_options.hpp>
#include <fstream>
#include <iostream>
#include <optional>

#include "ContextBuilderModule.h"
#include "Expr.h"
#include "parser.h"

using namespace boost;
namespace po = boost::program_options;

std::optional<std::string> readFile(const std::string& inputPath) {
    const char COMMENT_SEPARATOR = ';';
    std::ifstream input(inputPath);
    if (!input.good())
        return std::nullopt;
    std::string program;
    for (std::string line; getline(input, line);) {
        program += line.substr(0, line.find(COMMENT_SEPARATOR, 0));
    }
    input >> program;
    return program;
}

/**
 * cmake -Bbuild -H. -DCMAKE_BUILD_TYPE=Release
 * cmake --build build --target all -j 20
 * @return
 */
int main(int ac, char* av[]) {
    std::string inputPath;
    std::string outPath;

    po::options_description desc("CLI options");
    desc.add_options()
            ("help", "produce help message")
            ("input-file", po::value<std::string>(&inputPath), "input file")
            ("output-file,o", po::value<std::string>(&outPath)->default_value("a.ll"), "float value");

    po::positional_options_description p;
    p.add("input-file", -1);
    po::variables_map vm;
    po::store(po::command_line_parser(ac, av).
            options(desc).positional(p).run(), vm);
    po::notify(vm);

    if (vm.count("help") > 0) {
        std::cout << "Usage: options_description [options]\n";
        std::cout << desc;
        return 0;
    }

    if (vm.count("input-file") == 0) {
        std::cout << "fatal error: no input files" << std::endl;
        std::cout << "compilation terminated." << std::endl;
        return 0;
    }

    std::optional<std::string> program = readFile(inputPath);
    if (!program.has_value()) {
        std::cout << "Input file doesn't exist" << std::endl;
        return 1;
    }

    auto cbm = createContextBuilderModule();
    auto machine = cbm.init(2);
    auto expr = parse(cbm, program.value());
    expr->generate(machine);
    cbm.finalizeAndPrintIRtoFile(outPath);
    return 0;
}
