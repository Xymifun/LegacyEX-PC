#ifndef GAME_WORLD_GENERATION_ENDEREYESLAYER_HPP
#define GAME_WORLD_GENERATION_ENDEREYESLAYER_HPP

#include <boost/shared_ptr.hpp>

#include "game/client/render/RenderLayer.hpp"

class EndermanRenderer;
class LivingEntity;

class EnderEyesLayer : public RenderLayer
{
public:
    EnderEyesLayer(EndermanRenderer* renderer);
    virtual ~EnderEyesLayer();

    virtual void render(boost::shared_ptr<LivingEntity>& entity, float limbSwing, float limbSwingAmount,
                        float partialTicks, float ageInTicks, float netHeadYaw, float headPitch, float scale,
                        bool bUseDisplayList);
    virtual bool colorsOnDamage();

private:
    EndermanRenderer* mRenderer;
};

#endif
