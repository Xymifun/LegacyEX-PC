#ifndef GAME_CLIENT_RENDER_TNTMINECARTRENDERER_HPP
#define GAME_CLIENT_RENDER_TNTMINECARTRENDERER_HPP

#include <boost/shared_ptr.hpp>

#include "game/client/render/MinecartRenderer.hpp"

class BlockState;
class EntityRenderDispatcher;
class Minecart;

class TntMinecartRenderer : public MinecartRenderer
{
public:
    TntMinecartRenderer(EntityRenderDispatcher* dispatcher);
    virtual ~TntMinecartRenderer();

    virtual void renderMinecartContents(boost::shared_ptr<Minecart> minecart, float partialTick,
                                        const BlockState* state);
};

#endif
