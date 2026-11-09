#include "game/client/render/SkullBlockRenderer.hpp"

#include <cmath>

#include "game/client/render/CreeperHeadModel.hpp"
#include "game/client/render/DragonHeadModel.hpp"
#include "game/client/render/ModelDefinition.hpp"
#include "game/client/render/SkeletonHeadModel.hpp"
#include "game/client/render/TexturePack.hpp"
#include "game/core/Direction.hpp"
#include "game/core/GlStateManager.hpp"
#include "game/util/ResourceLocation.hpp"
#include "game/world/block/BlockEntity.hpp"
#include "game/world/block/SkullBlockEntity.hpp"

static ResourceLocation SKULL_TEXTURE_0(TN_MOB_SKELETON);
static ResourceLocation SKULL_TEXTURE_1(TN_MOB_SKELETON_WITHER);
static ResourceLocation SKULL_TEXTURE_2(TN_MOB_ZOMBIE);
static ResourceLocation SKULL_TEXTURE_3(TN_MOB_STEVE);
static ResourceLocation SKULL_TEXTURE_4(TN_MOB_CREEPER);
static ResourceLocation SKULL_TEXTURE_5(TN_MOB_ENDERDRAGON_ENDER);

SkullBlockRenderer* SkullBlockRenderer::instance = NULL;

SkullBlockRenderer::SkullBlockRenderer()
{
    mActiveSkeletonModels[0] = new SkeletonHeadModel(0, 0, 64, 32);
    mActiveSkeletonModels[1] = new SkeletonHeadModel(0, 0, 64, 32);
    mActiveSkeletonModels[2] = new SkeletonHeadModel(0, 0, 64, 32);
    mActiveSkeletonModels[3] = new SkeletonHeadModel(0, 0, 64, 64);
    mActiveCreeperModel = new CreeperHeadModel(0.0F);
    mActiveDragonModel = new DragonHeadModel(0.0F);
    mDragonModel = mActiveDragonModel;
    mCreeperModel = mActiveCreeperModel;
    mSkeletonModels[0] = mActiveSkeletonModels[0];
    mSkeletonModels[1] = mActiveSkeletonModels[1];
    mSkeletonModels[2] = mActiveSkeletonModels[2];
    mSkeletonModels[3] = mActiveSkeletonModels[3];
}

SkullBlockRenderer::~SkullBlockRenderer()
{
    if (mDragonModel != mActiveDragonModel && mDragonModel != NULL)
        delete mDragonModel;
    if (mCreeperModel != mActiveCreeperModel && mCreeperModel != NULL)
        delete mCreeperModel;
    if (mSkeletonModels[0] != mActiveSkeletonModels[0] && mSkeletonModels[0] != NULL)
        delete mSkeletonModels[0];
    if (mSkeletonModels[1] != mActiveSkeletonModels[1] && mSkeletonModels[1] != NULL)
        delete mSkeletonModels[1];
    if (mSkeletonModels[2] != mActiveSkeletonModels[2] && mSkeletonModels[2] != NULL)
        delete mSkeletonModels[2];
    if (mSkeletonModels[3] != mActiveSkeletonModels[3] && mSkeletonModels[3] != NULL)
        delete mSkeletonModels[3];
    delete mActiveSkeletonModels[0];
    delete mActiveSkeletonModels[1];
    delete mActiveSkeletonModels[2];
    delete mActiveSkeletonModels[3];
    delete mActiveCreeperModel;
    delete mActiveDragonModel;
}

void SkullBlockRenderer::renderSkull(float x, float y, float z, const Direction* direction, float rotation,
                                     float headPitch, int skullType, int destroyStage, float animationTime,
                                     bool useDisplayList)
{
    Model* model = NULL;
    switch (skullType)
    {
    case 5:
        model = mActiveDragonModel;
        break;
    case 4:
        model = mActiveCreeperModel;
        break;
    default:
        model = mActiveSkeletonModels[skullType];
        break;
    }

    if (destroyStage >= 0)
    {
        bindTexture(&BREAKING_LOCATIONS[destroyStage]);
        GlStateManager::matrixMode(2);
        GlStateManager::pushMatrix();
        GlStateManager::scalef(4.0F, 2.0F, 1.0F);
        GlStateManager::translatef(0.0625F, 0.0625F, 0.0625F);
        GlStateManager::matrixMode(0);
        GlStateManager::pushMatrix();
        GlStateManager::disableCull();
    }
    else
    {
        switch (skullType)
        {
        case 1:
            bindTexture(&SKULL_TEXTURE_1);
            break;
        case 2:
            bindTexture(&SKULL_TEXTURE_2);
            break;
        case 3:
            bindTexture(&SKULL_TEXTURE_3);
            break;
        case 4:
            bindTexture(&SKULL_TEXTURE_4);
            break;
        case 5:
            bindTexture(&SKULL_TEXTURE_5);
            break;
        default:
            bindTexture(&SKULL_TEXTURE_0);
            break;
        }
        GlStateManager::pushMatrix();
        GlStateManager::disableCull();
    }

    if (direction == Direction::UP)
    {
        GlStateManager::translatef(x + 0.5F, y, z + 0.5F);
        GlStateManager::enableRescaleNormal();
        GlStateManager::scalef(-1.0F, -1.0F, 1.0F);
        GlStateManager::enableAlphaTest();
    }
    else
    {
        const float offset = 0.25F;
        switch (direction->get3DDataValue())
        {
        case 2:
            GlStateManager::translatef(x + 0.5F, y + offset, z + 0.74F);
            break;
        case 3:
            GlStateManager::translatef(x + 0.5F, y + offset, z + 0.26F);
            rotation = 180.0F;
            break;
        case 4:
            GlStateManager::translatef(x + 0.74F, y + offset, z + 0.5F);
            rotation = 270.0F;
            break;
        default:
            GlStateManager::translatef(x + 0.26F, y + offset, z + 0.5F);
            rotation = 90.0F;
            break;
        }
        GlStateManager::enableRescaleNormal();
        GlStateManager::scalef(-1.0F, -1.0F, 1.0F);
        GlStateManager::enableAlphaTest();
    }

    boost::shared_ptr<Entity> entity;
    model->render(entity, animationTime, 0.0F, 0.0F, rotation, headPitch, 0.0625F, useDisplayList);
    GlStateManager::popMatrix();
    if (destroyStage >= 0)
    {
        GlStateManager::matrixMode(2);
        GlStateManager::popMatrix();
        GlStateManager::matrixMode(0);
    }
}

