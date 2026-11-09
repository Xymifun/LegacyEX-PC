#ifndef GAME_WORLD_GENERATION_SPIDEREYESLAYER_HPP
#define GAME_WORLD_GENERATION_SPIDEREYESLAYER_HPP

#include <boost/shared_ptr.hpp>

#include "game/client/render/RenderLayer.hpp"

class LivingEntity;
class SpiderRenderer;

class SpiderEyesLayer : public RenderLayer
{
public:
    SpiderEyesLayer(SpiderRenderer* renderer);
    virtual ~SpiderEyesLayer();

    virtual void render(boost::shared_ptr<LivingEntity>& entity, float limbSwing, float limbSwingAmount,
                        float partialTicks, float ageInTicks, float netHeadYaw, float headPitch, float scale,
                        bool bUseDisplayList);
    virtual bool colorsOnDamage();

private:
    SpiderRenderer* mRenderer;
};

#endif
