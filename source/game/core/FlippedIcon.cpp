#include "game/core/FlippedIcon.hpp"

FlippedIcon::FlippedIcon(TextureAtlasSprite* sprite, bool flipU, bool flipV)
    : TextureAtlasSprite(L"", L"", 0.0f, 0.0f, 0.0f, 0.0f, 256.0f, 256.0f),
      mSprite(sprite),
      mFlipU(flipU),
      mFlipV(flipV)
{
}

unsigned int FlippedIcon::getX() const
{
    return mSprite->getX();
}

unsigned int FlippedIcon::getY() const
{
    return mSprite->getY();
}

unsigned int FlippedIcon::getWidth() const
{
    return mSprite->getWidth();
}

unsigned int FlippedIcon::getHeight() const
{
    return mSprite->getHeight();
}

float FlippedIcon::getU0(bool applyFrame) const
{
    return mFlipU ? mSprite->getU1(applyFrame) : mSprite->getU0(applyFrame);
}

float FlippedIcon::getU1(bool applyFrame) const
{
    return mFlipU ? mSprite->getU0(applyFrame) : mSprite->getU1(applyFrame);
}

float FlippedIcon::getU(double value) const
{
    const float width = getU1(false) - getU0(false);
    return getU0(false) + width * (static_cast<float>(value) * 0.0625f);
}

float FlippedIcon::getV0(bool applyFrame) const
{
    return mSprite->getV0(applyFrame);
}

float FlippedIcon::getV1(bool applyFrame) const
{
    return mFlipV ? mSprite->getV0(applyFrame) : mSprite->getV1(applyFrame);
}

float FlippedIcon::getV(double value) const
{
    const float height = getV1(false) - getV0(false);
    return getV0(false) + height * (static_cast<float>(value) * 0.0625f);
}

std::wstring FlippedIcon::getName() const
{
    return mSprite->getName();
}

unsigned int FlippedIcon::getSourceWidth() const
{
    return mSprite->getSourceWidth();
}

unsigned int FlippedIcon::getSourceHeight() const
{
    return mSprite->getSourceHeight();
}

unsigned int FlippedIcon::getFlags() const
{
    return mSprite->getFlags();
}

void FlippedIcon::setFlags(int flags)
{
    mSprite->setFlags(flags);
}
