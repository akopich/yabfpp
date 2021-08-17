//
// Created by valerij on 8/17/21.
//

#include "VariableHandler.h"

VariableHandler::VariableHandler(CompilerState* state) : state(state) {}

/**
 * Does not initialize the pointee
 *
 * @param name
 * @return
 */
llvm::Value* VariableHandler::getVariablePtr(const std::string& name) {
    auto it = variableName2Ptr.find(name);
    if (it != variableName2Ptr.end()) {
        return it->second;
    }
    auto ptr = state->builder->CreateAlloca(state->builder->getInt8Ty());
    variableName2Ptr[name] = ptr;
    return ptr;
}

llvm::Value* VariableHandler::getVariableValue(const std::string& name) {
    return state->CreateLoad(getVariablePtr(name));
}
