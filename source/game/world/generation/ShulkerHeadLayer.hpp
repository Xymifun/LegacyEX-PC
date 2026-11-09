#ifndef GAME_WORLD_GENERATION_SHULKERHEADLAYER_HPP
#define GAME_WORLD_GENERATION_SHULKERHEADLAYER_HPP

#include <boost/shared_ptr.hpp>

#include "game/client/render/RenderLayer.hpp"

class LivingEntity;
class ResourceLocation;
class ShulkerRenderer;

class ShulkerHeadLayer : public RenderLayer
{
public:
    ShulkerHeadLayer(ShulkerRenderer* renderer);
    virtual ~ShulkerHeadLayer();

    virtual void render(boost::shared_ptr<LivingEntity>& entity, float limbSwing, float limbSwingAmount,
                        float partialTicks, float ageInTicks, float netHeadYaw, float headPitch, float scale,
                        bool bUseDisplayList);
    virtual bool colorsOnDamage();

private:
    static ResourceLocation SHULKER_LOCATION;

    ShulkerRenderer* mRenderer;
};

#endif
