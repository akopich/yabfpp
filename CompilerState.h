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
#include "CLibHandler.h"
#include "PlatformDependent.h"
#include "ConstantHelper.h"
#include "VariableHandler.h"

class BFMachine;

class CLibHandler;

class VariableHandler;

class CompilerState : public ConstantHelper {
private:
    const int CALL_STACK_SIZE = 100;

    void generateEntryPoint();

    void return0FromMain() const;

    std::unique_ptr<llvm::LLVMContext> context;
    std::unique_ptr<PlatformDependent> platformDependent;
    std::stack<std::shared_ptr<VariableHandler>> variableHandlerStack;

    const int initialTapeSize;

    llvm::Function* main{};


    void generateTapeDoublingFunction();

    void generateReadCharFunction();

protected:
    llvm::LLVMContext* getContext() const override;

public:
    friend CompilerState
    initCompilerState(const std::string& name, const std::string& targetTriple, const int tapeSize);

    CompilerState(std::unique_ptr<llvm::LLVMContext> context,
                  std::unique_ptr<llvm::Module> module,
                  std::unique_ptr<llvm::IRBuilder<>> builder,
                  std::unique_ptr<CLibHandler> clib,
                  std::unique_ptr<PlatformDependent> platformDependent,
                  int initialTapeSize);

    std::unique_ptr<llvm::Module> module;

    VariableHandler& getVariableHandler();

    void pushVariableHandlerStack();

    void popVariableHandlerStack();

    std::unique_ptr<llvm::IRBuilder<>> builder;

    std::unique_ptr<CLibHandler> clib;

    [[nodiscard]] llvm::BasicBlock* createBasicBlock(const std::string& s, llvm::Function* function) const;

    [[nodiscard]] llvm::BasicBlock* createBasicBlock(const std::string& s) const;

    BFMachine createBFMachine();

    void finalizeAndPrintIRtoFile(const std::string& outPath) const;

    void setCharArrayElement(llvm::Value* arr, llvm::Value* index, llvm::Value* theChar) const;

    llvm::Value* getCharArrayElement(llvm::Value* arr, llvm::Value* index) const;

    llvm::Value* CreateLoad(llvm::Value* ptr) const;

    llvm::Value* CreateStore(llvm::Value* value, llvm::Value* ptr) const;

    llvm::Value* CreateAdd(llvm::Value* lhs, llvm::Value* rhs, const std::string& name) const;

    void generateCallTapeDoublingFunction(BFMachine& machine, llvm::Value* newIndex) const;

    llvm::Value* generateCallReadCharFunction() const;

    llvm::Value* allocateAndInitialize(llvm::Type* type, llvm::Value* value) const;
};

CompilerState initCompilerState(const std::string& name, const std::string& targetTriple, const int tapeSize);


#endif //YABF_CONTEXTBUILDERMODULE_H
