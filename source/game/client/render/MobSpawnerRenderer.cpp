#include "game/client/render/MobSpawnerRenderer.hpp"

#include "game/client/render/EntityRenderDispatcher.hpp"
#include "game/core/GlStateManager.hpp"
#include "game/entity/mob/BaseMobSpawner.hpp"
#include "game/entity/mob/Entity.hpp"
#include "game/core/Minecraft.hpp"
#include "game/world/Level.hpp"
#include "game/world/block/Blocks.hpp"
#include "game/world/block/MobSpawnerBlockEntity.hpp"

#include <algorithm>

void MobSpawnerRenderer::render(boost::shared_ptr<BlockEntity> blockEntity, double x, double y, double z, float a,
                                int breakProgress, bool bTranslucent, float alpha, bool bUseDisplayList)
{
    boost::shared_ptr<MobSpawnerBlockEntity> spawnerBlockEntity = boost::static_pointer_cast<MobSpawnerBlockEntity>(
        blockEntity);

    render(spawnerBlockEntity->getSpawner(), x, y, z, a);

    GlStateManager::popMatrix();
}

void MobSpawnerRenderer::render(BaseMobSpawner* pSpawner, double x, double y, double z, float a)
{
    GlStateManager::pushMatrix();
    GlStateManager::translatef((float)x + 0.5F, (float)y, (float)z + 0.5F);

    boost::shared_ptr<Entity> entity = pSpawner->getOrCreateDisplayEntity(pSpawner->getLevel());
    if (entity && pSpawner->getLevel()->getBlock(pSpawner->getPos()) == Blocks::mob_spawner)
    {
        entity->setLevel(pSpawner->getLevel());

        float scale = 0.53125F;
        const float& width = entity->mBbWidth;
        const float& height = entity->mBbHeight;
        float maxSize = std::max(width, height);
        if (maxSize > 1.0)
        {
            scale = 0.53125F / maxSize;
        }

        GlStateManager::translatef(0.0F, 0.4F, 0.0F);

        GlStateManager::rotatef(
            (float)(pSpawner->getoSpin() + (pSpawner->getSpin() - pSpawner->getoSpin()) * a) * 10.0F, 0.0F, 1.0F, 0.0F);

        GlStateManager::translatef(0.0F, -0.4F, 0.0F);
        GlStateManager::rotatef(-30.0F, 1.0F, 0.0F, 0.0F);
        GlStateManager::scalef(scale, scale, scale);

        entity->moveTo(x, y, z, 0.0F, 0.0F);

        Minecraft::GetInstance()->getEntityRenderDispatcher()->render(entity, 0.0, 0.0, 0.0, 0.0F, a, false, false,
                                                                      true, false);
    }
}
