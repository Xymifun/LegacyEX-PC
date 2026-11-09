#ifndef GAME_CLIENT_RENDER_PLAYRENDERER_HPP
#define GAME_CLIENT_RENDER_PLAYRENDERER_HPP

#include <string>
#include <boost/shared_ptr.hpp>

#include "game/client/render/LivingEntityRenderer.hpp"
#include "game/client/render/PlayerModel.hpp"

class ElytraLayer;
class EntityRenderDispatcher;
class Entity;
class LivingEntity;
class Player;
class ModelPart;
class ResourceLocation;
class ElytraModel;

class PlayerRenderer : public LivingEntityRenderer
{
public:
    using EntityRenderer::bindTexture;

    PlayerRenderer(EntityRenderDispatcher* dispatcher, bool slimArms, bool is64x64);

    void _init();
    static unsigned int getNametagColour(int colour);
    virtual Model* getModel();
    ElytraModel* getElytraModel();
    void setModelProperties(boost::shared_ptr<Player> player, float partialTicks);
    void hideAdditionalModelParts(boost::shared_ptr<Player> player);
    void updatePartVisibility(ModelPart* part, boost::shared_ptr<Player> player, bool visible);
    void showAdditionalModelParts(boost::shared_ptr<Player> player);
    virtual void render(boost::shared_ptr<Entity>& entity, double x, double y, double z, float yaw, float partialTick);
    virtual const ResourceLocation* getTextureLocation(boost::shared_ptr<Entity>& entity);
    virtual void translateWeaponItem();
    virtual void renderNameTags(boost::shared_ptr<Entity>& entity, double x, double y, double z,
                                const std::wstring& name, double distance, int colour);
    virtual void scale(boost::shared_ptr<LivingEntity>& entity, float partialTicks);
    void renderRightHand(boost::shared_ptr<Player> player);
    void renderLeftHand(boost::shared_ptr<Player> player);
    virtual void setupPosition(boost::shared_ptr<LivingEntity>& entity, double x, double y, double z);
    virtual void setupRotations(boost::shared_ptr<LivingEntity>& entity, float ageInTicks, float rotationYaw,
                                float partialTicks);
    virtual void renderShadow(boost::shared_ptr<Entity>& entity, double x, double y, double z, float alpha,
                              float partialTicks);
    virtual bool bindTexture(boost::shared_ptr<Entity>& entity);
    virtual void renderLayers(boost::shared_ptr<LivingEntity>& entity, float limbSwing, float limbSwingAmount,
                              float partialTick, float ageInTicks, float netHeadYaw, float headPitch, float scale,
                              bool overlay);

private:
    friend class ElytraLayer;

    bool mSpecialRenderActive;
    float mSpecialRenderAlpha;
    bool mSlimArms;
    bool mIs64x64;
    ElytraLayer* mElytraLayer;
};

#endif
