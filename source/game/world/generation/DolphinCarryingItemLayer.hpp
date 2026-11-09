#ifndef GAME_WORLD_GENERATION_DOLPHINCARRYINGITEMLAYER_HPP
#define GAME_WORLD_GENERATION_DOLPHINCARRYINGITEMLAYER_HPP

#include <boost/shared_ptr.hpp>

#include "game/client/render/RenderLayer.hpp"
#include "game/item/not_null_ptr.hpp"

class ItemInstance;
class ItemRenderer;
class LivingEntity;
class LivingEntityRenderer;

class DolphinCarryingItemLayer : public RenderLayer
{
public:
    DolphinCarryingItemLayer(LivingEntityRenderer* renderer);
    virtual ~DolphinCarryingItemLayer();

    virtual void render(boost::shared_ptr<LivingEntity>& entity, float limbSwing, float limbSwingAmount,
                        float partialTicks, float ageInTicks, float netHeadYaw, float headPitch, float scale,
                        bool bUseDisplayList);
    virtual bool colorsOnDamage();

    void renderItemOnNose(const boost::shared_ptr<LivingEntity>& entity, const not_null_ptr<ItemInstance>& item);

private:
    LivingEntityRenderer* mLivingEntityRenderer;
    ItemRenderer* mItemRenderer;
};

#endif
