#include "game/client/render/RabbitRenderer.hpp"

#include "game/client/render/ModelDefinition.hpp"
#include "game/client/render/RabbitModel.hpp"
#include "game/util/ResourceLocation.hpp"
#include "game/util/Rabbit.hpp"

static ResourceLocation sBrownRabbitTexture(TN_MOB_RABBIT_BROWN);
static ResourceLocation sWhiteRabbitTexture(TN_MOB_RABBIT_WHITE);
static ResourceLocation sBlackRabbitTexture(TN_MOB_RABBIT_BLACK);
static ResourceLocation sGoldRabbitTexture(TN_MOB_RABBIT_GOLD);
static ResourceLocation sSaltRabbitTexture(TN_MOB_RABBIT_SALT);
static ResourceLocation sWhiteSplotchedRabbitTexture(TN_MOB_RABBIT_WHITE_SPLOTCHED);
static ResourceLocation sToastRabbitTexture(TN_MOB_RABBIT_TOAST);
static ResourceLocation sCaerbannogRabbitTexture(TN_MOB_RABBIT_CAERBANNOG);

RabbitRenderer::RabbitRenderer(EntityRenderDispatcher* dispatcher) : MobRenderer(dispatcher, new RabbitModel(), 0.3f)
{
}

const ResourceLocation* RabbitRenderer::getTextureLocation(boost::shared_ptr<Entity>& entity)
{
    boost::shared_ptr<Rabbit> rabbit = boost::static_pointer_cast<Rabbit>(entity);
    std::wstring name = entity->getName();
    if (name == L"Toast")
        return &sToastRabbitTexture;

    switch (rabbit->getRabbitType())
    {
    case 0:
        return &sBrownRabbitTexture;
    case 1:
        return &sWhiteRabbitTexture;
    case 2:
        return &sBlackRabbitTexture;
    case 4:
        return &sGoldRabbitTexture;
    case 5:
        return &sSaltRabbitTexture;
    case 3:
        return &sWhiteSplotchedRabbitTexture;
    case 99:
        return &sCaerbannogRabbitTexture;
    }

    return &sBrownRabbitTexture;
}

Model* RabbitRenderer::createModel(const ModelDefinition& definition)
{
    return new RabbitModel(definition);
}
