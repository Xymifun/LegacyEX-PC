#ifndef GAME_CLIENT_RENDER_BANNERRENDERER_HPP
#define GAME_CLIENT_RENDER_BANNERRENDERER_HPP

#include <vector>

#include <boost/shared_ptr.hpp>

#include "game/client/render/BlockEntityRenderer.hpp"

class BannerModel;
class BannerPattern;
class BlockEntity;
class DyeColor;

class BannerRenderer : public BlockEntityRenderer
{
public:
    BannerRenderer();
    virtual ~BannerRenderer();

    bool hasTexture(boost::shared_ptr<BlockEntity> blockEntity);
    static void getUV(int atlasIndex, bool layered, float& u, float& v);
    static void setupLayeredParams(const std::vector<const BannerPattern*>* patterns,
                                   const std::vector<const DyeColor*>* colors, bool layered);
    virtual void render(boost::shared_ptr<BlockEntity> blockEntity, double x, double y, double z, float partialTicks,
                        int breakProgress, bool bTranslucent, float alpha, bool bUseDisplayList);

private:
    BannerModel* mModel;
};

#endif
