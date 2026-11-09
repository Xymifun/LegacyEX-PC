#include "game/world/generation/StrayClothingLayer.hpp"

#include <string>

#include <boost/shared_ptr.hpp>

#include "game/core/GlStateManager.hpp"
#include "game/entity/mob/Entity.hpp"
#include "game/entity/mob/LivingEntity.hpp"
#include "game/client/render/LivingEntityRenderer.hpp"
#include "game/client/render/Model.hpp"
#include "game/client/render/ModelDefinition.hpp"
#include "game/client/render/SkeletonModel.hpp"
#include "game/client/render/TexturePack.hpp"
#include "game/client/render/ZombieModel.hpp"
#include "game/util/ResourceLocation.hpp"

static ResourceLocation STRAY_CLOTHING_LOCATION(TN_MOB_SKELETON_STRAY_OVERLAY);

StrayClothingLayer::StrayClothingLayer(LivingEntityRenderer* renderer)
    : mModel(new SkeletonModel(0.25f, true)),
      mOriginalModel(NULL)
{
    mRenderer = renderer;
}

StrayClothingLayer::~StrayClothingLayer()
{
}

void StrayClothingLayer::render(boost::shared_ptr<LivingEntity>& entity, float limbSwing, float limbSwingAmount,
                                float partialTicks, float ageInTicks, float netHeadYaw, float headPitch, float scale,
                                bool bUseDisplayList)
{
    mModel->copyPropertiesFrom(mRenderer->getModel());
    mModel->prepareMobModel(entity, limbSwing, limbSwingAmount, partialTicks);
    GlStateManager::color4f(1.0f, 1.0f, 1.0f, 1.0f);
    mRenderer->bindTexture(&STRAY_CLOTHING_LOCATION);

    boost::shared_ptr<Entity> renderEntity = entity;
    mModel->render(renderEntity, limbSwing, limbSwingAmount, ageInTicks, netHeadYaw, headPitch, scale, bUseDisplayList);
}

bool StrayClothingLayer::colorsOnDamage()
{
    return true;
}

void StrayClothingLayer::setupForTexturePack(TexturePack* pTexturePack)
{
    if (mOriginalModel == NULL)
    {
        mOriginalModel = mModel;
    }

    if (mModel != mOriginalModel)
    {
        if (mModel != NULL)
        {
            delete mModel;
        }
        mModel = NULL;
    }

    if (pTexturePack->HasCustomModel(L"stray.armor"))
    {
        mModel = new ZombieModel(*pTexturePack->GetCustomModel(L"stray.armor"));
    }
    else
    {
        mModel = mOriginalModel;
    }
}
