//
// Created by valerij on 7/30/21.
//

#include "BFMachine.h"
#include "ContextBuilderModule.h"

llvm::Value* BFMachine::getIndex() const {
    return cbm->CreateLoad(pointer);
}

llvm::Value* BFMachine::getCurrentChar() const {
    return cbm->getCharArrayElement(belt, getIndex());
}

void BFMachine::setCurrentChar(llvm::Value* theChar) const {
    cbm->setCharArrayElement(belt, getIndex(), theChar);
}
