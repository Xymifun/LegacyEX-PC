#include "game/client/render/WitherSkeletonRenderer.hpp"

#include "game/client/render/ModelDefinition.hpp"
#include "game/client/render/SkeletonModel.hpp"
#include "game/core/GlStateManager.hpp"
#include "game/util/ResourceLocation.hpp"

static ResourceLocation WITHER_SKELETON_LOCATION(TN_MOB_SKELETON_WITHER);

WitherSkeletonRenderer::WitherSkeletonRenderer(EntityRenderDispatcher* dispatcher)
    : SkeletonRenderer(dispatcher, new SkeletonModel(SkeletonModel::eSkeletonType_WitherSkeleton))
{
}

const ResourceLocation* WitherSkeletonRenderer::getTextureLocation(boost::shared_ptr<Entity>&)
{
    return &WITHER_SKELETON_LOCATION;
}

std::wstring WitherSkeletonRenderer::getModelName()
{
    return L"skeleton.wither";
}

void WitherSkeletonRenderer::scale(boost::shared_ptr<LivingEntity>&, float)
{
    GlStateManager::scalef(1.2F, 1.2F, 1.2F);
}

Model* WitherSkeletonRenderer::createModel(const ModelDefinition& definition)
{
    return new SkeletonModel(SkeletonModel::eSkeletonType_WitherSkeleton, definition);
}
