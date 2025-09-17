#pragma once

#include <llvm/IR/Value.h>

struct Pointer {
    // the value type, not the pointer type
    llvm::Type* valueType;
    llvm::Value* pointer;
};
