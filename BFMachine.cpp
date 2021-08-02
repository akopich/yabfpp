//
// Created by valerij on 7/30/21.
//

#include "BFMachine.h"

llvm::Value* BFMachine::getIndex() const {
    return cbm->CreateLoad(pointer);
}

llvm::Value* BFMachine::getCurrentChar() const {
    return cbm->getCharArrayElement(getTape(), getIndex());
}

void BFMachine::setCurrentChar(llvm::Value* theChar) const {
    cbm->setCharArrayElement(getTape(), getIndex(), theChar);
}

llvm::Value* BFMachine::getTapeSize() const {
    return cbm->CreateLoad(tapeSizePtr);
}

BFMachine::BFMachine(llvm::Value* tapePtr, llvm::Value* pointer, llvm::Value* tapeSizePtr, ContextBuilderModule* cbm)
        : tapePtr(tapePtr), pointer(pointer), tapeSizePtr(tapeSizePtr), cbm(cbm) {}

llvm::Value* BFMachine::getTape() const {
    return cbm->CreateLoad(tapePtr);
}

void BFMachine::setTapePtr(llvm::Value* tape) const {
    cbm->builder->CreateStore(tape, tapePtr);
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
    auto ptr = cbm->builder->CreateAlloca(cbm->builder->getInt8Ty());
    variableName2Ptr[name] = ptr;
    return ptr;
}

llvm::Value* BFMachine::getVariableValue(const std::string& name) {
    return cbm->builder->CreateLoad(getVariablePtr(name));
}
