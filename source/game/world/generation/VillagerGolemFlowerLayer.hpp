#ifndef GAME_WORLD_GENERATION_VILLAGERGOLEMFLOWERLAYER_HPP
#define GAME_WORLD_GENERATION_VILLAGERGOLEMFLOWERLAYER_HPP

#include <boost/shared_ptr.hpp>

#include "game/client/render/RenderLayer.hpp"

class LivingEntity;
class VillagerGolemRenderer;

class VillagerGolemFlowerLayer : public RenderLayer
{
public:
    VillagerGolemFlowerLayer(VillagerGolemRenderer* renderer);
    virtual ~VillagerGolemFlowerLayer();

    virtual void render(boost::shared_ptr<LivingEntity>& entity, float limbSwing, float limbSwingAmount,
                        float partialTicks, float ageInTicks, float netHeadYaw, float headPitch, float scale,
                        bool bUseDisplayList);
    virtual bool colorsOnDamage();

private:
    VillagerGolemRenderer* mRenderer;
};

#endif
