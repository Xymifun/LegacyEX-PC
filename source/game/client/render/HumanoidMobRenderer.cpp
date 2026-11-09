#include "game/client/render/HumanoidMobRenderer.hpp"

#include "game/client/render/HumanoidModel.hpp"
#include "game/core/GlStateManager.hpp"
#include "game/util/ResourceLocation.hpp"
#include "game/world/generation/CustomHeadLayer.hpp"
#include "game/world/generation/ItemInHandLayer.hpp"

static ResourceLocation HUMANOID_LOCATION(TN_MOB_STEVE);

HumanoidMobRenderer::HumanoidMobRenderer(EntityRenderDispatcher* dispatcher, HumanoidModel* model, float shadowRadius)
    : MobRenderer(dispatcher, model, shadowRadius)
{
    addLayer(new ItemInHandLayer(this));
    addLayer(new CustomHeadLayer(model->mHead));
    addLayer(new ItemInHandLayer(this));
}

const ResourceLocation* HumanoidMobRenderer::getTextureLocation(boost::shared_ptr<Entity>&)
{
    return &HUMANOID_LOCATION;
}

void HumanoidMobRenderer::translateWeaponItem()
{
    GlStateManager::translatef(0.0f, 0.1875f, 0.0f);
}
