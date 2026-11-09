#include "game/client/render/WolfRenderer.hpp"

#include <algorithm>

#include <boost/shared_ptr.hpp>

#include "game/client/render/Model.hpp"
#include "game/client/render/ModelDefinition.hpp"
#include "game/client/render/RenderLayer.hpp"
#include "game/client/render/WolfModel.hpp"
#include "game/core/DyeColor.hpp"
#include "game/core/GlStateManager.hpp"
#include "game/core/Wolf.hpp"
#include "game/util/ResourceLocation.hpp"
#include "game/world/biome/arrayWithLength.hpp"
#include "game/world/generation/WolfCollarLayer.hpp"

static ResourceLocation WOLF_LOCATION(TN_MOB_WOLF);
static ResourceLocation WOLF_ANGRY_LOCATION(TN_MOB_WOLF_ANGRY);
static ResourceLocation WOLF_TAME_LOCATION(TN_MOB_WOLF_TAME);

WolfRenderer::WolfRenderer(EntityRenderDispatcher* dispatcher, Model* model, float shadowRadius)
    : MobRenderer(dispatcher, model, shadowRadius),
      mCollarLayer(new WolfCollarLayer(this))
{
    addLayer(mCollarLayer);
}

float WolfRenderer::getBob(boost::shared_ptr<LivingEntity>& entity, float partialTicks)
{
    boost::shared_ptr<Wolf> wolf = boost::static_pointer_cast<Wolf>(entity);
    return wolf->getTailAngle();
}

void WolfRenderer::render(boost::shared_ptr<Entity>& entity, double x, double y, double z, float yaw, float partialTick)
{
    boost::shared_ptr<Wolf> wolf = boost::static_pointer_cast<Wolf>(entity);
    if (mModel->hasOverriddenParts() && wolf->isTame())
    {
        arrayWithLength<const float> colours = wolf->getCollarColor()->getTextureDiffuseColors();
        GlStateManager::color3f(colours.data[0], colours.data[1], colours.data[2]);
    }
    MobRenderer::render(entity, x, y, z, yaw, partialTick);
}

const ResourceLocation* WolfRenderer::getTextureLocation(boost::shared_ptr<Entity>& entity)
{
    boost::shared_ptr<Wolf> wolf = boost::static_pointer_cast<Wolf>(entity);
    if (wolf->isTame())
        return &WOLF_TAME_LOCATION;
    if (wolf->isAngry())
        return &WOLF_ANGRY_LOCATION;
    return &WOLF_LOCATION;
}

void WolfRenderer::setupForTexturePack(TexturePack* pack)
{
    LivingEntityRenderer::setupForTexturePack(pack);
    if (mModel->hasOverriddenParts())
    {
        removeLayer(mCollarLayer);
        return;
    }
    if (std::find(mLayers.begin(), mLayers.end(), mCollarLayer) == mLayers.end())
        addLayer(mCollarLayer);
}

Model* WolfRenderer::createModel(const ModelDefinition& definition)
{
    return new WolfModel(definition);
}
