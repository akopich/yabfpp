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

#include "ContextBuilderModule.h"

class ContextBuilderModule;

class BFMachine {
public:
    llvm::Value* belt;
    llvm::Value* pointer;
    ContextBuilderModule* cbm;

    llvm::Value* getIndex();

    llvm::Value* getCurrentChar();

    void setCurrentChar(llvm::Value* theChar);
};


#endif //YABF_BFMACHINE_H
