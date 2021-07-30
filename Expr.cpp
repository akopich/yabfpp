//
// Created by valerij on 7/30/21.
//

#include "Expr.h"

using namespace std;

void MovePtrExpr::generate(ContextBuilderModule &cbm) const {
    cout << "Moved by " << this->steps << endl;
}

MovePtrExpr::MovePtrExpr(int steps) : steps(steps) {}

void AddExpr::generate(ContextBuilderModule &cbm) const {
    cout << "added " << this->add << endl;
}

AddExpr::AddExpr(int add) : add(add) {}

void ReadExpr::generate(ContextBuilderModule &cbm) const {
    cout << "read"   << endl;
}

void PrintExpr::generate(ContextBuilderModule &cbm) const {
    cout << "print"   << endl;
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
    cout << "Loop" << endl;
    this->body->generate(cbm);
    cout << "Loop End" << endl;

}

LoopExpr::LoopExpr(  Expr* bbody) : body(std::unique_ptr<Expr>(bbody))  { }

