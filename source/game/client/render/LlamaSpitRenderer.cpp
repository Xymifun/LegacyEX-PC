#include "game/client/render/LlamaSpitRenderer.hpp"

#include "game/client/render/LlamaSpitModel.hpp"
#include "game/client/render/ModelDefinition.hpp"
#include "game/core/GlStateManager.hpp"
#include "game/entity/mob/Entity.hpp"
#include "game/util/ResourceLocation.hpp"

static ResourceLocation LLAMA_SPIT_LOCATION(TN_MOB_LLAMA_SPIT);

LlamaSpitRenderer::LlamaSpitRenderer(EntityRenderDispatcher* dispatcher)
    : EntityRenderer(dispatcher),
      mSpitModel(new LlamaSpitModel())
{
}

LlamaSpitRenderer::~LlamaSpitRenderer()
{
    if (mSpitModel != NULL)
        delete mSpitModel;
}

void LlamaSpitRenderer::render(boost::shared_ptr<Entity>& entity, double x, double y, double z, float yaw,
                               float partialTick)
{
    GlStateManager::pushMatrix();
    GlStateManager::translatef((float)x, (float)y + 0.15F, (float)z);

    GlStateManager::rotatef(entity->mYRotO + (entity->mYRot - entity->mYRotO) * partialTick - 90.0F, 0.0F, 1.0F, 0.0F);
    GlStateManager::rotatef(entity->mXRotO + (entity->mXRot - entity->mXRotO) * partialTick, 0.0F, 0.0F, 1.0F);

    bindTexture(entity);

    if (mSolidRender)
    {
        GlStateManager::enableColorMaterial();
        GlStateManager::setupSolidRenderingTextureCombine((int)getTeamColor(entity));
    }

    mSpitModel->render(entity, partialTick, 0.0F, -0.1F, 0.0F, 0.0F, 0.0625F, false);

    if (mSolidRender)
    {
        GlStateManager::tearDownSolidRenderingTextureCombine();
        GlStateManager::disableColorMaterial();
    }

    GlStateManager::popMatrix();
    EntityRenderer::render(entity, x, y, z, yaw, partialTick);
}

const ResourceLocation* LlamaSpitRenderer::getTextureLocation(boost::shared_ptr<Entity>& entity)
{
    return &LLAMA_SPIT_LOCATION;
}

Model* LlamaSpitRenderer::createModel(const ModelDefinition& definition)
{
    return new LlamaSpitModel(definition);
}
