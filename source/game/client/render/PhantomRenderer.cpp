#include "game/client/render/PhantomRenderer.hpp"

#include "game/client/render/ModelDefinition.hpp"
#include "game/client/render/PhantomModel.hpp"
#include "game/core/GlStateManager.hpp"
#include "game/core/Minecraft.hpp"
#include "game/entity/mob/LivingEntity.hpp"
#include "game/util/ResourceLocation.hpp"
#include "game/world/generation/MultiplayerLocalPlayer.hpp"

static ResourceLocation PHANTOM_LOCATION(TN_MOB_PHANTOM);

PhantomRenderer::PhantomRenderer(EntityRenderDispatcher* dispatcher) : MobRenderer(dispatcher, new PhantomModel(), 0.5F)
{
}

const ResourceLocation* PhantomRenderer::getTextureLocation(boost::shared_ptr<Entity>& entity)
{
    (void)entity;
    return &PHANTOM_LOCATION;
}

void PhantomRenderer::scale(boost::shared_ptr<LivingEntity>& entity, float partialTick)
{
    LivingEntityRenderer::scale(entity, partialTick);
    GlStateManager::scalef(1.0F, 1.0F, 1.0F);
    GlStateManager::translatef(0.0F, 1.3125F, 0.0F);
}

void PhantomRenderer::setupRotations(boost::shared_ptr<LivingEntity>& entity, float ageInTicks, float rotationYaw,
                                     float partialTicks)
{
    LivingEntityRenderer::setupRotations(entity, ageInTicks, rotationYaw, partialTicks);
    GlStateManager::rotatef(entity->mXRot, 1.0F, 0.0F, 0.0F);
}

Model* PhantomRenderer::createModel(const ModelDefinition& definition)
{
    return new PhantomModel(definition);
}

void PhantomRenderer::renderModel(boost::shared_ptr<LivingEntity>& entity, float limbSwing, float limbSwingAmount,
                                  float ageInTicks, float netHeadYaw, float headPitch, float scale)
{
    bool translucent = !isVisible(entity) && !entity->isInvisibleTo(Minecraft::GetInstance()->mPlayer);

    boost::shared_ptr<Entity> renderEntity = entity;
    if (!bindTexture(renderEntity))
        return;

    if (translucent)
        GlStateManager::setProfile();

    Model* model = getModel(entity);
    model->render(renderEntity, limbSwing, limbSwingAmount, ageInTicks, netHeadYaw, headPitch, scale, true);

    if (translucent)
        GlStateManager::unsetProfile();
}
