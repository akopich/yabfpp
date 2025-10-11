//
// Created by valerij on 8/1/21.
//

#include <cctype>
#include <memory>
#include <vector>


#include "Expr.h"
#include "parser.h"
#include "Source.h"
#include "SyntaxError.h"


void checkBlockClosed(Source::Iterator& i, char expected) {
    if (i.isEnd() || *i != expected) {
        std::string charAsString(1, expected);
        syntaxError(i, charAsString + " is expected.");
    }
    ++i;
}

Expr Parser::parseExpr(Source::Iterator& i) {
    char c = *i;
    i++;
    switch (c) {
        case '\\':
            return mkExpr<Return>();
        case '@':
            return parseBFFunctionDefinition(i);
        case '$':
            return parseBFFunctionCall(i);
        case '{':
            return parseIfElseExpr(i);
        case '^':
            return mkExpr<WriteToVariable>(parseVariableName(i));
        case '_':
            return mkExpr<AssignExpressionValueToTheCurrentCell>(parseInt8Expr(i, false));
        case '+':
        case '-':
            return parseAddExpr(i, c);
        case '.':
            return mkExpr<PrintExpr>();
        case ',':
            return mkExpr<ReadExpr>();
        case '*':
            return mkExpr<PrintIntExpr>();
        case '<':
        case '>':
            return parseMovePtrExpr(i, c);
        case '[':
            return parseLoopExpr(i);
        default:
            syntaxError(i, "Unexpected symbol.");
    }
}

Expr Parser::parseLoopExpr(Source::Iterator& i) {
    auto body = parse(i);
    checkBlockClosed(i, ']');
    return mkExpr<LoopExpr>(std::move(body));
}

Expr Parser::parseMovePtrExpr(Source::Iterator& i, char leadingChar) {
    auto step = parseInt8Expr(i, true);
    if (leadingChar == '<') {
        step = mkInt8Expr<MinusInt8Expr>(std::move(step));
    }
    return mkExpr<MovePtrExpr>(std::move(step));
}

Expr Parser::parseAddExpr(Source::Iterator& i, char leadingChar) {
    auto add = parseInt8Expr(i, true);
    if (leadingChar == '-') {
        add = mkInt8Expr<MinusInt8Expr>(std::move(add));
    }
    return mkExpr<AddExpr>(std::move(add));
}

Expr Parser::parseIfElseExpr(Source::Iterator& i) {
    auto ifExpr = parse(i);
    checkBlockClosed(i, '}');
    if (*i == '{') {
        i++;
        auto elseExpr = parse(i);
        checkBlockClosed(i, '}');
        return mkExpr<IfElse>(std::move(ifExpr), std::move(elseExpr));
    }

    return mkExpr<IfElse>(std::move(ifExpr), getNoOpExpr());
}

Expr Parser::parseBFFunctionCall(Source::Iterator& i) {
    std::string functionName = parseVariableName(i);
    auto functionIt = functionName2argNumber.find(functionName);
    if (functionIt == functionName2argNumber.end()) {
        syntaxError(i, "Function " + functionName + " is not defined");
    }
    auto argExprs = parseCallFunctionArgumentList(i);
    if (argExprs.size() != functionIt->second) {
        syntaxError(i,
                                   "Function " + functionName + " takes " + std::to_string(functionIt->second) +
                                   " arguments, " + std::to_string(argExprs.size()) + " supplied");
    }
    return mkExpr<BFFunctionCall>(functionName, std::move(argExprs));
}

Expr Parser::parseBFFunctionDefinition(Source::Iterator& i) {
    std::string functionName = parseVariableName(i);
    std::vector<std::string> argNames = parseFunctionArgumentList(i);
    functionName2argNumber[functionName] = argNames.size();
    checkBlockClosed(i, '{');
    auto body = parse(i);
    checkBlockClosed(i, '}');
    return mkExpr<BFFunctionDeclaration>(functionName, argNames, std::move(body));
}

std::vector<Int8Expr> Parser::parseCallFunctionArgumentList(Source::Iterator& i) {
    if (*i != '(')
        syntaxError(i, "opening bracket expected");
    ++i;
    std::vector<Int8Expr> arguments;
    while (*i != ')') {
        arguments.emplace_back(parseInt8Expr(i, false));
        if (*i != ',' && *i != ')') {
            syntaxError(i, "a comma, a closing bracket, variable name or an integer literal");
        }
        if (*i == ',')
            i++;
    }
    ++i;
    return arguments;
}

std::vector<std::string> Parser::parseFunctionArgumentList(Source::Iterator& i) {
    if (*i != '(')
        syntaxError(i, "opening bracket expected");
    ++i;
    std::vector<std::string> argNames;
    while (*i != ')') {
        argNames.push_back(parseVariableName(i));
        if (*i != ',' && *i != ')') {
            syntaxError(i, "a comma, a closing bracket or an alphabetic character expected");
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

Expr Parser::parse(Source::Iterator& i) {
    std::vector<Expr> v;
    while (!i.isEnd() && *i != ']' && *i != '}') {
        v.push_back(parseExpr(i));
    }
    return mkExpr<ListExpr>(std::move(v));
}

Int8Expr Parser::parseInt8Expr(Source::Iterator& i, bool defaultOneAllowed) {
    std::string varname = parseVariableName(i);
    if (!varname.empty())
        return mkInt8Expr<VariableInt8Expr>(varname);

    std::string intLiteral = parseIntLiteral(i);
    if (!intLiteral.empty())
        return mkInt8Expr<ConstInt8Expr>(std::stoi(intLiteral));

    if (defaultOneAllowed)
        return mkInt8Expr<ConstInt8Expr>(1);

    syntaxError(i, "a variable name or an integer literal is expected");
}

Expr Parser::parse(const Source& src) {
    functionName2argNumber.clear();
    auto i = src.begin();
    return parse(i);
}
