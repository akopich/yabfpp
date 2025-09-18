//
// Created by valerij on 7/30/21.
//

#include "Expr.h"

#include <utility>
#include <ranges>
#include "llvm/Transforms/Utils/BasicBlockUtils.h"

void MovePtrExpr::generate(BFMachine& bfMachine) const {
    llvm::Value* index = bfMachine.getIndex();
    llvm::Value* stepValueI8 = steps->generate(bfMachine);

    auto& state = *bfMachine.state;
    auto& builder = state.builder;

    llvm::Value* stepValueI32 = builder->CreateIntCast(stepValueI8, builder->getInt32Ty(), true);
    auto newIndex = state.CreateAdd(index, stepValueI32, "move pointer");

    bfMachine.generateCallTapeDoublingFunction(newIndex);

    builder->CreateStore(newIndex, bfMachine.pointer.pointer);
}

MovePtrExpr::MovePtrExpr(std::unique_ptr<Int8Expr> steps) : steps(move(steps)) {}

void AddExpr::generate(BFMachine& bfMachine) const {
    llvm::Value* theChar = bfMachine.getCurrentChar();
    llvm::Value* newChar = bfMachine.state->CreateAdd(theChar, add->generate(bfMachine), "add char");
    bfMachine.setCurrentChar(newChar);
}

AddExpr::AddExpr(std::unique_ptr<Int8Expr> add) : add(std::move(add)) {}


void ReadExpr::generate(BFMachine& bfMachine) const {
    llvm::Value* readChar = bfMachine.state->generateCallReadCharFunction();
    bfMachine.setCurrentChar(readChar);
}

void PrintExpr::generate(BFMachine& bfMachine) const {
    bfMachine.state->clib->generateCallPutChar(bfMachine.getCurrentChar());
}

void ListExpr::generate(BFMachine& bfMachine) const {
    for (auto& e : v) {
        e->generate(bfMachine);
    }
}

ListExpr::ListExpr(std::vector<std::shared_ptr<Expr>> v) : v(move(v)) {}

Expr::~Expr() = default;

void LoopExpr::generate(BFMachine& bfMachine) const {
    auto& state = *bfMachine.state;
    auto& builder = state.builder;
    llvm::BasicBlock* loopCondBB = state.createBasicBlock("loop cond");
    llvm::BasicBlock* loopBodyBB = state.createBasicBlock("loop body");
    llvm::BasicBlock* afterLoopBB = state.createBasicBlock("after loop");
    builder->CreateBr(loopCondBB);

    builder->SetInsertPoint(loopCondBB);
    auto cond = builder->CreateICmpNE(bfMachine.getCurrentChar(), state.getConstChar(0),
                                      "check loop condition");
    builder->CreateCondBr(cond, loopBodyBB, afterLoopBB);

    builder->SetInsertPoint(loopBodyBB);
    body->generate(bfMachine);
    builder->CreateBr(loopCondBB);

    builder->SetInsertPoint(afterLoopBB);
}

LoopExpr::LoopExpr(std::unique_ptr<Expr> body) : body(std::move(body)) {}

void WriteToVariable::generate(BFMachine& bfMachine) const {
    CompilerState* state = bfMachine.state;
    auto ptr = state->getVariableHandler().getVariablePtr(name);
    state->builder->CreateStore(bfMachine.getCurrentChar(), ptr.pointer);
}

WriteToVariable::WriteToVariable(std::string name) : name(std::move(name)) {}

void AssignExpressionValueToTheCurrentCell::generate(BFMachine& bfMachine) const {
    bfMachine.setCurrentChar(variable->generate(bfMachine));
}

AssignExpressionValueToTheCurrentCell::AssignExpressionValueToTheCurrentCell(std::unique_ptr<Int8Expr> variable)
        : variable(std::move(variable)) {}


llvm::Value* VariableInt8Expr::generate(BFMachine& bfMachine) const {
    return bfMachine.state->getVariableHandler().getVariableValue(name);
}

VariableInt8Expr::VariableInt8Expr(std::string name) : name(std::move(name)) {}

llvm::Value* ConstInt8Expr::generate(BFMachine& bfMachine) const {
    return bfMachine.state->getConstChar(value);
}

ConstInt8Expr::ConstInt8Expr(char value) : value(value) {}

llvm::Value* MinusInt8Expr::generate(BFMachine& bfMachine) const {
    auto beforeMinus = value->generate(bfMachine);
    CompilerState* state = bfMachine.state;
    return state->builder->CreateMul(beforeMinus, state->getConstChar(-1));
}

MinusInt8Expr::MinusInt8Expr(std::unique_ptr<Int8Expr> value) : value(move(value)) {}

