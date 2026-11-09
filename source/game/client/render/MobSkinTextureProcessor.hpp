#ifndef GAME_CLIENT_RENDER_MOBSKINTEXTUREPROCESSOR_HPP
#define GAME_CLIENT_RENDER_MOBSKINTEXTUREPROCESSOR_HPP

#include <stdint.h>

#include "game/client/render/HttpTextureProcessor.hpp"

class BufferedImage;

class MobSkinTextureProcessor : public HttpTextureProcessor
{
public:
    virtual BufferedImage* process(BufferedImage* image);
    uint32_t hasAlphaPixels(int x0, int y0, int x1, int y1);
    void setForceAlpha(int x0, int y0, int x1, int y1);
    void setNoAlpha(int x0, int y0, int x1, int y1);
};

#endif
