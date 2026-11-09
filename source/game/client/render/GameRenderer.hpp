#ifndef GAME_CLIENT_RENDER_GAMERENDERER_HPP
#define GAME_CLIENT_RENDER_GAMERENDERER_HPP

#include <string>
#include <boost/shared_ptr.hpp>

#include "game/util/Random.hpp"
#include "game/core/SmoothFloat.hpp"
#include "game/core/CriticalSection.hpp"
#include "game/item/not_null_ptr.hpp"
#include "game/world/biome/arrayWithLength.hpp"

class Minecraft;
class Vec3;
class ResourceManager;
class AbstractClientPlayer;
class Font;
class Level;
class BlockPos;
class LevelRenderer;
class Culler;
class Entity;
class LivingEntity;
class Player;
class ItemInstance;
class ResourceLocation;
class MapRenderer;
class ItemInHandRenderer;
class PostChain;
class SparseLightStorage;
class LevelChunk;
class ElementStorage;
class CompressedBlockStorage;
class SparseDataStorage;
class FloatBuffer;

class GameRenderer
{
public:
    static const int EFFECT_NONE = 24;
    static const float NAME_TAG_SCALE;

    struct PositionStruct
    {
        float x;
        float y;
        float z;
    };

    struct ElementStorageWithTick
    {
        int tick;
        ElementStorage* storage;
    };

    GameRenderer(Minecraft* minecraft, ResourceManager* resourceManager);
    ~GameRenderer();

    static void renderNameTagInWorld(Font*, const std::wstring&, float, float, float, int, float, float, bool, bool,
                                     int, float);
    unsigned int getLightTexture(int, Level*);
    static int runUpdate(void*);
    int getTickCount();
    unsigned int postEffectActive();
    void shutdownEffect();
    void checkEntityPostEffect(boost::shared_ptr<Entity>);
    void cycleEffect();
    void onResourceManagerReload(ResourceManager*);
    float GetFovVal();
    static void UpdateElementStorageDeleteTicks();
    void tickFov();
    void tickLightTexture();
    boost::shared_ptr<Entity> getCamera();
    void tickRain();
    void tick(bool);
    void pick(float);
    float getFov(float, bool);
    void bobHurt(float);
    void bobView(float);
    unsigned int getCameraThirdPerson();
    float getCameraYRot(float);
    float getCameraXRot(float);
    void moveCameraToPlayer(float);
    void getFovAndAspect(float&, float&, float, bool);
    void setupCamera(float, int);
    void turnOnLightLayer(double);
    void turnOffLightLayer(double);
    void renderItemInHand(float, int);
#ifdef MINECRAFT_PC
    void renderModHands(float partialTick);
#endif
    float getNightVisionScale(boost::shared_ptr<LivingEntity>, float);
    float getUnderWaterVisionScale(const boost::shared_ptr<Player>&, float);
    void updateLightTexture(float);
    static unsigned int fjGetFpsCap(int);
    static bool anaglyph3d;
    const BlockPos* getFogOffsets();
    float getTargetFogDistance(const BlockPos&);
    void getTargetFogColor(const BlockPos&, float&, float&, float&);
    float modifyFogDistance(const BlockPos&, float, float);
    void setupClearColor(float);
    FloatBuffer* getBuffer(float, float, float, float);
    void resetFogColor(bool);
    void setupFog(int, float);
    void prepareAndRenderClouds(LevelRenderer*, float, int, double, double, double);
    void renderThermal(Culler*, float);
    void renderGhost(float, boost::shared_ptr<Entity>);
    void renderGhostPath(float, boost::shared_ptr<Entity>);
    void renderSnowAndRain(float);
    void renderEyePass(int, float, long long, bool);
    void renderLevel(float, long long);
    void setupGuiScreen(int);
    void renderItemActivationAnimation(int, int, float);
    void render(float, long long, bool);
    int shouldRenderBlockOutline();
    void resetData();
    MapRenderer* getMapRenderer();
    static void AddForDelete(SparseLightStorage*);
    static void AddForDelete(LevelChunk*);
    static void AddForDelete(ElementStorage*);
    static void FinishedReassigning();
    void EnableUpdateThread(int);
    void DisableUpdateThread(int);
    static void EnterDeleteThreadCS();
    static void LeaveDeleteThreadCS();
    void displayItemActivation(not_null_ptr<ItemInstance>, int);

