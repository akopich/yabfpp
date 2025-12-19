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
#include "llvm/IR/Instructions.h"
#include "llvm/IR/Function.h"
#include "llvm/IR/DerivedTypes.h"
#include "llvm/IR/BasicBlock.h"
#include "llvm/ADT/APFloat.h"

#include "CompilerState.h"
#include "CLibHandler.h"
#include "Pointer.h"

class BFMachine {
public:
    Pointer tapePtr;
    Pointer pointer;
    Pointer tapeSizePtr;
    CompilerState* const state;
    int initialTapeSize;


    BFMachine(Pointer tapePtr, Pointer pointer, Pointer tapeSizePtr, CompilerState* state, int initialTapeSize) : tapePtr(tapePtr), pointer(pointer), tapeSizePtr(tapeSizePtr), state(state), initialTapeSize(initialTapeSize){}

    [[nodiscard]] llvm::Value* getIndex() const;

    [[nodiscard]] llvm::Value* getTape() const;

    void setTapePtr(llvm::Value* tape) const;

    [[nodiscard]] llvm::Value* getTapeSize() const;

    [[nodiscard]] llvm::Value* getCurrentChar() const;

    void setCurrentChar(llvm::Value* theChar) const;

    void generateCallTapeDoublingFunction(llvm::Value* newIndex) const {
        std::vector<llvm::Value*> printArgs = {tapePtr.pointer, newIndex, tapeSizePtr.pointer};
        state->builder.CreateCall(state->module.getFunction("doubleTapeIfNeeded"), printArgs);
    }
};

inline BFMachine createBFMachine(CompilerState* state, int initialTapeSize) {
    auto tape = state->clib.generateCallCalloc(state->getConstInt(initialTapeSize));

    auto* int32ty = state->builder.getInt32Ty();
    auto pointer = state->allocateAndInitialize(int32ty, state->getConstInt(0));
    auto tapeSizePtr = state->allocateAndInitialize(int32ty, state->getConstInt(initialTapeSize));
    auto tapePtr = state->allocateAndInitialize(state->getPtrTy(), tape);

    return {tapePtr, pointer, tapeSizePtr, state, initialTapeSize};
}


#endif //YABF_BFMACHINE_H
