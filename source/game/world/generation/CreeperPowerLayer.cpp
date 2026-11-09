#include "game/world/generation/CreeperPowerLayer.hpp"

#include <string>

#include <boost/shared_ptr.hpp>

#include "game/client/render/CreeperModel.hpp"
#include "game/client/render/CreeperRenderer.hpp"
#include "game/client/render/Model.hpp"
#include "game/client/render/ModelDefinition.hpp"
#include "game/core/Creeper.hpp"
#include "game/core/GlStateManager.hpp"
#include "game/entity/mob/Entity.hpp"
#include "game/entity/mob/LivingEntity.hpp"
#include "game/client/render/TexturePack.hpp"
#include "game/util/ResourceLocation.hpp"

static ResourceLocation CREEPER_ARMOR_LOCATION(TN_ARMOR_POWER);

CreeperPowerLayer::CreeperPowerLayer(CreeperRenderer* renderer) : mRenderer(renderer), mOriginalModel(NULL)
{
    mModel = new CreeperModel(2.0f);
}

CreeperPowerLayer::~CreeperPowerLayer()
{
}

void CreeperPowerLayer::render(boost::shared_ptr<LivingEntity>& entity, float limbSwing, float limbSwingAmount,
                               float partialTicks, float ageInTicks, float netHeadYaw, float headPitch, float scale,
                               bool bUseDisplayList)
{
    boost::shared_ptr<Creeper> creeper = boost::dynamic_pointer_cast<Creeper>(entity);
    boost::shared_ptr<Entity> renderEntity(creeper);

    if (!creeper->isPowered())
    {
        return;
    }

    bool invisible = creeper->isInvisible();
    GlStateManager::depthMask(!invisible);
    mRenderer->bindTexture(&CREEPER_ARMOR_LOCATION);
    GlStateManager::matrixMode(2);
    GlStateManager::loadIdentity();

    float textureOffset = ((float)creeper->mTickCount + partialTicks) * 0.01f;
    GlStateManager::translatef(textureOffset, textureOffset, 0.0f);

    GlStateManager::matrixMode(0);
    GlStateManager::enableBlend();
    GlStateManager::color4f(0.5f, 0.5f, 0.5f, 1.0f);
    GlStateManager::disableLighting();
    GlStateManager::blendFunc(1, 1);

    mModel->copyPropertiesFrom(mRenderer->getModel());
    mModel->render(renderEntity, limbSwing, limbSwingAmount, ageInTicks, netHeadYaw, headPitch, scale, bUseDisplayList);

    GlStateManager::matrixMode(2);
    GlStateManager::loadIdentity();
    GlStateManager::matrixMode(0);
    GlStateManager::enableLighting();
    GlStateManager::disableBlend();
    GlStateManager::depthMask(invisible);
}

bool CreeperPowerLayer::colorsOnDamage()
{
    return false;
}

void CreeperPowerLayer::setupForTexturePack(TexturePack* pTexturePack)
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

    if (pTexturePack->HasCustomModel(std::wstring(L"creeper")))
    {
        mModel = new CreeperModel(2.0f, *pTexturePack->GetCustomModel(std::wstring(L"creeper")));
    }
    else
    {
        mModel = mOriginalModel;
    }
}
