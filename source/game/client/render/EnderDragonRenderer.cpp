#include "game/client/render/EnderDragonRenderer.hpp"

#include <algorithm>
#include <cmath>

#include "game/client/render/DragonModel.hpp"
#include "game/client/render/Model.hpp"
#include "game/client/render/ModelDefinition.hpp"
#include "game/client/render/Tesselator.hpp"
#include "game/core/BufferBuilder.hpp"
#include "game/core/EnderCrystal.hpp"
#include "game/core/GlStateManager.hpp"
#include "game/core/Lighting.hpp"
#include "game/core/Mth.hpp"
#include "game/entity/mob/EnderDragon.hpp"
#include "game/util/ResourceLocation.hpp"
#include "game/world/biome/arrayWithLength.hpp"
#include "game/world/generation/EnderDragonDeathLayer.hpp"
#include "game/world/generation/EnderDragonEyesLayer.hpp"

ResourceLocation END_CRYSTAL_BEAM_LOCATION(TN_MOB_ENDERDRAGON_BEAM);
static ResourceLocation ENDER_DRAGON_EXPLODING_LOCATION(TN_MOB_ENDERDRAGON_SHUFFLE);
static ResourceLocation ENDER_DRAGON_LOCATION(TN_MOB_ENDERDRAGON_ENDER);
static float beamRotation;

EnderDragonRenderer::EnderDragonRenderer(EntityRenderDispatcher* dispatcher)
    : MobRenderer(dispatcher, new DragonModel(), 0.5f)
{
    mDragonModel = static_cast<DragonModel*>(mModel);
    mEyesLayer = new EnderDragonEyesLayer(this);
    addLayer(mEyesLayer);
    addLayer(new EnderDragonDeathLayer());
}

void EnderDragonRenderer::setupRotations(boost::shared_ptr<LivingEntity>& entity, float, float, float partialTicks)
{
    boost::shared_ptr<EnderDragon> dragon = boost::static_pointer_cast<EnderDragon>(entity);
    double positions[3];
    arrayWithLength<double> offsets;
    offsets.data = positions;
    offsets.length = 3;
    dragon->getLatencyPos(offsets, 7, partialTicks);
    float yRot = (float)positions[0];
    float tilt = dragon->getTilt(partialTicks);
    GlStateManager::rotatef(-yRot, 0.0f, 1.0f, 0.0f);
    GlStateManager::rotatef(tilt, 1.0f, 0.0f, 0.0f);
    GlStateManager::translatef(0.0f, 0.0f, 1.0f);
    if (dragon->LivingEntity::mDeathTime > 0)
    {
        float progress = sqrtf(((float)dragon->LivingEntity::mDeathTime + partialTicks - 1.0f) / 20.0f * 1.6f);
        if (progress > 1.0f)
            progress = 1.0f;
        GlStateManager::rotatef(progress * getFlipDegrees(entity), 0.0f, 0.0f, 1.0f);
    }
}

void EnderDragonRenderer::renderModel(boost::shared_ptr<LivingEntity>& entity, float limbSwing, float limbSwingAmount,
                                      float ageInTicks, float netHeadYaw, float headPitch, float scale)
{
    boost::shared_ptr<EnderDragon> dragon = boost::static_pointer_cast<EnderDragon>(entity);
    boost::shared_ptr<Entity> renderEntity = dragon;
    if (dragon->mDeathTime > 0)
    {
        float deathProgress = (float)dragon->mDeathTime / 200.0f;
        GlStateManager::depthFunc(3);
        GlStateManager::enableAlphaTest();
        GlStateManager::alphaFunc(4, deathProgress);
        bindTexture(&ENDER_DRAGON_EXPLODING_LOCATION);
        mModel->render(renderEntity, limbSwing, limbSwingAmount, ageInTicks, netHeadYaw, headPitch, scale, true);
        GlStateManager::alphaFunc(4, 0.1f);
        GlStateManager::depthFunc(2);
    }
    bindTexture(renderEntity);
    mModel->render(renderEntity, limbSwing, limbSwingAmount, ageInTicks, netHeadYaw, headPitch, scale, true);
    if (dragon->mHurtTime > 0)
    {
        GlStateManager::depthFunc(2);
        GlStateManager::disableTexture();
        GlStateManager::enableBlend();
        GlStateManager::blendFunc(4, 5);
        GlStateManager::color4f(1.0f, 0.0f, 0.0f, 0.5f);
        mModel->render(renderEntity, limbSwing, limbSwingAmount, ageInTicks, netHeadYaw, headPitch, scale, false);
        GlStateManager::enableTexture();
        GlStateManager::disableBlend();
        GlStateManager::depthFunc(3);
    }
    GlStateManager::depthFunc(3);
}

