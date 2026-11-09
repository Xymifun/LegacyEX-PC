#include "game/client/render/EnderCrystalRenderer.hpp"

#include <cmath>

#include "game/client/render/EnderCrystalModel.hpp"
#include "game/client/render/EnderDragonRenderer.hpp"
#include "game/client/render/ModelDefinition.hpp"
#include "game/core/EnderCrystal.hpp"
#include "game/core/GlStateManager.hpp"
#include "game/util/ResourceLocation.hpp"

static ResourceLocation ENDER_CRYSTAL_LOCATION(TN_MOB_ENDERDRAGON_CRYSTAL);

EnderCrystalRenderer::EnderCrystalRenderer(EntityRenderDispatcher* dispatcher) : EntityRenderer(dispatcher)
{
    mModelBase = new EnderCrystalModel(0.0F, true);
    mModelNoBase = new EnderCrystalModel(0.0F, false);
    mShadowRadius = 0.5F;
}

EnderCrystalRenderer::~EnderCrystalRenderer()
{
}

void EnderCrystalRenderer::render(boost::shared_ptr<Entity>& entity, double x, double y, double z, float yaw,
                                  float partialTick)
{
    boost::shared_ptr<EnderCrystal> crystal = boost::static_pointer_cast<EnderCrystal>(entity);
    float age = (float)crystal->mTime + partialTick;

    GlStateManager::pushMatrix();
    GlStateManager::translatef((float)x, (float)y, (float)z);
    bindTexture(&ENDER_CRYSTAL_LOCATION);
    float animation = std::sin(age * 0.2F) * 0.5F + 0.5F;
    animation = animation * animation + animation;
    if (mSolidRender)
    {
        GlStateManager::enableColorMaterial();
        GlStateManager::setupSolidRenderingTextureCombine(getTeamColor(entity));
    }

    if (crystal->showsBottom())
        mModelBase->render(entity, 0.0F, age * 3.0F, animation * 0.2F, 0.0F, 0.0F, 0.0625F, true);
    else
        mModelNoBase->render(entity, 0.0F, age * 3.0F, animation * 0.2F, 0.0F, 0.0F, 0.0625F, true);

    if (mSolidRender)
    {
        GlStateManager::tearDownSolidRenderingTextureCombine();
        GlStateManager::disableColorMaterial();
    }
    GlStateManager::popMatrix();

    BlockPos* beamTarget = crystal->getBeamTarget();
    if (beamTarget != NULL)
    {
        bindTexture(&END_CRYSTAL_BEAM_LOCATION);
        float targetX = (float)beamTarget->x + 0.5F;
        float targetY = (float)beamTarget->y + 0.5F;
        float targetZ = (float)beamTarget->z + 0.5F;
        double offsetX = targetX - crystal->mX;
        double offsetY = targetY - crystal->mY;
        double offsetZ = targetZ - crystal->mZ;
        GlStateManager::multiTexCoord2f(0.0F, 0.0F);
        EnderDragonRenderer::renderCrystalBeams(x + offsetX, (y - 0.3) + (double)(animation * 0.4F) + offsetY,
                                                z + offsetZ, partialTick, targetX, targetY, targetZ, crystal->mTime,
                                                crystal->mX, crystal->mY, crystal->mZ);
    }

    EntityRenderer::render(entity, x, y, z, yaw, partialTick);
}

const ResourceLocation* EnderCrystalRenderer::getTextureLocation(boost::shared_ptr<Entity>&)
{
    return &ENDER_CRYSTAL_LOCATION;
}

bool EnderCrystalRenderer::shouldRender(boost::shared_ptr<Entity>& entity, Culler* culler, double x, double y, double z)
{
    return EntityRenderer::shouldRender(entity, culler, x, y, z)
           || boost::static_pointer_cast<EnderCrystal>(entity)->getBeamTarget() != NULL;
}

bool EnderCrystalRenderer::shouldRender(boost::shared_ptr<Entity>& entity, Culler* culler, double distance)
{
    return EntityRenderer::shouldRender(entity, culler, distance)
           || boost::static_pointer_cast<EnderCrystal>(entity)->getBeamTarget() != NULL;
}

Model* EnderCrystalRenderer::createModel(const ModelDefinition& definition)
{
    return new EnderCrystalModel(0.0F, true, definition);
}
