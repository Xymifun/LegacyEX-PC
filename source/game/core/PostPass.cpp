#include "game/core/PostPass.hpp"
#include "game/item/Effect.hpp"
#include "game/core/GlStateManager.hpp"
#include "game/core/Minecraft.hpp"
#include "game/core/Uniform.hpp"
#include "game/core/BufferBuilder.hpp"
#include "game/client/render/Tesselator.hpp"
#include "game/client/render/RenderTarget.hpp"

template <typename T> std::wstring _toString(T value);


void PostPass::destroy()
{
    mEffect->destroy();
}

void PostPass::prepareState()
{
    GlStateManager::color4f(1.0f, 1.0f, 1.0f, 1.0f);
    GlStateManager::disableBlend();
    GlStateManager::disableDepthTest();
    GlStateManager::disableAlphaTest();
    GlStateManager::disableFog();
    GlStateManager::disableLighting();
    GlStateManager::disableColorMaterial();
    GlStateManager::enableTexture();
    GlStateManager::bindTexture(0);
}


void PostPass::addAuxAsset(std::wstring name, RenderTarget* target, int width, int height)
{
    mAuxAssetNames.push_back(name);
    mAuxAssets.push_back(target);
    mAuxWidths.push_back(width);
    mAuxHeights.push_back(height);
}

Effect* PostPass::getEffect()
{
    return mEffect;
}

void PostPass::process(float time)
{
    prepareState();
    const float outputWidth = static_cast<float>(mOutput->renderWidth);
    const float outputHeight = static_cast<float>(mOutput->renderHeight);
    GlStateManager::viewport(0, 0, static_cast<int>(outputWidth), static_cast<int>(outputHeight));
    mEffect->setSampler(L"DiffuseSampler", mInput);
    for (int i = 0; i < mAuxAssets.size(); ++i)
    {
        mEffect->setSampler(mAuxAssetNames.at(i), mAuxAssets.at(i));
        mEffect->safeGetUniform(L"AuxSize" + _toString(i))
            ->set(static_cast<float>(mAuxWidths.at(i)), static_cast<float>(mAuxHeights.at(i)));
    }
    mEffect->safeGetUniform(L"ProjMat")->set(mOrthoMatrix);
    mEffect->safeGetUniform(L"InSize")->set(static_cast<float>(mInput->renderWidth),
                                            static_cast<float>(mInput->renderHeight));
    mEffect->safeGetUniform(L"OutSize")->set(outputWidth, outputHeight);
    mEffect->safeGetUniform(L"Time")->set(time);
    Minecraft* minecraft = Minecraft::GetInstance();
    mEffect->safeGetUniform(L"ScreenSize")
        ->set(static_cast<float>(minecraft->mWidth), static_cast<float>(minecraft->mHeight));
    mEffect->apply();
    GlStateManager::depthMask(false);
    GlStateManager::colorMask(true, true, true, true);
    BufferBuilder* builder = Tesselator::getInstance()->getBuilder();
    builder->begin();
    builder->color(255, 255, 255, 255);
    builder->vertex(0.0f, outputHeight, 500.0f);
    builder->vertex(outputWidth, outputHeight, 500.0f);
    builder->vertex(outputWidth, 0.0f, 500.0f);
    builder->vertex(0.0f, 0.0f, 500.0f);
    builder->end();
    GlStateManager::depthMask(true);
    GlStateManager::colorMask(true, true, true, true);
    mEffect->clear();
    for (std::vector<RenderTarget*>::iterator it = mAuxAssets.begin(); it != mAuxAssets.end(); ++it)
        (*it)->unbindRead();
}
