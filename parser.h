//
// Created by valerij on 8/1/21.
//

#ifndef YABFPP_PARSER_H
#define YABFPP_PARSER_H
#include "Source.h"

std::unique_ptr<Expr> parse(const CompilerState& state, const Source& src);


#endif //YABFPP_PARSER_H
