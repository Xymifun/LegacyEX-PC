#ifndef GAME_CLIENT_RENDER_CONDUITRENDERER_HPP
#define GAME_CLIENT_RENDER_CONDUITRENDERER_HPP

#include "game/client/render/BlockEntityRenderer.hpp"

#include <boost/shared_ptr.hpp>

class BlockEntity;
class ConduitBaseModel;
class ConduitCageModel;
class ConduitWindModel;

class ConduitRenderer : public BlockEntityRenderer
{
public:
    ConduitRenderer();
    virtual ~ConduitRenderer();

    virtual void render(boost::shared_ptr<BlockEntity> blockEntity, double x, double y, double z, float partialTicks,
                        int breakProgress, bool bTranslucent, float alpha, bool bUseDisplayList);

    void _tessellateHeart() const;

    ConduitBaseModel* mBaseModel;
    ConduitCageModel* mCageModel;
    ConduitWindModel* mWindModel;
};

#endif
