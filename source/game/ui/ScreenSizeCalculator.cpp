#include <cmath>

#include "game/core/Options.hpp"

#include "game/core/Minecraft.hpp"
#include "game/ui/ScreenSizeCalculator.hpp"

ScreenSizeCalculator::ScreenSizeCalculator(Options* options, int guiScale)
{
    _init(options, Minecraft::GetInstance()->mWidth, Minecraft::GetInstance()->mHeight, guiScale);
}

ScreenSizeCalculator::ScreenSizeCalculator(Options* options, int width, int height, int guiScale)
{
    _init(options, width, height, guiScale);
}

void ScreenSizeCalculator::_init(Options* options, int width, int height, int guiScale)
{
    mWidth = width;
    mHeight = height;

    if (guiScale == -1)
    {
        mScale = 1;
        int maxScale = options->mGuiScale;
        if (maxScale == 0)
        {
            maxScale = 1000;
        }

        while (mScale < maxScale && mWidth / (mScale + 1) >= 320 && mHeight / (mScale + 1) >= 240)
        {
            mScale++;
        }
    }
    else
    {
        mScale = guiScale;
    }

    mWidthScale = (double)width / (double)mScale;
    mHeightScale = (double)height / (double)mScale;
    mWidth = (int)std::ceil(mWidthScale);
    mHeight = (int)std::ceil(mHeightScale);
}

uint32_t ScreenSizeCalculator::getWidth() const
{
    return mWidth;
}

uint32_t ScreenSizeCalculator::getHeight() const
{
    return mHeight;
}
