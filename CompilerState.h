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

    void return0FromMain();

    llvm::LLVMContext context{};
    PlatformDependent platformDependent;
    std::stack<VariableHandler> variableHandlerStack;

    std::stack<llvm::Function*> functionStack;

    void generateTapeDoublingFunction();

    void generateReadCharFunction();

    void initClib() {
        clib.init();
    }

public:
    friend std::unique_ptr<CompilerState> initCompilerState(std::string_view name,
            std::string_view targetTriple);

    CompilerState(std::string_view module_name,
                  std::string_view targetTriple,
                  PlatformDependent platformDependent)
        : ConstantHelper(&context),
          platformDependent(platformDependent),
          module(module_name, context),
          builder(context),
          clib(&module, &builder) {
              module.setTargetTriple(llvm::Triple{std::string{targetTriple}});
          }

    [[nodiscard]] llvm::Function* getCurrentFunction() const;

    auto* getPtrTy() {
        return llvm::PointerType::get(context, 0);
    }

    llvm::Function* declareBFFunction(const std::string& name, const std::vector<llvm::Type*>& args);

    void popFunctionStack();

    llvm::Module module;

    VariableHandler& getVariableHandler();

    void pushVariableHandlerStack();

    void popVariableHandlerStack();

    Builder builder;

    CLibHandler clib;

    [[nodiscard]] llvm::BasicBlock* createBasicBlock(const std::string& s, llvm::Function* function) ;

    [[nodiscard]] llvm::BasicBlock* createBasicBlock(const std::string& s) ;

    void finalizeAndPrintIRtoFile(const std::string& outPath) ;

    void setCharArrayElement(llvm::Value* arr, llvm::Value* index, llvm::Value* theChar) ;

    llvm::Value* getCharArrayElement(llvm::Value* arr, llvm::Value* index) ;

    llvm::Value* CreateStore(llvm::Value* value, llvm::Value* ptr) ;

    llvm::Value* CreateAdd(llvm::Value* lhs, llvm::Value* rhs, const std::string& name) ;

    [[nodiscard]] llvm::Value* generateCallReadCharFunction() ;

    Pointer allocateAndInitialize(llvm::Type* type, llvm::Value* value) {
        auto pointer = builder.CreateAlloca(type);
        builder.CreateStore(value, pointer);
        return {type, pointer};
    }
};

inline std::unique_ptr<CompilerState> initCompilerState(std::string_view name, std::string_view targetTriple) {
    auto platformDependent = getPlatformDependent(targetTriple);
    auto state = std::make_unique<CompilerState>(name, targetTriple, platformDependent);

    state->initClib();
    state->generateReadCharFunction();
    state->generateTapeDoublingFunction();
    state->generateEntryPoint();
    state->pushVariableHandlerStack();

    return state;
}


#endif //YABF_CONTEXTBUILDERMODULE_H
