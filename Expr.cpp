//
// Created by valerij on 7/30/21.
//

#include "Expr.h"

#include <utility>

using namespace std;

void MovePtrExpr::generate(BFMachine& machine) const {
    llvm::Value* index = machine.getIndex();
    llvm::Value* stepValueI8 = steps->generate(machine);
    llvm::Value* stepValueI32 = machine.cbm->builder->CreateIntCast(stepValueI8, machine.cbm->builder->getInt32Ty(),
                                                                    true);
    auto newIndex = machine.cbm->CreateAdd(index, stepValueI32, "move pointer");

    machine.cbm->generateCallTapeDoublingFunction(machine, newIndex);

    machine.cbm->builder->CreateStore(newIndex, machine.pointer);
}

MovePtrExpr::MovePtrExpr(std::unique_ptr<Int8Expr> steps) : steps(move(steps)) {}

void AddExpr::generate(BFMachine& machine) const {
    llvm::Value* theChar = machine.getCurrentChar();
    llvm::Value* newChar = machine.cbm->CreateAdd(theChar, this->add->generate(machine), "add char");
    machine.setCurrentChar(newChar);
}

AddExpr::AddExpr(unique_ptr<Int8Expr> add) : add(std::move(add)) {}


void ReadExpr::generate(BFMachine& machine) const {
    llvm::Value* readChar = machine.cbm->clib->generateCallGetChar();
    machine.setCurrentChar(readChar);
}

void PrintExpr::generate(BFMachine& machine) const {
    machine.cbm->clib->generateCallPutChar(machine.getCurrentChar());
}

void ListExpr::generate(BFMachine& machine) const {
    for_each(this->v.begin(), this->v.end(), [&](const Expr* e) {
        e->generate(machine);
    });
}

ListExpr::~ListExpr() {
    for_each(this->v.begin(), this->v.end(), [&](const Expr* e) {
        delete e;
    });
}

ListExpr::ListExpr(const vector<Expr*>& v) : v(v) {}

Expr::~Expr() = default;

void LoopExpr::generate(BFMachine& machine) const {
    llvm::BasicBlock* loopCondBB = machine.cbm->createBasicBlock("loop cond");
    llvm::BasicBlock* loopBodyBB = machine.cbm->createBasicBlock("loop body");
    llvm::BasicBlock* afterLoopBB = machine.cbm->createBasicBlock("after loop");
    machine.cbm->builder->CreateBr(loopCondBB);

    machine.cbm->builder->SetInsertPoint(loopCondBB);
    auto cond = machine.cbm->builder->CreateICmpNE(machine.getCurrentChar(), machine.cbm->getConstChar(0),
                                                   "check loop condition");
    machine.cbm->builder->CreateCondBr(cond, loopBodyBB, afterLoopBB);

    machine.cbm->builder->SetInsertPoint(loopBodyBB);
    body->generate(machine);
    machine.cbm->builder->CreateBr(loopCondBB);

    machine.cbm->builder->SetInsertPoint(afterLoopBB);
}

LoopExpr::LoopExpr(Expr* bbody) : body(std::unique_ptr<Expr>(bbody)) {}

void WriteToVariable::generate(BFMachine& machine) const {
    llvm::Value* ptr = machine.getVariablePtr(name);
    machine.cbm->builder->CreateStore(machine.getCurrentChar(), ptr);
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
    return machine.cbm->getConstChar(value);
}

ConstInt8Expr::ConstInt8Expr(char value) : value(value) {}

llvm::Value* MinusInt8Expr::generate(BFMachine& machine) const {
    auto beforeMinus = value->generate(machine);
    return machine.cbm->builder->CreateMul(beforeMinus, machine.cbm->getConstChar(-1));
}

MinusInt8Expr::MinusInt8Expr(unique_ptr<Int8Expr> value) : value(move(value)) {}

void PrintIntExpr::generate(BFMachine& machine) const {
    machine.cbm->clib->generateCallPrintfInt(machine.getCurrentChar());
}
