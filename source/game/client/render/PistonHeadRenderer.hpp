#ifndef GAME_CLIENT_RENDER_PISTONHEADRENDERER_HPP
#define GAME_CLIENT_RENDER_PISTONHEADRENDERER_HPP

#include <boost/shared_ptr.hpp>

#include "game/client/render/BlockEntityRenderer.hpp"

class BlockEntity;
class BlockRenderer;
class Level;

class PistonHeadRenderer : public BlockEntityRenderer
{
public:
    PistonHeadRenderer();
    virtual ~PistonHeadRenderer();

    virtual void render(boost::shared_ptr<BlockEntity> blockEntity, double x, double y, double z, float partialTick,
                        int breakProgress, bool bTranslucent, float alpha, bool bUseDisplayList);
    virtual void onNewLevel(Level* level);

private:
    BlockRenderer* mBlockRenderer;
};

#endif
