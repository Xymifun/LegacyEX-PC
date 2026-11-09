#ifndef GAME_WORLD_GENERATION_ABSTRACTARMORLAYER_HPP
#define GAME_WORLD_GENERATION_ABSTRACTARMORLAYER_HPP

#include <boost/shared_ptr.hpp>

#include "game/client/render/RenderLayer.hpp"
#include "game/item/not_null_ptr.hpp"

class ArmorItem;
class EquipmentSlot;
class ItemInstance;
class LivingEntity;
class LivingEntityRenderer;
class Model;
class ResourceLocation;
class TexturePack;

class AbstractArmorLayer : public RenderLayer
{
public:
    AbstractArmorLayer(LivingEntityRenderer* renderer);
    virtual ~AbstractArmorLayer() {}
    virtual void render(boost::shared_ptr<LivingEntity>& entity, float limbSwing, float limbSwingAmount,
                        float partialTicks, float ageInTicks, float netHeadYaw, float headPitch, float scale,
                        bool bUseDisplayList);
    virtual bool colorsOnDamage();
    virtual void resetColor();
    virtual void setColor(float red, float green, float blue, float alpha);
    virtual Model* getArmorModel(const EquipmentSlot* slot);
    virtual void createArmorModels() = 0;
    virtual void setPartVisibility(Model* model, const EquipmentSlot* slot) = 0;
    virtual void hideAllArmor(Model* model) = 0;
    bool usesInnerModel(const EquipmentSlot* slot);
    not_null_ptr<ItemInstance> getSlot(boost::shared_ptr<LivingEntity>& entity, const EquipmentSlot* slot);
    ResourceLocation* getArmorLocation(ArmorItem* armor, bool inner, bool overlay);
    static void renderFoil(LivingEntityRenderer* renderer, boost::shared_ptr<LivingEntity>& entity, Model* model,
                           float limbSwing, float limbSwingAmount, float partialTicks, float ageInTicks,
                           float netHeadYaw, float headPitch, float scale, bool bUseDisplayList);
    void renderArmorPiece(boost::shared_ptr<LivingEntity>& entity, float limbSwing, float limbSwingAmount,
                          float partialTicks, float ageInTicks, float netHeadYaw, float headPitch, float scale,
                          const EquipmentSlot* slot, bool bUseDisplayList);
    ResourceLocation* getArmorLocation(ArmorItem* armor, bool inner);

protected:
    Model* mModelArmorInner;
    Model* mModelArmor;
    LivingEntityRenderer* mRenderer;
    float mAlpha;
    float mRed;
    float mGreen;
    float mBlue;
    bool mColorSet;
};

#endif
