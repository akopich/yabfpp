//
// Created by valerij on 8/1/21.
//

#ifndef YABFPP_PARSER_H
#define YABFPP_PARSER_H

#include "Source.h"
#include "Expr.h"
#include <map>
#include <string>

class Parser {
private:
    std::map<std::string, size_t> functionName2argNumber;

    Expr parseExpr(Source::Iterator& i);

    template<typename P>
    std::string parseWithPredicate(Source::Iterator& i, P predicate) {
        std::string name;
        while (!i.isEnd() && predicate(*i)) {
            name += *i;
            i++;
        }
        return name;
    }

    Int8Expr parseInt8Expr(Source::Iterator& i, bool defaultOneAllowed);

    Expr parse(Source::Iterator& i);

    std::string parseVariableName(Source::Iterator& i);

    std::string parseIntLiteral(Source::Iterator& i);

    std::vector<std::string> parseFunctionArgumentList(Source::Iterator& i);

    std::vector<Int8Expr> parseCallFunctionArgumentList(Source::Iterator& i);

    Expr parseBFFunctionDefinition(Source::Iterator& i);

    Expr parseBFFunctionCall(Source::Iterator& i);

    Expr parseIfElseExpr(Source::Iterator& i);

    Expr parseAddExpr(Source::Iterator& i, char leadingChar);

    Expr parseMovePtrExpr(Source::Iterator& i, char leadingChar);

    Expr parseLoopExpr(Source::Iterator& i);

public:

    Expr parse(const Source& src);
};


#endif //YABFPP_PARSER_H
