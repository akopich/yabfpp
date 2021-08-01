//
// Created by valerij on 7/30/21.
//

#include "BFMachine.h"

llvm::Value* BFMachine::getIndex() const {
    return cbm->CreateLoad(pointer);
}

llvm::Value* BFMachine::getCurrentChar() const {
    return cbm->getCharArrayElement(getBelt(), getIndex());
}

void BFMachine::setCurrentChar(llvm::Value* theChar) const {
    cbm->setCharArrayElement(getBelt(), getIndex(), theChar);
}

llvm::Value* BFMachine::getBeltSize() const {
    return cbm->CreateLoad(beltSizePtr);
}

BFMachine::BFMachine(llvm::Value* beltPtr, llvm::Value* pointer, llvm::Value* beltSizePtr, ContextBuilderModule* cbm)
        : beltPtr(beltPtr), pointer(pointer), beltSizePtr(beltSizePtr), cbm(cbm) {}

llvm::Value* BFMachine::getBelt() const {
    return cbm->CreateLoad(beltPtr);
}

void BFMachine::setBeltPtr(llvm::Value* belt) {
    cbm->builder->CreateStore(belt, beltPtr);
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
