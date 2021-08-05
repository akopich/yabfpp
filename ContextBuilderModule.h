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
    llvm::Function* declareFunction(const std::vector<llvm::Type*>& argTypes,
                                    llvm::Type* resultType,
                                    const bool isVariadic,
                                    const std::string& name) const;

    void generateEntryPoint();

    void generatePrintfInt() const;

    void generateFree() const;

    void generatePutChar() const;

    void generateCalloc() const;

    void generateGetChar() const;

    void generateMemcpy() const;

    void return0FromMain() const;

    std::unique_ptr<llvm::LLVMContext> context;
    std::unique_ptr<llvm::Module> module;
    llvm::Function* main{};

public:
    friend ContextBuilderModule createContextBuilderModule(const std::string& targetTriple);

    ContextBuilderModule(std::unique_ptr<llvm::LLVMContext> context,
                         std::unique_ptr<llvm::Module> module,
                         std::unique_ptr<llvm::IRBuilder<>> builder);

    std::unique_ptr<llvm::IRBuilder<>> builder;

    [[nodiscard]] llvm::BasicBlock* createBasicBlock(const std::string& s, llvm::Function* function) const;

    [[nodiscard]] llvm::BasicBlock* createBasicBlock(const std::string& s) const;

    BFMachine init(int tapeSize);

    void generateCallPutChar(llvm::Value* theChar) const;

    void generateCallFree(llvm::Value* ptr) const;

    void generateCallPrintfInt(llvm::Value* theInt) const;

    void generateCallMemcpy(llvm::Value* dest, llvm::Value* src, llvm::Value* size) const;

    llvm::Value* generateCallCalloc(llvm::Value* size) const;

    [[nodiscard]] llvm::Value* generateCallGetChar() const;

    [[nodiscard]] llvm::Value* getConstInt(int i) const;

    [[nodiscard]] llvm::Value* getConst64(int i) const;

    [[nodiscard]] llvm::Value* getConstChar(char c) const;

    void finalizeAndPrintIRtoFile(const std::string& outPath) const;

    void setCharArrayElement(llvm::Value* arr, llvm::Value* index, llvm::Value* theChar) const;

    llvm::Value* getCharArrayElement(llvm::Value* arr, llvm::Value* index) const;

    llvm::Value* CreateLoad(llvm::Value* ptr) const;

    llvm::Value* CreateAdd(llvm::Value* lhs, llvm::Value* rhs, const std::string& name) const;

    void generateTapeDoublingFunction();

    void generateCallTapeDoublingFunction(BFMachine& machine, llvm::Value* newIndex);

    llvm::Value* allocateAndInitialize(llvm::Type* type, llvm::Value* value);
};

ContextBuilderModule createContextBuilderModule(const std::string& targetTriple);


#endif //YABF_CONTEXTBUILDERMODULE_H
