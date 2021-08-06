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
        : tapePtr(tapePtr), pointer(pointer), tapeSizePtr(tapeSizePtr), state(state) {
    builder = state->builder.get();
}

llvm::Value* BFMachine::getTape() const {
    return state->CreateLoad(tapePtr);
}

void BFMachine::setTapePtr(llvm::Value* tape) const {
    state->builder->CreateStore(tape, tapePtr);
}

/**
 * Does not initialize the pointee
 *
 * @param name
 * @return
 */
llvm::Value* BFMachine::getVariablePtr(const std::string& name) {
    auto it = variableName2Ptr.find(name);
    if (it != variableName2Ptr.end()) {
        return it->second;
    }
    auto ptr = state->builder->CreateAlloca(state->builder->getInt8Ty());
    variableName2Ptr[name] = ptr;
    return ptr;
}

llvm::Value* BFMachine::getVariableValue(const std::string& name) {
    return state->builder->CreateLoad(getVariablePtr(name));
}
