//
// Created by valerij on 7/30/21.
//

#include "BFMachine.h"

llvm::Value* BFMachine::getIndex() const {
    return state->CreateLoad(state->builder->getInt32Ty(), pointer);
}

llvm::Value* BFMachine::getCurrentChar() const {
    return state->getCharArrayElement(getTape(), getIndex());
}

void BFMachine::setCurrentChar(llvm::Value* theChar) const {
    state->setCharArrayElement(getTape(), getIndex(), theChar);
}

llvm::Value* BFMachine::getTapeSize() const {
    return state->CreateLoad(state->builder->getInt32Ty(), tapeSizePtr);
}

BFMachine::BFMachine(llvm::Value* tapePtr, llvm::Value* pointer, llvm::Value* tapeSizePtr, CompilerState* state)
        : tapePtr(tapePtr), pointer(pointer), tapeSizePtr(tapeSizePtr), state(state) {}

llvm::Value* BFMachine::getTape() const {
    return state->CreateLoad(state->getInt8PtrTy(), tapePtr);
}

void BFMachine::setTapePtr(llvm::Value* tape) const {
    state->builder->CreateStore(tape, tapePtr);
}
