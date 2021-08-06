//
// Created by valerij on 8/6/21.
//

#include "PlatformDependent.h"
#include <iostream>
#include <climits>

int X86_64PCLinuxGNU::getEOF() {
    return -1;
}

bool X86_64PCLinuxGNU::isCharSigned() {
    return true;
}

PlatformDependent::~PlatformDependent() {

}

int DefaultPlatform::getEOF() {
    return EOF;
}

bool DefaultPlatform::isCharSigned() {
    return CHAR_MIN < 0;
}


std::unique_ptr<PlatformDependent> getPlatformDependent(const std::string& target) {
    if (target == "x86_64-pc-linux-gnu")
        return std::make_unique<X86_64PCLinuxGNU>();

    std::cout << "The platform " << target << " is not explicitly supported. The IR will"
              << " be generated for the platform the compiler was built on." << std::endl;
    return std::make_unique<DefaultPlatform>();
}
