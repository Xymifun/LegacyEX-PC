#ifndef GAME_WORLD_GENERATION_CREEPERPOWERLAYER_HPP
#define GAME_WORLD_GENERATION_CREEPERPOWERLAYER_HPP

#include <boost/shared_ptr.hpp>

#include "game/client/render/RenderLayer.hpp"

class CreeperRenderer;
class CreeperModel;
class LivingEntity;
class TexturePack;

class CreeperPowerLayer : public RenderLayer
{
public:
    CreeperPowerLayer(CreeperRenderer* renderer);
    virtual ~CreeperPowerLayer();

    virtual void render(boost::shared_ptr<LivingEntity>& entity, float limbSwing, float limbSwingAmount,
                        float partialTicks, float ageInTicks, float netHeadYaw, float headPitch, float scale,
                        bool bUseDisplayList);
    virtual bool colorsOnDamage();
    virtual void setupForTexturePack(TexturePack* pTexturePack);

private:
    CreeperRenderer* mRenderer;
    CreeperModel* mModel;
    CreeperModel* mOriginalModel;
};

#endif
