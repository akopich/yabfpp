//
// Created by valerij on 8/6/21.
//

#include "PlatformDependent.h"

int X86_64PCLinuxGNU::getEOF() {
    return -1;
}

bool X86_64PCLinuxGNU::isCharSigned() {
    return true;
}

PlatformDependent::~PlatformDependent() {

}

std::unique_ptr<PlatformDependent> getPlatformDependent(const std::string& target) {
    if (target == "x86_64-pc-linux-gnu")
        return std::make_unique<X86_64PCLinuxGNU>();
    throw std::invalid_argument("Unsupported target.");
}
