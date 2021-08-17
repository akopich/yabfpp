//
// Created by valerij on 8/17/21.
//

#include "CallStack.h"

CallStack::CallStack(const int size,
                     CompilerState* state,
                     llvm::Value* tapePtr,
                     llvm::Value* pointer,
                     llvm::Value* tapeSizePtr) : SIZE(size), state(state), tapePtr(tapePtr), pointer(pointer),
                                                 tapeSizePtr(tapeSizePtr) {
    currentHeadIndex = state->allocateAndInitialize(state->builder->getInt32Ty(), state->getConstInt(0));
}

BFMachine CallStack::getBFMachine() {
    auto currentHeadIndexValue = state->CreateLoad(currentHeadIndex);
    return {state->builder->CreateGEP(state->builder->getInt8PtrTy(), tapePtr, currentHeadIndexValue),
            state->builder->CreateGEP(state->builder->getInt32Ty(), pointer, currentHeadIndexValue),
            state->builder->CreateGEP(state->builder->getInt32Ty(), tapeSizePtr, currentHeadIndexValue),
            state
    };
}

llvm::Value* allocateAndInitialize(llvm::IRBuilder<>* builder,
                                   llvm::Type* type,
                                   llvm::Value* value,
                                   llvm::Value* size) { //todo possible code duplication of CompilerState
    auto pointer = builder->CreateAlloca(type, size);
    builder->CreateStore(value, pointer);
    return pointer;
}


std::unique_ptr<CallStack> initCallStack(CompilerState* state, int tapeSize, int callStackSize) {
    auto tape = state->clib->generateCallCalloc(state->getConstInt(tapeSize));

    auto callStackSizeI = state->getConstInt(callStackSize);

    auto pointer = allocateAndInitialize(state->builder.get(), state->builder->getInt32Ty(), state->getConstInt(0),
                                         callStackSizeI);
    auto tapeSizePtr = allocateAndInitialize(state->builder.get(), state->builder->getInt32Ty(),
                                             state->getConstInt(tapeSize), callStackSizeI);
    auto tapePtr = allocateAndInitialize(state->builder.get(), state->builder->getInt8PtrTy(), tape, callStackSizeI);

    return std::make_unique<CallStack>(callStackSize, state, tapePtr, pointer, tapeSizePtr);
}
