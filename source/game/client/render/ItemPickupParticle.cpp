#include "game/client/render/ItemPickupParticle.hpp"
#include "game/core/EntityType.hpp"

#include "game/client/render/EntityRenderDispatcher.hpp"
#include "game/core/BufferBuilder.hpp"
#include "game/core/GlStateManager.hpp"
#include "game/entity/mob/Entity.hpp"
#include "game/core/Minecraft.hpp"

ItemPickupParticle::ItemPickupParticle(Level* level, boost::shared_ptr<Entity> itemEntity,
                                       boost::shared_ptr<Entity> target, float yOffs)
    : Particle(level, itemEntity->mX, itemEntity->mY, itemEntity->mZ, itemEntity->mXd, itemEntity->mYd, itemEntity->mZd)
{
    this->entityRenderDispatcher = Minecraft::GetInstance()->getEntityRenderDispatcher();
    this->itemEntity = itemEntity;
    this->target = target;
    this->yOffs = yOffs;
}

uint64_t ItemPickupParticle::GetType()
{
    return ParticleKind::ITEM_PICKUP;
}

void ItemPickupParticle::tick()
{
    age++;

    if (age == lifetime)
    {
        remove();
    }
}

int ItemPickupParticle::getParticleTexture()
{
    return 3;
}

void ItemPickupParticle::render(BufferBuilder* buffer, const boost::shared_ptr<Entity>& camera, float partialTicks,
                                float xa, float ya, float za, float xa2, float za2)
{
    float t = ((float)age + partialTicks) / (float)lifetime;
    t *= t;

    double fromX = itemEntity->mX;
    double fromY = itemEntity->mY;
    double fromZ = itemEntity->mZ;

    double toX = target->mXOld + (target->mX - target->mXOld) * partialTicks;
    double toY = target->mYOld + (target->mY - target->mYOld) * partialTicks + (double)yOffs;
    double toZ = target->mZOld + (target->mZ - target->mZOld) * partialTicks;

    double px = fromX + (toX - fromX) * t;
    double py = fromY + (toY - fromY) * t;
    double pz = fromZ + (toZ - fromZ) * t;

    int lightColor = getLightColor(partialTicks);
    int u = lightColor % 65536;
    int v = lightColor / 65536;
    GlStateManager::multiTexCoord2f((float)u, (float)v);
    GlStateManager::color4f(1.0f, 1.0f, 1.0f, 1.0f);

    px -= xOff;
    py -= yOff;
    pz -= zOff;

    GlStateManager::enableLighting();

    entityRenderDispatcher->render(itemEntity, (float)px, (float)py, (float)pz, itemEntity->mYRot, partialTicks, false,
                                   false, true, false);
}