void EnderDragonRenderer::render(boost::shared_ptr<Entity>& entity, double x, double y, double z, float yaw,
                                 float partialTick)
{
    boost::shared_ptr<EnderDragon> dragon = boost::static_pointer_cast<EnderDragon>(entity);
    MobRenderer::render(entity, x, y, z, yaw, partialTick);
    if (dragon->mNearestCrystal && !dragon->mNearestCrystal->mRemoved)
    {
        bindTexture(&END_CRYSTAL_BEAM_LOCATION);
        float animation = std::sin(((float)dragon->mNearestCrystal->mTickCount + partialTick) * 0.2f) * 0.5f + 0.5f;
        animation = (animation * animation + animation) * 0.2f;
        renderCrystalBeams(x, y, z, partialTick, dragon->mX + (dragon->mXo - dragon->mX) * (double)(1.0f - partialTick),
                           dragon->mY + (dragon->mYo - dragon->mY) * (double)(1.0f - partialTick),
                           dragon->mZ + (dragon->mZo - dragon->mZ) * (double)(1.0f - partialTick), dragon->mTickCount,
                           dragon->mNearestCrystal->mX, animation + dragon->mNearestCrystal->mY,
                           dragon->mNearestCrystal->mZ);
    }
}

const ResourceLocation* EnderDragonRenderer::getTextureLocation(boost::shared_ptr<Entity>&)
{
    return &ENDER_DRAGON_LOCATION;
}

void EnderDragonRenderer::setupForTexturePack(TexturePack* pack)
{
    LivingEntityRenderer::setupForTexturePack(pack);
    if (mModel->hasOverriddenParts())
    {
        removeLayer(mEyesLayer);
        return;
    }
    if (std::find(mLayers.begin(), mLayers.end(), mEyesLayer) == mLayers.end())
        addLayer(mEyesLayer);
}

Model* EnderDragonRenderer::createModel(const ModelDefinition& definition)
{
    return new DragonModel(definition);
}

void EnderDragonRenderer::renderCrystalBeams(double x, double y, double z, float partialTick, double targetX,
                                             double targetY, double targetZ, int time, double crystalX, double crystalY,
                                             double crystalZ)
{
    float dx = (float)(crystalX - targetX);
    float dy = (float)(crystalY - 1.0 - targetY);
    float dz = (float)(crystalZ - targetZ);
    float horizontal = Mth::sqrt(dx * dx + dz * dz);
    float distance = Mth::sqrt(dx * dx + dy * dy + dz * dz);
    GlStateManager::color4f(1.0f, 1.0f, 1.0f, 1.0f);
    GlStateManager::pushMatrix();
    GlStateManager::translatef((float)x, (float)y + 2.0f, (float)z);
    GlStateManager::rotatef(-(std::atan2(dz, dx) * Mth::RAD_TO_DEG) - 90.0f, 0.0f, 1.0f, 0.0f);
    GlStateManager::rotatef(-(std::atan2(horizontal, dy) * Mth::RAD_TO_DEG) - 90.0f, 1.0f, 0.0f, 0.0f);
    GlStateManager::rotatef(beamRotation, 0.0f, 0.0f, 1.0f);
    beamRotation += 0.5f;
    if (beamRotation >= 360.0f)
        beamRotation = 0.0f;
    Tesselator* tesselator = Tesselator::getInstance();
    BufferBuilder* builder = tesselator->getBuilder();
    Lighting::turnOff();
    GlStateManager::disableCull();
    GlStateManager::enableBlend();
    GlStateManager::blendFunc(4, 6);
    GlStateManager::shadeModel(0);
    float phase = ((float)time + partialTick) * 0.005f;
    float negPhase = -phase;
    float length = (float)std::sqrt(dx * dx + dy * dy + dz * dz) * 0.03125f - phase;
    builder->begin(1);
    for (int i = 0; i <= 8; ++i)
    {
        float angle = (float)(i % 8) * 6.2831855f * 0.125f;
        float sine = std::sin(angle) * 0.75f;
        float cosine = std::cos(angle) * 0.75f;
        float section = (float)(i % 8) * 0.125f;
        builder->vertexUV(sine * 0.2f, cosine * 0.2f, 0.0f, section, length);
        builder->vertexUV(sine, cosine, distance, section, negPhase);
    }
    builder->end();
    GlStateManager::enableCull();
    GlStateManager::shadeModel(0);
    GlStateManager::disableBlend();
    GlStateManager::popMatrix();
    Lighting::turnOn();
}
