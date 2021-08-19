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

protected:
    [[nodiscard]] virtual llvm::LLVMContext* getContext() const = 0;

public:
    [[nodiscard]] llvm::Value* getConstInt(int i) const;

    [[nodiscard]] llvm::Value* getConst64(int i) const;

    [[nodiscard]] llvm::Value* getConstChar(char c) const;
};


#endif //YABFPP_CONSTANTHELPER_H
