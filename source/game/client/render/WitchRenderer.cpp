#include "game/client/render/WitchRenderer.hpp"

#include "game/client/render/ModelDefinition.hpp"
#include "game/client/render/WitchModel.hpp"
#include "game/core/GlStateManager.hpp"
#include "game/entity/mob/Entity.hpp"
#include "game/entity/mob/Mob.hpp"
#include "game/item/ItemInstance.hpp"
#include "game/item/not_null_ptr.hpp"
#include "game/util/ResourceLocation.hpp"
#include "game/world/generation/WitchItemLayer.hpp"

static ResourceLocation WITCH_LOCATION(TN_MOB_WITCH);

WitchRenderer::WitchRenderer(EntityRenderDispatcher* dispatcher) : MobRenderer(dispatcher, new WitchModel(0.0F), 0.5F)
{
    addLayer(new WitchItemLayer(this));
}

void WitchRenderer::render(boost::shared_ptr<Entity>& entity, double x, double y, double z, float yaw,
                           float partialTick)
{
    boost::shared_ptr<Mob> mob = boost::static_pointer_cast<Mob>(entity);
    {
        not_null_ptr<ItemInstance> itemInstance = mob->getMainHandItem();
        static_cast<WitchModel*>(mModel)->mHoldingItem = !itemInstance->isEmpty();
    }
    MobRenderer::render(entity, x, y, z, yaw, partialTick);
}

const ResourceLocation* WitchRenderer::getTextureLocation(boost::shared_ptr<Entity>& entity)
{
    return &WITCH_LOCATION;
}

void WitchRenderer::translateWeaponItem()
{
    GlStateManager::translatef(0.0F, 0.1875F, 0.0F);
}

void WitchRenderer::scale(boost::shared_ptr<LivingEntity>& entity, float partialTicks)
{
    GlStateManager::scalef(0.9375F, 0.9375F, 0.9375F);
}

Model* WitchRenderer::createModel(const ModelDefinition& definition)
{
    return new WitchModel(0.0F, definition);
}
