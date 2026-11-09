#ifndef GAME_CLIENT_RENDER_RENDERLAYER_HPP
#define GAME_CLIENT_RENDER_RENDERLAYER_HPP

#include <boost/shared_ptr.hpp>

class LivingEntity;
class TexturePack;

class RenderLayer
{
public:
    virtual ~RenderLayer() {}

    virtual void render(boost::shared_ptr<LivingEntity>& entity, float limbSwing, float limbSwingAmount,
                        float partialTicks, float ageInTicks, float netHeadYaw, float headPitch, float scale,
                        bool bUseDisplayList) = 0;
    virtual bool colorsOnDamage() = 0;
    virtual void setupForTexturePack(TexturePack* pTexturePack);
};

#endif
