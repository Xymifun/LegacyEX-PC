#include "game/client/render/SpectralArrowRenderer.hpp"

#include "game/util/ResourceLocation.hpp"

static ResourceLocation SPECTRAL_ARROW_LOCATION(TN_ITEM_SPECTRAL_ARROW);

SpectralArrowRenderer::SpectralArrowRenderer(EntityRenderDispatcher* dispatcher) : ArrowRenderer(dispatcher)
{
}

SpectralArrowRenderer::~SpectralArrowRenderer()
{
}

const ResourceLocation* SpectralArrowRenderer::getTextureLocation(boost::shared_ptr<Entity>& entity)
{
    return &SPECTRAL_ARROW_LOCATION;
}
