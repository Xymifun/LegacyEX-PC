#ifndef GAME_CLIENT_RENDER_MOBSPAWNERRENDERER_HPP
#define GAME_CLIENT_RENDER_MOBSPAWNERRENDERER_HPP

#include "game/client/render/BlockEntityRenderer.hpp"

#include <string>
#include <boost/unordered_map.hpp>
#include <boost/shared_ptr.hpp>

class BaseMobSpawner;
class BlockEntity;
class Entity;

class MobSpawnerRenderer : public BlockEntityRenderer
{
public:
    virtual void render(boost::shared_ptr<BlockEntity> blockEntity, double x, double y, double z, float a,
                        int breakProgress, bool bTranslucent, float alpha, bool bUseDisplayList);

    static void render(BaseMobSpawner* pSpawner, double x, double y, double z, float a);

private:
    boost::unordered_map<std::wstring, boost::shared_ptr<Entity> > mEntityMap;
};

#endif
