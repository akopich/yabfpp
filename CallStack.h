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
    const int initialTapeSize;

    CompilerState* state;

    llvm::Value* tapePtr;                   // i8**
    llvm::Value* pointer;                   // i32*
    llvm::Value* tapeSizePtr;               // i32*
    llvm::Value* currentHeadIndex;          // i32*

    void incrementCurrentHeadIndex();

    void decrementCurrentHeadIndex();

    llvm::Value* getCurrentHeadIndex();

    llvm::Value* getCurrentTapePtr();

    llvm::Value* getCurrentPointer();

    llvm::Value* getCurrentTapeSizePtr();

public:

    CallStack(int size,
              int initialTapeSize,
              CompilerState* state,
              llvm::Value* tapePtr,
              llvm::Value* pointer,
              llvm::Value* tapeSizePtr);

    BFMachine getBFMachine();

    void push();

    void pop();
};

std::unique_ptr<CallStack> initCallStack(CompilerState* state, int tapeSize, int callStackSize);

#endif //YABFPP_CALLSTACK_H
