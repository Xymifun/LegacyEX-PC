#ifndef GAME_CORE_COMPASSATLASSprite_HPP
#define GAME_CORE_COMPASSATLASSprite_HPP

#include "game/client/render/TextureAtlasSprite.hpp"

class Level;

class CompassAtlasSprite : public TextureAtlasSprite
{
public:
    CompassAtlasSprite();
    CompassAtlasSprite(int playerIndex, CompassAtlasSprite* parent);

    void updateFromPosition(Level* level, double x, double z, double yRot, bool noJitter, bool instant);
    virtual void cycleFrames();
    virtual unsigned int getSourceWidth() const;
    virtual unsigned int getSourceHeight() const;
    virtual int getFrames();
    virtual void freeFrameTextures();
    virtual unsigned int hasOwnData();

    static CompassAtlasSprite* instance;

private:
    unsigned int mPlayerIndex;
    CompassAtlasSprite* mParent;
    double mAngle;
    double mAngleDelta;
};

#endif
