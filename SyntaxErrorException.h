//
// Created by valerij on 8/13/21.
//

#ifndef YABFPP_SYNTAXERROREXCEPTION_H
#define YABFPP_SYNTAXERROREXCEPTION_H

#include <exception>
#include "Source.h"


class SyntaxErrorException : public std::exception {
private:
    Source::Iterator it;
    std::string reasonToThrow;
public:
    SyntaxErrorException(const Source::Iterator& it, std::string msg);

    const char* what() const noexcept override;
};


#endif //YABFPP_SYNTAXERROREXCEPTION_H
