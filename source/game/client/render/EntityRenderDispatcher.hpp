#ifndef GAME_CLIENT_RENDER_ENTITYRENDERDISPATCHER_HPP
#define GAME_CLIENT_RENDER_ENTITYRENDERDISPATCHER_HPP

#include <boost/shared_ptr.hpp>
#include <boost/unordered_map.hpp>

#include "game/client/render/EntityRenderer.hpp"
#include "game/client/render/SheepFurModel.hpp"

class Culler;
class Font;
class ItemRenderer;
class Level;
class Options;
class TexturePack;
class Textures;
class PlayerRenderer;

enum EPlayerModelType
{
    EPLAYER_MODEL_DEFAULT = 0,
    EPLAYER_MODEL_SLIM = 1,
    EPLAYER_MODEL_SLIM_64 = 2
};

class EntityRenderDispatcher : public boost::unordered_map<unsigned long long, EntityRenderer*>
{
public:
    EntityRenderDispatcher(Textures* textures, ItemRenderer* itemRenderer);

    double distanceToSqr(double x, double y, double z);
    unsigned char shouldRenderShadow();
    Font* getFont();
    void setPosition(double x, double y, double z);
    EntityRenderer* getRenderer(unsigned long long type);
    PlayerRenderer* getPlayerRenderer(EPlayerModelType modelType);
    EntityRenderer* getRenderer(boost::shared_ptr<Entity> entity);
    void prepare(Level* level, Font* font, boost::shared_ptr<Entity> camera, boost::shared_ptr<Entity> player,
                 Options* options, float partialTicks);
    bool hasSecondPass(boost::shared_ptr<Entity>& entity);
    bool shouldRender(boost::shared_ptr<Entity>& entity, Culler* culler, double x, double y, double z);
    bool shouldRender(boost::shared_ptr<Entity>& entity, Culler* culler, double distance);
    void renderHitbox(boost::shared_ptr<Entity>& entity, double x, double y, double z, float yaw, float partialTicks,
                      int color);
    void render(boost::shared_ptr<Entity>& entity, double x, double y, double z, float yaw, float partialTicks,
                bool renderOutlines, bool itemFrame, bool renderShadow, bool renderLeash);
    void render(boost::shared_ptr<Entity>& entity, float partialTicks, bool renderOutlines);
    void renderSecondPass(boost::shared_ptr<Entity>& entity, float partialTicks);
    void setupForTexturePack(TexturePack* texturePack);
    void setPlayerRotY(float value);
    void setRenderShadow(bool value);
    void setRenderHitBoxes(bool value);
    bool shouldRenderHitBoxes();
    void setSolidRendering(bool value);

    boost::unordered_map<EPlayerModelType, PlayerRenderer*> mPlayerRenderers;
    PlayerRenderer* mPlayerRenderer;
    Font* mFont;
    double mX;
    double mY;
    double mZ;
    Textures* mTextures;
    Level* mLevel;
    boost::shared_ptr<Entity> mCamera;
    boost::shared_ptr<Entity> mPlayer;
    float mCameraYRot;
    float mCameraXRot;
    Options* mOptions;
    bool mRenderingGuiPlayer;
    bool mRenderLeash;
    double mCameraX;
    double mCameraY;
    double mCameraZ;
    bool mSolidRendering;
    bool mRenderShadow;
    bool mRenderHitBoxes;
};

#endif
