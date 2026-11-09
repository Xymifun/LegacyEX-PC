#ifndef GAME_WORLD_GENERATION_PIGSADDLELAYER_HPP
#define GAME_WORLD_GENERATION_PIGSADDLELAYER_HPP

#include <boost/shared_ptr.hpp>

#include "game/client/render/RenderLayer.hpp"

class LivingEntity;
class PigModel;
class PigRenderer;
class ResourceLocation;

class PigSaddleLayer : public RenderLayer
{
public:
    PigSaddleLayer(PigRenderer* renderer);
    virtual ~PigSaddleLayer();

    virtual void render(boost::shared_ptr<LivingEntity>& entity, float limbSwing, float limbSwingAmount,
                        float partialTicks, float ageInTicks, float netHeadYaw, float headPitch, float scale,
                        bool bUseDisplayList);
    virtual bool colorsOnDamage();

    const ResourceLocation* getSaddleTextureLocation() const;

private:
    PigRenderer* mRenderer;
    PigModel* mModel;
};

#endif
