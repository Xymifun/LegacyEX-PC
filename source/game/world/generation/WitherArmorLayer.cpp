#include "game/world/generation/WitherArmorLayer.hpp"

#include <string>

#include <boost/shared_ptr.hpp>

#include "game/client/render/ModelDefinition.hpp"
#include "game/client/render/Model.hpp"
#include "game/client/render/TexturePack.hpp"
#include "game/client/render/WitherBossModel.hpp"
#include "game/client/render/WitherBossRenderer.hpp"
#include "game/core/GlStateManager.hpp"
#include "game/core/Mth.hpp"
#include "game/core/WitherBoss.hpp"
#include "game/entity/mob/Entity.hpp"
#include "game/util/ResourceLocation.hpp"

static ResourceLocation WITHER_ARMOR_LOCATION(TN_MOB_WITHER_WITHER_ARMOR);

WitherArmorLayer::WitherArmorLayer(WitherBossRenderer* renderer) : mRenderer(renderer)
{
    mModel = new WitherBossModel(0.5f);
    mOriginalModel = mModel;
}

WitherArmorLayer::~WitherArmorLayer()
{
}

void WitherArmorLayer::render(boost::shared_ptr<LivingEntity>& entity, float limbSwing, float limbSwingAmount,
                              float partialTicks, float ageInTicks, float netHeadYaw, float headPitch, float scale,
                              bool bUseDisplayList)
{
    boost::shared_ptr<WitherBoss> wither = boost::static_pointer_cast<WitherBoss>(entity);
    boost::shared_ptr<Entity> renderEntity = wither;

    if (!wither->isPowered() || wither->IsSpawning())
        return;

    GlStateManager::depthMask(!wither->isInvisible());
    mRenderer->bindTexture(&WITHER_ARMOR_LOCATION);
    GlStateManager::matrixMode(2);
    GlStateManager::loadIdentity();

    float textureOffset = (float)wither->mTickCount + partialTicks;
    float xOffset = Mth::cos(textureOffset * 0.02f) * 3.0f;
    float yOffset = textureOffset * 0.01f;
    float zOffset = 0.0f;
    GlStateManager::translatef(xOffset, yOffset, zOffset);

    GlStateManager::matrixMode(0);
    GlStateManager::enableBlend();
    GlStateManager::color4f(0.5f, 0.5f, 0.5f, 1.0f);
    GlStateManager::disableLighting();
    GlStateManager::blendFunc(1, 1);

    mModel->prepareMobModel(entity, limbSwing, limbSwingAmount, partialTicks);
    mModel->copyPropertiesFrom(mRenderer->getModel());
    mModel->render(renderEntity, limbSwing, limbSwingAmount, ageInTicks, netHeadYaw, headPitch, scale, bUseDisplayList);

    GlStateManager::matrixMode(2);
    GlStateManager::loadIdentity();
    GlStateManager::matrixMode(0);
    GlStateManager::enableLighting();
    GlStateManager::disableBlend();
}

bool WitherArmorLayer::colorsOnDamage()
{
    return false;
}

void WitherArmorLayer::setupForTexturePack(TexturePack* pTexturePack)
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

    if (pTexturePack->HasCustomModel(L"witherBoss"))
    {
        mModel = new WitherBossModel(0.5f, *pTexturePack->GetCustomModel(L"witherBoss"));
    }
    else
    {
        mModel = mOriginalModel;
    }
}
