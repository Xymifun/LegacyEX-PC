#ifndef GAME_WORLD_GENERATION_SHEEPFURLAYER_HPP
#define GAME_WORLD_GENERATION_SHEEPFURLAYER_HPP

#include <boost/shared_ptr.hpp>

#include "game/client/render/RenderLayer.hpp"

class LivingEntity;
class SheepRenderer;

class SheepFurLayer : public RenderLayer
{
public:
    SheepFurLayer(SheepRenderer* renderer);
    virtual ~SheepFurLayer();

    virtual void render(boost::shared_ptr<LivingEntity>& entity, float limbSwing, float limbSwingAmount,
                        float partialTicks, float ageInTicks, float netHeadYaw, float headPitch, float scale,
                        bool bUseDisplayList);
    virtual bool colorsOnDamage();

private:
    SheepRenderer* mRenderer;
};

#endif
