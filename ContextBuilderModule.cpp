//
// Created by valerij on 7/30/21.
//

#include <vector>
#include <string>
#include <memory>
#include <map>
#include <cstdlib>
#include <cstdio>
#include <cctype>
#include <algorithm>
#include "llvm/Target/TargetOptions.h"
#include "llvm/Target/TargetMachine.h"
#include "llvm/Support/TargetSelect.h"
#include "llvm/Support/TargetRegistry.h"
#include "llvm/Support/raw_ostream.h"
#include "llvm/Support/Host.h"
#include "llvm/Support/FileSystem.h"
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
#include <iostream>
#include "ContextBuilderModule.h"
#include "BFMachine.h"


void ContextBuilderModule::generateEntryPoint() {
    llvm::FunctionType *mainType = llvm::FunctionType::get(builder->getInt32Ty(), false);
    main = llvm::Function::Create(mainType, llvm::Function::ExternalLinkage, "main", module.get());
    llvm::BasicBlock *entry = llvm::BasicBlock::Create(*context, "main", main);
    builder->SetInsertPoint(entry);
}

void ContextBuilderModule::generatePrintfInt() {
    std::vector<llvm::Type *> args;
    args.push_back(llvm::Type::getInt8PtrTy(*context));
    llvm::FunctionType *printfType = llvm::FunctionType::get(builder->getInt32Ty(), args, true);
    llvm::Function::Create(printfType, llvm::Function::ExternalLinkage, "printf", module.get());
}

void ContextBuilderModule::generateCallPrintfInt(llvm::Value *theInt) {
    std::vector<llvm::Value*> printArgs;
    llvm::Value *formatStr = builder->CreateGlobalStringPtr("%d\n");
    printArgs.push_back(formatStr);
    printArgs.push_back(theInt);
    builder->CreateCall(module->getFunction("printf"), printArgs);
}

llvm::Value *ContextBuilderModule::getCharArrayElement(llvm::Value *arr, llvm::Value *index) {
    auto elemPtr = builder->CreateGEP(arr, index);
    return builder->CreateLoad(elemPtr);
}

void ContextBuilderModule::setCharArrayElement(llvm::Value *arr, llvm::Value *index, llvm::Value *theChar) {
    auto elemPtr = builder->CreateGEP(arr, index);
    builder->CreateStore(theChar, elemPtr);
}

void ContextBuilderModule::finalizeAndPrintIRtoFile(const std::string &outPath) {
    return0FromMain();
    std::error_code EC;
    llvm::raw_ostream* out = new llvm::raw_fd_ostream(outPath, EC, llvm::sys::fs::F_None);
    module->print(*out, nullptr);
}

void ContextBuilderModule::return0FromMain() {
    builder->CreateRet(getConstInt(0));
}

llvm::Value *ContextBuilderModule::getConstChar(const char c) {
    return llvm::ConstantInt::get(*context, llvm::APInt(8, c));
}

llvm::Value *ContextBuilderModule::getConst64(const int i) {
    return llvm::ConstantInt::get(*context, llvm::APInt(64, i));
}

llvm::Value *ContextBuilderModule::getConstInt(const int i) {
    return llvm::ConstantInt::get(*context, llvm::APInt(32, i));
}

void ContextBuilderModule::generatePutChar() {
    std::vector<llvm::Type *> args {llvm::Type::getInt8Ty(*context) };
    llvm::FunctionType *putCharType = llvm::FunctionType::get(builder->getInt32Ty(), args, false);
    llvm::Function::Create(putCharType, llvm::Function::ExternalLinkage, "putchar", module.get());
}

void ContextBuilderModule::generateCallPutChar(llvm::Value *theChar) {
    builder->CreateCall(module->getFunction("putchar"), {theChar});
}

llvm::Value *ContextBuilderModule::generateCallGetChar() {
    std::vector<llvm::Value *> args = {};
    return builder->CreateCall(module->getFunction("getchar"), args);
}

void ContextBuilderModule::generateGetChar() {
    llvm::FunctionType* getCharType = llvm::FunctionType::get(builder->getInt8Ty(), {}, false);
    llvm::Function::Create(getCharType, llvm::Function::ExternalLinkage, "getchar", module.get());
}

llvm::Value *ContextBuilderModule::generateCallCalloc(llvm::Value *size) {
    return builder->CreateCall(module->getFunction("calloc"), {size, getConstInt(8)});
}

void ContextBuilderModule::generateCalloc() {
    std::vector<llvm::Type *> args { builder->getInt32Ty(), builder->getInt32Ty() };
    llvm::FunctionType *mallocType = llvm::FunctionType::get(builder->getInt8PtrTy(), args, false);
    llvm::Function::Create(mallocType, llvm::Function::ExternalLinkage, "calloc", module.get());
}

const int BELT_SIZE = 30000;

BFMachine ContextBuilderModule::init() {
    generateCalloc();
    auto belt = generateCallCalloc(getConstInt(BELT_SIZE));
    auto pointer = builder->CreateAlloca(builder->getInt32Ty());

    builder->CreateStore(getConstInt(0), pointer);
    return {belt, pointer, this};
}

llvm::BasicBlock *ContextBuilderModule::createBasicBlock(const std::string &s) {
    return llvm::BasicBlock::Create(*context, "loop cond", main);
}

ContextBuilderModule createContextBuilderModule() {
    std::unique_ptr<llvm::LLVMContext> context = std::make_unique<llvm::LLVMContext>();
    std::unique_ptr<llvm::IRBuilder<>> builder = std::make_unique<llvm::IRBuilder<>>(*context);
    std::unique_ptr<llvm::Module> module = std::make_unique<llvm::Module>("compiler", *context);
    ContextBuilderModule cbm{move(context), move(builder), move(module)};

    cbm.generateEntryPoint();
    cbm.generatePrintfInt();
    cbm.generatePutChar();
    cbm.generateCalloc();
    cbm.generateGetChar();
    cbm.generateGetChar();

    return cbm;
}
