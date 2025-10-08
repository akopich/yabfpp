#pragma once

#include "../Source.h"
#include <string>
#include <vector>
#include <random>
#include <stack>
#include <string>


inline constexpr Source generateBFProgram(size_t length, size_t seed) {
    const std::vector<char> commands = {'>', '<', '+', '-', '.', ',', '[', ']', '*', '_'};

    std::random_device rd;
    std::mt19937 gen(seed);
    std::uniform_int_distribution<> distrib(0, commands.size() - 1);
    std::uniform_int_distribution<> num_distrib(0, 255);

    std::string program = "";
    std::stack<char> bracket_stack;

    for (size_t i = 0; i < length; ++i) {
        char command = commands[distrib(gen)];
        std::string num_str;

        switch (command) {
            case '[':
                program += command;
                bracket_stack.push(command);
                break;
            case ']':
                if (!bracket_stack.empty()) {
                    program += command;
                    bracket_stack.pop();
                }
                break;
            case '_':
                program += command;
                num_str = std::to_string(num_distrib(gen));
                program += num_str;
                i += num_str.length();
                break;
            default:
                program += command;
                break;
        }
    }

    // After generating the program, close any remaining open brackets
    while (!bracket_stack.empty()) {
        program += ']';
        bracket_stack.pop();
    }

    return getSource({program}, /*legacyMode=*/ false);
}
