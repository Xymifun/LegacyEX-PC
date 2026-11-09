#include "game/client/render/ParrotRenderer.hpp"
#include "game/core/EntityType.hpp"

#include "game/client/render/ModelDefinition.hpp"
#include "game/client/render/ParrotModel.hpp"
#include "game/core/Minecraft.hpp"
#include "game/core/Mth.hpp"
#include "game/core/Parrot.hpp"
#include "game/entity/mob/Entity.hpp"
#include "game/entity/mob/LivingEntity.hpp"
#include "game/util/ResourceLocation.hpp"
#include "game/world/generation/MultiplayerLocalPlayer.hpp"
#include "game/world/generation/Player.hpp"

ResourceLocation PARROT_LOCATIONS[5] = {
    ResourceLocation(TN_MOB_PARROT_PARROT_RED_BLUE), ResourceLocation(TN_MOB_PARROT_PARROT_BLUE),
    ResourceLocation(TN_MOB_PARROT_PARROT_GREEN), ResourceLocation(TN_MOB_PARROT_PARROT_YELLOW_BLUE),
    ResourceLocation(TN_MOB_PARROT_PARROT_GREY)};

ParrotRenderer::ParrotRenderer(EntityRenderDispatcher* dispatcher) : MobRenderer(dispatcher, new ParrotModel(), 0.3F)
{
}

const ResourceLocation* ParrotRenderer::getSpectateTextureLocation(boost::shared_ptr<MultiplayerLocalPlayer> player)
{
    return &PARROT_LOCATIONS[player->getId() % 5];
}

const ResourceLocation* ParrotRenderer::getTextureLocation(boost::shared_ptr<Entity>& entity)
{
    if (Minecraft::InMiniGame(eMiniGameId_Any, true) && eTYPE_DERIVED_FROM(EntityType::PLAYER, entity->GetType()))
    {
        return getSpectateTextureLocation(boost::static_pointer_cast<MultiplayerLocalPlayer>(entity));
    }

    boost::shared_ptr<Parrot> parrot = boost::static_pointer_cast<Parrot>(entity);
    int variant = parrot->getVariant();
    return &PARROT_LOCATIONS[variant];
}

float ParrotRenderer::getCustomBob(boost::shared_ptr<Parrot> parrot, float partialTicks)
{
    float flap = parrot->mOFlap + (parrot->mFlap - parrot->mOFlap) * partialTicks;
    float flapSpeed = parrot->mOFlapSpeed + (parrot->mFlapSpeed - parrot->mOFlapSpeed) * partialTicks;
    return (Mth::sin(flap) + 1.0F) * flapSpeed;
}

float ParrotRenderer::getCustomBob(boost::shared_ptr<Player> player, float partialTicks)
{
    float flap = player->mOFlap + (player->mFlap - player->mOFlap) * partialTicks;
    float flapSpeed = player->mOFlapSpeed + (player->mFlapSpeed - player->mOFlapSpeed) * partialTicks;
    return (Mth::sin(flap) + 1.0F) * flapSpeed;
}

float ParrotRenderer::getBob(boost::shared_ptr<LivingEntity>& entity, float partialTicks)
{
    const bool isParrot = eTYPE_DERIVED_FROM(EntityType::PARROT, entity->GetType());
    if (isParrot)
    {
        return getCustomBob(boost::static_pointer_cast<Parrot>(entity), partialTicks);
    }

    if (Minecraft::InMiniGame(eMiniGameId_Any, true) && eTYPE_DERIVED_FROM(EntityType::PLAYER, entity->GetType()))
    {
        return getCustomBob(boost::static_pointer_cast<Player>(entity), partialTicks);
    }

    return 0.0F;
}

Model* ParrotRenderer::createModel(const ModelDefinition& definition)
{
    return new ParrotModel(definition);
}
