#ifndef GAME_WORLD_GENERATION_LLAMADECORLAYER_HPP
#define GAME_WORLD_GENERATION_LLAMADECORLAYER_HPP

#include <boost/shared_ptr.hpp>

#include "game/client/render/RenderLayer.hpp"

class LlamaRenderer;
class LlamaModel;
class LivingEntity;
class TexturePack;

class LlamaDecorLayer : public RenderLayer
{
public:
    LlamaDecorLayer(LlamaRenderer* renderer);
    virtual ~LlamaDecorLayer();

    virtual void render(boost::shared_ptr<LivingEntity>& entity, float limbSwing, float limbSwingAmount,
                        float partialTicks, float ageInTicks, float netHeadYaw, float headPitch, float scale,
                        bool bUseDisplayList);
    virtual bool colorsOnDamage();
    virtual void setupForTexturePack(TexturePack* texturePack);

private:
    LlamaRenderer* mRenderer;
    LlamaModel* mModel;
};

#endif
