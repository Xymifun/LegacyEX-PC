#ifndef GAME_WORLD_GENERATION_WITCHITEMLAYER_HPP
#define GAME_WORLD_GENERATION_WITCHITEMLAYER_HPP

#include <boost/shared_ptr.hpp>

#include "game/client/render/RenderLayer.hpp"

class LivingEntity;
class WitchRenderer;

class WitchItemLayer : public RenderLayer
{
public:
    WitchItemLayer(WitchRenderer* renderer);
    virtual ~WitchItemLayer();

    virtual void render(boost::shared_ptr<LivingEntity>& entity, float limbSwing, float limbSwingAmount,
                        float partialTicks, float ageInTicks, float netHeadYaw, float headPitch, float scale,
                        bool bUseDisplayList);
    virtual bool colorsOnDamage();

    WitchRenderer* mWitchRenderer;
};

#endif
