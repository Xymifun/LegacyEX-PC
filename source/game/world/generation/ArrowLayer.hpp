#ifndef GAME_WORLD_GENERATION_ARROWLAYER_HPP
#define GAME_WORLD_GENERATION_ARROWLAYER_HPP

#include <stdint.h>

#include "game/client/render/RenderLayer.hpp"

class LivingEntityRenderer;

class ArrowLayer : public RenderLayer
{
public:
    ArrowLayer(LivingEntityRenderer* renderer);
    virtual ~ArrowLayer();
    virtual void render(boost::shared_ptr<LivingEntity>& entity, float limbSwing, float limbSwingAmount,
                        float partialTicks, float ageInTicks, float netHeadYaw, float headPitch, float scale,
                        bool bUseDisplayList);
    virtual bool colorsOnDamage();

private:
    LivingEntityRenderer* mRenderer;
};

#endif
