#ifndef GAME_WORLD_GENERATION_CAPELAYER_HPP
#define GAME_WORLD_GENERATION_CAPELAYER_HPP

#include <boost/shared_ptr.hpp>

#include "game/client/render/RenderLayer.hpp"

class LivingEntity;
class PlayerRenderer;

class CapeLayer : public RenderLayer
{
public:
    CapeLayer(PlayerRenderer* renderer);
    virtual ~CapeLayer();

    virtual void render(boost::shared_ptr<LivingEntity>& entity, float limbSwing, float limbSwingAmount,
                        float partialTicks, float ageInTicks, float netHeadYaw, float headPitch, float scale,
                        bool bUseDisplayList);
    virtual bool colorsOnDamage();

private:
    PlayerRenderer* mRenderer;
};

#endif
