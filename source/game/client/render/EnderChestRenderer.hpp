#ifndef GAME_CLIENT_RENDER_ENDERCHESTRENDERER_HPP
#define GAME_CLIENT_RENDER_ENDERCHESTRENDERER_HPP

#include "game/client/render/BlockEntityRenderer.hpp"
#include "game/client/render/ChestModel.hpp"
#include "game/client/render/ModelDefinition.hpp"

#include <boost/shared_ptr.hpp>

class BlockEntity;

class EnderChestRenderer : public BlockEntityRenderer
{
public:
    virtual void render(boost::shared_ptr<BlockEntity> blockEntity, double x, double y, double z, float a,
                        int breakProgress, bool bTranslucent, float alpha, bool bUseDisplayList);

    ChestModel mChestModel;
};

#endif
