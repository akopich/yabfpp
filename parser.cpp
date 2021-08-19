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
#include "parser.h"
#include "Source.h"
#include "SyntaxErrorException.h"


void checkBlockClosed(Source::Iterator& i, char expected) {
    if (i.isEnd() || *i != expected) {
        std::string charAsString(1, expected);
        throw SyntaxErrorException(i, charAsString + " is expected.");
    }
    ++i;
}

std::unique_ptr<Expr> Parser::parseToken(const CompilerState& state, Source::Iterator& i) {
    char c = *i;
    i++;
    switch (c) {
        case '\\':
            return std::make_unique<Return>();
        case '@': {
            std::string functionName = parseVariableName(i);
            std::vector<std::string> argNames = parseFunctionArgumentList(i);
            functionName2argNumber[functionName] = argNames.size();
            checkBlockClosed(i, '{');
            auto body = parse(state, i);
            checkBlockClosed(i, '}');
            return std::make_unique<BFFunctionDeclaration>(functionName, argNames, move(body));
        }
        case '$': {
            std::string functionName = parseVariableName(i);
            auto functionIt = functionName2argNumber.find(functionName);
            if (functionIt == functionName2argNumber.end()) {
                throw SyntaxErrorException(i, "Function " + functionName + " is not defined");
            }
            auto argExprs = parseCallFunctionArgumentList(i);
            if (argExprs.size() != functionIt->second) {
                throw SyntaxErrorException(i,
                                           "Function " + functionName + " takes " + std::to_string(functionIt->second) +
                                           " arguments, " +
                                           std::to_string(argExprs.size()) + " supplied");
            }
            return std::make_unique<BFFunctionCall>(functionName, argExprs);
        }
        case '{': {
            auto ifExpr = parse(state, i);
            checkBlockClosed(i, '}');
            if (*i == '{') {
                i++;
                auto elseExpr = parse(state, i);
                checkBlockClosed(i, '}');
                return std::make_unique<IfElse>(move(ifExpr), move(elseExpr));
            }

            return std::make_unique<IfElse>(move(ifExpr), getNoOpExpr());
        }
        case '^':
            return std::make_unique<WriteToVariable>(parseVariableName(i));
        case '_': {
            return std::make_unique<AssignExpressionValueToTheCurrentCell>(parseInt8Expr(i, false));
        }
        case '+':
        case '-': {
            auto add = parseInt8Expr(i, true);
            if (c == '-') {
                add = std::make_unique<MinusInt8Expr>(move(add));
            }
            return std::make_unique<AddExpr>(move(add));
        }
        case '.':
            return std::make_unique<PrintExpr>();
        case ',':
            return std::make_unique<ReadExpr>();
        case '*':
            return std::make_unique<PrintIntExpr>();
        case '<':
        case '>': {
            auto step = parseInt8Expr(i, true);
            if (c == '<') {
                step = std::make_unique<MinusInt8Expr>(move(step));
            }
            return std::make_unique<MovePtrExpr>(move(step));
        }
        case '[': {
            auto body = parse(state, i);
            checkBlockClosed(i, ']');
            return std::make_unique<LoopExpr>(move(body));;
        }
        default:
            throw SyntaxErrorException(i, "Unexpected symbol.");
    }
}

std::vector<std::shared_ptr<Int8Expr>> Parser::parseCallFunctionArgumentList(Source::Iterator& i) {
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

std::vector<std::string> Parser::parseFunctionArgumentList(Source::Iterator& i) {
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

std::string Parser::parseVariableName(Source::Iterator& i) {
    return parseWithPredicate(i, isalpha);
}

std::string Parser::parseIntLiteral(Source::Iterator& i) {
    return parseWithPredicate(i, isdigit);
}

std::unique_ptr<Expr> Parser::parse(const CompilerState& state, Source::Iterator& i) {
    std::vector<std::shared_ptr<Expr>> v;
    while (!i.isEnd() && *i != ']' && *i != '}') {
        v.push_back(move(parseToken(state, i)));
    }
    return std::make_unique<ListExpr>(v);
}

std::unique_ptr<Int8Expr> Parser::parseInt8Expr(Source::Iterator& i, bool defaultOneAllowed) {
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

std::unique_ptr<Expr> Parser::parse(const CompilerState& state, const Source& src) {
    functionName2argNumber = std::map<std::string, int>();
    auto it = src.begin();
    return std::unique_ptr<Expr>(parse(state, it));
}