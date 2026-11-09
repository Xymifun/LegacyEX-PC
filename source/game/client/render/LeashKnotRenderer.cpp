#include "game/client/render/LeashKnotRenderer.hpp"

#include "game/client/render/Model.hpp"
#include "game/client/render/ModelDefinition.hpp"
#include "game/core/GlStateManager.hpp"
#include "game/util/ResourceLocation.hpp"

static ResourceLocation LEASH_KNOT_LOCATION(TN_ITEM_LEAD_KNOT);

LeashKnotRenderer::LeashKnotRenderer(EntityRenderDispatcher* dispatcher) : EntityRenderer(dispatcher)
{
    mKnotModel = new LeashKnotModel();
}

LeashKnotRenderer::~LeashKnotRenderer()
{
    delete mKnotModel;
}

void LeashKnotRenderer::render(boost::shared_ptr<Entity>& entity, double x, double y, double z, float yaw,
                               float partialTick)
{
    GlStateManager::pushMatrix();
    GlStateManager::disableCull();
    GlStateManager::translatef((float)x, (float)y, (float)z);
    float scale = 0.0625F;
    GlStateManager::enableRescaleNormal();
    GlStateManager::scalef(-1.0F, -1.0F, 1.0F);
    GlStateManager::enableAlphaTest();
    bindTexture(entity);

    if (mSolidRender)
    {
        GlStateManager::enableColorMaterial();
        GlStateManager::setupSolidRenderingTextureCombine((int)getTeamColor(entity));
    }

    mKnotModel->render(entity, 0.0F, 0.0F, 0.0F, 0.0F, 0.0F, scale, true);

    if (mSolidRender)
    {
        GlStateManager::tearDownSolidRenderingTextureCombine();
        GlStateManager::disableColorMaterial();
    }

    GlStateManager::popMatrix();
    EntityRenderer::render(entity, x, y, z, yaw, partialTick);
}

const ResourceLocation* LeashKnotRenderer::getTextureLocation(boost::shared_ptr<Entity>&)
{
    return &LEASH_KNOT_LOCATION;
}

Model* LeashKnotRenderer::createModel(const ModelDefinition& definition)
{
    return new LeashKnotModel(definition);
}
