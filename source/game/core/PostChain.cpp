#include "game/core/PostChain.hpp"

#include "game/client/render/RenderTarget.hpp"
#include "game/core/Matrix4.hpp"
#include "game/core/Matrix4f.hpp"
#include "game/core/PostPass.hpp"
#include "game/util/ResourceLocation.hpp"

PostChain::PostChain(TextureManager* textureManager, ResourceManager* resourceManager, RenderTarget* screenTarget,
                     ResourceLocation* name)
{
    mScreenTarget = screenTarget;
    mResourceManager = resourceManager;
    mWidth = screenTarget->width;
    mTime = 0.0f;
    mLastStamp = 0.0f;
    mHeight = screenTarget->height;
    (void)textureManager;
    mName = name->toString();
    updateOrthoMatrix();
}

void PostChain::updateOrthoMatrix()
{
    mOrthoMatrix = new Matrix4f();
    mOrthoMatrix->setIdentity();
    mOrthoMatrix->m[0] = 2.0f / static_cast<float>(mScreenTarget->renderWidth);
    mOrthoMatrix->m[5] = 2.0f / static_cast<float>(-mScreenTarget->renderHeight);
    mOrthoMatrix->m[10] = -0.0020001999f;
    mOrthoMatrix->m[15] = 1.0f;
    mOrthoMatrix->m[3] = -1.0f;
    mOrthoMatrix->m[7] = 1.0f;
    mOrthoMatrix->m[11] = -1.0001999f;
}

void PostChain::destroy()
{
    std::map<std::wstring, RenderTarget*>::const_iterator target = mCustomRenderTargets.begin();
    while (target != mCustomRenderTargets.end())
        ++target;

    std::vector<PostPass*>::iterator pass = mPostPasses.begin();
    while (pass != mPostPasses.end())
    {
        (*pass)->destroy();
        ++pass;
    }
    mPostPasses.clear();
}
