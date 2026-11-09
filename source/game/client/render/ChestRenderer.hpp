#ifndef GAME_CLIENT_RENDER_CHESTRENDERER_HPP
#define GAME_CLIENT_RENDER_CHESTRENDERER_HPP

#include "game/client/render/BlockEntityRenderer.hpp"
#include "game/client/render/ChestModel.hpp"
#include "game/client/render/LargeChestModel.hpp"

#include <boost/shared_ptr.hpp>

class BlockEntity;

class ChestRenderer : public BlockEntityRenderer
{
public:
    ChestRenderer();
    virtual ~ChestRenderer();

    virtual void render(boost::shared_ptr<BlockEntity> blockEntity, double x, double y, double z, float a,
                        int breakProgress, bool bTranslucent, float alpha, bool bUseDisplayList);

private:
    ChestModel* mChestModel;
    LargeChestModel* mLargeChestModel;
};

#endif