    void setRenderHand(bool);
    bool shouldRenderHand();
    void setRenderBlockOutline(bool);
    void setPanoramicMode(bool);
    void setPanoramicSection(int);
    void togglePostEffect();
    void loadEffect(ResourceLocation*);
    void SetFovVal(float);
    PostChain* currentEffect();
    void resize(int, int);
    void zoomRegion(double, double, double);
    void unZoomRegion();
    void renderLevel(float);
    void updateAllChunks();
    static void AddForDelete(unsigned char*);
    static void AddForDelete(CompressedBlockStorage*);
    static void AddForDelete(SparseDataStorage*);
    bool IsUpdateThreadEnabled();

private:
    OSFastMutex mUpdateThreadMutex;
    unsigned int mUpdateThreadDisableMask;
    Minecraft* mMinecraft;
    ResourceManager* mResourceManager;
    Random mRandom;
    float mFogDistance;

public:
    ItemInHandRenderer* mItemInHandRenderer;

private:
    MapRenderer* mMapRenderer;
    int mTickCount;
    boost::shared_ptr<Entity> mPointedEntity;
    SmoothFloat mSmoothTurnX;
    SmoothFloat mSmoothTurnY;
    SmoothFloat mSmoothDummy1;
    SmoothFloat mSmoothDummy2;
    SmoothFloat mSmoothDummy3;
    SmoothFloat mSmoothDummy4;
    float mThirdPersonDistance;
    float mThirdPersonDistanceO;
    float mDebugCamYaw;
    float mDebugCamYawO;
    float mDebugCamPitch;
    float mDebugCamPitchO;
    float mSmoothCamYaw;
    float mSmoothCamPitch;
    float mSmoothCamFilterX;
    float mSmoothCamFilterY;
    float mSmoothCamPartialTicks;
    Vec3* mZeroVector;
    float mFov[4];
    float mFovO[4];
    float mUnusedPlayerFloats[4];
    float mFovSetting;
    float mDarkenWorldAmount;
    float mDarkenWorldAmountO;
    bool mCloudFog;
    bool mRenderHand;
    bool mRenderBlockOutline;
    long long mLastActiveTime;
    unsigned int mLightTextures[4];
    arrayWithLength<int> mLightPixels[4];
    float mBlockLightRedFlicker;
    float mBlockLightRedFlickerSpeed;
    int mRainSoundTime;
    float mRainSizeX[1024];
    float mRainSizeZ[1024];
    FloatBuffer* mFogBuffer;
    float mFogRed;
    float mFogGreen;
    float mFogBlue;
    float mPlayerFogRed[4];
    float mPlayerFogGreen[4];
    float mPlayerFogBlue[4];
    float mPlayerFogDistance[4];
    float mTargetFogRed[4];
    float mTargetFogGreen[4];
    float mTargetFogBlue[4];
    float mTargetFogDistance[4];
    float mFogBrightnessO;
    float mFogBrightness;
    bool mFogHeightLocked;
    long long mFogColourChangeTime[4];
    long long mFogDistanceChangeTime[4];
    double mFogHeight;
    unsigned int mPanoramicSection;
    bool mPanoramicMode;
    float mZoom;
    float mZoomX;
    float mZoomY;
    not_null_ptr<ItemInstance> mItemActivationItems[4];
    int mItemActivationTicks[4];
    float mItemActivationOffX[4];
    float mItemActivationOffY[4];
    PostChain* mPostEffect;
    int mEffectIndex;
    bool mEffectActive;
    int mFrameCount;
    long long mLastNanos;
    bool mLightTextureDirty;
    unsigned char* mOwnedArrays[2];
    FloatBuffer* mUnusedBuffer;
    int mDebugView;
};

#endif
