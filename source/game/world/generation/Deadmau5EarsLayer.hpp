#ifndef GAME_WORLD_GENERATION_DEADMAU5EARSLAYER_HPP
#define GAME_WORLD_GENERATION_DEADMAU5EARSLAYER_HPP

#include <boost/shared_ptr.hpp>

#include "game/client/render/RenderLayer.hpp"

class LivingEntity;
class PlayerRenderer;

class Deadmau5EarsLayer : public RenderLayer
{
public:
    Deadmau5EarsLayer(PlayerRenderer* renderer);
    virtual ~Deadmau5EarsLayer();

    virtual void render(boost::shared_ptr<LivingEntity>& entity, float limbSwing, float limbSwingAmount,
                        float partialTicks, float ageInTicks, float netHeadYaw, float headPitch, float scale,
                        bool bUseDisplayList);
    virtual bool colorsOnDamage();

private:
    PlayerRenderer* mRenderer;
};

#endif
