#include "game/client/render/TippableArrowRenderer.hpp"

#include "game/entity/projectile/TippableArrow.hpp"
#include "game/util/ResourceLocation.hpp"

static ResourceLocation TIPPABLE_ARROW_LOCATION(TN_ITEM_ARROWS);
static ResourceLocation TIPPED_ARROW_LOCATION(TN_ITEM_TIPPED_ARROW);

TippableArrowRenderer::TippableArrowRenderer(EntityRenderDispatcher* dispatcher) : ArrowRenderer(dispatcher)
{
}

TippableArrowRenderer::~TippableArrowRenderer()
{
}

const ResourceLocation* TippableArrowRenderer::getTextureLocation(boost::shared_ptr<Entity>& entity)
{
    boost::shared_ptr<TippableArrow> arrow = boost::static_pointer_cast<TippableArrow>(entity);
    return arrow->getColor() > 0 ? &TIPPED_ARROW_LOCATION : &TIPPABLE_ARROW_LOCATION;
}
