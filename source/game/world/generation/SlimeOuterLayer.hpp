#ifndef GAME_WORLD_GENERATION_SLIMEOUTERLAYER_HPP
#define GAME_WORLD_GENERATION_SLIMEOUTERLAYER_HPP

#include <boost/shared_ptr.hpp>

#include "game/client/render/RenderLayer.hpp"

class LivingEntity;
class Model;
class SlimeRenderer;
class TexturePack;

class SlimeOuterLayer : public RenderLayer
{
public:
    SlimeOuterLayer(SlimeRenderer* renderer);
    virtual ~SlimeOuterLayer();

    virtual void render(boost::shared_ptr<LivingEntity>& entity, float limbSwing, float limbSwingAmount,
                        float partialTicks, float ageInTicks, float netHeadYaw, float headPitch, float scale,
                        bool bUseDisplayList);
    virtual bool colorsOnDamage();
    virtual void setupForTexturePack(TexturePack* pTexturePack);

private:
    SlimeRenderer* mRenderer;
    Model* mModel;
    Model* mOriginalModel;
};

#endif
