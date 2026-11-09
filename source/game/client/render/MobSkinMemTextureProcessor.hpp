#ifndef GAME_CLIENT_RENDER_MOBSKINMEMTEXTUREPROCESSOR_HPP
#define GAME_CLIENT_RENDER_MOBSKINMEMTEXTUREPROCESSOR_HPP

#include "game/client/render/MemTextureProcessor.hpp"

class BufferedImage;

class MobSkinMemTextureProcessor : public MemTextureProcessor
{
private:
    int* m_data;
    int m_width;
    int m_height;

public:
    ~MobSkinMemTextureProcessor();
    void setNoAlpha(int x0, int y0, int x1, int y1);
    bool hasAlpha(int x0, int y0, int x1, int y1);
    void setForceAlpha(int x0, int y0, int x1, int y1);
    virtual BufferedImage* process(BufferedImage* image);
};

#endif
