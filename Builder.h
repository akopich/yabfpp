#pragma once


#include "Pointer.h"
#include <llvm/IR/IRBuilder.h>

struct Builder : public llvm::IRBuilder<> {
    Builder(llvm::LLVMContext& context): llvm::IRBuilder<>(context) {}

    llvm::Value* CreateLoad(Pointer p)  {
        return static_cast<llvm::IRBuilder<>*>(this)->CreateLoad(p.valueType, p.pointer);
    }

    llvm::Value* getCharArrayElement(llvm::Value* arr, llvm::Value* index) {
        auto* type = getInt8Ty();
        auto elemPtr = CreateGEP(type, arr, index);
        return CreateLoad(Pointer{type, elemPtr});
    }

    void setCharArrayElement(llvm::Value* arr, llvm::Value* index, llvm::Value* theChar) {
        auto* type = getInt8Ty();
        auto elemPtr = CreateGEP(type, arr, index);
        CreateStore(theChar, elemPtr);
    }
};
