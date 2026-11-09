#include "game/client/render/ElderGuardianRenderer.hpp"

#include "game/core/ElderGuardian.hpp"
#include "game/core/GlStateManager.hpp"
#include "game/util/ResourceLocation.hpp"

static ResourceLocation ELDER_GUARDIAN_LOCATION(TN_MOB_GUARDIAN_ELDER);

ElderGuardianRenderer::ElderGuardianRenderer(EntityRenderDispatcher* dispatcher) : GuardianRenderer(dispatcher)
{
}

void ElderGuardianRenderer::scale(boost::shared_ptr<LivingEntity>&, float)
{
    GlStateManager::scalef(ElderGuardian::ELDER_SIZE_SCALE, ElderGuardian::ELDER_SIZE_SCALE,
                           ElderGuardian::ELDER_SIZE_SCALE);
}

const ResourceLocation* ElderGuardianRenderer::getTextureLocation(boost::shared_ptr<Entity>&)
{
    return &ELDER_GUARDIAN_LOCATION;
}
