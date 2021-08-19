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

std::unique_ptr<Int8Expr> parseInt8Expr(Source::Iterator& i, bool defaultOneAllowed);

Expr* parse(const CompilerState& state, Source::Iterator& i);

std::string parseVariableName(Source::Iterator& i);

std::string parseIntLiteral(Source::Iterator& i);

std::vector<std::string> parseFunctionArgumentList(Source::Iterator& i);

std::vector<std::shared_ptr<Int8Expr>> parseCallFunctionArgumentList(Source::Iterator& i);

Expr* parseToken(const CompilerState& state, Source::Iterator& i) {
    char c = *i;
    i++;
    switch (c) {
        case '\\':
            return new Return();
        case '@': {
            std::string functionName = parseVariableName(i);
            std::vector<std::string> argNames = parseFunctionArgumentList(i);
            ++i;
            Expr* body = parse(state, i);
            ++i;
            return new BFFunctionDeclaration(functionName, argNames, std::unique_ptr<Expr>(body));
        }
        case '$': {
            std::string functionName = parseVariableName(i);
            auto argExprs = parseCallFunctionArgumentList(i);
            return new BFFunctionCall(functionName, argExprs);
        }
        case '{': {
            auto ifExpr = parse(state, i);
            i++;
            if (*i == '{') {
                i++;
                auto elseExpr = parse(state, i);
                i++;
                return new IfElse(std::unique_ptr<Expr>(ifExpr), std::unique_ptr<Expr>(elseExpr));
            }

            return new IfElse(std::unique_ptr<Expr>(ifExpr), getNoOpExpr());
        }
        case '^':
            return new WriteToVariable(parseVariableName(i));
        case '_': {
            return new AssignExpressionValueToTheCurrentCell(parseInt8Expr(i, false));
        }
        case '+':
        case '-': {
            auto add = parseInt8Expr(i, true);
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
            auto step = parseInt8Expr(i, true);
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

std::vector<std::shared_ptr<Int8Expr>> parseCallFunctionArgumentList(Source::Iterator& i) {
    if (*i != '(')
        throw SyntaxErrorException(i, "opening bracket expected");
    ++i;
    std::vector<std::shared_ptr<Int8Expr>> arguments;
    while (*i != ')') {
        auto argument = parseInt8Expr(i, false);
        arguments.push_back(move(argument));
        if (*i != ',' && *i != ')') {
            throw SyntaxErrorException(i, "a comma, a closing bracket, variable name or an integer literal");
        }
        if (*i == ',')
            i++;
    }
    ++i;
    return arguments;
}

std::vector<std::string> parseFunctionArgumentList(Source::Iterator& i) {
    if (*i != '(')
        throw SyntaxErrorException(i, "opening bracket expected");
    ++i;
    std::vector<std::string> argNames;
    while (*i != ')') {
        argNames.push_back(parseVariableName(i));
        if (*i != ',' && *i != ')') {
            throw SyntaxErrorException(i, "a comma, a closing bracket or an alphabetic character expected");
        }
        if (*i == ',')
            i++;
    }
    ++i;
    return argNames;
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
    return parseWithPredicate(i, isalpha);
}

std::string parseIntLiteral(Source::Iterator& i) {
    return parseWithPredicate(i, isdigit);
}

Expr* parse(const CompilerState& state, Source::Iterator& i) {
    std::vector<Expr*> v;
    while (!i.isEnd() && *i != ']' && *i != '}') {
        v.push_back(parseToken(state, i));
    }
    return new ListExpr(v);
}

std::unique_ptr<Int8Expr> parseInt8Expr(Source::Iterator& i, bool defaultOneAllowed) {
    std::string varname = parseVariableName(i);
    if (!varname.empty())
        return std::make_unique<VariableInt8Expr>(varname);

    std::string intLiteral = parseIntLiteral(i);
    if (!intLiteral.empty())
        return std::make_unique<ConstInt8Expr>(std::stoi(intLiteral));

    if (defaultOneAllowed)
        return std::make_unique<ConstInt8Expr>(1);

    throw SyntaxErrorException(i, "a variable name or an integer literal is expected");
}

std::unique_ptr<Expr> parse(const CompilerState& state, const Source& src) {
    auto it = src.begin();
    return std::unique_ptr<Expr>(parse(state, it));
}