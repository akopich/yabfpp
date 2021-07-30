//
// Created by valerij on 7/30/21.
//

#ifndef YABF_CONTEXTBUILDERMODULE_H
#define YABF_CONTEXTBUILDERMODULE_H


#include <vector>
#include <string>
#include <memory>
#include <map>
#include <cstdlib>
#include <cstdio>
#include <cctype>
#include <algorithm>
#include <iostream>
#include "llvm/Target/TargetOptions.h"
#include "llvm/Target/TargetMachine.h"
#include "llvm/Support/TargetSelect.h"
#include "llvm/Support/TargetRegistry.h"
#include "llvm/Support/raw_ostream.h"
#include "llvm/Support/Host.h"
#include "llvm/Support/FileSystem.h"
#include "llvm/IR/Verifier.h"
#include "llvm/IR/Type.h"
#include "llvm/IR/Module.h"
#include "llvm/IR/LegacyPassManager.h"
#include "llvm/IR/LLVMContext.h"
#include "llvm/IR/IRBuilder.h"
#include "llvm/IR/Instructions.h"
#include "llvm/IR/Function.h"
#include "llvm/IR/DerivedTypes.h"
#include "llvm/IR/Constants.h"
#include "llvm/IR/BasicBlock.h"
#include "llvm/ADT/STLExtras.h"
#include "llvm/ADT/Optional.h"
#include "llvm/ADT/APFloat.h"
#include "BFMachine.h"

class BFMachine;

class ContextBuilderModule {
private:
    void generateEntryPoint();

    void generatePrintfInt();

    void generatePutChar();

    void generateCalloc();

    void generateGetChar();

    void return0FromMain();
public:
    friend ContextBuilderModule createContextBuilderModule();

    std::unique_ptr<llvm::LLVMContext> context;
    std::unique_ptr<llvm::IRBuilder<>> builder;
    std::unique_ptr<llvm::Module> module;
    llvm::Function* main;

    llvm::BasicBlock* createBasicBlock(const std::string& s);

    BFMachine init();

    void generateCallPutChar(llvm::Value* theChar);

    void generateCallPrintfInt(llvm::Value* theInt);

    llvm::Value* generateCallCalloc(llvm::Value* size);

    llvm::Value* generateCallGetChar();

    llvm::Value* getConstInt(int i);

    llvm::Value* getConst64(int i);

    llvm::Value* getConstChar(char c);

    void finalizeAndPrintIRtoFile(const std::string& outPath);

    void setCharArrayElement(llvm::Value* arr, llvm::Value* index, llvm::Value* theChar);

    llvm::Value* getCharArrayElement(llvm::Value* arr, llvm::Value* index);
};

ContextBuilderModule createContextBuilderModule();



#endif //YABF_CONTEXTBUILDERMODULE_H
