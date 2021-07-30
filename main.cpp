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

Expr* parse(const string& s, int& i);

Expr* parseToken(const string s, int& i) {
    char c = s[i];
    i++;
    switch (c) {
        case '+':
            return new AddExpr(1);
        case '-':
            return new AddExpr(-1);
        case '.':
            return new PrintExpr();
        case ',':
            return new ReadExpr();
        case '<':
            return new MovePtrExpr(-1);
        case '>':
            return new MovePtrExpr(1);
        case '[':
            auto loop = new LoopExpr(parse(s, i));
            i++;
            return loop;
    }
}

Expr* parse(const string& s, int& i) {
    vector<Expr*> v;
    while (i < s.size() && s[i] != ']') {
        v.push_back(parseToken(s, i));
    }
    return new ListExpr(v);
}


string preprocessor(const string& s) {
    string res = s;
    res.erase(std::remove_if(res.begin(), res.end(), ::isspace), res.end());
    return res;
}


unique_ptr<Expr> parse(const string& s) {
    int i = 0;
    return unique_ptr<Expr>(parse(preprocessor(s), i));
}

int main() {
    auto cbm = createContextBuilderModule();
    auto machine = cbm.init();
    auto expr = parse("++++++++[>++++[>++>+++>+++>+<<<<-]>+>+>->>+[<]<-]>>.>---.+++++++..+++.>>.<-.<.+++.------.--------.>>+.>++.");
    expr->generate(machine);

    cbm.finalizeAndPrintIRtoFile("/home/valerij/test.ll");

    return 0;
}
