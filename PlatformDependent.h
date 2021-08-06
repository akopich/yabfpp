//
// Created by valerij on 8/6/21.
//

#ifndef YABFPP_PLATFORMDEPENDENT_H
#define YABFPP_PLATFORMDEPENDENT_H

#include <string>
#include <memory>
#include <stdexcept>


class PlatformDependent {
public:
    virtual int getEOF() = 0;

    virtual bool isCharSigned() = 0;

    virtual ~PlatformDependent();
};

class X86_64PCLinuxGNU : public PlatformDependent {
public:
    int getEOF() override;

    bool isCharSigned() override;
};

std::unique_ptr<PlatformDependent> getPlatformDependent(const std::string& target);

#endif //YABFPP_PLATFORMDEPENDENT_H
