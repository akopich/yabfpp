//
// Created by valerij on 7/30/21.
//

#include "Expr.h"

using namespace std;

void MovePtrExpr::generate(const BFMachine& machine) const {
    llvm::Value* index = machine.getIndex();
    auto newIndex = machine.cbm->CreateAdd(index, machine.cbm->getConstInt(this->steps), "move pointer");
    machine.cbm->builder->CreateStore(newIndex, machine.pointer);
}

MovePtrExpr::MovePtrExpr(int steps) : steps(steps) {}

void AddExpr::generate(const BFMachine& machine) const {
    llvm::Value* theChar = machine.getCurrentChar();
    llvm::Value* newChar = machine.cbm->CreateAdd(theChar, machine.cbm->getConstChar((char) this->add), "add char");
    machine.setCurrentChar(newChar);
}

AddExpr::AddExpr(int add) : add(add) {}

void ReadExpr::generate(const BFMachine& machine) const {
    llvm::Value* readChar = machine.cbm->generateCallGetChar();
    machine.setCurrentChar(readChar);
}

void PrintExpr::generate(const BFMachine& machine) const {
    machine.cbm->generateCallPutChar(machine.getCurrentChar());
}

void ListExpr::generate(const BFMachine& machine) const {
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

void LoopExpr::generate(const BFMachine& machine) const {
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

