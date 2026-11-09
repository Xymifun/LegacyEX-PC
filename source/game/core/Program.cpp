#include "game/core/Program.hpp"

Program::Type::Type(const std::wstring& name, const std::wstring& extension, int glType)
{
    mName = name;
    mExtension = extension;
    mGlType = glType;
}
