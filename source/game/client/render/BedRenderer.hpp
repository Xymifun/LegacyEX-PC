#ifndef GAME_CLIENT_RENDER_BEDRENDERER_HPP
#define GAME_CLIENT_RENDER_BEDRENDERER_HPP

#include <boost/shared_ptr.hpp>

#include "game/client/render/BlockEntityRenderer.hpp"

class BedModel;
class BlockEntity;
class TexturePack;

class BedRenderer : public BlockEntityRenderer
{
public:
    BedRenderer();
    virtual ~BedRenderer();

    virtual void render(boost::shared_ptr<BlockEntity> blockEntity, double x, double y, double z, float partialTicks,
                        int breakProgress, bool bTranslucent, float alpha, bool bUseDisplayList);
    virtual void setupForTexturePack(TexturePack* pTexturePack);

private:
    BedModel* mModel;
    BedModel* mDefaultModel;
    unsigned int mModelVersion;
};

#endif
