#ifndef GAME_WORLD_GENERATION_CUSTOMHEADLAYER_HPP
#define GAME_WORLD_GENERATION_CUSTOMHEADLAYER_HPP

#include <boost/shared_ptr.hpp>

#include "game/client/render/RenderLayer.hpp"

class LivingEntity;
class ModelPart;

class CustomHeadLayer : public RenderLayer
{
public:
    CustomHeadLayer(ModelPart* head);

    virtual ~CustomHeadLayer();

    virtual void render(boost::shared_ptr<LivingEntity>& entity, float limbSwing, float limbSwingAmount,
                        float partialTicks, float ageInTicks, float netHeadYaw, float headPitch, float scale,
                        bool bUseDisplayList);
    virtual bool colorsOnDamage();

    ModelPart* mHead;
};

#endif
