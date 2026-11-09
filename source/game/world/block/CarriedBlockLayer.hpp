#ifndef GAME_WORLD_BLOCK_CARRIEDBLOCKLAYER_HPP
#define GAME_WORLD_BLOCK_CARRIEDBLOCKLAYER_HPP

#include <boost/shared_ptr.hpp>

#include "game/client/render/RenderLayer.hpp"

class EndermanRenderer;
class LivingEntity;
class TexturePack;

class CarriedBlockLayer : public RenderLayer
{
public:
    CarriedBlockLayer(EndermanRenderer* renderer);
    virtual ~CarriedBlockLayer();

    virtual void render(boost::shared_ptr<LivingEntity>& entity, float limbSwing, float limbSwingAmount,
                        float partialTicks, float ageInTicks, float netHeadYaw, float headPitch, float scale,
                        bool bUseDisplayList);
    virtual bool colorsOnDamage();

    EndermanRenderer* mEndermanRenderer;
};

#endif
