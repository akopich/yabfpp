#include <iostream>
#include "llvm/ADT/APFloat.h"
#include "llvm/ADT/Optional.h"
#include "llvm/ADT/STLExtras.h"
#include "llvm/IR/BasicBlock.h"
#include "llvm/IR/Constants.h"
#include "llvm/IR/DerivedTypes.h"
#include "llvm/IR/Function.h"
#include "llvm/IR/Instructions.h"
#include "llvm/IR/IRBuilder.h"
#include "llvm/IR/LLVMContext.h"
#include "llvm/IR/LegacyPassManager.h"
#include "llvm/IR/Module.h"
#include "llvm/IR/Type.h"
#include "llvm/IR/Verifier.h"
#include "llvm/Support/FileSystem.h"
#include "llvm/Support/Host.h"
#include "llvm/Support/raw_ostream.h"
#include "llvm/Support/TargetRegistry.h"
#include "llvm/Support/TargetSelect.h"
#include "llvm/Target/TargetMachine.h"
#include "llvm/Target/TargetOptions.h"
#include "ContextBuilderModule.h"
#include <algorithm>
#include <cctype>
#include <cstdio>
#include <cstdlib>
#include <map>
#include <memory>
#include <string>
#include <vector>
#include "ContextBuilderModule.h"
#include "Expr.h"

using namespace llvm;
using namespace std;

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
        case '+': {
            auto varname = parseVariableName(s, i);
            std::unique_ptr<Int8Expr> add;
            if (varname.empty()) {
                add = make_unique<ConstInt8Expr>(1);
            } else {
                add = make_unique<VarableInt8Expr>(varname);
            }
            return new AddExpr(move(add));
        }
        case '-': {
            auto varname = parseVariableName(s, i);
            std::unique_ptr<Int8Expr> add;
            if (varname.empty()) {
                add = make_unique<ConstInt8Expr>(-1);
            } else {
                add = make_unique<MinusInt8Expr>(make_unique<VarableInt8Expr>(varname));
            }
            return new AddExpr(move(add));
        }
        case '.':
            return new PrintExpr();
        case ',':
            return new ReadExpr();
        case '<':
            return new MovePtrExpr(-1);
        case '>':
            return new MovePtrExpr(1);
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
    auto expr = parse(cbm, ",^x>,-x^y>,+y.");
    expr->generate(machine);
    cbm.finalizeAndPrintIRtoFile("/home/valerij/test.ll");
    return 0;
}
