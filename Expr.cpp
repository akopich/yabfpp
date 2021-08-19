//
// Created by valerij on 7/30/21.
//

#include "Expr.h"

#include <utility>

void MovePtrExpr::generate(BFMachine& bfMachine) const {
    llvm::Value* index = bfMachine.getIndex();
    llvm::Value* stepValueI8 = steps->generate(bfMachine);

    auto& state = *bfMachine.state;

    llvm::Value* stepValueI32 = state.builder->CreateIntCast(stepValueI8, state.builder->getInt32Ty(), true);
    auto newIndex = state.CreateAdd(index, stepValueI32, "move pointer");

    state.generateCallTapeDoublingFunction(bfMachine, newIndex);

    state.builder->CreateStore(newIndex, bfMachine.pointer);
}

MovePtrExpr::MovePtrExpr(std::unique_ptr<Int8Expr> steps) : steps(move(steps)) {}

void AddExpr::generate(BFMachine& bfMachine) const {
    llvm::Value* theChar = bfMachine.getCurrentChar();
    llvm::Value* newChar = bfMachine.state->CreateAdd(theChar, this->add->generate(bfMachine), "add char");
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
    for (Expr* e : v) {
        e->generate(bfMachine);
    }
}

ListExpr::~ListExpr() {
    for (Expr* e : v) {
        delete e;
    }
}

ListExpr::ListExpr(std::vector<Expr*> v) : v(move(v)) {}

Expr::~Expr() = default;

void LoopExpr::generate(BFMachine& bfMachine) const {
    auto& state = *bfMachine.state;
    llvm::BasicBlock* loopCondBB = state.createBasicBlock("loop cond");
    llvm::BasicBlock* loopBodyBB = state.createBasicBlock("loop body");
    llvm::BasicBlock* afterLoopBB = state.createBasicBlock("after loop");
    state.builder->CreateBr(loopCondBB);

    state.builder->SetInsertPoint(loopCondBB);
    auto cond = state.builder->CreateICmpNE(bfMachine.getCurrentChar(), state.getConstChar(0),
                                            "check loop condition");
    state.builder->CreateCondBr(cond, loopBodyBB, afterLoopBB);

    state.builder->SetInsertPoint(loopBodyBB);
    body->generate(bfMachine);
    state.builder->CreateBr(loopCondBB);

    state.builder->SetInsertPoint(afterLoopBB);
}

LoopExpr::LoopExpr(Expr* bbody) : body(std::unique_ptr<Expr>(bbody)) {}

void WriteToVariable::generate(BFMachine& bfMachine) const {
    llvm::Value* ptr = bfMachine.state->getVariableHandler().getVariablePtr(name);
    bfMachine.state->builder->CreateStore(bfMachine.getCurrentChar(), ptr);
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
    return bfMachine.state->builder->CreateMul(beforeMinus, bfMachine.state->getConstChar(-1));
}

MinusInt8Expr::MinusInt8Expr(std::unique_ptr<Int8Expr> value) : value(move(value)) {}

void PrintIntExpr::generate(BFMachine& bfMachine) const {
    bfMachine.state->clib->generateCallPrintfInt(bfMachine.getCurrentChar());
}

IfElse::IfElse(std::unique_ptr<Expr> ifExpr, std::unique_ptr<Expr> elseExpr) : ifExpr(move(ifExpr)),
                                                                               elseExpr(move(elseExpr)) {}

void IfElse::generate(BFMachine& bfMachine) const {
    auto& builder = *bfMachine.state->builder;
    auto& state = *bfMachine.state;
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
    return std::make_unique<ListExpr>(std::vector<Expr*>());
}

BFFunctionDeclaration::BFFunctionDeclaration(std::string functionName,
                                             std::vector<std::string> variableNames,
                                             std::unique_ptr<Expr> body) : functionName(std::move(functionName)),
                                                                           argumentNames(move(variableNames)),
                                                                           body(move(body)) {}

void BFFunctionDeclaration::generate(BFMachine& bfMachine) const {
    bfMachine.state->pushVariableHandlerStack();
    auto oldBB = bfMachine.state->builder->GetInsertBlock();
    auto oldInsertPoint = bfMachine.state->builder->GetInsertPoint();

    std::vector<llvm::Type*> argTypes(argumentNames.size(), bfMachine.state->builder->getInt8Ty());

    llvm::Function* function = bfMachine.state->declareBFFunction(functionName, argTypes);

    llvm::BasicBlock* functionBody = bfMachine.state->createBasicBlock(functionName);
    bfMachine.state->builder->SetInsertPoint(functionBody);

    for (int i = 0; i < argumentNames.size(); ++i) { // todo use boost/zip?
        auto argPtr = bfMachine.state->getVariableHandler().getVariablePtr(argumentNames[i]);
        bfMachine.state->CreateStore(function->args().begin() + i, argPtr);
    }

    BFMachine localBFMachine = bfMachine.state->createBFMachine();
    body->generate(localBFMachine);

    bfMachine.state->popVariableHandlerStack();
    bfMachine.state->popFunctionStack();
    bfMachine.state->builder->SetInsertPoint(oldBB, oldInsertPoint);
}

void Return::generate(BFMachine& bfMachine) const {
    bfMachine.state->builder->CreateRet(bfMachine.getCurrentChar());
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
