#ifndef GAME_CLIENT_RENDER_HTTPTEXTURE_HPP
#define GAME_CLIENT_RENDER_HTTPTEXTURE_HPP

#include <string>

class HttpTextureProcessor;

class HttpTexture
{
public:
    HttpTexture(const std::wstring& name, HttpTextureProcessor* processor);
};

#endif
