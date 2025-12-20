//
// Created by valerij on 7/30/21.
//

#include "BFMachine.h"
#include "llvm/Transforms/Utils/BasicBlockUtils.h"
#include <utility>
#include <woid.hpp>

#ifndef YABF_EXPR_H
#define YABF_EXPR_H

namespace detail{

using Storage = woid::AnyBuilder
                    ::WithSize<8>
                    ::DisableCopy
                    ::With<woid::FunPtr::DEDICATED>
                    ::Build;

template <typename R>
struct ExprBase : woid::InterfaceBuilder
                      ::With<woid::VTableOwnership::DEDICATED>
                      ::WithStorage<Storage>
                      ::Fun<"generate", [](const auto& obj, BFMachine& bfm) -> R { return obj.generate(bfm); }>
                      ::Build {
    R generate(BFMachine& bfm) const { return this-> template call<"generate">(bfm); }
};

inline auto mkExprBase = []<typename E, typename T, typename ... Args>(std::in_place_type_t<E>, std::in_place_type_t<T>, Args&&... args) { 
    return E{{std::in_place_type<T>, std::forward<Args>(args)...}}; 
};

}

using Expr = detail::ExprBase<void>;

template <typename T>
auto mkExpr = std::bind_front(detail::mkExprBase, std::in_place_type<Expr>, std::in_place_type<T>);

using Int8Expr = detail::ExprBase<llvm::Value*>;

template <typename T>
auto mkInt8Expr = std::bind_front(detail::mkExprBase, std::in_place_type<Int8Expr>, std::in_place_type<T>);

class MinusInt8Expr {
    Int8Expr value;
public:
    explicit MinusInt8Expr(Int8Expr value) : value(std::move(value)) {}

    llvm::Value* generate(BFMachine& bfMachine) const  {
        auto beforeMinus = value.generate(bfMachine);
        CompilerState* state = bfMachine.state;
        return state->builder.CreateMul(beforeMinus, state->getConstChar(-1));
    }
};

class VariableInt8Expr {
private:
    std::string name;
public:
    explicit VariableInt8Expr(std::string name) : name(std::move(name)) {}

    llvm::Value* generate(BFMachine& bfMachine) const {
        return bfMachine.state->getVariableHandler().getVariableValue(name);
    }
};

class ConstInt8Expr {
private:
    char value;
public:
    explicit ConstInt8Expr(char value) : value(value) { }

    llvm::Value* generate(BFMachine& bfMachine) const {
        return bfMachine.state->getConstChar(value);
    }
};

class MovePtrExpr {
private:
    Int8Expr steps;
public:
    explicit MovePtrExpr(Int8Expr steps) : steps(std::move(steps)) {}

    void generate(BFMachine& bfMachine) const {
        llvm::Value* index = bfMachine.getIndex();
        llvm::Value* stepValueI8 = steps.generate(bfMachine);

        auto& state = *bfMachine.state;
        auto& builder = state.builder;

        llvm::Value* stepValueI32 = builder.CreateIntCast(stepValueI8, builder.getInt32Ty(), true);
        auto newIndex = state.CreateAdd(index, stepValueI32, "move pointer");

        bfMachine.generateCallTapeDoublingFunction(newIndex);

        builder.CreateStore(newIndex, bfMachine.pointer.pointer);
    }
};

class AddExpr {
private:
    Int8Expr add;
public:
    explicit AddExpr(Int8Expr add) : add(std::move(add)) {}

    void generate(BFMachine& bfMachine) const {
        llvm::Value* theChar = bfMachine.getCurrentChar();
        llvm::Value* newChar = bfMachine.state->CreateAdd(theChar, add.generate(bfMachine), "add char");
        bfMachine.setCurrentChar(newChar);
    }
};

class ReadExpr {
public:
    void generate(BFMachine& bfMachine) const {
        llvm::Value* readChar = bfMachine.state->generateCallReadCharFunction();
        bfMachine.setCurrentChar(readChar);
    }
};

class PrintExpr {
public:
    void generate(BFMachine& bfMachine) const {
        bfMachine.state->clib.generateCallPutChar(bfMachine.getCurrentChar());
    }
};

class PrintIntExpr {
public:
    void generate(BFMachine& bfMachine) const {
        bfMachine.state->clib.generateCallPrintfInt(bfMachine.getCurrentChar());
    }
};


class LoopExpr {
private:
    Expr body;
public:
    explicit LoopExpr(Expr body): body(std::move(body)) {}

    void generate(BFMachine& bfMachine) const  {
        auto& state = *bfMachine.state;
        auto& builder = state.builder;
        llvm::BasicBlock* loopCondBB = state.createBasicBlock("loop cond");
        llvm::BasicBlock* loopBodyBB = state.createBasicBlock("loop body");
        llvm::BasicBlock* afterLoopBB = state.createBasicBlock("after loop");
        builder.CreateBr(loopCondBB);

        builder.SetInsertPoint(loopCondBB);
        auto cond = builder.CreateICmpNE(bfMachine.getCurrentChar(), state.getConstChar(0),
                "check loop condition");
        builder.CreateCondBr(cond, loopBodyBB, afterLoopBB);

        builder.SetInsertPoint(loopBodyBB);
        body.generate(bfMachine);
        builder.CreateBr(loopCondBB);

        builder.SetInsertPoint(afterLoopBB);
    }
};

class ListExpr {
private:
    std::vector<Expr> v;

public:
    explicit ListExpr(std::vector<Expr>&& v) : v(std::move(v)) {}

