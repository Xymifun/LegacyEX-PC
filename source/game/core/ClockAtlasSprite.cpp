#include "game/core/ClockAtlasSprite.hpp"
#include "game/client/render/Texture.hpp"
#include "game/util/Math.hpp"
#include "game/core/Minecraft.hpp"
#include "game/core/Mth.hpp"
#include "game/world/Dimension.hpp"
#include "game/world/generation/MultiPlayerLevel.hpp"
#include "game/world/generation/MultiplayerLocalPlayer.hpp"

ClockAtlasSprite::ClockAtlasSprite() : TextureAtlasSprite(L"clock", L"clock", 0.0f, 0.0f, 0.0f, 0.0f, 256.0f, 256.0f)
{
    mLastTime = mRotation = 0.0;
    mPlayerIndex = 255;
    mParent = NULL;
}

ClockAtlasSprite::ClockAtlasSprite(int playerIndex, ClockAtlasSprite* parent)
    : TextureAtlasSprite(L"clock", L"clock", 0.0f, 0.0f, 0.0f, 0.0f, 256.0f, 256.0f)
{
    mPlayerIndex = playerIndex;
    mParent = parent;
    mLastTime = mRotation = 0.0;
}

void ClockAtlasSprite::cycleFrames()
{
    Minecraft* minecraft = Minecraft::GetInstance();
    double time = 0.0;
    if (mPlayerIndex >= 0 && mPlayerIndex < 4 && minecraft->mLevel != NULL
        && minecraft->mLocalPlayers[mPlayerIndex] != NULL)
    {
        time = minecraft->mLocalPlayers[mPlayerIndex]->mLevel->getTimeOfDay(1.0f);
        if (!minecraft->mLocalPlayers[mPlayerIndex]->mLevel->mDimension->isNaturalDimension())
        {
            time = Math::random();
        }
    }
    else
    {
        mFrame = 1;
    }

    double delta = time - mLastTime;
    while (delta < -0.5)
    {
        delta += 1.0;
    }
    while (delta >= 0.5)
    {
        delta -= 1.0;
    }
    delta = Mth::clamp(delta, -1.0, 1.0);
    mRotation = (delta * 0.1 + mRotation) * 0.8;
    mLastTime += mRotation;

    if (mParent != NULL)
    {
        int frame = static_cast<int>((mLastTime + 1.0) * mParent->mFrames->size()) % mParent->mFrames->size();
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
        int frame = static_cast<int>((mLastTime + 1.0) * mFrames->size()) % mFrames->size();
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

unsigned int ClockAtlasSprite::getSourceWidth() const
{
    return mSource->getWidth();
}

unsigned int ClockAtlasSprite::getSourceHeight() const
{
    return mSource->getHeight();
}

int ClockAtlasSprite::getFrames()
{
    if (mParent == 0)
        return TextureAtlasSprite::getFrames();
    return mParent->getFrames();
}

void ClockAtlasSprite::freeFrameTextures()
{
    if (mParent != 0)
        return;
    TextureAtlasSprite::freeFrameTextures();
}

unsigned int ClockAtlasSprite::hasOwnData()
{
    return mParent == 0;
}
