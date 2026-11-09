#ifndef GAME_WORLD_GENERATION_SPINATTACKLAYER_HPP
#define GAME_WORLD_GENERATION_SPINATTACKLAYER_HPP

#include <boost/shared_ptr.hpp>

#include "game/client/render/RenderLayer.hpp"

class LivingEntity;
class LivingEntityRenderer;
class SpinAttackModel;
class TexturePack;

class SpinAttackLayer : public RenderLayer
{
public:
    SpinAttackLayer(LivingEntityRenderer* renderer);
    virtual ~SpinAttackLayer();

    virtual void render(boost::shared_ptr<LivingEntity>& entity, float limbSwing, float limbSwingAmount,
                        float partialTicks, float ageInTicks, float netHeadYaw, float headPitch, float scale,
                        bool bUseDisplayList);
    virtual bool colorsOnDamage();

private:
    LivingEntityRenderer* mRenderer;
    SpinAttackModel* mModel;
};

#endif
