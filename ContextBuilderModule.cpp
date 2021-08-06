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
    main = clib->declareFunction({},
                                 builder->getInt32Ty(),
                                 false,
                                 "main");
    llvm::BasicBlock* entry = llvm::BasicBlock::Create(*context, "main", main);
    builder->SetInsertPoint(entry);
}

llvm::Value* ContextBuilderModule::getCharArrayElement(llvm::Value* arr, llvm::Value* index) const {
    auto elemPtr = builder->CreateGEP(arr, index);
    return builder->CreateLoad(elemPtr);
}

void ContextBuilderModule::setCharArrayElement(llvm::Value* arr, llvm::Value* index, llvm::Value* theChar) const {
    auto elemPtr = builder->CreateGEP(arr, index);
    builder->CreateStore(theChar, elemPtr);
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


BFMachine ContextBuilderModule::init(const int tapeSize) {
    auto tape = clib->generateCallCalloc(getConstInt(tapeSize));

    auto pointer = allocateAndInitialize(builder->getInt32Ty(), getConstInt(0));
    auto tapeSizePtr = allocateAndInitialize(builder->getInt32Ty(), getConstInt(tapeSize));
    auto tapePtr = allocateAndInitialize(builder->getInt8PtrTy(), tape);

    return {tapePtr, pointer, tapeSizePtr, this};
}

llvm::BasicBlock* ContextBuilderModule::createBasicBlock(const std::string& s, llvm::Function* function) const {
    return llvm::BasicBlock::Create(*context, s, function);
}

void ContextBuilderModule::generateTapeDoublingFunction() {
    std::vector<llvm::Type*> argTypes = {llvm::PointerType::get(builder->getInt8PtrTy(), 0),
                                         builder->getInt32Ty(),
                                         llvm::PointerType::get(builder->getInt32Ty(), 0)};
    llvm::Function* doubler = clib->declareFunction(argTypes,
                                                    builder->getVoidTy(),
                                                    false,
                                                    "doubleTapeIfNeeded");

    llvm::BasicBlock* functionBody = llvm::BasicBlock::Create(module->getContext(), "doubleTapeIfNeeded", doubler);
    builder->SetInsertPoint(functionBody);

    auto it = doubler->args().begin();

    llvm::Value* tapePtr = it;
    llvm::Value* newIndex = it + 1;
    llvm::Value* tapeSizePtr = it + 2;

    llvm::Value* tapeSize = builder->CreateLoad(tapeSizePtr);
    llvm::Value* tape = builder->CreateLoad(tapePtr);
    llvm::Value* needsToGrow = builder->CreateICmpUGE(newIndex, tapeSize, "check if the tapePtr needs to grow");

    auto doublingTapeBB = createBasicBlock("Doubling the tapePtr", doubler);
    auto afterDoublingTapeBB = createBasicBlock("After doubling the tapePtr", doubler);
    builder->CreateCondBr(needsToGrow, doublingTapeBB, afterDoublingTapeBB);

    builder->SetInsertPoint(doublingTapeBB);
    llvm::Value* newTapeSize = builder->CreateMul(tapeSize, getConstInt(2));
    llvm::Value* newTape = clib->generateCallCalloc(newTapeSize);
    clib->generateCallMemcpy(newTape, tape, tapeSize);
    clib->generateCallFree(tape);
    builder->CreateStore(newTapeSize, tapeSizePtr);
    builder->CreateStore(newTape, tapePtr);

    builder->CreateBr(afterDoublingTapeBB);
    builder->SetInsertPoint(afterDoublingTapeBB);


    builder->CreateRetVoid();
}


void ContextBuilderModule::generateCallTapeDoublingFunction(BFMachine& machine, llvm::Value* newIndex) {
    std::vector<llvm::Value*> printArgs = {machine.tapePtr, newIndex, machine.tapeSizePtr};
    builder->CreateCall(module->getFunction("doubleTapeIfNeeded"), printArgs);
}

ContextBuilderModule::ContextBuilderModule(std::unique_ptr<llvm::LLVMContext> context,
                                           std::unique_ptr<llvm::Module> module,
                                           std::unique_ptr<llvm::IRBuilder<>> builder,
                                           std::unique_ptr<CLibHandler> clib) : context(move(context)),
                                                                                module(move(module)),
                                                                                builder(move(builder)),
                                                                                clib(move(clib)) {}

llvm::Value* ContextBuilderModule::CreateLoad(llvm::Value* ptr) const {
    return builder->CreateLoad(ptr);
}

llvm::Value* ContextBuilderModule::CreateAdd(llvm::Value* lhs, llvm::Value* rhs, const std::string& name) const {
    return builder->CreateAdd(lhs, rhs, name);
}

llvm::BasicBlock* ContextBuilderModule::createBasicBlock(const std::string& s) const {
    return createBasicBlock(s, main);
}

void ContextBuilderModule::finalizeAndPrintIRtoFile(const std::string& outPath) const {
    return0FromMain();
    std::error_code EC;
    llvm::raw_ostream* out = new llvm::raw_fd_ostream(outPath, EC, llvm::sys::fs::F_None);
    module->print(*out, nullptr);
}

llvm::Value* ContextBuilderModule::allocateAndInitialize(llvm::Type* type, llvm::Value* value) const {
    auto pointer = builder->CreateAlloca(type);
    builder->CreateStore(value, pointer);
    return pointer;
}


ContextBuilderModule createContextBuilderModule(const std::string& name, const std::string& targetTriple) {
    std::unique_ptr<llvm::LLVMContext> context = std::make_unique<llvm::LLVMContext>();
    std::unique_ptr<llvm::IRBuilder<>> builder = std::make_unique<llvm::IRBuilder<>>(*context);
    std::unique_ptr<llvm::Module> module = std::make_unique<llvm::Module>(name, *context);

    module->setTargetTriple(targetTriple);

    std::unique_ptr<CLibHandler> clib = std::make_unique<CLibHandler>(module.get(), builder.get());
    clib->init();
    ContextBuilderModule cbm(move(context), move(module), move(builder), move(clib));

    cbm.generateTapeDoublingFunction();
    cbm.generateEntryPoint();

    return cbm;
}