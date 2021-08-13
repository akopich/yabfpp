//
// Created by valerij on 8/1/21.
//

#include <iostream>
#include "CompilerState.h"
#include <algorithm>
#include <cctype>
#include <memory>
#include <vector>
#include <set>


#include "Expr.h"
#include "Source.h"
#include "SyntaxErrorException.h"

std::unique_ptr<Int8Expr> parseTrailingVariable(Source::Iterator& i, bool defaultOneAllowed);

Expr* parse(const CompilerState& state, Source::Iterator& i);

std::string parseVariableName(Source::Iterator& i);

std::string parseIntLiteral(Source::Iterator& i);


Expr* parseToken(const CompilerState& state, Source::Iterator& i) {
    char c = *i;
    i++;
    switch (c) {
        case '^':
            return new WriteToVariable(parseVariableName(i));
        case '_': {
            return new AssignExpressionValueToTheCurrentCell(parseTrailingVariable(i, false));
        }
        case '+':
        case '-': {
            auto add = parseTrailingVariable(i, true);
            if (c == '-') {
                add = std::make_unique<MinusInt8Expr>(move(add));
            }
            return new AddExpr(move(add));
        }
        case '.':
            return new PrintExpr();
        case ',':
            return new ReadExpr();
        case '*':
            return new PrintIntExpr();
        case '<':
        case '>': {
            auto step = parseTrailingVariable(i, true);
            if (c == '<') {
                step = std::make_unique<MinusInt8Expr>(move(step));
            }
            return new MovePtrExpr(move(step));
        }
        case '[': {
            auto loop = new LoopExpr(parse(state, i));
            i++;
            return loop;
        }
        default:
            throw SyntaxErrorException(i, "Unexpected symbol.");
    }
}

template<typename P>
std::string parseWithPredicate(Source::Iterator& i, P predicate) {
    std::string name;
    while (!i.isEnd() && predicate(*i)) {
        name += *i;
        i++;
    }
    return name;
}

std::string parseVariableName(Source::Iterator& i) {
    return parseWithPredicate(i, [](const char c) { return isalpha(c); });
}

std::string parseIntLiteral(Source::Iterator& i) {
    return parseWithPredicate(i, [](const char c) { return isdigit(c); });
}

Expr* parse(const CompilerState& state, Source::Iterator& i) {
    std::vector<Expr*> v;
    while (!i.isEnd() && *i != ']') {
        v.push_back(parseToken(state, i));
    }
    return new ListExpr(v);
}

std::unique_ptr<Int8Expr> parseTrailingVariable(Source::Iterator& i, bool defaultOneAllowed) {
    std::string varname = parseVariableName(i);
    if (!varname.empty())
        return std::make_unique<VariableInt8Expr>(varname);

    std::string intLiteral = parseIntLiteral(i);
    if (!intLiteral.empty())
        return std::make_unique<ConstInt8Expr>(std::stoi(intLiteral));

    if (defaultOneAllowed)
        return std::make_unique<ConstInt8Expr>(1);

    throw SyntaxErrorException(i, "_ should be followed by a variable name or by an integer literal");
}

std::unique_ptr<Expr> parse(const CompilerState& state, const Source& src) {
    auto it = src.begin();
    return std::unique_ptr<Expr>(parse(state, it));
}