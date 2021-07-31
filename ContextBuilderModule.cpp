//
// Created by valerij on 7/30/21.
//

#include <vector>
#include <memory>
#include "llvm/Support/raw_ostream.h"
#include "llvm/Support/FileSystem.h"
#include "llvm/IR/Type.h"
#include "llvm/IR/Module.h"
#include "llvm/IR/LLVMContext.h"
#include "llvm/IR/IRBuilder.h"
#include "llvm/IR/Function.h"
#include "llvm/IR/DerivedTypes.h"
#include "llvm/IR/Constants.h"
#include "llvm/IR/BasicBlock.h"
#include "llvm/ADT/APFloat.h"
#include <iostream>
#include "ContextBuilderModule.h"
#include "BFMachine.h"


void ContextBuilderModule::generateEntryPoint() {
    llvm::FunctionType* mainType = llvm::FunctionType::get(builder->getInt32Ty(), false);
    main = llvm::Function::Create(mainType, llvm::Function::ExternalLinkage, "main", module.get());
    llvm::BasicBlock* entry = llvm::BasicBlock::Create(*context, "main", main);
    builder->SetInsertPoint(entry);
}

void ContextBuilderModule::generateBeltDoublingFunction() {
    std::vector<llvm::Type*> agrs = {llvm::PointerType::get(builder->getInt8PtrTy(), 0),
                                     builder->getInt32Ty(),
                                     llvm::PointerType::get(builder->getInt32Ty(), 0)};
    llvm::FunctionType* doublerType = llvm::FunctionType::get(builder->getVoidTy(), agrs, false);
    llvm::Function* doubler = llvm::Function::Create(doublerType, llvm::Function::ExternalLinkage, "doubleBeltIfNeeded",
                                                     module.get());
    llvm::BasicBlock* functionBody = llvm::BasicBlock::Create(*context, "doubleBeltIfNeeded", doubler);
    builder->SetInsertPoint(functionBody);

    auto it = doubler->args().begin();

    llvm::Value* beltPtr = it;
    llvm::Value* newIndex = it + 1;
    llvm::Value* beltSizePtr = it + 2;

    llvm::Value* beltSize = builder->CreateLoad(beltSizePtr);
    llvm::Value* belt = builder->CreateLoad(beltPtr);
    llvm::Value* needsToGrow = builder->CreateICmpUGE(newIndex, beltSize, "check if the beltPtr needs to grow");

    auto doublingBeltBB = createBasicBlock("Doubling the beltPtr", doubler);
    auto afterDoublingBeltBB = createBasicBlock("After doubling the beltPtr", doubler);
    builder->CreateCondBr(needsToGrow, doublingBeltBB, afterDoublingBeltBB);

    builder->SetInsertPoint(doublingBeltBB);
    llvm::Value* newBeltSize = builder->CreateMul(beltSize, getConstInt(2));
    llvm::Value* newBelt = generateCallCalloc(newBeltSize);
    generateCallMemcpy(newBelt, belt, beltSize);
    builder->CreateStore(newBeltSize, beltSizePtr);
    builder->CreateStore(newBelt, beltPtr);

    builder->CreateBr(afterDoublingBeltBB);
    builder->SetInsertPoint(afterDoublingBeltBB);


    builder->CreateRetVoid();
}


void ContextBuilderModule::generateCallBeltDoublingFunction(BFMachine& machine, llvm::Value* newIndex) {
    std::vector<llvm::Value*> printArgs = {machine.beltPtr, newIndex, machine.beltSizePtr};
    builder->CreateCall(module->getFunction("doubleBeltIfNeeded"), printArgs);
}


void ContextBuilderModule::generatePrintfInt() const {
    std::vector<llvm::Type*> args = {llvm::Type::getInt8PtrTy(*context)};
    llvm::FunctionType* printfType = llvm::FunctionType::get(builder->getInt32Ty(), args, true);
    llvm::Function::Create(printfType, llvm::Function::ExternalLinkage, "printf", module.get());
}

void ContextBuilderModule::generateCallPrintfInt(llvm::Value* theInt) const {
    llvm::Value* formatStr = builder->CreateGlobalStringPtr("%d\n");
    std::vector<llvm::Value*> printArgs = {formatStr, theInt};
    builder->CreateCall(module->getFunction("printf"), printArgs);
}

llvm::Value* ContextBuilderModule::getCharArrayElement(llvm::Value* arr, llvm::Value* index) const {
    auto elemPtr = builder->CreateGEP(arr, index);
    return builder->CreateLoad(elemPtr);
}

void ContextBuilderModule::setCharArrayElement(llvm::Value* arr, llvm::Value* index, llvm::Value* theChar) const {
    auto elemPtr = builder->CreateGEP(arr, index);
    builder->CreateStore(theChar, elemPtr);
}

void ContextBuilderModule::finalizeAndPrintIRtoFile(const std::string& outPath) const {
    return0FromMain();
    std::error_code EC;
    llvm::raw_ostream* out = new llvm::raw_fd_ostream(outPath, EC, llvm::sys::fs::F_None);
    module->print(*out, nullptr);
}

void ContextBuilderModule::return0FromMain() const {
    builder->CreateRet(getConstInt(0));
}

