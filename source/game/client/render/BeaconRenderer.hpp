#ifndef GAME_CLIENT_RENDER_BEACONRENDERER_HPP
#define GAME_CLIENT_RENDER_BEACONRENDERER_HPP

#include "game/client/render/BlockEntityRenderer.hpp"
#include "game/world/biome/arrayWithLength.hpp"
#include "game/world/block/BeaconBlockEntity.hpp"

#include <boost/shared_ptr.hpp>
#include <vector>

class BlockEntity;
class BeaconBlockEntity;

class BeaconRenderer : public BlockEntityRenderer
{
private:
    static void renderBeaconBeam(double x, double y, double z, double partialTicks, double beamHeight, double gameTime,
                                 int startY, int endY, arrayWithLength<const float> colors);

    void renderBeaconBeam(double x, double y, double z, double partialTicks, double beamHeight,
                          std::vector<BeaconBlockEntity::BeaconBeamSection>* sections, double gameTime);

public:
    virtual ~BeaconRenderer();

    static void renderBeaconBeam(double x, double y, double z, double partialTicks, double beamHeight, double gameTime,
                                 int startY, int endY, arrayWithLength<const float> colors, double innerRadius,
                                 double outerRadius);

    virtual void render(boost::shared_ptr<BlockEntity> blockEntity, double x, double y, double z, float a,
                        int breakProgress, bool bTranslucent, float alpha, bool bUseDisplayList);

    void renderSpecialBeamDirection(boost::shared_ptr<BlockEntity> blockEntity, double x, double y, double z,
                                    double xOffset, double yOffset, double zOffset, float a, int breakProgress,
                                    bool bTranslucent, float alpha, bool bUseDisplayList);

    virtual bool shouldRenderOffScreen(boost::shared_ptr<BlockEntity> blockEntity);
};

#endif
