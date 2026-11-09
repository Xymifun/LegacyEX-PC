#ifndef GAME_CLIENT_RENDER_SKULLBLOCKRENDERER_HPP
#define GAME_CLIENT_RENDER_SKULLBLOCKRENDERER_HPP

#include <string>

#include <boost/shared_ptr.hpp>

#include "game/client/render/BlockEntityRenderer.hpp"

class BlockEntity;
class BlockEntityRenderDispatcher;
class CreeperHeadModel;
class Direction;
class DragonHeadModel;
class SkeletonHeadModel;
class TexturePack;

class SkullBlockRenderer : public BlockEntityRenderer
{
public:
    static SkullBlockRenderer* instance;

    SkullBlockRenderer();
    virtual ~SkullBlockRenderer();

    void renderSkull(float x, float y, float z, const Direction* direction, float rotation, float headPitch,
                     int skullType, int destroyStage, float animationTime, bool useDisplayList);
    virtual void render(boost::shared_ptr<BlockEntity> blockEntity, double x, double y, double z, float partialTicks,
                        int breakProgress, bool bTranslucent, float alpha, bool bUseDisplayList);
    virtual void init(BlockEntityRenderDispatcher* pDispatcher);
    bool dragonJawCheck();
    void updateModel(int modelIndex, const std::wstring& modelName, const std::wstring& originalModelName,
                     TexturePack* pTexturePack);
    virtual void setupForTexturePack(TexturePack* pTexturePack);

private:
    CreeperHeadModel* mActiveCreeperModel;
    DragonHeadModel* mActiveDragonModel;
    SkeletonHeadModel* mActiveSkeletonModels[4];
    DragonHeadModel* mDragonModel;
    CreeperHeadModel* mCreeperModel;
    SkeletonHeadModel* mSkeletonModels[4];
};

#endif
