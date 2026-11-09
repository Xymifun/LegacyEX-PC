#include "game/client/render/LlamaRenderer.hpp"

#include "game/client/render/LlamaModel.hpp"
#include "game/client/render/ModelDefinition.hpp"
#include "game/core/Llama.hpp"
#include "game/core/Mth.hpp"
#include "game/util/ResourceLocation.hpp"
#include "game/world/generation/LlamaDecorLayer.hpp"

static ResourceLocation LLAMA_TEXTURES[4] = {
    ResourceLocation(TN_MOB_LLAMA_LLAMA_CREAMY), ResourceLocation(TN_MOB_LLAMA_LLAMA_WHITE),
    ResourceLocation(TN_MOB_LLAMA_LLAMA_BROWN), ResourceLocation(TN_MOB_LLAMA_LLAMA_GRAY)};

LlamaRenderer::LlamaRenderer(EntityRenderDispatcher* dispatcher) : MobRenderer(dispatcher, new LlamaModel(0.0F), 0.7F)
{
    addLayer(new LlamaDecorLayer(this));
}

const ResourceLocation* LlamaRenderer::getTextureLocation(boost::shared_ptr<Entity>& entity)
{
    boost::shared_ptr<Llama> llama = boost::static_pointer_cast<Llama>(entity);
    int variant = Mth::clamp(llama->getVariant(), 0, 3);
    return &LLAMA_TEXTURES[variant];
}

Model* LlamaRenderer::createModel(const ModelDefinition& definition)
{
    return new LlamaModel(0.0F, definition);
}
