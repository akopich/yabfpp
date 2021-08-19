//
// Created by valerij on 7/30/21.
//

#ifndef YABF_BFMACHINE_H
#define YABF_BFMACHINE_H

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

#include "CompilerState.h"
#include "CLibHandler.h"

class CompilerState;

class BFMachine {
public:
    llvm::Value* tapePtr;
    llvm::Value* pointer;
    llvm::Value* tapeSizePtr;
    CompilerState* const state;


    BFMachine(llvm::Value* tapePtr, llvm::Value* pointer, llvm::Value* tapeSizePtr, CompilerState* state);

    [[nodiscard]] llvm::Value* getIndex() const;

    [[nodiscard]] llvm::Value* getTape() const;

    void setTapePtr(llvm::Value* tape) const;

    [[nodiscard]] llvm::Value* getTapeSize() const;

    [[nodiscard]] llvm::Value* getCurrentChar() const;

    void setCurrentChar(llvm::Value* theChar) const;
};


#endif //YABF_BFMACHINE_H
