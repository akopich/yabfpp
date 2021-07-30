//
// Created by valerij on 7/30/21.
//

#include "BFMachine.h"
#include "ContextBuilderModule.h"

llvm::Value *BFMachine::getIndex() {
    return cbm->builder->CreateLoad(pointer);
}

llvm::Value *BFMachine::getCurrentChar() {
    return cbm->getCharArrayElement(belt, getIndex());
}

void BFMachine::setCurrentChar(llvm::Value *theChar) {
    cbm->setCharArrayElement(belt, getIndex(), theChar);
}
