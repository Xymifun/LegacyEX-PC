#include "game/world/generation/WolfCollarLayer.hpp"

#include <boost/shared_ptr.hpp>

#include "game/client/render/Model.hpp"
#include "game/client/render/WolfRenderer.hpp"
#include "game/core/DyeColor.hpp"
#include "game/core/GlStateManager.hpp"
#include "game/core/Wolf.hpp"
#include "game/entity/mob/Entity.hpp"
#include "game/entity/mob/LivingEntity.hpp"
#include "game/util/ResourceLocation.hpp"
#include "game/world/biome/arrayWithLength.hpp"

static ResourceLocation WOLF_COLLAR_LOCATION(TN_MOB_WOLF_COLLAR);

WolfCollarLayer::WolfCollarLayer(WolfRenderer* renderer) : mRenderer(renderer)
{
}

WolfCollarLayer::~WolfCollarLayer()
{
}

void WolfCollarLayer::render(boost::shared_ptr<LivingEntity>& entity, float limbSwing, float limbSwingAmount,
                             float partialTicks, float ageInTicks, float netHeadYaw, float headPitch, float scale,
                             bool bUseDisplayList)
{
    boost::shared_ptr<Wolf> wolf = boost::dynamic_pointer_cast<Wolf>(entity);

    if (!wolf->isTame() || wolf->isInvisible())
        return;

    mRenderer->bindTexture(&WOLF_COLLAR_LOCATION);

    arrayWithLength<const float> colours = wolf->getCollarColor()->getTextureDiffuseColors();
    GlStateManager::color3f(colours.data[0], colours.data[1], colours.data[2]);

    boost::shared_ptr<Entity> renderEntity = wolf;
    mRenderer->getModel()->render(renderEntity, limbSwing, limbSwingAmount, ageInTicks, netHeadYaw, headPitch, scale,
                                  bUseDisplayList);
}

bool WolfCollarLayer::colorsOnDamage()
{
    return true;
}
