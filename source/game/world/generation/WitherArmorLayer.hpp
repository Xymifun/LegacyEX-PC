#ifndef GAME_WORLD_GENERATION_WITHERARMORLAYER_HPP
#define GAME_WORLD_GENERATION_WITHERARMORLAYER_HPP

#include <boost/shared_ptr.hpp>

#include "game/client/render/RenderLayer.hpp"

class LivingEntity;
class TexturePack;
class WitherBossRenderer;
class WitherBossModel;

class WitherArmorLayer : public RenderLayer
{
public:
    WitherArmorLayer(WitherBossRenderer* renderer);
    virtual ~WitherArmorLayer();

    virtual void render(boost::shared_ptr<LivingEntity>& entity, float limbSwing, float limbSwingAmount,
                        float partialTicks, float ageInTicks, float netHeadYaw, float headPitch, float scale,
                        bool bUseDisplayList);
    virtual bool colorsOnDamage();
    virtual void setupForTexturePack(TexturePack* pTexturePack);

private:
    WitherBossRenderer* mRenderer;
    WitherBossModel* mModel;
    WitherBossModel* mOriginalModel;
};

#endif
