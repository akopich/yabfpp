//
// Created by valerij on 8/17/21.
//

#include "CallStack.h"

CallStack::CallStack(const int size,
                     int initialTapeSize,
                     CompilerState* state,
                     llvm::Value* tapePtr,
                     llvm::Value* pointer,
                     llvm::Value* tapeSizePtr) : SIZE(size),
                                                 initialTapeSize(initialTapeSize),
                                                 state(state),
                                                 tapePtr(tapePtr),
                                                 pointer(pointer),
                                                 tapeSizePtr(tapeSizePtr) {
    currentHeadIndex = state->allocateAndInitialize(state->builder->getInt32Ty(), state->getConstInt(0));
}

BFMachine CallStack::getBFMachine() {
    return {getCurrentTapePtr(),
            getCurrentPointer(),
            getCurrentTapeSizePtr(),
            state
    };
}

std::unique_ptr<CallStack> initCallStack(CompilerState* state, int tapeSize, int callStackSize) {
    auto callStackSizeI = state->getConstInt(callStackSize);

    auto pointer = state->builder->CreateAlloca(state->builder->getInt32Ty(), callStackSizeI);
    state->CreateStore(state->getConstInt(-1), pointer);

    auto tapeSizePtr = state->builder->CreateAlloca(state->builder->getInt32Ty(), callStackSizeI);
    auto tapePtr = state->builder->CreateAlloca(state->builder->getInt8PtrTy(), callStackSizeI);

    auto result = std::make_unique<CallStack>(callStackSize, tapeSize, state, tapePtr, pointer, tapeSizePtr);
    result->push();
    return result;
}


llvm::Value* CallStack::getCurrentHeadIndex() {
    return state->CreateLoad(currentHeadIndex);
}

void CallStack::incrementCurrentHeadIndex() {
    auto newHead = state->CreateAdd(getCurrentHeadIndex(), state->getConstInt(1),
                                    "increment current call stack head index");
    state->CreateStore(newHead, currentHeadIndex);
}

void CallStack::push() {    // todo add stackoverflow check
    incrementCurrentHeadIndex();
    auto tape = state->clib->generateCallCalloc(state->getConstInt(initialTapeSize));
    state->CreateStore(tape, getCurrentTapePtr());
    state->CreateStore(state->getConstInt(0), getCurrentPointer());
    state->CreateStore(state->getConstInt(initialTapeSize), getCurrentTapeSizePtr());
}

llvm::Value* CallStack::getCurrentTapePtr() {
    return state->builder->CreateGEP(state->builder->getInt8PtrTy(), tapePtr, getCurrentHeadIndex());
}

llvm::Value* CallStack::getCurrentPointer() {
    return state->builder->CreateGEP(state->builder->getInt32Ty(), pointer, getCurrentHeadIndex());
}

llvm::Value* CallStack::getCurrentTapeSizePtr() {
    return state->builder->CreateGEP(state->builder->getInt32Ty(), tapeSizePtr, getCurrentHeadIndex());
}

void CallStack::pop() {
    state->clib->generateCallFree(state->CreateLoad(getCurrentTapePtr()));
    decrementCurrentHeadIndex();
}

void CallStack::decrementCurrentHeadIndex() {
    auto newHead = state->CreateAdd(getCurrentHeadIndex(), state->getConstInt(-1),
                                    "decrement current call stack head index");
    state->CreateStore(newHead, currentHeadIndex);
}


