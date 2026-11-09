#ifndef GAME_CORE_CLOCKATLASSprite_HPP
#define GAME_CORE_CLOCKATLASSprite_HPP

#include "game/client/render/TextureAtlasSprite.hpp"

class ClockAtlasSprite : public TextureAtlasSprite
{
public:
    ClockAtlasSprite();
    ClockAtlasSprite(int playerIndex, ClockAtlasSprite* parent);
    virtual void cycleFrames();
    virtual unsigned int getSourceWidth() const;
    virtual unsigned int getSourceHeight() const;
    virtual int getFrames();
    virtual void freeFrameTextures();
    virtual unsigned int hasOwnData();

private:
    double mLastTime;
    double mRotation;
    int mPlayerIndex;
    ClockAtlasSprite* mParent;
};

#endif
