#ifndef GAME_WORLD_GENERATION_ENDERDRAGONEYESLAYER_HPP
#define GAME_WORLD_GENERATION_ENDERDRAGONEYESLAYER_HPP

#include <boost/shared_ptr.hpp>

#include "game/client/render/RenderLayer.hpp"

class EnderDragonRenderer;
class LivingEntity;

class EnderDragonEyesLayer : public RenderLayer
{
public:
    EnderDragonEyesLayer(EnderDragonRenderer* renderer);
    virtual ~EnderDragonEyesLayer();

    virtual void render(boost::shared_ptr<LivingEntity>& entity, float limbSwing, float limbSwingAmount,
                        float partialTicks, float ageInTicks, float netHeadYaw, float headPitch, float scale,
                        bool bUseDisplayList);
    virtual bool colorsOnDamage();

private:
    EnderDragonRenderer* mRenderer;
};

#endif