void PrintIntExpr::generate(BFMachine& bfMachine) const {
    bfMachine.state->clib->generateCallPrintfInt(bfMachine.getCurrentChar());
}

IfElse::IfElse(std::unique_ptr<Expr> ifExpr, std::unique_ptr<Expr> elseExpr) : ifExpr(move(ifExpr)),
                                                                               elseExpr(move(elseExpr)) {}

void IfElse::generate(BFMachine& bfMachine) const {
    auto& state = *bfMachine.state;
    auto& builder = *state.builder;
    auto cond = builder.CreateICmpNE(bfMachine.getCurrentChar(), state.getConstChar(0),
                                     "check if/else condition");
    llvm::BasicBlock* ifBodyBB = state.createBasicBlock("if branch body");
    llvm::BasicBlock* elseBodyBB = state.createBasicBlock("else branch body");
    llvm::BasicBlock* afterBodyBB = state.createBasicBlock("after if/else block");

    builder.CreateCondBr(cond, ifBodyBB, elseBodyBB);

    builder.SetInsertPoint(ifBodyBB);
    ifExpr->generate(bfMachine);
    builder.CreateBr(afterBodyBB);

    builder.SetInsertPoint(elseBodyBB);
    elseExpr->generate(bfMachine);
    builder.CreateBr(afterBodyBB);

    builder.SetInsertPoint(afterBodyBB);
}


std::unique_ptr<Expr> getNoOpExpr() {
    return std::make_unique<ListExpr>(std::vector<std::shared_ptr<Expr>>());
}

BFFunctionDeclaration::BFFunctionDeclaration(std::string functionName,
                                             std::vector<std::string> variableNames,
                                             std::unique_ptr<Expr> body) : functionName(std::move(functionName)),
                                                                           argumentNames(move(variableNames)),
                                                                           body(move(body)) {}

void BFFunctionDeclaration::generate(BFMachine& bfMachine) const {
    CompilerState* state = bfMachine.state;
    auto& builder = state->builder;

    state->pushVariableHandlerStack();
    auto oldBB = builder->GetInsertBlock();
    auto oldInsertPoint = builder->GetInsertPoint();

    std::vector<llvm::Type*> argTypes(argumentNames.size(), builder->getInt8Ty());

    llvm::Function* function = state->declareBFFunction(functionName, argTypes);

    llvm::BasicBlock* functionBody = state->createBasicBlock(functionName);
    builder->SetInsertPoint(functionBody);

    for (const auto&[argValue, argName] : std::ranges::views::zip(function->args(), argumentNames)) {
        auto argPtr = state->getVariableHandler().getVariablePtr(argName);
        state->CreateStore(&argValue, argPtr.pointer);
    }

    BFMachine localBFMachine = createBFMachine(state, bfMachine.initialTapeSize);
    body->generate(localBFMachine);

    Return defaultReturn;
    defaultReturn.generate(localBFMachine);

    llvm::EliminateUnreachableBlocks(*function);

    state->popVariableHandlerStack();
    state->popFunctionStack();
    builder->SetInsertPoint(oldBB, oldInsertPoint);
}

void Return::generate(BFMachine& bfMachine) const {
    auto* state = bfMachine.state;
    auto& builder = state->builder;
    llvm::Value* valueToReturn = bfMachine.getCurrentChar();

    state->clib->generateCallFree(bfMachine.getTape());

    builder->CreateRet(valueToReturn);

    // This is a hack. Everything added to the block after the ret instruction
    // is considered to be a new unnamed block, which is numbered and the numbering is shared between
    // the instructions and the unnamed blocks. Hence, we make the block named. And it is unreachable.
    // Ultimately, it will be eliminated, as BFFunctionDeclaration calls llvm::EliminateUnreachableBlocks.
    builder->SetInsertPoint(state->createBasicBlock("dead code"));
    // every block needs to have a terminating instruction. 0 is arbitrary.
    builder->CreateRet(state->getConstChar(0));
}

BFFunctionCall::BFFunctionCall(std::string functionName,
                               std::vector<std::shared_ptr<Int8Expr>> arguments)
        : functionName(move(functionName)),
          arguments(move(arguments)) {}

void BFFunctionCall::generate(BFMachine& bfMachine) const {
    std::vector<llvm::Value*> argValues;
    std::transform(arguments.begin(), arguments.end(), std::back_inserter(argValues),
                   [&](auto expr) { return expr->generate(bfMachine); });

    llvm::Value* returnValue = bfMachine.state->builder->CreateCall(bfMachine.state->module->getFunction(functionName),
                                                                    argValues);

    bfMachine.setCurrentChar(returnValue);
}
