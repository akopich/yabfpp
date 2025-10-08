//
// Created by valerij on 8/13/21.
//

#ifndef YABFPP_SYNTAXERROREXCEPTION_H
#define YABFPP_SYNTAXERROREXCEPTION_H

#include "Source.h"
#include <cstdlib>
#include <print>


[[noreturn]] void syntaxError(auto it, std::string msg) {
    std::println( "Syntax error at line {} at position {}. {}", 
          std::to_string(it.getLine())
        , std::to_string(it.getLinePosition())
        , msg);
    std::abort();
}


#endif //YABFPP_SYNTAXERROREXCEPTION_H
