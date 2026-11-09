#include "game/client/render/ThrownPotionRenderer.hpp"

#include "game/client/render/EntityRenderDispatcher.hpp"
#include "game/entity/mob/Entity.hpp"
#include "game/item/Items.hpp"
#include "game/item/ThrownPotion.hpp"

ThrownPotionRenderer::ThrownPotionRenderer(EntityRenderDispatcher* dispatcher)
    : ThrownItemRenderer(dispatcher, Items::potion)
{
}

ThrownPotionRenderer::~ThrownPotionRenderer()
{
}

not_null_ptr<ItemInstance> ThrownPotionRenderer::getItem(boost::shared_ptr<Entity> entity)
{
    boost::shared_ptr<ThrownPotion> potion = boost::static_pointer_cast<ThrownPotion>(entity);
    return potion->getItem();
}
