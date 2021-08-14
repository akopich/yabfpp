//
// Created by valerij on 7/30/21.
//

#include "Expr.h"

#include <utility>

using namespace std;

void MovePtrExpr::generate(BFMachine& machine) const {
    llvm::Value* index = machine.getIndex();
    llvm::Value* stepValueI8 = steps->generate(machine);

    llvm::Value* stepValueI32 = machine.builder->CreateIntCast(stepValueI8, machine.builder->getInt32Ty(),
                                                               true);
    auto newIndex = machine.state->CreateAdd(index, stepValueI32, "move pointer");

    machine.state->generateCallTapeDoublingFunction(machine, newIndex);

    machine.builder->CreateStore(newIndex, machine.pointer);
}

MovePtrExpr::MovePtrExpr(std::unique_ptr<Int8Expr> steps) : steps(move(steps)) {}

void AddExpr::generate(BFMachine& machine) const {
    llvm::Value* theChar = machine.getCurrentChar();
    llvm::Value* newChar = machine.state->CreateAdd(theChar, this->add->generate(machine), "add char");
    machine.setCurrentChar(newChar);
}

AddExpr::AddExpr(unique_ptr<Int8Expr> add) : add(std::move(add)) {}


void ReadExpr::generate(BFMachine& machine) const {
    llvm::Value* readChar = machine.state->generateCallReadCharFunction();
    machine.setCurrentChar(readChar);
}

void PrintExpr::generate(BFMachine& machine) const {
    machine.state->clib->generateCallPutChar(machine.getCurrentChar());
}

void ListExpr::generate(BFMachine& machine) const {
    for_each(this->v.begin(), this->v.end(), [&](const Expr* e) { // TODO use range-based for
        e->generate(machine);
    });
}

ListExpr::~ListExpr() {
    for_each(this->v.begin(), this->v.end(), [&](const Expr* e) { // TODO use range-based for
        delete e;
    });
}

ListExpr::ListExpr(vector<Expr*> v) : v(move(v)) {}

Expr::~Expr() = default;

void LoopExpr::generate(BFMachine& machine) const {
    llvm::BasicBlock* loopCondBB = machine.state->createBasicBlock("loop cond");
    llvm::BasicBlock* loopBodyBB = machine.state->createBasicBlock("loop body");
    llvm::BasicBlock* afterLoopBB = machine.state->createBasicBlock("after loop");
    machine.builder->CreateBr(loopCondBB);

    machine.builder->SetInsertPoint(loopCondBB);
    auto cond = machine.builder->CreateICmpNE(machine.getCurrentChar(), machine.state->getConstChar(0),
                                              "check loop condition");
    machine.builder->CreateCondBr(cond, loopBodyBB, afterLoopBB);

    machine.builder->SetInsertPoint(loopBodyBB);
    body->generate(machine);
    machine.builder->CreateBr(loopCondBB);

    machine.builder->SetInsertPoint(afterLoopBB);
}

LoopExpr::LoopExpr(Expr* bbody) : body(std::unique_ptr<Expr>(bbody)) {}

void WriteToVariable::generate(BFMachine& machine) const {
    llvm::Value* ptr = machine.getVariablePtr(name);
    machine.builder->CreateStore(machine.getCurrentChar(), ptr);
}

WriteToVariable::WriteToVariable(string name) : name(std::move(name)) {}

void AssignExpressionValueToTheCurrentCell::generate(BFMachine& machine) const {
    machine.setCurrentChar(variable->generate(machine));
}

AssignExpressionValueToTheCurrentCell::AssignExpressionValueToTheCurrentCell(std::unique_ptr<Int8Expr> variable)
        : variable(std::move(variable)) {}


llvm::Value* VariableInt8Expr::generate(BFMachine& machine) const {
    return machine.getVariableValue(name);
}

VariableInt8Expr::VariableInt8Expr(string name) : name(std::move(name)) {}

llvm::Value* ConstInt8Expr::generate(BFMachine& machine) const {
    return machine.state->getConstChar(value);
}

ConstInt8Expr::ConstInt8Expr(char value) : value(value) {}

llvm::Value* MinusInt8Expr::generate(BFMachine& machine) const {
    auto beforeMinus = value->generate(machine);
    return machine.builder->CreateMul(beforeMinus, machine.state->getConstChar(-1));
}

MinusInt8Expr::MinusInt8Expr(unique_ptr<Int8Expr> value) : value(move(value)) {}

void PrintIntExpr::generate(BFMachine& machine) const {
    machine.state->clib->generateCallPrintfInt(machine.getCurrentChar());
}

IfElse::IfElse(unique_ptr<Expr> ifExpr, unique_ptr<Expr> elseExpr) : ifExpr(move(ifExpr)), elseExpr(move(elseExpr)) {}

void IfElse::generate(BFMachine& machine) const {
    auto cond = machine.builder->CreateICmpNE(machine.getCurrentChar(), machine.state->getConstChar(0),
                                              "check if/else condition");
    llvm::BasicBlock* ifBodyBB = machine.state->createBasicBlock("if branch body");
    llvm::BasicBlock* elseBodyBB = machine.state->createBasicBlock("else branch body");
    llvm::BasicBlock* afterBodyBB = machine.state->createBasicBlock("after if/else block");

    machine.builder->CreateCondBr(cond, ifBodyBB, elseBodyBB);

    machine.builder->SetInsertPoint(ifBodyBB);
    ifExpr->generate(machine);
    machine.builder->CreateBr(afterBodyBB);

    machine.builder->SetInsertPoint(elseBodyBB);
    elseExpr->generate(machine);
    machine.builder->CreateBr(afterBodyBB);

    machine.builder->SetInsertPoint(afterBodyBB);
}


std::unique_ptr<Expr> getNoOpExpr() {
    return std::make_unique<ListExpr>(std::vector<Expr*>());
}
