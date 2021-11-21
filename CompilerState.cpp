//
// Created by valerij on 7/30/21.
//

#include "CompilerState.h"


void CompilerState::generateEntryPoint() {
    auto main = clib->declareFunction({},
                                      builder->getInt32Ty(),
                                      false,
                                      "main");
    functionStack.push(main);
    llvm::BasicBlock* entry = llvm::BasicBlock::Create(*context, "main", main);
    builder->SetInsertPoint(entry);
}

llvm::Value* CompilerState::getCharArrayElement(llvm::Value* arr, llvm::Value* index) const {
    auto elemPtr = builder->CreateGEP(arr, index);
    return builder->CreateLoad(elemPtr);
}

void CompilerState::setCharArrayElement(llvm::Value* arr, llvm::Value* index, llvm::Value* theChar) const {
    auto elemPtr = builder->CreateGEP(arr, index);
    builder->CreateStore(theChar, elemPtr);
}


void CompilerState::return0FromMain() const {
    builder->CreateRet(getConstInt(0));
}

BFMachine CompilerState::createBFMachine() {
    auto tape = clib->generateCallCalloc(getConstInt(initialTapeSize));

    auto pointer = allocateAndInitialize(builder->getInt32Ty(), getConstInt(0));
    auto tapeSizePtr = allocateAndInitialize(builder->getInt32Ty(), getConstInt(initialTapeSize));
    auto tapePtr = allocateAndInitialize(builder->getInt8PtrTy(), tape);

    return {tapePtr, pointer, tapeSizePtr, this};
}

llvm::BasicBlock* CompilerState::createBasicBlock(const std::string& s, llvm::Function* function) const {
    return llvm::BasicBlock::Create(*context, s, function);
}

void CompilerState::generateTapeDoublingFunction() {
    std::vector<llvm::Type*> argTypes = {llvm::PointerType::get(builder->getInt8PtrTy(), 0),
                                         builder->getInt32Ty(),
                                         llvm::PointerType::get(builder->getInt32Ty(), 0)};
    llvm::Function* doubler = clib->declareFunction(argTypes,
                                                    builder->getVoidTy(),
                                                    false,
                                                    "doubleTapeIfNeeded");

    llvm::BasicBlock* functionBody = createBasicBlock("doubleTapeIfNeeded", doubler);

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


void CompilerState::generateCallTapeDoublingFunction(BFMachine& machine, llvm::Value* newIndex) const {
    std::vector<llvm::Value*> printArgs = {machine.tapePtr, newIndex, machine.tapeSizePtr};
    builder->CreateCall(module->getFunction("doubleTapeIfNeeded"), printArgs);
}

void CompilerState::generateReadCharFunction() {
    llvm::Function* readChar = clib->declareFunction({},
                                                     builder->getInt8Ty(),
                                                     false,
                                                     "readChar");
    llvm::BasicBlock* functionBody = createBasicBlock("readChar", readChar);
    builder->SetInsertPoint(functionBody);

    llvm::Value* readInt = clib->generateCallGetChar();
    llvm::Value* EOFValue = getConstInt(platformDependent->getEOF());
    llvm::Value* isEOF = builder->CreateICmpEQ(readInt, EOFValue, "EOF check");

    llvm::BasicBlock* isEOFBB = createBasicBlock("is EOF Block", readChar);
    llvm::BasicBlock* notEOFBB = createBasicBlock("no EOF Block", readChar);
    builder->CreateCondBr(isEOF, isEOFBB, notEOFBB);

    builder->SetInsertPoint(isEOFBB);
    builder->CreateRet(getConstChar(0));


    builder->SetInsertPoint(notEOFBB);
    llvm::Value* readI8 = builder->CreateIntCast(readInt, builder->getInt8Ty(), platformDependent->isCharSigned());
    builder->CreateRet(readI8);
}

[[nodiscard]] llvm::Value* CompilerState::generateCallReadCharFunction() const {
    return builder->CreateCall(module->getFunction("readChar"), {});
}

CompilerState::CompilerState(std::unique_ptr<llvm::LLVMContext> context,
                             std::unique_ptr<llvm::Module> module,
                             std::unique_ptr<llvm::IRBuilder<>> builder,
                             std::unique_ptr<CLibHandler> clib,
                             std::unique_ptr<PlatformDependent> platformDependent,
                             int initialTapeSize)
        : context(move(context)),
          module(move(module)),
          builder(move(builder)),
          clib(move(clib)),
          platformDependent(move(platformDependent)),
          initialTapeSize(initialTapeSize) {}

llvm::Value* CompilerState::CreateLoad(llvm::Value* ptr) const {
    return builder->CreateLoad(ptr);
}


llvm::Value* CompilerState::CreateAdd(llvm::Value* lhs, llvm::Value* rhs, const std::string& name) const {
    return builder->CreateAdd(lhs, rhs, name);
}

llvm::BasicBlock* CompilerState::createBasicBlock(const std::string& s) const {
    return createBasicBlock(s, getCurrentFunction());
}

void CompilerState::finalizeAndPrintIRtoFile(const std::string& outPath) const {
    return0FromMain();
    std::error_code EC;
    llvm::raw_ostream *out = new llvm::raw_fd_ostream(outPath, EC, llvm::sys::fs::OF_None);
    module->print(*out, nullptr);
}


llvm::Value* CompilerState::allocateAndInitialize(llvm::Type* type, llvm::Value* value) const {
    auto pointer = builder->CreateAlloca(type);
    builder->CreateStore(value, pointer);
    return pointer;
}

CompilerState initCompilerState(const std::string& name, const std::string& targetTriple, const int tapeSize) {
    std::unique_ptr<llvm::LLVMContext> context = std::make_unique<llvm::LLVMContext>();
    std::unique_ptr<llvm::IRBuilder<>> builder = std::make_unique<llvm::IRBuilder<>>(*context);
    std::unique_ptr<llvm::Module> module = std::make_unique<llvm::Module>(name, *context);

    module->setTargetTriple(targetTriple);

    std::unique_ptr<CLibHandler> clib = std::make_unique<CLibHandler>(module.get(), builder.get());
    clib->init();
    auto platformDependent = getPlatformDependent(targetTriple);
    CompilerState state(move(context), move(module), move(builder), move(clib), move(platformDependent), tapeSize);

    state.generateReadCharFunction();
    state.generateTapeDoublingFunction();
    state.generateEntryPoint();

    return state;
}

[[nodiscard]] llvm::LLVMContext* CompilerState::getContext() const {
    return context.get();
}

VariableHandler& CompilerState::getVariableHandler() {
    return *variableHandlerStack.top();
}

llvm::Value* CompilerState::CreateStore(llvm::Value* value, llvm::Value* ptr) const {
    return builder->CreateStore(value, ptr);
}

void CompilerState::pushVariableHandlerStack() {
    variableHandlerStack.emplace(std::make_shared<VariableHandler>(this));
}

void CompilerState::popVariableHandlerStack() {
    variableHandlerStack.pop();
}

[[nodiscard]] llvm::Function* CompilerState::getCurrentFunction() const {
    return functionStack.top();
}

void CompilerState::popFunctionStack() {
    functionStack.pop();
}

llvm::Function* CompilerState::declareBFFunction(const std::string& name, const std::vector<llvm::Type*>& args) {
    auto f = clib->declareFunction(args,
                                   builder->getInt8Ty(),
                                   false,
                                   name);
    functionStack.push(f);
    return f;
}

