#ifndef GAME_CORE_MINECRAFT_HPP
#define GAME_CORE_MINECRAFT_HPP

#include <list>
#include <string>
#include <vector>

#include <boost/shared_ptr.hpp>

#include "game/core/CriticalSection.hpp"
#include "game/core/File.hpp"
#include "game/minigame/EMiniGameId.hpp"
#include "game/network/DisconnectPacket.hpp"
#include "game/world/biome/arrayWithLength.hpp"

class AbstractClientPlayer;
class BlockColors;
class BlockEntity;
class BlockRenderDispatcher;
class Canvas;
class Level;
class ClientMasterGameMode;
class ClientPacketListener;
class ColourTable;
class Component;
class ConsoleSaveFile;
class DataFixerUpper;
class Entity;
class EntityRenderDispatcher;
class Font;
class FrameTimer;
class GameRenderer;
class GhostController;
class Gui;
class HitResult;
class ItemColors;
class ItemInHandRenderer;
class ItemInstance;
class ItemRenderer;
class LevelRenderer;
class LevelSettings;
class LevelStorageSource;
class MaterialTable;
class MiniGameDef;
class MinecraftApplet;
class MultiPlayerGameMode;
class MultiPlayerLevel;
class MultiplayerLocalPlayer;
class Options;
class ParticleEngine;
class Player;
class ProgressRenderer;
class Screen;
class SoundEngine;
class StatsCounter;
class StructureManager;
class TextureAtlas;
class TexturePackRepository;
class Textures;
class Timer;
class User;

class Minecraft
{
    friend class ClientPacketListener;

public:
    Minecraft(Component* parent, Canvas* canvas, MinecraftApplet* applet, int width, int height, bool fullscreen);

    static Minecraft* GetInstance();
    static long long currentTimeMillis();
    static void main();
    static void start(const std::wstring& userName, const std::wstring& sessionId);
    static void startAndConnectTo(const std::wstring& userName, const std::wstring& sessionId,
                                  const std::wstring& server);

    static bool InMiniGame(EMiniGameId miniGameId, bool includeNextGameMode);
    static EMiniGameId GetMiniGameId();

    static bool renderNames();
    static bool useFancyGraphics();
    static bool useAmbientOcclusion();
    static bool renderDebug();
    static int getAverageFps();
    static int maxSupportedTextureSize();

    static void levelTickThreadInitFunc();
    static void levelTickUpdateFunc(void* level);
    static int InGame_SignInReturned(void* userData, bool signedIn, int userIndex, int playerIdx);
    static boost::shared_ptr<ItemInstance> addCustomNbtData(boost::shared_ptr<ItemInstance> item,
                                                            boost::shared_ptr<BlockEntity> blockEntity);

    void init();
    void run();
    void run_middle();
    void run_end();
    void stop();
    void destroy();
    void verify();
    void pauseGame();
    void resize(int width, int height);
    void emergencySave();
    void reloadSound();
    void delayTextureReload();
    void fjReloadTexturePack();

    void tick(bool handleInput, bool lastTickOfFrame);
    void tickAllConnections();
    void renderInfoBars();
    void renderChunkInfoMap();
    void renderLoadingScreen();
    void renderFpsMeter(long long frameStartTime);
    void blit(int x, int y, int width, int height, int u, int v, int uWidth, int vHeight, int textureWidth,
              int textureHeight);
    void checkGlError(const std::wstring& stage);

    void setScreen(Screen* screen);
    Screen* getScreen();

    bool isClientSide();
    bool isTutorial();
    bool showOnlyReducedInfo();
    bool isUsingDefaultSkin();

    MultiPlayerLevel* getLevel(int dimension);
    LevelStorageSource* getLevelSource();
    void setLevel(MultiPlayerLevel* level, int dimension, boost::shared_ptr<Player> player, bool doForceStatsSave,
                  bool primaryPlayerSignedOut);
    void forceaddLevel(MultiPlayerLevel* level);
    void releaseLevel(int dimension);
    void prepareLevel(int dimension);
    void selectLevel(ConsoleSaveFile* saveFile, const std::wstring& levelId, const std::wstring& levelName,
                     LevelSettings* settings);
    void saveSlot(int slot, const std::wstring& levelName);
    void loadSlot(const std::wstring& levelName, int slot);

    void connectTo(const std::wstring& server, int port);
    void handleClientSideCommand(const std::wstring& command);
    ClientPacketListener* getConnection(int playerIdx);
    void addPendingLocalConnection(int playerIdx, ClientPacketListener* connection);
    void clearConnectionFailed();

    bool addLocalPlayer(int playerIdx);
    void createPrimaryLocalPlayer(int playerIdx);
    boost::shared_ptr<MultiplayerLocalPlayer> createExtraLocalPlayer(int playerIdx, const std::wstring& name,
                                                                     int userIndex, int dimension,
                                                                     ClientPacketListener* connection,
                                                                     MultiPlayerLevel* level, int gameTypeId,
                                                                     int gameModeUserIndex);
    void storeExtraLocalPlayer(int playerIdx);
    void removeLocalPlayerIdx(int playerIdx);
    bool setLocalPlayerIdx(int playerIdx);
    int getLocalPlayerIdx();
    void respawnPlayer(int playerIdx, int dimension, int entityId, bool alive);
    void checkViewportAssignment(int playerCount);
    void updatePlayerViewportAssignments();