llvm::Value* ContextBuilderModule::getConstChar(const char c) const {
    return llvm::ConstantInt::get(*context, llvm::APInt(8, c));
}

llvm::Value* ContextBuilderModule::getConst64(const int i) const {
    return llvm::ConstantInt::get(*context, llvm::APInt(64, i));
}

llvm::Value* ContextBuilderModule::getConstInt(const int i) const {
    return llvm::ConstantInt::get(*context, llvm::APInt(32, i));
}

void ContextBuilderModule::generatePutChar() const {
    std::vector<llvm::Type*> args{llvm::Type::getInt8Ty(*context)};
    llvm::FunctionType* putCharType = llvm::FunctionType::get(builder->getInt32Ty(), args, false);
    llvm::Function::Create(putCharType, llvm::Function::ExternalLinkage, "putchar", module.get());
}

void ContextBuilderModule::generateCallPutChar(llvm::Value* theChar) const {
    builder->CreateCall(module->getFunction("putchar"), {theChar});
}

llvm::Value* ContextBuilderModule::generateCallGetChar() const {
    std::vector<llvm::Value*> args = {};
    return builder->CreateCall(module->getFunction("getchar"), args);
}

void ContextBuilderModule::generateGetChar() const {
    llvm::FunctionType* getCharType = llvm::FunctionType::get(builder->getInt8Ty(), {}, false);
    llvm::Function::Create(getCharType, llvm::Function::ExternalLinkage, "getchar", module.get());
}

llvm::Value* ContextBuilderModule::generateCallCalloc(llvm::Value* size) const {
    return builder->CreateCall(module->getFunction("calloc"), {size, getConstInt(8)});
}

void ContextBuilderModule::generateCalloc() const {
    std::vector<llvm::Type*> args{builder->getInt32Ty(), builder->getInt32Ty()};
    llvm::FunctionType* mallocType = llvm::FunctionType::get(builder->getInt8PtrTy(), args, false);
    llvm::Function::Create(mallocType, llvm::Function::ExternalLinkage, "calloc", module.get());
}

BFMachine ContextBuilderModule::init(const int beltSize) {
    auto belt = generateCallCalloc(getConstInt(beltSize));
    auto pointer = builder->CreateAlloca(builder->getInt32Ty());
    builder->CreateStore(getConstInt(0), pointer);

    auto beltSizePtr = builder->CreateAlloca(builder->getInt32Ty());
    builder->CreateStore(getConstInt(beltSize), beltSizePtr);

    auto beltPtr = builder->CreateAlloca(builder->getInt8PtrTy());
    builder->CreateStore(belt, beltPtr);

    return {beltPtr, pointer, beltSizePtr, this};
}

llvm::BasicBlock* ContextBuilderModule::createBasicBlock(const std::string& s, llvm::Function* function) const {
    return llvm::BasicBlock::Create(*context, s, function);
}

ContextBuilderModule createContextBuilderModule() {
    std::unique_ptr<llvm::LLVMContext> context = std::make_unique<llvm::LLVMContext>();
    std::unique_ptr<llvm::IRBuilder<>> builder = std::make_unique<llvm::IRBuilder<>>(*context);
    std::unique_ptr<llvm::Module> module = std::make_unique<llvm::Module>("compiler", *context);
    ContextBuilderModule cbm(move(context), move(module), move(builder));


    cbm.generatePrintfInt();
    cbm.generatePutChar();
    cbm.generateCalloc();
    cbm.generateGetChar();
    cbm.generateGetChar();
    cbm.generateMemcpy();
    cbm.generateBeltDoublingFunction();
    cbm.generateEntryPoint();

    return cbm;
}

ContextBuilderModule::ContextBuilderModule(std::unique_ptr<llvm::LLVMContext> context,
                                           std::unique_ptr<llvm::Module> module,
                                           std::unique_ptr<llvm::IRBuilder<>> builder) : context(move(context)),
                                                                                         module(move(module)),
                                                                                         builder(move(builder)) {}

llvm::Value* ContextBuilderModule::CreateLoad(llvm::Value* ptr) const {
    return builder->CreateLoad(ptr);
}

llvm::Value* ContextBuilderModule::CreateAdd(llvm::Value* lhs, llvm::Value* rhs, const std::string& name) const {
    return builder->CreateAdd(lhs, rhs, name);
}

void ContextBuilderModule::generateMemcpy() const {
    std::vector<llvm::Type*> args{builder->getInt8PtrTy(), builder->getInt8PtrTy(), builder->getInt32Ty()};
    llvm::FunctionType* memcpyTy = llvm::FunctionType::get(builder->getInt8PtrTy(), args, false);
    llvm::Function::Create(memcpyTy, llvm::Function::ExternalLinkage, "memcpy", module.get());
}

void ContextBuilderModule::generateCallMemcpy(llvm::Value* dest, llvm::Value* src, llvm::Value* size) const {
    builder->CreateCall(module->getFunction("memcpy"), {dest, src, size});
}

llvm::BasicBlock* ContextBuilderModule::createBasicBlock(const std::string& s) const {
    return createBasicBlock(s, main);
}
