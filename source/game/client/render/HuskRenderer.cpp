#include "game/client/render/HuskRenderer.hpp"

#include "game/core/GlStateManager.hpp"
#include "game/client/render/LivingEntityRenderer.hpp"
#include "game/util/ResourceLocation.hpp"

static ResourceLocation HUSK_LOCATION(TN_MOB_ZOMBIE_HUSK);

HuskRenderer::HuskRenderer(EntityRenderDispatcher* dispatcher) : ZombieRenderer(dispatcher)
{
}

std::wstring HuskRenderer::getModelName()
{
    return L"zombie.husk";
}

void HuskRenderer::scale(boost::shared_ptr<LivingEntity>& entity, float partialTick)
{
    GlStateManager::scalef(1.0625F, 1.0625F, 1.0625F);
    this->LivingEntityRenderer::scale(entity, partialTick);
}

const ResourceLocation* HuskRenderer::getTextureLocation(boost::shared_ptr<Entity>&)
{
    return &HUSK_LOCATION;
}
