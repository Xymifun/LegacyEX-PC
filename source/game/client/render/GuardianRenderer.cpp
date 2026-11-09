#include "game/client/render/GuardianRenderer.hpp"

#include <cmath>

#include <boost/shared_ptr.hpp>

#include "game/client/render/GuardianModel.hpp"
#include "game/client/render/ModelDefinition.hpp"
#include "game/client/render/Tesselator.hpp"
#include "game/core/BufferBuilder.hpp"
#include "game/core/Culler.hpp"
#include "game/core/GlStateManager.hpp"
#include "game/core/Guardian.hpp"
#include "game/core/Mth.hpp"
#include "game/entity/mob/LivingEntity.hpp"
#include "game/world/Level.hpp"
#include "game/util/AABB.hpp"
#include "game/util/ResourceLocation.hpp"
#include "game/util/Vec3.hpp"

static ResourceLocation GUARDIAN_LOCATION(TN_MOB_GUARDIAN);
static ResourceLocation GUARDIAN_BEAM_LOCATION(TN_MOB_GUARDIAN_BEAM);

GuardianRenderer::GuardianRenderer(EntityRenderDispatcher* dispatcher)
    : MobRenderer(dispatcher, new GuardianModel(), 0.5f)
{
}

Vec3* GuardianRenderer::getPosition(boost::shared_ptr<Entity>& entity, double yOffset, float partialTicks)
{
    double x = entity->mXOld + (entity->mX - entity->mXOld) * partialTicks;
    double y = yOffset + entity->mYOld + (entity->mY - entity->mYOld) * partialTicks;
    double z = entity->mZOld + (entity->mZ - entity->mZOld) * partialTicks;
    return Vec3::newTemp(x, y, z);
}

bool GuardianRenderer::shouldRender(boost::shared_ptr<Entity>& entity, Culler* culler, double x, double y, double z)
{
    if (MobRenderer::shouldRender(entity, culler, x, y, z))
        return true;

    boost::shared_ptr<Guardian> guardian = boost::static_pointer_cast<Guardian>(entity);
    if (guardian->hasActiveAttackTarget())
    {
        boost::shared_ptr<Entity> target = guardian->getActiveAttackTarget();
        if (target)
        {
            boost::shared_ptr<LivingEntity> living = boost::static_pointer_cast<LivingEntity>(guardian);
            Vec3* targetPosition = getPosition(target, (double)target->mBbHeight * 0.5, 1.0f);
            Vec3* guardianPosition = getPosition(entity, guardian->getEyeHeight(), 1.0f);
            if (culler->isVisible(AABB::minmax(guardianPosition->x, guardianPosition->y, guardianPosition->z,
                                               targetPosition->x, targetPosition->y, targetPosition->z)))
                return true;
        }
    }

    return false;
}

