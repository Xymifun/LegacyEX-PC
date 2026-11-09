#include "game/client/render/SkeletonRenderer.hpp"

#include "game/client/render/ModelDefinition.hpp"
#include "game/client/render/SkeletonModel.hpp"
#include "game/core/GlStateManager.hpp"
#include "game/util/ResourceLocation.hpp"
#include "game/world/generation/ItemInHandLayer.hpp"
#include "game/world/generation/SkeletonArmorLayer.hpp"

static ResourceLocation SKELETON_LOCATION(TN_MOB_SKELETON);

SkeletonRenderer::SkeletonRenderer(EntityRenderDispatcher* dispatcher)
    : HumanoidMobRenderer(dispatcher, new SkeletonModel(SkeletonModel::eSkeletonType_Skeleton), 0.5f)
{
    addLayer(new ItemInHandLayer(this));
    addLayer(new SkeletonArmorLayer(this));
}

SkeletonRenderer::SkeletonRenderer(EntityRenderDispatcher* dispatcher, SkeletonModel* model)
    : HumanoidMobRenderer(dispatcher, model, 0.5f)
{
    addLayer(new ItemInHandLayer(this));
    addLayer(new SkeletonArmorLayer(this));
}

void SkeletonRenderer::translateWeaponItem()
{
    GlStateManager::translatef(0.09375f, 0.1875f, 0.0f);
}

const ResourceLocation* SkeletonRenderer::getTextureLocation(boost::shared_ptr<Entity>&)
{
    return &SKELETON_LOCATION;
}

Model* SkeletonRenderer::createModel(const ModelDefinition& definition)
{
    return new SkeletonModel(SkeletonModel::eSkeletonType_Skeleton, definition);
}
