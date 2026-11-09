#include "game/core/CompassAtlasSprite.hpp"

#include <cmath>

#include "game/client/render/Texture.hpp"
#include "game/core/Minecraft.hpp"
#include "game/core/Mth.hpp"
#include "game/util/Math.hpp"
#include "game/world/Dimension.hpp"
#include "game/world/block/BlockPos.hpp"
#include "game/world/generation/MultiPlayerLevel.hpp"
#include "game/world/generation/MultiplayerLocalPlayer.hpp"

CompassAtlasSprite* CompassAtlasSprite::instance = NULL;

CompassAtlasSprite::CompassAtlasSprite()
    : TextureAtlasSprite(L"compass", L"compass", 0.0f, 0.0f, 0.0f, 0.0f, 256.0f, 256.0f),
      mPlayerIndex(255),
      mParent(0),
      mAngle(0.0),
      mAngleDelta(0.0)
{
    instance = this;
}

CompassAtlasSprite::CompassAtlasSprite(int playerIndex, CompassAtlasSprite* parent)
    : TextureAtlasSprite(L"compass", L"compass", 0.0f, 0.0f, 0.0f, 0.0f, 256.0f, 256.0f),
      mPlayerIndex(playerIndex),
      mParent(parent),
      mAngle(0.0),
      mAngleDelta(0.0)
{
}

void CompassAtlasSprite::updateFromPosition(Level* level, double x, double z, double yRot, bool noJitter, bool instant)
{
    const double pi = 3.1415927f;
    double target = 0.0;
    if (level != NULL && !noJitter)
    {
        BlockPos spawn = level->getSharedSpawnPos();
        double dx = spawn.x - x;
        double dz = spawn.z - z;
        yRot = static_cast<int>(yRot) % 360;
        target = -((yRot - 90.0) * pi / 180.0 - std::atan2(dz, dx));
        if (!level->mDimension->isNaturalDimension())
        {
            target = Math::random() * pi * 2.0;
        }
    }

    if (instant)
    {
        mAngle = target;
    }
    else
    {
        double delta = target - mAngle;
        while (delta < -pi)
        {
            delta += pi * 2.0;
        }
        while (delta >= pi)
        {
            delta -= pi * 2.0;
        }
        if (delta < -1.0)
        {
            delta = -1.0;
        }
        if (delta > 1.0)
        {
            delta = 1.0;
        }
        mAngleDelta = (delta * 0.1 + mAngleDelta) * 0.8;
        mAngle += mAngleDelta;
    }

    if (mParent != NULL)
    {
        int frame = static_cast<int>((mAngle / (pi * 2.0) + 1.0) * mParent->mFrames->size()) % mParent->mFrames->size();
        while (frame < 0)
        {
            frame = (frame + static_cast<int>(mParent->mFrames->size())) % static_cast<int>(mParent->mFrames->size());
        }
        if (frame != mFrame)
        {
            mFrame = frame;
            mParent->mSource->blit(mX, mY, mParent->mFrames->at(frame), mRotated);
        }
    }
    else
    {
        int frame = static_cast<int>((mAngle / (pi * 2.0) + 1.0) * mFrames->size()) % mFrames->size();
        while (frame < 0)
        {
            frame = (frame + static_cast<int>(mFrames->size())) % static_cast<int>(mFrames->size());
        }
        if (frame != mFrame)
        {
            mFrame = frame;
            mSource->blit(mX, mY, mFrames->at(frame), mRotated);
        }
    }
}

void CompassAtlasSprite::cycleFrames()
{
    Minecraft* minecraft = Minecraft::GetInstance();
    if (mPlayerIndex < 4 && minecraft->mLevel != NULL && minecraft->mLocalPlayers[mPlayerIndex].get() != NULL)
    {
        updateFromPosition(minecraft->mLocalPlayers[mPlayerIndex]->mLevel, minecraft->mLocalPlayers[mPlayerIndex]->mX,
                           minecraft->mLocalPlayers[mPlayerIndex]->mZ, minecraft->mLocalPlayers[mPlayerIndex]->mYRot,
                           false, false);
        return;
    }
    mFrame = 1;
    updateFromPosition(NULL, 0.0, 0.0, 0.0, false, true);
}

unsigned int CompassAtlasSprite::getSourceWidth() const
{
    return mSource->getWidth();
}

unsigned int CompassAtlasSprite::getSourceHeight() const
{
    return mSource->getHeight();
}

int CompassAtlasSprite::getFrames()
{
    if (!mParent)
        return TextureAtlasSprite::getFrames();
    return mParent->getFrames();
}

void CompassAtlasSprite::freeFrameTextures()
{
    if (mParent)
        return;
    TextureAtlasSprite::freeFrameTextures();
}

unsigned int CompassAtlasSprite::hasOwnData()
{
    return mParent == 0;
}
