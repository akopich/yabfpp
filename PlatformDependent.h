//
// Created by valerij on 8/6/21.
//

#ifndef YABFPP_PLATFORMDEPENDENT_H
#define YABFPP_PLATFORMDEPENDENT_H

#include <climits>
#include <string_view>
#include <iostream>

struct PlatformDependent {
    int eOF;
    bool isCharSigned;
};

inline PlatformDependent getPlatformDependent(std::string_view target) {
    static constexpr PlatformDependent kX86_64PCLinuxGNU {
        .eOF = -1,
        .isCharSigned = true
    };

    static constexpr PlatformDependent kDefaultPlatform {
        .eOF = EOF,
        .isCharSigned = CHAR_MIN < 0
    };

    if (target == "x86_64-pc-linux-gnu" || target == "wasm32-unknown-emscripten")
        return kX86_64PCLinuxGNU;

    std::cout << "The platform " << target << " is not explicitly supported. The IR will"
        << " be generated for the platform the compiler was built on." << std::endl;
    return kDefaultPlatform;
}

#endif //YABFPP_PLATFORMDEPENDENT_H
