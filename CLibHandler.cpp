//
// Created by valerij on 8/6/21.
//

#include "CLibHandler.h"
#include "BFMachine.h"


void CLibHandler::generatePrintfInt() const {
    declareFunction({getInt8PtrTy()},
                    builder->getInt32Ty(),
                    true,
                    "printf");
}

void CLibHandler::generateCallPrintfInt(llvm::Value* theInt) const {
    llvm::Value* formatStr = builder->CreateGlobalStringPtr("%d\n");
    std::vector<llvm::Value*> printArgs = {formatStr, theInt};
    builder->CreateCall(module->getFunction("printf"), printArgs);
}

void CLibHandler::generatePutChar() const {
    declareFunction({builder->getInt8Ty()},
                    builder->getInt32Ty(),
                    false,
                    "putchar");
}

void CLibHandler::generateCallPutChar(llvm::Value* theChar) const {
    builder->CreateCall(module->getFunction("putchar"), {theChar});
}

void CLibHandler::generateGetChar() const {
    declareFunction({}, builder->getInt32Ty(), false, "getchar");
}

llvm::Value* CLibHandler::generateCallGetChar() const {
    return builder->CreateCall(module->getFunction("getchar"), {});
}


llvm::Value* CLibHandler::generateCallCalloc(llvm::Value* size) const {
    return builder->CreateCall(module->getFunction("calloc"), {size, getConstInt(8)});
}

void CLibHandler::generateCalloc() const {
    declareFunction({builder->getInt32Ty(), builder->getInt32Ty()},
                    getInt8PtrTy(),
                    false,
                    "calloc");
}

void CLibHandler::generateMemcpy() const {
    declareFunction({getInt8PtrTy(), getInt8PtrTy(), builder->getInt32Ty()},
                    getInt8PtrTy(),
                    false,
                    "memcpy");
}

void CLibHandler::generateCallMemcpy(llvm::Value* dest, llvm::Value* src, llvm::Value* size) const {
    builder->CreateCall(module->getFunction("memcpy"), {dest, src, size});
}


void CLibHandler::generateFree() const {
    declareFunction({getInt8PtrTy()},
                    builder->getVoidTy(),
                    false,
                    "free");
}

void CLibHandler::generateCallFree(llvm::Value* ptr) const {
    builder->CreateCall(module->getFunction("free"), {ptr});
}

llvm::Function* CLibHandler::declareFunction(const std::vector<llvm::Type*>& argTypes,
                                             llvm::Type* resultType,
                                             const bool isVariadic,
                                             const std::string& name) const {
    llvm::FunctionType* functionType = llvm::FunctionType::get(resultType, argTypes, isVariadic);
    return llvm::Function::Create(functionType, llvm::Function::ExternalLinkage, name, module);
}


void CLibHandler::init() const {
    generatePrintfInt();
    generatePutChar();
    generateCalloc();
    generateFree();
    generateGetChar();
    generateMemcpy();
}

CLibHandler::CLibHandler(llvm::Module* module, llvm::IRBuilder<>* builder) : module(module), builder(builder) {}

llvm::LLVMContext* CLibHandler::getContext() const {
    return &module->getContext();
}

