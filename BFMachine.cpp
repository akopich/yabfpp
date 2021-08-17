//
// Created by valerij on 7/30/21.
//

#include "BFMachine.h"

llvm::Value* BFMachine::getIndex() const {
    return state->CreateLoad(pointer);
}

llvm::Value* BFMachine::getCurrentChar() const {
    return state->getCharArrayElement(getTape(), getIndex());
}

void BFMachine::setCurrentChar(llvm::Value* theChar) const {
    state->setCharArrayElement(getTape(), getIndex(), theChar);
}

llvm::Value* BFMachine::getTapeSize() const {
    return state->CreateLoad(tapeSizePtr);
}

BFMachine::BFMachine(llvm::Value* tapePtr, llvm::Value* pointer, llvm::Value* tapeSizePtr, CompilerState* state)
        : tapePtr(tapePtr), pointer(pointer), tapeSizePtr(tapeSizePtr), state(state) {}

llvm::Value* BFMachine::getTape() const {
    return state->CreateLoad(tapePtr);
}

void BFMachine::setTapePtr(llvm::Value* tape) const {
    state->builder->CreateStore(tape, tapePtr);
}
