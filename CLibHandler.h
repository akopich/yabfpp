//
// Created by valerij on 8/6/21.
//

#ifndef YABFPP_CLIBHANDLER_H
#define YABFPP_CLIBHANDLER_H

#include <memory>
#include "llvm/IR/Module.h"
#include "llvm/IR/IRBuilder.h"
#include "CompilerState.h"
#include "ConstantHelper.h"

class CompilerState;

class CLibHandler : public ConstantHelper {
private:
    llvm::Module* module;

    llvm::IRBuilder<>* builder;

    void generatePrintfInt() const;

    void generateFree() const;

    void generatePutChar() const;

    void generateCalloc() const;

    void generateGetChar() const;

    void generateMemcpy() const;

    void init() const;

protected:
    llvm::LLVMContext* getContext() const override;

public:
    friend CompilerState
    initCompilerState(const std::string& name, const std::string& targetTriple, const int tapeSize);

    auto* getInt8PtrTy() const {
        return llvm::PointerType::get(builder->getInt8Ty(), 0);
    }

    CLibHandler(llvm::Module* module, llvm::IRBuilder<>* builder);

    llvm::Function* declareFunction(const std::vector<llvm::Type*>& argTypes,
                                    llvm::Type* resultType,
                                    const bool isVariadic,
                                    const std::string& name) const;

    void generateCallPutChar(llvm::Value* theChar) const;

    void generateCallFree(llvm::Value* ptr) const;

    void generateCallPrintfInt(llvm::Value* theInt) const;

    void generateCallMemcpy(llvm::Value* dest, llvm::Value* src, llvm::Value* size) const;

    llvm::Value* generateCallCalloc(llvm::Value* size) const;

    [[nodiscard]] llvm::Value* generateCallGetChar() const;
};


#endif //YABFPP_CLIBHANDLER_H
