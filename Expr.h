//
// Created by valerij on 7/30/21.
//

#include "ContextBuilderModule.h"

#ifndef YABF_EXPR_H
#define YABF_EXPR_H


class Expr {
public:
    virtual void generate(ContextBuilderModule& cbm) const = 0;

    virtual ~Expr();
};

class MovePtrExpr : public Expr {
private:
    int steps{};
public:
    explicit MovePtrExpr(int steps);

    void generate(ContextBuilderModule &cbm)  const  override;
};

class AddExpr: public Expr {
private:
    int add;
public:
    explicit AddExpr(int add);

    void generate(ContextBuilderModule &cbm) const  override;
};

class ReadExpr : public Expr {
public:
    void generate(ContextBuilderModule &cbm) const  override;
};

class PrintExpr : public Expr {
public:
    void generate(ContextBuilderModule &cbm) const override;
};

class LoopExpr: public Expr {
private:
    std::unique_ptr<Expr> body;
public:
    explicit LoopExpr(  Expr* body);

    void generate(ContextBuilderModule &cbm) const override;
};

class ListExpr: public Expr {
private:
    std::vector<Expr*> v;

public:
    explicit ListExpr(const std::vector<Expr*> &v);

    ListExpr(const ListExpr &e) = delete;

    ListExpr& operator=(const ListExpr& e) = delete;

    void generate(ContextBuilderModule &cbm) const override;

    ~ListExpr() override;
};


#endif //YABF_EXPR_H
