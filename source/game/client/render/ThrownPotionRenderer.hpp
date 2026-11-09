#ifndef GAME_CLIENT_RENDER_THROWNPOTIONRENDERER_HPP
#define GAME_CLIENT_RENDER_THROWNPOTIONRENDERER_HPP

#include <boost/shared_ptr.hpp>

#include "game/client/render/ThrownItemRenderer.hpp"

class Entity;
class EntityRenderDispatcher;

class ThrownPotionRenderer : public ThrownItemRenderer
{
public:
    ThrownPotionRenderer(EntityRenderDispatcher* dispatcher);
    virtual ~ThrownPotionRenderer();
    virtual not_null_ptr<ItemInstance> getItem(boost::shared_ptr<Entity> entity);
};

#endif
