//
// Created by valerij on 8/17/21.
//

#ifndef YABFPP_CALLSTACK_H
#define YABFPP_CALLSTACK_H


#include <memory>
#include <llvm/IR/Value.h>
#include "CompilerState.h"
#include "BFMachine.h"

class BFMachine;

class CallStack {
private:
    const int SIZE;

    CompilerState* state;

    llvm::Value* tapePtr;                   // i8**
    llvm::Value* pointer;                   // i32*
    llvm::Value* tapeSizePtr;               // i32*
    llvm::Value* currentHeadIndex;          // i32*

public:

    CallStack(int size,
              CompilerState* state,
              llvm::Value* tapePtr,
              llvm::Value* pointer,
              llvm::Value* tapeSizePtr);

    BFMachine getBFMachine();
};

std::unique_ptr<CallStack> initCallStack(CompilerState* state, int tapeSize, int callStackSize);

#endif //YABFPP_CALLSTACK_H
