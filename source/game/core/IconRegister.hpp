#ifndef GAME_CORE_ICONREGISTER_HPP
#define GAME_CORE_ICONREGISTER_HPP

#include <string>

class TextureAtlasSprite;

class IconRegister
{
public:
    virtual ~IconRegister() {}

    virtual TextureAtlasSprite* registerIcon(const std::wstring& iconName) = 0;
    virtual int getIconType() = 0;
};

#endif