void SkullBlockRenderer::render(boost::shared_ptr<BlockEntity> blockEntity, double x, double y, double z,
                                float partialTicks, int breakProgress, bool bTranslucent, float alpha,
                                bool bUseDisplayList)
{
    boost::shared_ptr<SkullBlockEntity> skull = boost::static_pointer_cast<SkullBlockEntity>(blockEntity);
    const Direction* direction = Direction::from3DDataValue(skull->getData() & 7);
    if (skull->isRenderReady())
    {
        float animationTime = skull->getMouthAnimation(partialTicks);
        renderSkull((float)x, (float)y, (float)z, direction, (float)(skull->getRotation() * 360) * 0.0625F, 0.0F,
                    skull->getSkullType(), breakProgress, animationTime, bUseDisplayList);
    }
}

void SkullBlockRenderer::init(BlockEntityRenderDispatcher* pDispatcher)
{
    BlockEntityRenderer::init(pDispatcher);
    instance = this;
}

bool SkullBlockRenderer::dragonJawCheck()
{
    return mActiveDragonModel->hasJawPart();
}

void SkullBlockRenderer::updateModel(int modelIndex, const std::wstring& modelName,
                                     const std::wstring& originalModelName, TexturePack* pTexturePack)
{
    if (mSkeletonModels[modelIndex] == NULL)
        mSkeletonModels[modelIndex] = mActiveSkeletonModels[modelIndex];
    if (mActiveSkeletonModels[modelIndex] != mSkeletonModels[modelIndex])
    {
        if (mActiveSkeletonModels[modelIndex] != NULL)
            delete mActiveSkeletonModels[modelIndex];
        mActiveSkeletonModels[modelIndex] = NULL;
    }
    if (pTexturePack->HasCustomModel(modelName))
        mActiveSkeletonModels[modelIndex] = new SkeletonHeadModel(0, 0, 64, 32,
                                                                  *pTexturePack->GetCustomModel(modelName));
    else if (pTexturePack->HasCustomModel(originalModelName))
        mActiveSkeletonModels[modelIndex] = new SkeletonHeadModel(0, 0, 64, 32,
                                                                  *pTexturePack->GetCustomModel(originalModelName));
    else
        mActiveSkeletonModels[modelIndex] = mSkeletonModels[modelIndex];
}

void SkullBlockRenderer::setupForTexturePack(TexturePack* pTexturePack)
{
    updateModel(0, L"skeleton_head", L"skeleton", pTexturePack);
    updateModel(1, L"skeleton_wither_head", L"skeleton.wither", pTexturePack);
    updateModel(2, L"zombie_head", L"zombie", pTexturePack);

    if (mDragonModel == NULL)
        mDragonModel = mActiveDragonModel;
    if (mActiveDragonModel != mDragonModel)
    {
        if (mActiveDragonModel != NULL)
            delete mActiveDragonModel;
        mActiveDragonModel = NULL;
    }
    if (pTexturePack->HasCustomModel(L"dragon_head"))
        mActiveDragonModel = new DragonHeadModel(0.0F, *pTexturePack->GetCustomModel(L"dragon_head"));
    else
        mActiveDragonModel = mDragonModel;

    if (mCreeperModel == NULL)
        mCreeperModel = mActiveCreeperModel;
    if (mActiveCreeperModel != mCreeperModel)
    {
        if (mActiveCreeperModel != NULL)
            delete mActiveCreeperModel;
        mActiveCreeperModel = NULL;
    }
    if (pTexturePack->HasCustomModel(L"creeper_head"))
        mActiveCreeperModel = new CreeperHeadModel(0.0F, *pTexturePack->GetCustomModel(L"creeper_head"));
    else
        mActiveCreeperModel = mCreeperModel;
}
