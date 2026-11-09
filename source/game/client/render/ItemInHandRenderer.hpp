#ifndef GAME_CLIENT_RENDER_ITEMINHANDRENDERER_HPP
#define GAME_CLIENT_RENDER_ITEMINHANDRENDERER_HPP

#include <boost/shared_ptr.hpp>
#include "game/core/InteractionHand.hpp"
#include "game/item/ItemTransforms.hpp"
#include "game/item/not_null_ptr.hpp"

class Minecraft;
class Tesselator;
class Block;
class HumanoidArm;
class TextureAtlasSprite;
class LivingEntity;
class AbstractClientPlayer;
class LocalPlayer;
class ItemInstance;
class EntityRenderDispatcher;
class ItemRenderer;

class ItemInHandRenderer
{
public:
    enum eItemRenderMode
    {
        ITEM_RENDER_MODE_NONE,
        ITEM_RENDER_MODE_GUI,
        ITEM_RENDER_MODE_3D,
        ITEM_RENDER_MODE_FLAT,
        ITEM_RENDER_MODE_EXTRUDED,
        ITEM_RENDER_MODE_EXTRUDED_HIGH_RES
    };
    ItemInHandRenderer(Minecraft*);
    unsigned long long renderItem3D(Tesselator*, float, float, float, float, int, int, float, eItemRenderMode);
    void Reset();
    void renderItem(boost::shared_ptr<LivingEntity>, not_null_ptr<ItemInstance>, ItemTransforms::TransformType, bool,
                    int, bool);
    bool isTransparentBlock(Block*);
    void enableLight(float, float);
#ifdef MINECRAFT_PC
    void renderModHand(const HumanoidArm* arm, const float* viewFromHand);
    void renderModItem(const boost::shared_ptr<ItemInstance>& item, const float* viewFromItem);
#endif
    void setLightValue(boost::shared_ptr<AbstractClientPlayer>);
    void setPlayerBob(boost::shared_ptr<LocalPlayer>, float);
    float calculateMapTilt(float);
    void renderMapHand(const HumanoidArm*);
    void renderMapHands();
    void renderPlayerArm(float, float, const HumanoidArm*);
    void renderMap(not_null_ptr<ItemInstance>, int);
    void renderOneHandedMap(float, const HumanoidArm*, float, not_null_ptr<ItemInstance>);
    void renderTwoHandedMap(float, float, float);
    void applyItemAttackTransform(float);
    void applyEatTransform(float, const HumanoidArm*, not_null_ptr<ItemInstance>);
    void applyItemArmAttackTransform(const HumanoidArm*, float);
    void applyItemArmTransform(const HumanoidArm*, float);
    void renderArmWithItem(boost::shared_ptr<LocalPlayer>, float, float, InteractionHand::EInteractionHand, float,
                           not_null_ptr<ItemInstance>, float);
    void render(float);
    void renderTex(TextureAtlasSprite*);
    void renderWater(float);
    void renderFire();
    void renderScreenEffect(float);
    void CalculateNewHandHeights(bool);
    void tick();
    void itemUsed(InteractionHand::EInteractionHand);

private:
    Minecraft* mMinecraft;
    not_null_ptr<ItemInstance> mMainHandItem;
    not_null_ptr<ItemInstance> mOffHandItem;
    float mMainHandHeight;
    float mMainHandHeightOld;
    float mOffHandHeight;
    float mOffHandHeightOld;
    EntityRenderDispatcher* mEntityRenderDispatcher;
    ItemRenderer* mItemRenderer;
    int mListId;
};

#endif
