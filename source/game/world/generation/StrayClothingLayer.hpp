#ifndef GAME_WORLD_GENERATION_STRAYCLOTHINGLAYER_HPP
#define GAME_WORLD_GENERATION_STRAYCLOTHINGLAYER_HPP

#include <boost/shared_ptr.hpp>

#include "game/client/render/RenderLayer.hpp"

class LivingEntity;
class LivingEntityRenderer;
class Model;
class TexturePack;

class StrayClothingLayer : public RenderLayer
{
public:
    StrayClothingLayer(LivingEntityRenderer* renderer);
    virtual ~StrayClothingLayer();

    virtual void render(boost::shared_ptr<LivingEntity>& entity, float limbSwing, float limbSwingAmount,
                        float partialTicks, float ageInTicks, float netHeadYaw, float headPitch, float scale,
                        bool bUseDisplayList);
    virtual bool colorsOnDamage();
    virtual void setupForTexturePack(TexturePack* pTexturePack);

private:
    LivingEntityRenderer* mRenderer;
    Model* mModel;
    Model* mOriginalModel;
};

#endif
