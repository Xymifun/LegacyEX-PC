#ifndef GAME_WORLD_GENERATION_WOLFCOLLARLAYER_HPP
#define GAME_WORLD_GENERATION_WOLFCOLLARLAYER_HPP

#include <boost/shared_ptr.hpp>

#include "game/client/render/RenderLayer.hpp"

class LivingEntity;
class WolfRenderer;

class WolfCollarLayer : public RenderLayer
{
public:
    WolfCollarLayer(WolfRenderer* renderer);
    virtual ~WolfCollarLayer();

    virtual void render(boost::shared_ptr<LivingEntity>& entity, float limbSwing, float limbSwingAmount,
                        float partialTicks, float ageInTicks, float netHeadYaw, float headPitch, float scale,
                        bool bUseDisplayList);
    virtual bool colorsOnDamage();

private:
    WolfRenderer* mRenderer;
};

#endif
