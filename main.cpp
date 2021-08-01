#include <iostream>
#include "ContextBuilderModule.h"
#include <algorithm>
#include <cctype>
#include <memory>
#include <vector>

#include "ContextBuilderModule.h"
#include "Expr.h"

using namespace llvm;
using namespace std;


unique_ptr<Int8Expr> parseTrailingVariable(const string& s, int& i);

Expr* parse(ContextBuilderModule& cbm, const string& s, int& i);

string parseVariableName(const string& s, int& i);

Expr* parseToken(ContextBuilderModule& cbm, const string& s, int& i) {
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
                add = make_unique<MinusInt8Expr>(move(add));
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
                step = make_unique<MinusInt8Expr>(move(step));
            }
            return new MovePtrExpr(move(step));
        }
        case '[':
            auto loop = new LoopExpr(parse(cbm, s, i));
            i++;
            return loop;
    }
}


template<typename P>
string parseWithPredicate(const string& s, int& i, P predicate) {
    string name;
    while (i < s.size() && predicate(s[i])) {
        name += s[i];
        i++;
    }
    return name;
}

string parseVariableName(const string& s, int& i) {
    return parseWithPredicate(s, i, [](const char c) { return isalpha(c); });
}

Expr* parse(ContextBuilderModule& cbm, const string& s, int& i) {
    vector<Expr*> v;
    while (i < s.size() && s[i] != ']') {
        v.push_back(parseToken(cbm, s, i));
    }
    return new ListExpr(v);
}

unique_ptr<Int8Expr> parseTrailingVariable(const string& s, int& i) {
    auto varname = parseVariableName(s, i);
    if (varname.empty()) {
        return make_unique<ConstInt8Expr>(1);
    } else {
        return make_unique<VarableInt8Expr>(varname);
    }
}


string preprocessor(const string& s) {
    string res = s;
    res.erase(std::remove_if(res.begin(), res.end(), ::isspace), res.end());
    return res;
}

unique_ptr<Expr> parse(ContextBuilderModule& cbm, const string& s) {
    int i = 0;
    return unique_ptr<Expr>(parse(cbm, preprocessor(s), i));
}

int main() {
    auto cbm = createContextBuilderModule();
    auto machine = cbm.init(2);
//    auto expr = parse(cbm,"++++++++[>++++[>++>+++>+++>+<<<<-]>+>+>->>+[<]<-]>>.>---.+++++++..+++.>>.<-.<.+++.------.--------.>>+.>++.");
    auto expr = parse(cbm, ",^x>x,-x^y>y,+y.");
    expr->generate(machine);
    cbm.finalizeAndPrintIRtoFile("/home/valerij/test.ll");
    return 0;
}