void GuardianRenderer::render(boost::shared_ptr<Entity>& entity, double x, double y, double z, float yaw,
                              float partialTick)
{
    boost::shared_ptr<Guardian> guardian = boost::static_pointer_cast<Guardian>(entity);
    MobRenderer::render(entity, x, y, z, yaw, partialTick);

    boost::shared_ptr<Entity> target = guardian->getActiveAttackTarget();
    if (target != NULL)
    {
        float attackScale = guardian->getAttackAnimationScale(partialTick);
        Tesselator* tesselator = Tesselator::getInstance();
        BufferBuilder* builder = tesselator->getBuilder();
        bindTexture(&GUARDIAN_BEAM_LOCATION);
        GlStateManager::texParameteri(3, 1);
        GlStateManager::texParameteri(4, 1);
        GlStateManager::disableLighting();
        GlStateManager::disableCull();
        GlStateManager::disableBlend();
        GlStateManager::depthMask(true);
        GlStateManager::multiTexCoord2f(240.0f, 240.0f);
        GlStateManager::blendFuncSeparate(4, 1, 1, 0);

        float gameTime = (float)guardian->mLevel->getGameTime() + partialTick;
        float guardianEyeHeight = guardian->getEyeHeight();
        GlStateManager::pushMatrix();
        GlStateManager::translatef((float)x, (float)y + guardianEyeHeight, (float)z);

        Vec3* source = getPosition(target, (double)target->mBbHeight * 0.5, partialTick);
        Vec3* destination = getPosition(entity, guardianEyeHeight, partialTick);
        Vec3* direction = source->subtract(destination);
        double length = direction->length() + 1.0;
        direction = direction->normalize();
        float xAngle = (float)std::acos(direction->y);
        float yAngle = (float)std::atan2(direction->z, direction->x);
        GlStateManager::rotatef((1.5707964f - yAngle) * Mth::RAD_TO_DEG, 0.0f, 1.0f, 0.0f);
        GlStateManager::rotatef(xAngle * Mth::RAD_TO_DEG, 1.0f, 0.0f, 0.0f);

        double time = (double)gameTime * 0.05 * -1.5;
        builder->begin();
        float attackScaleSquared = attackScale * attackScale;
        int intensity = (int)(attackScaleSquared * 191.0f);
        builder->color(intensity + 64, intensity + 32, 128 - (int)(attackScaleSquared * 64.0f), 255);

        const double guardianRadius = 0.282;
        double guardianX0 = std::cos(time + 2.35619455575943) * guardianRadius;
        double guardianZ0 = std::sin(time + 2.35619455575943) * guardianRadius;
        double guardianX1 = std::cos(time + 0.7853981852531433) * guardianRadius;
        double guardianZ1 = std::sin(time + 0.7853981852531433) * guardianRadius;
        double guardianX2 = std::cos(time + 3.9269909262657166) * guardianRadius;
        double guardianZ2 = std::sin(time + 3.9269909262657166) * guardianRadius;
        double guardianX3 = std::cos(time + 5.497787296772003) * guardianRadius;
        double guardianZ3 = std::sin(time + 5.497787296772003) * guardianRadius;

        const double radius = 0.2;
        double x0 = std::cos(time + 3.1415927410125732) * radius;
        double z0 = std::sin(time + 3.1415927410125732) * radius;
        double x1 = std::cos(time) * radius;
        double z1 = std::sin(time) * radius;
        double x2 = std::cos(time + 1.5707963705062866) * radius;
        double z2 = std::sin(time + 1.5707963705062866) * radius;
        double x3 = std::cos(time + 4.71238911151886) * radius;
        double z3 = std::sin(time + 4.71238911151886) * radius;
        double vOffset = -1.0;
        double v = length * 2.5 + vOffset;
        builder->vertexUV((float)x0, (float)length, (float)z0, 0.4999f, (float)v);
        builder->vertexUV((float)x0, 0.0f, (float)z0, 0.4999f, (float)vOffset);
        builder->vertexUV((float)x1, 0.0f, (float)z1, 0.0f, (float)vOffset);
        builder->vertexUV((float)x1, (float)length, (float)z1, 0.0f, (float)v);
        builder->vertexUV((float)x2, (float)length, (float)z2, 0.4999f, (float)v);
        builder->vertexUV((float)x2, 0.0f, (float)z2, 0.4999f, (float)vOffset);
        builder->vertexUV((float)x3, 0.0f, (float)z3, 0.0f, (float)vOffset);
        builder->vertexUV((float)x3, (float)length, (float)z3, 0.0f, (float)v);
        double frameV = 0.0;
        if ((guardian->mTickCount & 1) == 0)
            frameV = 0.5;
        builder->vertexUV((float)guardianX0, (float)length, (float)guardianZ0, 0.5f, (float)frameV + 0.5f);
        builder->vertexUV((float)guardianX1, (float)length, (float)guardianZ1, 1.0f, (float)frameV + 0.5f);
        builder->vertexUV((float)guardianX3, (float)length, (float)guardianZ3, 1.0f, (float)frameV);
        builder->vertexUV((float)guardianX2, (float)length, (float)guardianZ2, 0.5f, (float)frameV);
        builder->end();
        GlStateManager::popMatrix();
    }
}

const ResourceLocation* GuardianRenderer::getTextureLocation(boost::shared_ptr<Entity>&)
{
    return &GUARDIAN_LOCATION;
}

Model* GuardianRenderer::createModel(const ModelDefinition& definition)
{
    return new GuardianModel(definition);
}
