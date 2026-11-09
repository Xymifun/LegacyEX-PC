#include "game/client/render/SpiderRenderer.hpp"

#include <algorithm>

#include "game/client/render/Model.hpp"
#include "game/client/render/ModelDefinition.hpp"
#include "game/client/render/SpiderModel.hpp"
#include "game/core/GlStateManager.hpp"
#include "game/core/Minecraft.hpp"
#include "game/entity/mob/Entity.hpp"
#include "game/entity/mob/LivingEntity.hpp"
#include "game/util/ResourceLocation.hpp"
#include "game/world/generation/MultiplayerLocalPlayer.hpp"
#include "game/world/generation/SpiderEyesLayer.hpp"

static ResourceLocation SPIDER_LOCATION(TN_MOB_SPIDER);

SpiderRenderer::SpiderRenderer(EntityRenderDispatcher* dispatcher) : MobRenderer(dispatcher, new SpiderModel(), 1.0F)
{
    mSpiderEyesLayer = new SpiderEyesLayer(this);
    addLayer(mSpiderEyesLayer);
}

float SpiderRenderer::getFlipDegrees(boost::shared_ptr<LivingEntity>&)
{
    return 180.0F;
}

const ResourceLocation* SpiderRenderer::getTextureLocation(boost::shared_ptr<Entity>&)
{
    return &SPIDER_LOCATION;
}

void SpiderRenderer::setupForTexturePack(TexturePack* pack)
{
    LivingEntityRenderer::setupForTexturePack(pack);
    if (mModel->hasOverriddenParts())
    {
        removeLayer(mSpiderEyesLayer);
        return;
    }

    if (std::find(mLayers.begin(), mLayers.end(), mSpiderEyesLayer) == mLayers.end())
        addLayer(mSpiderEyesLayer);
}

void SpiderRenderer::renderModel(boost::shared_ptr<LivingEntity>& entity, float limbSwing, float limbSwingAmount,
                                 float ageInTicks, float netHeadYaw, float headPitch, float scale)
{
    bool solid = !isVisible(entity) && !entity->isInvisibleTo(Minecraft::GetInstance()->mPlayer);

    boost::shared_ptr<Entity> baseEntity = entity;
    if (!bindTexture(baseEntity))
        return;

    if (solid)
        GlStateManager::setProfile();

    Model* model = getModel(entity);
    model->render(baseEntity, limbSwing, limbSwingAmount, ageInTicks, netHeadYaw, headPitch, scale, true);

    if (solid)
        GlStateManager::unsetProfile();
}

Model* SpiderRenderer::createModel(const ModelDefinition& definition)
{
    return new SpiderModel(definition);
}
