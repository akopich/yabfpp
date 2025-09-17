//
// Created by valerij on 8/17/21.
//

#ifndef YABFPP_VARIABLEHANDLER_H
#define YABFPP_VARIABLEHANDLER_H

#include <string>
#include <map>
#include <llvm/IR/Value.h>
#include "Builder.h"

class VariableHandler {
private:
    std::map<std::string, llvm::Value*> variableName2Ptr;

    Builder* builder;
public:
    VariableHandler(Builder* builder): builder(builder) {}

    Pointer getVariablePtr(const std::string& name) {
        auto it = variableName2Ptr.find(name);
        if (it != variableName2Ptr.end()) {
            return {builder->getInt8Ty(), it->second};
        }
        auto ptr = builder->CreateAlloca(builder->getInt8Ty());
        variableName2Ptr[name] = ptr;
        return {builder->getInt8Ty(), ptr};
    }


    llvm::Value* getVariableValue(const std::string& name) {
        return builder->CreateLoad(getVariablePtr(name));
    }
};


#endif //YABFPP_VARIABLEHANDLER_H
