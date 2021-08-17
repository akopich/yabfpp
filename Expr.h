//
// Created by valerij on 7/30/21.
//

#include "CompilerState.h"

#ifndef YABF_EXPR_H
#define YABF_EXPR_H


class Expr {
public:
    virtual void generate(CompilerState& state) const = 0;

    virtual ~Expr();
};

class Int8Expr {
public:
    virtual llvm::Value* generate(CompilerState& state) const = 0;

    virtual ~Int8Expr() = default;
};

class MinusInt8Expr : public Int8Expr {
    std::unique_ptr<Int8Expr> value;
public:
    explicit MinusInt8Expr(std::unique_ptr<Int8Expr> value);

    llvm::Value* generate(CompilerState& state) const override;
};

class VariableInt8Expr : public Int8Expr {
private:
    std::string name;
public:
    explicit VariableInt8Expr(std::string name);

    llvm::Value* generate(CompilerState& state) const override;
};

class ConstInt8Expr : public Int8Expr {
private:
    char value;
public:
    explicit ConstInt8Expr(char value);

    llvm::Value* generate(CompilerState& state) const override;
};

class MovePtrExpr : public Expr {
private:
    std::unique_ptr<Int8Expr> steps;
public:
    explicit MovePtrExpr(std::unique_ptr<Int8Expr> steps);

    void generate(CompilerState& state) const override;
};

class AddExpr : public Expr {
private:
    std::unique_ptr<Int8Expr> add;
public:
    explicit AddExpr(std::unique_ptr<Int8Expr> add);

    void generate(CompilerState& state) const override;
};

class ReadExpr : public Expr {
public:
    void generate(CompilerState& state) const override;
};

class PrintExpr : public Expr {
public:
    void generate(CompilerState& state) const override;
};

class PrintIntExpr : public Expr {
public:
    void generate(CompilerState& state) const override;
};


class LoopExpr : public Expr {
private:
    std::unique_ptr<Expr> body;
public:
    explicit LoopExpr(Expr* body);

    void generate(CompilerState& state) const override;
};

class ListExpr : public Expr {
private:
    std::vector<Expr*> v;

public:
    explicit ListExpr(std::vector<Expr*> v);

    ListExpr(const ListExpr& e) = delete;

    ListExpr& operator=(const ListExpr& e) = delete;

    void generate(CompilerState& state) const override;

    ~ListExpr() override;
};

class WriteToVariable : public Expr {
private:
    std::string name;
public:
    explicit WriteToVariable(std::string name);

    void generate(CompilerState& state) const override;
};

class AssignExpressionValueToTheCurrentCell : public Expr {
private:
    std::unique_ptr<Int8Expr> variable;
public:
    explicit AssignExpressionValueToTheCurrentCell(std::unique_ptr<Int8Expr> name);

    void generate(CompilerState& state) const override;
};


class IfElse : public Expr {
private:
    std::unique_ptr<Expr> ifExpr;
    std::unique_ptr<Expr> elseExpr;
public:
    IfElse(std::unique_ptr<Expr> ifExpr, std::unique_ptr<Expr> elseExpr);

    void generate(CompilerState& state) const override;
};


std::unique_ptr<Expr> getNoOpExpr();

#endif //YABF_EXPR_H