    void playerStartedTutorial(int playerIdx);
    void playerLeftTutorial(int playerIdx);

    void forceStatsSave(int playerIdx);
    void forceLeaderboardSave(int playerIdx);

    bool addPendingClientTextureRequest(const std::wstring& textureName);
    void handleClientTextureReceived(const std::wstring& textureName);
    unsigned int getCurrentTexturePackId();

    boost::shared_ptr<Entity> getCameraEntity();
    void setCameraEntity(boost::shared_ptr<Entity> entity);
    boost::shared_ptr<AbstractClientPlayer> GetLocalPlayerByName(const std::wstring& name);
    boost::shared_ptr<Player> GetPlayerByPlayerIndex(int playerIndex);

    int GetSpectateMode();
    unsigned long long GetSpectateEntity();

    const MiniGameDef* GetMiniGame();
    const MiniGameDef* GetFirstMiniGame();
    void SetupMiniGameInstance(const MiniGameDef& miniGame, int variant);
    ClientMasterGameMode* GetClientMasterGameMode();

    GhostController* GetGhostController();
    void SetGhostController(GhostController* ghostController);

    void SetJoiningFromFriendsTabOrInvite(bool joining);
    bool IsJoiningFromFriendsTabOrInvite() const;

    DataFixerUpper* getFixerUpper();
    ColourTable* getColourTable();
    MaterialTable* getMaterialTable();
    StructureManager* getStructureManager();
    Textures* getTextureManager() { return mTextures; }
    FrameTimer* getFrameTimer() { return mFrameTimer; }
    float getFrameTime();
    BlockRenderDispatcher* getBlockRenderer();
    ItemRenderer* getItemRenderer();
    ItemInHandRenderer* getItemInHandRenderer();
    EntityRenderDispatcher* getEntityRenderDispatcher();
    BlockColors* getBlockColors();
    ItemColors* getItemColors();

    DataFixerUpper* mFixerUpper;
    MultiPlayerGameMode* mGameMode;
    bool mFullscreen;
    bool mHasCrashed;
    int mMissTime;
    int mWidth;
    int mHeight;
    int mFullWidth;
    int mFullHeight;
    Timer* mTimer;
    bool mReloadTextures;
    int mPendingLevelChange;
    MultiPlayerLevel* mLevel;
    LevelRenderer* mLevelRenderer;
    boost::shared_ptr<MultiplayerLocalPlayer> mPlayer;
    arrayWithLength<MultiPlayerLevel*> mLevels;
    boost::shared_ptr<MultiplayerLocalPlayer> mLocalPlayers[4];
    MultiPlayerGameMode* mGameModes[4];
    int mLocalPlayerIdx;
    ItemInHandRenderer* mLocalItemInHandRenderers[4];
    int mConnectionStartTime[4];
    bool mConnectionFailed[4];
    DisconnectPacket::eDisconnectReason mDisconnectReasons[4];
    ClientPacketListener* mPendingConnections[4];
    int mEmptyViewport;
    boost::shared_ptr<Entity> mCameraEntity;
    boost::shared_ptr<Entity> mSpectateEntity;
    ParticleEngine* mParticleEngine;
    User* mUser;
    std::wstring mServer;
    int mServerPort;
    bool mPaused;
    bool mAppPaused;
    Textures* mTextures;
    Font* mFont;
    Font* mAlternateFont;
    Screen* mScreen;
    ProgressRenderer* mProgressRenderer;
    GameRenderer* mGameRenderer;
    EntityRenderDispatcher* mEntityRenderDispatcher;
    ItemInHandRenderer* mItemInHandRenderer;
    BlockColors* mBlockColors;
    ItemColors* mItemColors;
    TextureAtlas* mTerrainAtlas;
    ItemRenderer* mItemRenderer;
    BlockRenderDispatcher* mBlockRenderer;
    int mRightClickDelay;
    int mTicks;
    int mLastClickTick;
    int mDefaultHeight;
    Gui* mGui;
    bool mNoRender;
    HitResult* mHitResult;
    Options* mOptions;
    int mMouseGrabTicks;
    SoundEngine* mSoundEngine;
    int mSoundReloadDelay;
    TexturePackRepository* mTexturePackRepository;
    File mWorkingDirectory;
    LevelStorageSource* mLevelSource;
    StatsCounter* mStats[4];
    std::wstring mLevelName;
    int mLevelId;
    bool mRunning;
    std::wstring mLevelDirName;
    long long mLastRenderTime;
    int mFrameNumber;
    FrameTimer* mFrameTimer;
    long long mLastFrameTime;
    OSFastMutex mLevelLock;
    unsigned char mTutorialPlayers;
    Level* mAnimateTickLevel;
    std::vector<std::wstring> mPendingClientTextureRequests;
    std::vector<std::wstring> mReceivedClientTextures;
    const MiniGameDef* mMiniGame;
    ClientMasterGameMode* mClientMasterGameMode;
    bool mJoiningFromFriendsTabOrInvite;
    GhostController* mGhostController;
    std::list<GhostController*> mFinishedGhostControllers;
};

#endif
