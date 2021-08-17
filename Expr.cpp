//
// Created by valerij on 7/30/21.
//

#include "Expr.h"

#include <utility>

using namespace std;

void MovePtrExpr::generate(CompilerState& state) const {
    BFMachine* bfMachine = state.getBFMachine();
    llvm::Value* index = bfMachine->getIndex();
    llvm::Value* stepValueI8 = steps->generate(state);

    llvm::Value* stepValueI32 = state.builder->CreateIntCast(stepValueI8, state.builder->getInt32Ty(), true);
    auto newIndex = state.CreateAdd(index, stepValueI32, "move pointer");

    state.generateCallTapeDoublingFunction(*bfMachine, newIndex);

    state.builder->CreateStore(newIndex, bfMachine->pointer);
}

MovePtrExpr::MovePtrExpr(std::unique_ptr<Int8Expr> steps) : steps(move(steps)) {}

void AddExpr::generate(CompilerState& state) const {
    BFMachine* bfMachine = state.getBFMachine();
    llvm::Value* theChar = bfMachine->getCurrentChar();
    llvm::Value* newChar = state.CreateAdd(theChar, this->add->generate(state), "add char");
    bfMachine->setCurrentChar(newChar);
}

AddExpr::AddExpr(unique_ptr<Int8Expr> add) : add(std::move(add)) {}


void ReadExpr::generate(CompilerState& state) const {
    llvm::Value* readChar = state.generateCallReadCharFunction();
    state.getBFMachine()->setCurrentChar(readChar);
}

void PrintExpr::generate(CompilerState& state) const {
    state.clib->generateCallPutChar(state.getBFMachine()->getCurrentChar());
}

void ListExpr::generate(CompilerState& state) const {
    for (Expr* e : v) {
        e->generate(state);
    }
}

ListExpr::~ListExpr() {
    for (Expr* e : v) {
        delete e;
    }
}

ListExpr::ListExpr(vector<Expr*> v) : v(move(v)) {}

Expr::~Expr() = default;

void LoopExpr::generate(CompilerState& state) const {
    llvm::BasicBlock* loopCondBB = state.createBasicBlock("loop cond");
    llvm::BasicBlock* loopBodyBB = state.createBasicBlock("loop body");
    llvm::BasicBlock* afterLoopBB = state.createBasicBlock("after loop");
    state.builder->CreateBr(loopCondBB);

    state.builder->SetInsertPoint(loopCondBB);
    auto cond = state.builder->CreateICmpNE(state.getBFMachine()->getCurrentChar(), state.getConstChar(0),
                                            "check loop condition");
    state.builder->CreateCondBr(cond, loopBodyBB, afterLoopBB);

    state.builder->SetInsertPoint(loopBodyBB);
    body->generate(state);
    state.builder->CreateBr(loopCondBB);

    state.builder->SetInsertPoint(afterLoopBB);
}

LoopExpr::LoopExpr(Expr* bbody) : body(std::unique_ptr<Expr>(bbody)) {}

void WriteToVariable::generate(CompilerState& state) const {
    BFMachine* bfMachine = state.getBFMachine();
    llvm::Value* ptr = bfMachine->getVariablePtr(name);
    state.builder->CreateStore(bfMachine->getCurrentChar(), ptr);
}

WriteToVariable::WriteToVariable(string name) : name(std::move(name)) {}

void AssignExpressionValueToTheCurrentCell::generate(CompilerState& state) const {
    state.getBFMachine()->setCurrentChar(variable->generate(state));
}

AssignExpressionValueToTheCurrentCell::AssignExpressionValueToTheCurrentCell(std::unique_ptr<Int8Expr> variable)
        : variable(std::move(variable)) {}


llvm::Value* VariableInt8Expr::generate(CompilerState& state) const {
    return state.getBFMachine()->getVariableValue(name);
}

VariableInt8Expr::VariableInt8Expr(string name) : name(std::move(name)) {}

llvm::Value* ConstInt8Expr::generate(CompilerState& state) const {
    return state.getConstChar(value);
}

ConstInt8Expr::ConstInt8Expr(char value) : value(value) {}

llvm::Value* MinusInt8Expr::generate(CompilerState& state) const {
    auto beforeMinus = value->generate(state);
    return state.builder->CreateMul(beforeMinus, state.getConstChar(-1));
}

MinusInt8Expr::MinusInt8Expr(unique_ptr<Int8Expr> value) : value(move(value)) {}

void PrintIntExpr::generate(CompilerState& state) const {
    state.clib->generateCallPrintfInt(state.getBFMachine()->getCurrentChar());
}

IfElse::IfElse(unique_ptr<Expr> ifExpr, unique_ptr<Expr> elseExpr) : ifExpr(move(ifExpr)), elseExpr(move(elseExpr)) {}

void IfElse::generate(CompilerState& state) const {
    auto cond = state.builder->CreateICmpNE(state.getBFMachine()->getCurrentChar(), state.getConstChar(0),
                                            "check if/else condition");
    llvm::BasicBlock* ifBodyBB = state.createBasicBlock("if branch body");
    llvm::BasicBlock* elseBodyBB = state.createBasicBlock("else branch body");
    llvm::BasicBlock* afterBodyBB = state.createBasicBlock("after if/else block");

    state.builder->CreateCondBr(cond, ifBodyBB, elseBodyBB);

    state.builder->SetInsertPoint(ifBodyBB);
    ifExpr->generate(state);
    state.builder->CreateBr(afterBodyBB);

    state.builder->SetInsertPoint(elseBodyBB);
    elseExpr->generate(state);
    state.builder->CreateBr(afterBodyBB);

    state.builder->SetInsertPoint(afterBodyBB);
}


std::unique_ptr<Expr> getNoOpExpr() {
    return std::make_unique<ListExpr>(std::vector<Expr*>());
}
