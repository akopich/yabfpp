//
// Created by valerij on 8/17/21.
//

#ifndef YABFPP_VARIABLEHANDLER_H
#define YABFPP_VARIABLEHANDLER_H

#include <string>
#include <map>
#include <llvm/IR/Value.h>
#include "CompilerState.h"

class VariableHandler {
private:
    std::map<std::string, llvm::Value*> variableName2Ptr;

    CompilerState* state;
public:
    VariableHandler(CompilerState* state);

    llvm::Value* getVariablePtr(const std::string& name);

    llvm::Value* getVariableValue(const std::string& name);
};


#endif //YABFPP_VARIABLEHANDLER_H
