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

std::unique_ptr<Int8Expr> parseTrailingVariable(const std::string& s, int& i, bool defaultOneAllowed);

Expr* parse(const CompilerState& state, const std::string& s, int& i);

std::string parseVariableName(const std::string& s, int& i);

std::string parseIntLiteral(const std::string& s, int& i);

Expr* parseToken(const CompilerState& state, const std::string& s, int& i) {
    char c = s[i];
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
            std::cerr << "Unexpected symbol at " << i << std::endl;
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

std::string parseIntLiteral(const std::string& s, int& i) {
    return parseWithPredicate(s, i, [](const char c) { return isdigit(c); });
}

Expr* parse(const CompilerState& state, const std::string& s, int& i) {
    std::vector<Expr*> v;
    while (i < s.size() && s[i] != ']') {
        v.push_back(parseToken(state, s, i));
    }
    return new ListExpr(v);
}

std::unique_ptr<Int8Expr> parseTrailingVariable(const std::string& s, int& i, bool defaultOneAllowed) {
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

template<typename P>
std::string filterString(const std::string& s, P predicate) {
    std::string res = s;
    res.erase(std::remove_if(res.begin(), res.end(), predicate), res.end());
    return res;
}

std::string removeWhitespaces(const std::string& s) {
    return filterString(s, ::isspace);
}

std::string removeAllButLegacyCharacters(const std::string& s) {
    std::set<char> legacyCharacters = {'[', ']', '>', '<', '+', '-', '.', ','};
    return filterString(s, [&](const char c) { return legacyCharacters.count(c) == 0; });
}

std::unique_ptr<Expr> parse(const CompilerState& state, const std::string& s, bool legacyMode) {
    int i = 0;
    std::string preprocessed;
    if (legacyMode) {
        preprocessed = removeAllButLegacyCharacters(s);
    } else {
        preprocessed = removeWhitespaces(s);
    }
    return std::unique_ptr<Expr>(parse(state, preprocessed, i));
}