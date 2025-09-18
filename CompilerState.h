//
// Created by valerij on 7/30/21.
//

#ifndef YABF_CONTEXTBUILDERMODULE_H
#define YABF_CONTEXTBUILDERMODULE_H


#include <vector>
#include <string>
#include <memory>
#include "llvm/Target/TargetMachine.h"
#include "llvm/IR/Verifier.h"
#include "llvm/IR/Type.h"
#include "llvm/IR/Module.h"
#include "llvm/IR/LegacyPassManager.h"
#include "llvm/IR/LLVMContext.h"
#include "llvm/IR/Instructions.h"
#include "llvm/IR/Function.h"
#include "llvm/IR/DerivedTypes.h"
#include "llvm/IR/BasicBlock.h"
#include "Builder.h"
#include "CLibHandler.h"
#include "PlatformDependent.h"
#include "ConstantHelper.h"
#include "Pointer.h"
#include "VariableHandler.h"
#include <stack>

class CompilerState : public ConstantHelper {
private:
    void generateEntryPoint();

    void return0FromMain() const;

    std::unique_ptr<llvm::LLVMContext> context;
    PlatformDependent platformDependent;
    std::stack<std::shared_ptr<VariableHandler>> variableHandlerStack;

    std::stack<llvm::Function*> functionStack;

    void generateTapeDoublingFunction();

    void generateReadCharFunction();

public:
    friend CompilerState initCompilerState(const std::string& name,
                                           const std::string& targetTriple);

    CompilerState(std::unique_ptr<llvm::LLVMContext> context,
                  std::unique_ptr<llvm::Module> module,
                  std::unique_ptr<Builder> builder,
                  std::unique_ptr<CLibHandler> clib,
                  PlatformDependent platformDependent);

    [[nodiscard]] llvm::Function* getCurrentFunction() const;

    auto* getInt8PtrTy() const {
        return llvm::PointerType::get(llvm::Type::getInt8Ty(*context), 0);
    }

    llvm::Function* declareBFFunction(const std::string& name, const std::vector<llvm::Type*>& args);

    void popFunctionStack();

    std::unique_ptr<llvm::Module> module;

    VariableHandler& getVariableHandler();

    void pushVariableHandlerStack();

    void popVariableHandlerStack();

    std::unique_ptr<Builder> builder;

    std::unique_ptr<CLibHandler> clib;

    [[nodiscard]] llvm::BasicBlock* createBasicBlock(const std::string& s, llvm::Function* function) const;

    [[nodiscard]] llvm::BasicBlock* createBasicBlock(const std::string& s) const;

    void finalizeAndPrintIRtoFile(const std::string& outPath) const;

    void setCharArrayElement(llvm::Value* arr, llvm::Value* index, llvm::Value* theChar) const;

    llvm::Value* getCharArrayElement(llvm::Value* arr, llvm::Value* index) const;

    llvm::Value* CreateStore(llvm::Value* value, llvm::Value* ptr) const;

    llvm::Value* CreateAdd(llvm::Value* lhs, llvm::Value* rhs, const std::string& name) const;

    [[nodiscard]] llvm::Value* generateCallReadCharFunction() const;

    Pointer allocateAndInitialize(llvm::Type* type, llvm::Value* value) const {
        auto pointer = builder->CreateAlloca(type);
        builder->CreateStore(value, pointer);
        return {type, pointer};
    }
};

CompilerState initCompilerState(const std::string& name, const std::string& targetTriple);


#endif //YABF_CONTEXTBUILDERMODULE_H
