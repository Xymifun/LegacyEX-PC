#ifndef GAME_WORLD_GENERATION_ELYTRALAYER_HPP
#define GAME_WORLD_GENERATION_ELYTRALAYER_HPP

#include <boost/shared_ptr.hpp>

#include "game/client/render/ElytraModel.hpp"
#include "game/client/render/RenderLayer.hpp"

class LivingEntity;
class LivingEntityRenderer;

class ElytraLayer : public RenderLayer
{
public:
    ElytraLayer(LivingEntityRenderer* renderer);
    virtual ~ElytraLayer();

    virtual void render(boost::shared_ptr<LivingEntity>& entity, float limbSwing, float limbSwingAmount,
                        float partialTicks, float ageInTicks, float netHeadYaw, float headPitch, float scale,
                        bool bUseDisplayList);
    virtual bool colorsOnDamage();

    ElytraModel* getElytraModel();

private:
    LivingEntityRenderer* mRenderer;
    ElytraModel* mElytraModel;
};

#endif
