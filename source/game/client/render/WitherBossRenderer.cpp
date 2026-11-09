#include "game/client/render/WitherBossRenderer.hpp"

#include <boost/shared_ptr.hpp>

#include "game/client/render/ModelDefinition.hpp"
#include "game/client/render/WitherBossModel.hpp"
#include "game/core/GlStateManager.hpp"
#include "game/core/Mth.hpp"
#include "game/core/WitherBoss.hpp"
#include "game/util/ResourceLocation.hpp"
#include "game/world/generation/WitherArmorLayer.hpp"

static ResourceLocation WITHER_INVULNERABLE_LOCATION(TN_MOB_WITHER_WITHER_INVULNERABLE);
static ResourceLocation WITHER_LOCATION(TN_MOB_WITHER_WITHER);

WitherBossRenderer::WitherBossRenderer(EntityRenderDispatcher* dispatcher)
    : MobRenderer(dispatcher, new WitherBossModel(0.0F), 1.0F)
{
    addLayer(new WitherArmorLayer(this));
}

const ResourceLocation* WitherBossRenderer::getTextureLocation(boost::shared_ptr<Entity>& entity)
{
    boost::shared_ptr<WitherBoss> wither = boost::static_pointer_cast<WitherBoss>(entity);
    int invulnerableTicks = wither->getInvulnerableTicks();

    if (invulnerableTicks <= 0 || (invulnerableTicks <= 80 && (invulnerableTicks / 5) % 2 == 1))
        return &WITHER_LOCATION;

    return &WITHER_INVULNERABLE_LOCATION;
}

void WitherBossRenderer::scale(boost::shared_ptr<LivingEntity>& entity, float partialTicks)
{
    boost::shared_ptr<WitherBoss> wither = boost::static_pointer_cast<WitherBoss>(entity);
    float swelling = wither->getSwellAmount(partialTicks);
    float scale = 1.0F + Mth::sin(swelling * 100.0F) * swelling * 0.01F;

    if (swelling < 0.0F)
        swelling = 0.0F;
    else if (swelling > 1.0F)
        swelling = 1.0F;
    swelling *= swelling;
    swelling *= swelling;

    float horizontalScale = (1.0F + swelling * 0.4F) * scale;
    float verticalScale = (1.0F + swelling * 0.1F) / scale;
    GlStateManager::scalef(horizontalScale * 2.0F, verticalScale * 2.0F, horizontalScale * 2.0F);
}

int WitherBossRenderer::getOverlayColor(boost::shared_ptr<LivingEntity>& entity, float u, float v)
{
    int invulnerableTicks = boost::static_pointer_cast<WitherBoss>(entity)->getInvulnerableTicks();
    if (invulnerableTicks > 0)
    {
        int overlay;
        {
            boost::shared_ptr<WitherBoss> wither = boost::static_pointer_cast<WitherBoss>(entity);
            float alpha = wither->getOverlayAlpha();
            if (alpha < 0.0F)
                alpha = 0.0F;
            else if (alpha > 1.0F)
                alpha = 1.0F;

            overlay = Mth::floor(alpha * 255.0F);
        }
        return overlay << 24 | 0x00FFFFFF;
    }

    return LivingEntityRenderer::getOverlayColor(entity, u, v);
}

Model* WitherBossRenderer::createModel(const ModelDefinition& definition)
{
    return new WitherBossModel(0.0F, definition);
}
