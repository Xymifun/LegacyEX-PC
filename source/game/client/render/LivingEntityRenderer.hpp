#ifndef GAME_CLIENT_RENDER_LIVINGENTITYRENDERER_HPP
#define GAME_CLIENT_RENDER_LIVINGENTITYRENDERER_HPP

#include <vector>
#include <boost/shared_ptr.hpp>

#include "game/client/render/EntityRenderer.hpp"

class LivingEntity;
class RenderLayer;
class TexturePack;

class LivingEntityRenderer : public EntityRenderer
{
public:
    LivingEntityRenderer(EntityRenderDispatcher* dispatcher, Model* model, float shadowRadius);
    virtual ~LivingEntityRenderer() {}

    bool addLayer(RenderLayer* layer);
    bool removeLayer(RenderLayer* layer);

    virtual Model* getModel();
    virtual Model* getModel(boost::shared_ptr<LivingEntity>& entity);
    float rotlerp(float from, float to, float delta);
    virtual void translateWeaponItem();
    float setupScale(boost::shared_ptr<LivingEntity>& entity, float partialTicks);
    bool setupSolidState(boost::shared_ptr<LivingEntity>& entity);
    bool setupOverlayColor(boost::shared_ptr<LivingEntity>& entity, float partialTicks, bool combineTextures);
    bool setupOverlayColor(boost::shared_ptr<LivingEntity>& entity, float partialTicks);
    void teardownOverlayColor();
    void tearDownSolidState();
    virtual void render(boost::shared_ptr<Entity>& entity, double x, double y, double z, float yaw, float partialTick);
    virtual void setupForTexturePack(TexturePack* pack);
    virtual void renderModel(boost::shared_ptr<LivingEntity>& entity, float limbSwing, float limbSwingAmount,
                             float ageInTicks, float netHeadYaw, float headPitch, float scale);
    virtual bool isVisible(boost::shared_ptr<LivingEntity>& entity);
    virtual void setupPosition(boost::shared_ptr<LivingEntity>& entity, double x, double y, double z);
    virtual void setupRotations(boost::shared_ptr<LivingEntity>& entity, float ageInTicks, float rotationYaw,
                                float partialTicks);
    virtual float getAttackAnim(boost::shared_ptr<LivingEntity>& entity, float partialTicks);
    virtual float getBob(boost::shared_ptr<LivingEntity>& entity, float partialTicks);
    virtual void renderLayers(boost::shared_ptr<LivingEntity>& entity, float limbSwing, float limbSwingAmount,
                              float partialTick, float ageInTicks, float netHeadYaw, float headPitch, float scale,
                              bool overlay);
    virtual float getFlipDegrees(boost::shared_ptr<LivingEntity>& entity);
    virtual int getOverlayColor(boost::shared_ptr<LivingEntity>& entity, float u, float v);
    virtual void scale(boost::shared_ptr<LivingEntity>& entity, float partialTicks);
    virtual void renderName(boost::shared_ptr<Entity>& entity, double x, double y, double z);
    virtual bool shouldShowName(boost::shared_ptr<Entity>& entity);

protected:
    std::vector<RenderLayer*> mLayers;
    bool mItemFrame;
};

#endif
