#include "game/client/render/BlockEntityRenderer.hpp"

#include "game/client/render/BlockEntityRenderDispatcher.hpp"
#include "game/client/render/GameRenderer.hpp"
#include "game/client/render/Textures.hpp"
#include "game/core/HitResult.hpp"
#include "game/entity/mob/Entity.hpp"
#include "game/util/ResourceLocation.hpp"
#include "game/world/block/BlockEntity.hpp"
#include "game/world/block/BlockPos.hpp"

ResourceLocation BlockEntityRenderer::BREAKING_LOCATIONS[10] = {
    ResourceLocation(TN_TEXTURES_DESTROY_STAGE_0), ResourceLocation(TN_TEXTURES_DESTROY_STAGE_1),
    ResourceLocation(TN_TEXTURES_DESTROY_STAGE_2), ResourceLocation(TN_TEXTURES_DESTROY_STAGE_3),
    ResourceLocation(TN_TEXTURES_DESTROY_STAGE_4), ResourceLocation(TN_TEXTURES_DESTROY_STAGE_5),
    ResourceLocation(TN_TEXTURES_DESTROY_STAGE_6), ResourceLocation(TN_TEXTURES_DESTROY_STAGE_7),
    ResourceLocation(TN_TEXTURES_DESTROY_STAGE_8), ResourceLocation(TN_TEXTURES_DESTROY_STAGE_9)};

Font* BlockEntityRenderer::getFont()
{
    return mDispatcher->getFont();
}

Level* BlockEntityRenderer::getLevel()
{
    return mDispatcher->mLevel;
}

void BlockEntityRenderer::renderNameTag(boost::shared_ptr<BlockEntity> blockEntity, std::wstring name, double x,
                                        double y, double z, int maxDistance)
{
    boost::shared_ptr<Entity> camera = mDispatcher->mCamera;

    if (blockEntity->distanceToSqr(camera->mX, camera->mY, camera->mZ) > (double)(maxDistance * maxDistance))
    {
        return;
    }

    float cameraYRot = mDispatcher->mCameraYRot;
    float cameraXRot = mDispatcher->mCameraXRot;

    GameRenderer::renderNameTagInWorld(getFont(), name, (float)x + 0.5F, (float)y + 1.5F, (float)z + 0.5F, 0, cameraYRot,
                                       cameraXRot, false, false, 0xFF000000, 1.0F);
}

void BlockEntityRenderer::render(boost::shared_ptr<BlockEntity> blockEntity, double x, double y, double z, float a,
                                 int breakProgress, bool bTranslucent, float alpha, bool bUseDisplayList)
{
    std::wstring name = blockEntity->getDisplayName();

    if (!name.empty() && mDispatcher->mCameraHitResult != NULL
        && blockEntity->getBlockPos().equals(mDispatcher->mCameraHitResult->getBlockPos()))
    {
        renderNameTag(blockEntity, name, x, y, z, 12);
    }
}

void BlockEntityRenderer::bindTexture(ResourceLocation* pResourceLocation)
{
    Textures* pTextures = mDispatcher->mTextures;
    if (pTextures != NULL)
    {
        pTextures->bind(pTextures->loadTexture(pResourceLocation->getTexture()));
    }
}

void BlockEntityRenderer::bindTextureLayers(ResourceLocation* pResourceLocation)
{
    if (mDispatcher->mTextures != NULL)
    {
        mDispatcher->mTextures->bindTextureLayers(pResourceLocation);
    }
}

void BlockEntityRenderer::init(BlockEntityRenderDispatcher* pDispatcher)
{
    mDispatcher = pDispatcher;
}

bool BlockEntityRenderer::shouldRenderOffScreen(boost::shared_ptr<BlockEntity> blockEntity)
{
    return false;
}

void BlockEntityRenderer::setupForTexturePack(TexturePack* pTexturePack)
{
}

void BlockEntityRenderer::onNewLevel(Level* pLevel)
{
}
