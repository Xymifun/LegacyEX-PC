#include "game/client/render/TheEndGatewayRenderer.hpp"

#include "game/client/render/BeaconRenderer.hpp"
#include "game/core/DyeColor.hpp"
#include "game/core/GlStateManager.hpp"
#include "game/core/Mth.hpp"
#include "game/entity/mob/TheEndGatewayEntity.hpp"
#include "game/util/ResourceLocation.hpp"
#include "game/world/Level.hpp"
#include "game/world/biome/arrayWithLength.hpp"
#include "game/world/block/BlockEntity.hpp"

#include <boost/shared_ptr.hpp>

static ResourceLocation END_GATEWAY_BEAM_LOCATION(TN_MISC_BEACON_BEAM);

TheEndGatewayRenderer::~TheEndGatewayRenderer()
{
}

void TheEndGatewayRenderer::render(boost::shared_ptr<BlockEntity> blockEntity, double x, double y, double z, float a,
                                   int breakProgress, bool bTranslucent, float alpha, bool bUseDisplayList)
{
    GlStateManager::disableFog();

    boost::shared_ptr<TheEndGatewayEntity> gateway = boost::static_pointer_cast<TheEndGatewayEntity>(blockEntity);

    if (gateway->isSpawning() || gateway->isCoolingDown())
    {
        GlStateManager::alphaFunc(GL_GREATER, 0.01F);

        bindTexture(&END_GATEWAY_BEAM_LOCATION);

        float percent = gateway->isSpawning() ? gateway->getSpawnPercent(a) : gateway->getCooldownPercent(a);
        double beamLength = gateway->isSpawning() ? 256.0 - y : 25.0;

        percent = Mth::sin(percent * 3.1415927F);

        int beamHeight = Mth::floor(percent * beamLength);

        arrayWithLength<const float> colours = gateway->isSpawning() ? DyeColor::MAGENTA->getTextureDiffuseColors()
                                                                     : DyeColor::PURPLE->getTextureDiffuseColors();

        BeaconRenderer::renderBeaconBeam(x, y, z, a, percent, (double)gateway->getLevel()->getGameTime(), 0, beamHeight,
                                         colours, 0.15, 0.175);
        BeaconRenderer::renderBeaconBeam(x, y, z, a, percent, (double)gateway->getLevel()->getGameTime(), 0,
                                         -beamHeight, colours, 0.15, 0.175);
    }

    TheEndPortalRenderer::render(blockEntity, x, y, z, a, breakProgress, bTranslucent, alpha, bUseDisplayList);

    GlStateManager::enableFog();
}

int TheEndGatewayRenderer::getPasses(double distance)
{
    return TheEndPortalRenderer::getPasses(distance) + 1;
}

double TheEndGatewayRenderer::getOffset()
{
    float offset = 1.0F;
    return offset;
}
