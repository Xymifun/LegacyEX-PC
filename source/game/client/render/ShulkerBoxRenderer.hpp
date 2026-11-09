#ifndef GAME_CLIENT_RENDER_SHULKERBOXRENDERER_HPP
#define GAME_CLIENT_RENDER_SHULKERBOXRENDERER_HPP

#include <boost/shared_ptr.hpp>

#include "game/client/render/BlockEntityRenderer.hpp"
#include "game/client/render/ShulkerModel.hpp"

class BlockEntity;
class Model;
class TexturePack;

class ShulkerBoxRenderer : public BlockEntityRenderer
{
public:
    explicit ShulkerBoxRenderer(ShulkerModel* model);
    virtual ~ShulkerBoxRenderer();

    virtual void render(boost::shared_ptr<BlockEntity> blockEntity, double x, double y, double z, float partialTick,
                        int breakProgress, bool translucent, float alpha, bool useDisplayList);
    virtual void setupForTexturePack(TexturePack* texturePack);

private:
    Model* mModel;
    ShulkerModel* mShulkerModel;
};

#endif