    void generate(BFMachine& bfMachine) const {
        for (auto& e : v) {
            e.generate(bfMachine);
        }
    }
};

inline Expr getNoOpExpr() {
    return mkExpr<ListExpr>( std::vector<Expr>{});
}

class WriteToVariable {
private:
    std::string name;
public:
    explicit WriteToVariable(std::string name) : name(std::move(name)) {}

    void generate(BFMachine& bfMachine) const {
        CompilerState* state = bfMachine.state;
        auto ptr = state->getVariableHandler().getVariablePtr(name);
        state->builder.CreateStore(bfMachine.getCurrentChar(), ptr.pointer);
    }
};

class AssignExpressionValueToTheCurrentCell {
private:
    Int8Expr variable;
public:
    explicit AssignExpressionValueToTheCurrentCell(Int8Expr variable): variable(std::move(variable)) {}

    void generate(BFMachine& bfMachine) const {
        bfMachine.setCurrentChar(variable.generate(bfMachine));
    }
};

class IfElse {
private:
    Expr ifExpr;
    Expr elseExpr;
public:
    IfElse(Expr ifExpr, Expr elseExpr) : ifExpr(std::move(ifExpr)), elseExpr(std::move(elseExpr)) {}

    void generate(BFMachine& bfMachine) const {
        auto& state = *bfMachine.state;
        auto& builder = state.builder;
        auto cond = builder.CreateICmpNE(bfMachine.getCurrentChar(), state.getConstChar(0),
                "check if/else condition");
        llvm::BasicBlock* ifBodyBB = state.createBasicBlock("if branch body");
        llvm::BasicBlock* elseBodyBB = state.createBasicBlock("else branch body");
        llvm::BasicBlock* afterBodyBB = state.createBasicBlock("after if/else block");

        builder.CreateCondBr(cond, ifBodyBB, elseBodyBB);

        builder.SetInsertPoint(ifBodyBB);
        ifExpr.generate(bfMachine);
        builder.CreateBr(afterBodyBB);

        builder.SetInsertPoint(elseBodyBB);
        elseExpr.generate(bfMachine);
        builder.CreateBr(afterBodyBB);

        builder.SetInsertPoint(afterBodyBB);
    }
};


class Return {
public:
    void generate(BFMachine& bfMachine) const  {
        auto* state = bfMachine.state;
        auto& builder = state->builder;
        llvm::Value* valueToReturn = bfMachine.getCurrentChar();

        state->clib.generateCallFree(bfMachine.getTape());

        builder.CreateRet(valueToReturn);

        // This is a hack. Everything added to the block after the ret instruction
        // is considered to be a new unnamed block, which is numbered and the numbering is shared between
        // the instructions and the unnamed blocks. Hence, we make the block named. And it is unreachable.
        // Ultimately, it will be eliminated, as BFFunctionDeclaration calls llvm::EliminateUnreachableBlocks.
        builder.SetInsertPoint(state->createBasicBlock("dead code"));
        // every block needs to have a terminating instruction. 0 is arbitrary.
        builder.CreateRet(state->getConstChar(0));
    }
};

class BFFunctionDeclaration {
private:
    std::string functionName;
    std::vector<std::string> argumentNames;
    Expr body;
public:
    BFFunctionDeclaration(std::string functionName,
            std::vector<std::string> variableNames,
            Expr body): functionName(std::move(functionName)),
    argumentNames(std::move(variableNames)),
    body(std::move(body)) {}

    void generate(BFMachine& bfMachine) const {
        CompilerState* state = bfMachine.state;
        auto& builder = state->builder;

        state->pushVariableHandlerStack();
        auto oldBB = builder.GetInsertBlock();
        auto oldInsertPoint = builder.GetInsertPoint();

        std::vector<llvm::Type*> argTypes(argumentNames.size(), builder.getInt8Ty());

        llvm::Function* function = state->declareBFFunction(functionName, argTypes);

        llvm::BasicBlock* functionBody = state->createBasicBlock(functionName);
        builder.SetInsertPoint(functionBody);

        for (const auto&[argValue, argName] : std::ranges::views::zip(function->args(), argumentNames)) {
            auto argPtr = state->getVariableHandler().getVariablePtr(argName);
            state->CreateStore(&argValue, argPtr.pointer);
        }

        BFMachine localBFMachine = createBFMachine(state, bfMachine.initialTapeSize);
        body.generate(localBFMachine);

        Return defaultReturn;
        defaultReturn.generate(localBFMachine);

        llvm::EliminateUnreachableBlocks(*function);

        state->popVariableHandlerStack();
        state->popFunctionStack();
        builder.SetInsertPoint(oldBB, oldInsertPoint);
    }
};

class BFFunctionCall  {
private:
    std::string functionName;
    std::vector<Int8Expr> arguments;
public:
    BFFunctionCall(std::string functionName, std::vector<Int8Expr>&& arguments)
        : functionName(std::move(functionName)),
        arguments(std::move(arguments)) {}

    void generate(BFMachine& bfMachine) const {

        auto argValues = arguments | std::ranges::views::transform([&](auto& expr) { return expr.generate(bfMachine) ; }) 
            | std::ranges::to<std::vector>();

        llvm::Value* returnValue = bfMachine.state->builder.CreateCall(bfMachine.state->module.getFunction(functionName),
                argValues);

        bfMachine.setCurrentChar(returnValue);
    }
};


#endif //YABF_EXPR_H
