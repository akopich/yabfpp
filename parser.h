//
// Created by valerij on 8/1/21.
//

#ifndef YABFPP_PARSER_H
#define YABFPP_PARSER_H

#include "Source.h"
#include <map>
#include <string>

class Parser {
private:
    std::map<std::string, int> functionName2argNumber;

    std::unique_ptr<Expr> parseToken(const CompilerState& state, Source::Iterator& i);

    template<typename P>
    std::string parseWithPredicate(Source::Iterator& i, P predicate) {
        std::string name;
        while (!i.isEnd() && predicate(*i)) {
            name += *i;
            i++;
        }
        return name;
    }

    std::unique_ptr<Int8Expr> parseInt8Expr(Source::Iterator& i, bool defaultOneAllowed);

    std::unique_ptr<Expr> parse(const CompilerState& state, Source::Iterator& i);

    std::string parseVariableName(Source::Iterator& i);

    std::string parseIntLiteral(Source::Iterator& i);

    std::vector<std::string> parseFunctionArgumentList(Source::Iterator& i);

    std::vector<std::shared_ptr<Int8Expr>> parseCallFunctionArgumentList(Source::Iterator& i);

public:
    std::unique_ptr<Expr> parse(const CompilerState& state, const Source& src);
};


#endif //YABFPP_PARSER_H
