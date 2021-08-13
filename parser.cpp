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

std::unique_ptr<Int8Expr> parseTrailingVariable(const Source& s, Source::Iterator& i, bool defaultOneAllowed);

Expr* parse(const CompilerState& state, const Source& s, Source::Iterator& i);

std::string parseVariableName(const Source& s, Source::Iterator& i);

std::string parseIntLiteral(const Source& s, Source::Iterator& i);

Expr* parseToken(const CompilerState& state, const Source& s, Source::Iterator& i) {
    char c = *i;
    i++;
    switch (c) {
        case '^':
            return new WriteToVariable(parseVariableName(s, i));
        case '_': {
            return new AssignExpressionValueToTheCurrentCell(parseTrailingVariable(s, i, false));
        }
        case '+':
        case '-': {
            auto add = parseTrailingVariable(s, i, true);
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
            auto step = parseTrailingVariable(s, i, true);
            if (c == '<') {
                step = std::make_unique<MinusInt8Expr>(move(step));
            }
            return new MovePtrExpr(move(step));
        }
        case '[': {
            auto loop = new LoopExpr(parse(state, s, i));
            i++;
            return loop;
        }
        default:
            std::cerr << "Unexpected symbol at line: " << i.getLine() << " position:" << i.getLinePosition()
                      << std::endl;
            return nullptr;
    }
}


template<typename P>
std::string parseWithPredicate(const Source& s, Source::Iterator& i, P predicate) {
    std::string name;
    while (!i.isEnd() && predicate(*i)) {
        name += *i;
        i++;
    }
    return name;
}

std::string parseVariableName(const Source& s, Source::Iterator& i) {
    return parseWithPredicate(s, i, [](const char c) { return isalpha(c); });
}

std::string parseIntLiteral(const Source& s, Source::Iterator& i) {
    return parseWithPredicate(s, i, [](const char c) { return isdigit(c); });
}

Expr* parse(const CompilerState& state, const Source& s, Source::Iterator& i) {
    std::vector<Expr*> v;
    while (!i.isEnd() && *i != ']') {
        v.push_back(parseToken(state, s, i));
    }
    return new ListExpr(v);
}

std::unique_ptr<Int8Expr> parseTrailingVariable(const Source& s, Source::Iterator& i, bool defaultOneAllowed) {
    std::string varname = parseVariableName(s, i);
    if (varname.empty()) {
        std::string intLiteral = parseIntLiteral(s, i);
        if (intLiteral.empty()) {
            if (defaultOneAllowed)
                return std::make_unique<ConstInt8Expr>(1);
            else
                throw std::invalid_argument("_ should be followed by a variable name or by an integer literal");
        } else {
            return std::make_unique<ConstInt8Expr>(std::stoi(intLiteral));
        }
    } else {
        return std::make_unique<VariableInt8Expr>(varname);
    }
}

std::unique_ptr<Expr> parse(const CompilerState& state, const Source& src) {
    auto it = src.begin();
    return std::unique_ptr<Expr>(parse(state, src, it));
}