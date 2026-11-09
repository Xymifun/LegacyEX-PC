#include "game/world/generation/SpinAttackLayer.hpp"

#include <boost/shared_ptr.hpp>

#include "game/client/render/LivingEntityRenderer.hpp"
#include "game/client/render/ModelDefinition.hpp"
#include "game/client/render/SpinAttackModel.hpp"
#include "game/entity/mob/Entity.hpp"
#include "game/entity/mob/LivingEntity.hpp"
#include "game/util/ResourceLocation.hpp"

static ResourceLocation SPIN_ATTACK_LOCATION(TN_ITEM_TRIDENT_RIPTIDE);

SpinAttackLayer::SpinAttackLayer(LivingEntityRenderer* renderer) : mRenderer(renderer)
{
    mModel = new SpinAttackModel();
}

SpinAttackLayer::~SpinAttackLayer()
{
    delete mModel;
}

void SpinAttackLayer::render(boost::shared_ptr<LivingEntity>& entity, float limbSwing, float limbSwingAmount,
                             float partialTicks, float ageInTicks, float netHeadYaw, float headPitch, float scale,
                             bool bUseDisplayList)
{
    if (!entity->getDamageNearbyMobs())
    {
        return;
    }

    mRenderer->bindTexture(&SPIN_ATTACK_LOCATION);
    mModel->renderSpin(entity, ageInTicks, scale, bUseDisplayList);
}

bool SpinAttackLayer::colorsOnDamage()
{
    return false;
}
