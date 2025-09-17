//
// Created by valerij on 7/30/21.
//

#include "BFMachine.h"

llvm::Value* BFMachine::getIndex() const {
    return state->builder->CreateLoad(pointer);
}

llvm::Value* BFMachine::getCurrentChar() const {
    return state->getCharArrayElement(getTape(), getIndex());
}

void BFMachine::setCurrentChar(llvm::Value* theChar) const {
    state->setCharArrayElement(getTape(), getIndex(), theChar);
}

llvm::Value* BFMachine::getTapeSize() const {
    return state->builder->CreateLoad(tapeSizePtr);
}

llvm::Value* BFMachine::getTape() const {
    return state->builder->CreateLoad(tapePtr);
}

void BFMachine::setTapePtr(llvm::Value* tape) const {
    state->builder->CreateStore(tape, tapePtr.pointer);
}
