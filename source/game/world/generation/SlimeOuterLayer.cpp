#include "game/world/generation/SlimeOuterLayer.hpp"

#include <boost/shared_ptr.hpp>

#include "game/core/GlStateManager.hpp"
#include "game/client/render/Model.hpp"
#include "game/client/render/ModelDefinition.hpp"
#include "game/client/render/SlimeModel.hpp"
#include "game/client/render/SlimeRenderer.hpp"
#include "game/client/render/TexturePack.hpp"
#include "game/entity/mob/Entity.hpp"
#include "game/entity/mob/LivingEntity.hpp"

SlimeOuterLayer::SlimeOuterLayer(SlimeRenderer* renderer) : mRenderer(renderer)
{
    SlimeModel* model = new SlimeModel(0);
    mModel = model;
    mOriginalModel = NULL;
}

SlimeOuterLayer::~SlimeOuterLayer()
{
}

void SlimeOuterLayer::render(boost::shared_ptr<LivingEntity>& entity, float limbSwing, float limbSwingAmount,
                             float partialTicks, float ageInTicks, float netHeadYaw, float headPitch, float scale,
                             bool bUseDisplayList)
{
    if (entity->isInvisible())
    {
        return;
    }

    bool cull = mModel->hasOverriddenParts();
    if (cull)
    {
        GlStateManager::enableCull();
        GlStateManager::cullFace(1);
    }

    GlStateManager::color4f(1.0f, 1.0f, 1.0f, 1.0f);
    GlStateManager::enableNormalize();
    GlStateManager::enableBlend();
    GlStateManager::blendFunc(4, 5);

    mModel->copyPropertiesFrom(mRenderer->getModel());
    boost::shared_ptr<Entity> renderEntity = entity;
    mModel->render(renderEntity, limbSwing, limbSwingAmount, ageInTicks, netHeadYaw, headPitch, scale, bUseDisplayList);

    GlStateManager::disableBlend();
    GlStateManager::disableNormalize();
    if (cull)
    {
        GlStateManager::disableCull();
    }
}

bool SlimeOuterLayer::colorsOnDamage()
{
    return true;
}

void SlimeOuterLayer::setupForTexturePack(TexturePack* pTexturePack)
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

    if (pTexturePack->HasCustomModel(L"slime.armor"))
    {
        mModel = new SlimeModel(0, *pTexturePack->GetCustomModel(L"slime.armor"));
    }
    else
    {
        mModel = mOriginalModel;
    }
}
