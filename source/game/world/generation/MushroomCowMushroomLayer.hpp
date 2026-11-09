#ifndef GAME_WORLD_GENERATION_MUSHROOMCOWMUSHROOMLAYER_HPP
#define GAME_WORLD_GENERATION_MUSHROOMCOWMUSHROOMLAYER_HPP

#include <boost/shared_ptr.hpp>

#include "game/client/render/RenderLayer.hpp"

class LivingEntity;
class MushroomCowRenderer;

class MushroomCowMushroomLayer : public RenderLayer
{
public:
    MushroomCowMushroomLayer(MushroomCowRenderer* renderer);
    virtual ~MushroomCowMushroomLayer();

    virtual void render(boost::shared_ptr<LivingEntity>& entity, float limbSwing, float limbSwingAmount,
                        float partialTicks, float ageInTicks, float netHeadYaw, float headPitch, float scale,
                        bool bUseDisplayList);
    virtual bool colorsOnDamage();

private:
    MushroomCowRenderer* mRenderer;
};

#endif
