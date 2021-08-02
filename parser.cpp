//
// Created by valerij on 8/1/21.
//

#include <iostream>
#include "ContextBuilderModule.h"
#include <algorithm>
#include <cctype>
#include <memory>
#include <vector>


#include "Expr.h"

std::unique_ptr<Int8Expr> parseTrailingVariable(const std::string& s, int& i);

Expr* parse(ContextBuilderModule& cbm, const std::string& s, int& i);

std::string parseVariableName(const std::string& s, int& i);

Expr* parseToken(ContextBuilderModule& cbm, const std::string& s, int& i) {
    char c = s[i];
    i++;
    switch (c) {
        case '^':
            return new WriteToVariable(parseVariableName(s, i));
        case '_':
            return new ReadFromVariable(parseVariableName(s, i));
        case '+':
        case '-': {
            auto add = parseTrailingVariable(s, i);
            if (c == '-') {
                add = std::make_unique<MinusInt8Expr>(move(add));
            }
            return new AddExpr(move(add));
        }
        case '.':
            return new PrintExpr();
        case ',':
            return new ReadExpr();
        case '<':
        case '>': {
            auto step = parseTrailingVariable(s, i);
            if (c == '<') {
                step = std::make_unique<MinusInt8Expr>(move(step));
            }
            return new MovePtrExpr(move(step));
        }
        case '[': {
            auto loop = new LoopExpr(parse(cbm, s, i));
            i++;
            return loop;
        }
        default:
            std::cerr << "Unexpected symbol at " << i;
            return nullptr;
    }
}


template<typename P>
std::string parseWithPredicate(const std::string& s, int& i, P predicate) {
    std::string name;
    while (i < s.size() && predicate(s[i])) {
        name += s[i];
        i++;
    }
    return name;
}

std::string parseVariableName(const std::string& s, int& i) {
    return parseWithPredicate(s, i, [](const char c) { return isalpha(c); });
}

Expr* parse(ContextBuilderModule& cbm, const std::string& s, int& i) {
    std::vector<Expr*> v;
    while (i < s.size() && s[i] != ']') {
        v.push_back(parseToken(cbm, s, i));
    }
    return new ListExpr(v);
}

std::unique_ptr<Int8Expr> parseTrailingVariable(const std::string& s, int& i) {
    auto varname = parseVariableName(s, i);
    if (varname.empty()) {
        return std::make_unique<ConstInt8Expr>(1);
    } else {
        return std::make_unique<VariableInt8Expr>(varname);
    }
}

std::string removeWhetespaces(const std::string& s) {
    std::string res = s;
    res.erase(std::remove_if(res.begin(), res.end(), ::isspace), res.end());
    return res;
}

std::unique_ptr<Expr> parse(ContextBuilderModule& cbm, const std::string& s) {
    int i = 0;
    return std::unique_ptr<Expr>(parse(cbm, removeWhetespaces(s), i));
}