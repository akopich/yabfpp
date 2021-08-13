//
// Created by valerij on 8/13/21.
//

#include "SyntaxErrorException.h"

#include <utility>

const char* SyntaxErrorException::what() const noexcept {
    const std::string& fullMessage = "Syntax error at line "
                                     + std::to_string(this->it.getLine())
                                     + " at position "
                                     + std::to_string(this->it.getLinePosition())
                                     + ". " + this->reasonToThrow;

    auto* cstr = new std::vector<char>(fullMessage.c_str(), fullMessage.c_str() + fullMessage.size() + 1);
    return &(cstr->operator[](0));
}

SyntaxErrorException::SyntaxErrorException(const Source::Iterator& it, std::string msg) : it(it), reasonToThrow(
        std::move(msg)) {}
