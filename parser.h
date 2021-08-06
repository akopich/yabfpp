//
// Created by valerij on 8/1/21.
//

#ifndef YABFPP_PARSER_H
#define YABFPP_PARSER_H


std::unique_ptr<Expr> parse(const CompilerState& state, const std::string& s, bool legacyMode);


#endif //YABFPP_PARSER_H
