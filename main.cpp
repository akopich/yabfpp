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
#include <algorithm>
#include <cctype>
#include <cstdio>
#include <cstdlib>
#include <map>
#include <memory>
#include <string>
#include <vector>


using namespace llvm;
using namespace std;

class ContextBuilderModule {
public:
    unique_ptr<LLVMContext> context;
    unique_ptr<IRBuilder<>> builder;
    unique_ptr<Module> module;

    void generateEntryPoint() {
        FunctionType *mainType = FunctionType::get(builder->getInt32Ty(), false);
        Function *main = Function::Create(mainType, Function::ExternalLinkage, "main", module.get());
        BasicBlock *entry = BasicBlock::Create(*context, "main", main);
        builder->SetInsertPoint(entry);
    }

    void generatePrintfInt() {
        vector<Type *> args;
        args.push_back(Type::getInt8PtrTy(*context));
        FunctionType *printfType = FunctionType::get(builder->getInt32Ty(), args, true);
        Function::Create(printfType, Function::ExternalLinkage, "printf", module.get());
    }

    void generateCallPrintfInt(Value* theInt) {
        vector<Value *> printArgs;
        Value *formatStr = builder->CreateGlobalStringPtr("%d\n");
        printArgs.push_back(formatStr);
        printArgs.push_back(theInt);
        builder->CreateCall(module->getFunction("printf"), printArgs);
    }

    void generatePutChar() {
        vector<Type *> args { Type::getInt8Ty(*context) };
        FunctionType *putCharType = FunctionType::get(builder->getInt32Ty(), args, false);
        Function::Create(putCharType, Function::ExternalLinkage, "putchar", module.get());
    }

    void generateCallPutChar(Value* theChar) {
        vector<Value *> args = {theChar};
        builder->CreateCall(module->getFunction("putchar"), args);
    }


    void generateGetChar() {
        FunctionType* getCharType = FunctionType::get(builder->getInt8Ty(), {}, false);
        Function::Create(getCharType, Function::ExternalLinkage, "getchar", module.get());
    }

    Value* generateCallGetChar() {
        vector<Value *> args = {};
        return builder->CreateCall(module->getFunction("getchar"), args);
    }


    Value* getConstInt(const int i) {
        return ConstantInt::get(*context, APInt(32, i));
    }

    Value* getConstChar(const char c) {
        return ConstantInt::get(*context, APInt(8, c));
    }

    void return0FromMain() {
        builder->CreateRet(getConstInt(0));
    }

    void printIRtoFile(const string& outPath) const {
        std::error_code EC;
        raw_ostream* out = new raw_fd_ostream(outPath, EC, sys::fs::F_None);
        module->print(*out, nullptr);
    }
};

ContextBuilderModule createContextBuilderModule() {
    unique_ptr<LLVMContext> context = make_unique<LLVMContext>();
    unique_ptr<IRBuilder<>> builder = make_unique<IRBuilder<>>(*context);
    unique_ptr<Module> module = make_unique<Module>("compiler", *context);
    return {move(context), move(builder), move(module)};
}

int main() {
    auto cbm = createContextBuilderModule();
    cbm.generateEntryPoint();

    cbm.generatePrintfInt();

    cbm.generatePutChar();
    cbm.generateGetChar();
    auto a = cbm.generateCallGetChar();
    cbm.generateCallPutChar(a);
    cbm.return0FromMain();

    auto out = "/home/valerij/test11.ll";
    cbm.printIRtoFile(out);

    return 0;
}
