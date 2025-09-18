//
// Created by valerij on 8/14/21.
//

#ifndef YABFPP_CONSTANTHELPER_H
#define YABFPP_CONSTANTHELPER_H


#include <llvm/IR/LLVMContext.h>
#include <llvm/IR/Value.h>

class ConstantHelper {
private:
    [[nodiscard]] llvm::Value* getConstInt(const llvm::APInt& v) const;

    llvm::LLVMContext* context;
public:

    ConstantHelper(llvm::LLVMContext* context) : context(context) {}

    [[nodiscard]] llvm::Value* getConstInt(int i) const;

    [[nodiscard]] llvm::Value* getConst64(int i) const;

    [[nodiscard]] llvm::Value* getConstChar(char c) const;
};


#endif //YABFPP_CONSTANTHELPER_H
