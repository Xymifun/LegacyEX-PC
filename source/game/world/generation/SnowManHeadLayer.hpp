#ifndef GAME_WORLD_GENERATION_SNOWMANHEADLAYER_HPP
#define GAME_WORLD_GENERATION_SNOWMANHEADLAYER_HPP

#include <boost/shared_ptr.hpp>

#include "game/client/render/RenderLayer.hpp"

class LivingEntity;
class SnowManRenderer;

class SnowManHeadLayer : public RenderLayer
{
public:
    SnowManHeadLayer(SnowManRenderer* renderer);
    virtual ~SnowManHeadLayer();

    virtual void render(boost::shared_ptr<LivingEntity>& entity, float limbSwing, float limbSwingAmount,
                        float partialTicks, float ageInTicks, float netHeadYaw, float headPitch, float scale,
                        bool bUseDisplayList);
    virtual bool colorsOnDamage();

private:
    SnowManRenderer* mRenderer;
};

#endif
