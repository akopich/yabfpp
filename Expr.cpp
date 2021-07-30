//
// Created by valerij on 7/30/21.
//

#include "Expr.h"

using namespace std;

llvm::Value* getIndex(ContextBuilderModule &cbm) {
    return cbm.builder->CreateLoad(cbm.pointer);
}

llvm::Value* getCurrentChar(ContextBuilderModule &cbm) {
    return cbm.getCharArrayElement(cbm.belt, getIndex(cbm));
}

void setCurrentChar(ContextBuilderModule &cbm, llvm::Value* theChar) {
    cbm.setCharArrayElement(cbm.belt, getIndex(cbm), theChar);
}

void MovePtrExpr::generate(ContextBuilderModule &cbm) const {
    llvm::Value *index = getIndex(cbm);
    auto newIndex = cbm.builder->CreateAdd(index, cbm.getConstInt(this->steps), "move pointer");
    cbm.builder->CreateStore(newIndex, cbm.pointer);
}

MovePtrExpr::MovePtrExpr(int steps) : steps(steps) {}

void AddExpr::generate(ContextBuilderModule &cbm) const {
    llvm::Value* theChar = getCurrentChar(cbm);
    llvm::Value* newChar = cbm.builder->CreateAdd(theChar, cbm.getConstChar( this->add), "add char");
    setCurrentChar(cbm, newChar);
}

AddExpr::AddExpr(int add) : add(add) {}

void ReadExpr::generate(ContextBuilderModule &cbm) const {
    llvm::Value* readChar = cbm.generateCallGetChar();
    setCurrentChar(cbm, readChar);
}

void PrintExpr::generate(ContextBuilderModule &cbm) const {
    cbm.generateCallPutChar(getCurrentChar(cbm));
}

void ListExpr::generate(ContextBuilderModule &cbm) const  {
    for_each(this->v.begin(), this->v.end(), [&](const Expr* e) {
        e->generate(cbm);
    });
}

ListExpr::~ListExpr() {
    for_each(this->v.begin(), this->v.end(), [&](const Expr* e) {
        delete e;
    });
}

ListExpr::ListExpr(const vector<Expr *> &v) : v(v) {}

Expr::~Expr() = default;

void LoopExpr::generate(ContextBuilderModule &cbm) const {
    llvm::BasicBlock *loopCondBB = cbm.createBasicBlock("loop cond");
    llvm::BasicBlock *loopBodyBB = cbm.createBasicBlock("loop body");
    llvm::BasicBlock *afterLoopBB = cbm.createBasicBlock("after loop");
    cbm.builder->CreateBr(loopCondBB);

    cbm.builder->SetInsertPoint(loopCondBB);
    auto cond = cbm.builder->CreateICmpNE(getCurrentChar(cbm), cbm.getConstChar(0), "check loop condition");
    cbm.builder->CreateCondBr(cond, loopBodyBB, afterLoopBB);

    cbm.builder->SetInsertPoint(loopBodyBB);
    body->generate(cbm);
    cbm.builder->CreateBr(loopCondBB);

    cbm.builder->SetInsertPoint(afterLoopBB);
}

LoopExpr::LoopExpr(  Expr* bbody) : body(std::unique_ptr<Expr>(bbody))  { }

