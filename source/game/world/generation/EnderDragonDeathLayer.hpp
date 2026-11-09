#ifndef GAME_WORLD_GENERATION_ENDERDRAGONDEATHLAYER_HPP
#define GAME_WORLD_GENERATION_ENDERDRAGONDEATHLAYER_HPP

#include <boost/shared_ptr.hpp>

#include "game/client/render/RenderLayer.hpp"

class LivingEntity;

class EnderDragonDeathLayer : public RenderLayer
{
public:
    virtual ~EnderDragonDeathLayer();

    virtual void render(boost::shared_ptr<LivingEntity>& entity, float limbSwing, float limbSwingAmount,
                        float partialTicks, float ageInTicks, float netHeadYaw, float headPitch, float scale,
                        bool bUseDisplayList);
    virtual bool colorsOnDamage();
};

#endif
