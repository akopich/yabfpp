//
// Created by valerij on 8/14/21.
//

#include <llvm/IR/Constants.h>
#include "ConstantHelper.h"

llvm::Value* ConstantHelper::getConstChar(const char c) const {
    return getConstInt(llvm::APInt(8, c));
}

llvm::Value* ConstantHelper::getConst64(const int i) const {
    return getConstInt(llvm::APInt(64, i));
}

llvm::Value* ConstantHelper::getConstInt(const int i) const {
    return getConstInt(llvm::APInt(32, i));
}

llvm::Value* ConstantHelper::getConstInt(const llvm::APInt& v) const {
    return llvm::ConstantInt::get(*getContext(), v);
}
