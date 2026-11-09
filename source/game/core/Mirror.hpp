#ifndef GAME_CORE_MIRROR_HPP
#define GAME_CORE_MIRROR_HPP

#include <string>

class Direction;
class Rotation;

class Mirror
{
public:
    enum eTYPE
    {
        eTYPE_None = 0,
        eTYPE_LeftRight = 1,
        eTYPE_FrontBack = 2
    };

    static Mirror* NONE;
    static Mirror* LEFT_RIGHT;
    static Mirror* FRONT_BACK;

    Mirror(const std::wstring& name, eTYPE type);

    static Mirror* byName(std::wstring name);

    eTYPE getType() const { return type; }

    int mirror(int rotation, int rotationCount);
    Rotation* getRotation(const Direction* direction);
    const Direction* mirror(const Direction* direction);

    static Mirror* VALUES[3];

private:
    std::wstring name;
    eTYPE type;
};

#endif
