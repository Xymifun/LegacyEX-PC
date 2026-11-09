#ifndef GAME_CLIENT_RENDER_ENTITYRENDERER_HPP
#define GAME_CLIENT_RENDER_ENTITYRENDERER_HPP

#include <string>
#include <boost/shared_ptr.hpp>

class AABB;
class BlockPos;
class BlockState;
class Culler;
class Entity;
class EntityRenderDispatcher;
class Font;
class Level;
class Model;
class ModelDefinition;
class ResourceLocation;
class TexturePack;

class EntityRenderer
{
public:
    EntityRenderer(EntityRenderDispatcher* dispatcher);
    virtual ~EntityRenderer() {}
    virtual bool shouldRender(boost::shared_ptr<Entity>& entity, Culler* culler, double x, double y, double z);
    virtual bool shouldRender(boost::shared_ptr<Entity>& entity, Culler* culler, double distance);
    virtual void render(boost::shared_ptr<Entity>& entity, double x, double y, double z, float yaw, float partialTick);
    virtual void render(AABB* box, double x, double y, double z);
    virtual void renderName(boost::shared_ptr<Entity>& entity, double x, double y, double z);
    virtual bool shouldShowName(boost::shared_ptr<Entity>& entity);
    virtual void renderNameTags(boost::shared_ptr<Entity>& entity, double x, double y, double z,
                                const std::wstring& name, double distance, int color);
    virtual bool bindTexture(boost::shared_ptr<Entity>& entity);
    virtual void bindTexture(const ResourceLocation* location);
    virtual bool bindTexture(const std::wstring& path, int index);
    virtual bool bindTexture(const std::wstring& path, const std::wstring& alternate);
    virtual const ResourceLocation* getTextureLocation(boost::shared_ptr<Entity>& entity);
    virtual void renderFlame(boost::shared_ptr<Entity>& entity, double x, double y, double z, float partialTick);
    virtual void renderShadow(boost::shared_ptr<Entity>& entity, double x, double y, double z, float alpha,
                              float partialTick);
    virtual Level* getLevel();
    virtual void renderBlockShadow(const BlockState* state, double x, double y, double z, const BlockPos& pos,
                                   float alpha, float radius, double offsetX, double offsetY, double offsetZ);
    virtual void postRender(boost::shared_ptr<Entity>& entity, double x, double y, double z, float yaw,
                            float partialTick, bool renderShadow);
    virtual Font* getFont();
    virtual void renderNameTag(boost::shared_ptr<Entity>& entity, const std::wstring& name, double x, double y,
                               double z, int maxDistance, int color);
    virtual bool hasSecondPass();
    virtual void renderSecondPass(boost::shared_ptr<Entity>& entity, double x, double y, double z, float yaw,
                                  float partialTick);
    virtual void setupForTexturePack(TexturePack* pack);
    virtual void SetItemFrame(bool itemFrame);
    virtual Model* getModel();
    virtual std::wstring getModelName();
    virtual Model* createModel(const ModelDefinition& definition);
    EntityRenderDispatcher* getDispatcher();
    std::wstring getOriginalModelName();
    unsigned int getTeamColor(boost::shared_ptr<Entity>& entity);
    void setSolidRender(bool solid);

protected:
    EntityRenderDispatcher* mDispatcher;
    float mShadowRadius;
    float mShadowStrength;
    bool mSolidRender;
    Model* mModel;
    Model* mOriginalModel;
};

#endif
