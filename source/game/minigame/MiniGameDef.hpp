#ifndef GAME_MINIGAME_MINIGAMEDEF_HPP
#define GAME_MINIGAME_MINIGAMEDEF_HPP

#include <deque>
#include <map>
#include <string>
#include <utility>
#include <vector>
#include <boost/shared_ptr.hpp>
#include <boost/unordered_map.hpp>
#include "game/core/EquipmentSlot.hpp"
#include "game/core/maybe.hpp"
#include "game/minigame/EMiniGameId.hpp"
#include "game/item/not_null_ptr.hpp"
#include "game/world/biome/arrayWithLength.hpp"
#include "game/world/generation/MiniGamePlayerSettings.hpp"

class XmlObject;
class GameCondition;
class LaunchMoreOptionsMenuInitData;
class ItemInstance;
class HtmlString;
class Container;
class ServerPlayer;
class ArchiveFile;
class ClientboundMGPlayerSettingsUpdatePacket;
class Packet;
class WeighedTreasure;
enum EGameModeSound
{
    EGameModeSound_OnShowdown,
    EGameModeSound_OnRoundStart,
    EGameModeSound_OnRoundPlay,
    EGameModeSound_OnRoundEnding,
    EGameModeSound_OnChestRefill,
    EGameModeSound_OnDeath,
    EGameModeSound_LobbyTimer,
    EGameModeSound_RoundStartTimer,
    EGameModeSound_GraceTimer,
    EGameModeSound_RoundLimitTimer,
    EGameModeSound_OnFalling
};
enum EBuildMode
{
    EBuildMode_NoRestrictions,
    EBuildMode_WhiteList,
    EBuildMode_BlackList
};
enum ESpectateMode
{
    eSpectateMode_HostOption = -1,
    eSpectateMode_Invisible,
    eSpectateMode_Bat,
    eSpectateMode_Parrot,
    eSpectateMode_Vex,
    eSpectateMode_FloatingHead
};

class GameType;

class MiniGameDef
{
public:
    static const std::wstring sPlayerInventoryLootGroup;

    enum ELootDistributionMethod
    {
        eLootDistributionMethod_Spread,
        eLootDistributionMethod_KitKPRandom,
        eLootDistributionMethod_KitKPGreedy,
        eLootDistributionMethod_Fixed,
        eLootDistributionMethod_WeightedRandom
    };
    struct BlockData
    {
        maybe<bool> mTickOverride;
        maybe<bool> mTickDefault;
        boost::unordered_map<int, bool> mTick;
        maybe<bool> mCollisionOverride;
        maybe<bool> mCollisionDefault;
        boost::unordered_map<int, bool> mCollision;
        BlockData();
    };
    struct ChestItemsInfo
    {
        not_null_ptr<ItemInstance> mItem;
        int mMinStackCount;
        int mMaxStackCount;
        int mMinCount;
        int mMaxCount;
        int mPlayersForMin;
        int mWeight;
        int mKpScore;
    };
    struct ChestKitInfo
    {
        std::vector<not_null_ptr<ItemInstance> > mItems;
        int mWeight;
        int mKpScore;
    };
    struct PlayerEquipmentInfo
    {
        maybe<not_null_ptr<ItemInstance> > mSlots[6];
    };
    struct ChestRefill
    {
        maybe<int> mCentralRefillChance;
        maybe<int> mOuterRefillChance;
        maybe<int> mHVRefillChance;
        maybe<int> mNumberOfChestsToRefill;
        maybe<int> mMinPlayerDistanceToEffectChance;
        maybe<int> mMaxPlayerDistanceToEffectChance;
        maybe<std::wstring> mCentralLootGroup;
        maybe<std::wstring> mOuterLootGroup;
        maybe<std::wstring> mHVLootGroup;
        ChestRefill();
    };
    struct ItemBalance
    {
        boost::unordered_map<int, double> mBaseDamage;
        boost::unordered_map<int, int> mProtection;
        boost::unordered_map<int, double> mDegradation;
        boost::unordered_map<int, int> mNutrition;
        boost::unordered_map<int, float> mSaturationFactor;
        boost::unordered_map<int, int> mPotionDuration;
        boost::unordered_map<int, int> mLingeringDuration;
    };
    struct FoodBalance
    {
        maybe<int> mMaxFood;
        maybe<float> mMaxSaturation;
        maybe<float> mStartSaturation;
        maybe<float> mSaturationFloor;
        maybe<float> mExhaustionDrop;
        maybe<int> mHealthTickCount;
        maybe<int> mHealthTickCountSaturated;
        maybe<int> mHealLevel;
        maybe<int> mStarveLevel;
        maybe<int> mSprintLevel;
        maybe<float> mExhaustionHeal;
        maybe<float> mExhaustionJump;
        maybe<float> mExhaustionSprintJump;
        maybe<float> mExhaustionMine;
        maybe<float> mExhaustionAttack;
        maybe<float> mExhaustionDamage;
        maybe<float> mExhaustionSneak;
        maybe<float> mExhaustionWalk;
        maybe<float> mExhaustionSprint;
        maybe<float> mExhaustionSwim;
        maybe<float> mExhaustionIdle;
        FoodBalance();
        FoodBalance(const FoodBalance& other);
    };
    struct SoundSpec
    {
        maybe<int> mForLast;
        maybe<float> mVolume;
        SoundSpec();
        SoundSpec(const XmlObject& object);
        unsigned int ForLast() const;
        float Volume() const;
    };
    struct TimerSpec
    {
        maybe<int> mDuration;
        maybe<int> mAnnounceEvery;
        maybe<int> mChangeAnnounceAfter;
        maybe<int> mChangeAnnounceWhenRemaining;
        maybe<int> mChangedAnnounce;
        TimerSpec();
        TimerSpec(const XmlObject& object);
        void SetDurationS(int value);
        unsigned int TotalDurationS() const;
        int TotalDurationMs() const;
        unsigned int AnnounceEveryS(bool isFinal) const;
        int AnnounceEveryMs(bool isFinal) const;
        int ChangeAnnounceAfterS() const;
        int ChangeAnnounceAfterMs() const;
        void SetChangeAnnounceAfterS(int value);
    };
    struct ModeVariationLootGroup;
    struct LootSetDef
    {
        boost::unordered_map<std::wstring, std::wstring> mAttributes;
        boost::unordered_map<std::wstring, ModeVariationLootGroup> mGroups;
        LootSetDef();
        const ModeVariationLootGroup* getLootGroup(const std::wstring& name) const;
        std::wstring getAttribute(const std::wstring& group, std::wstring name) const;
    };
    struct ModeVariationLootGroup : public std::vector<ChestItemsInfo>
    {
        std::vector<ChestKitInfo> mKits;
        PlayerEquipmentInfo mPlayerEquipment;
        int mChestKpScore;
        ELootDistributionMethod mDistributionMethod;
        ModeVariationLootGroup();
        ModeVariationLootGroup(const ModeVariationLootGroup& other);
        std::vector<not_null_ptr<ItemInstance> >* GetChestItems() const;
        const std::vector<ChestItemsInfo>& GetAllChestItems() const;
        const std::vector<ChestKitInfo>& GetAllKits() const;
        unsigned int GetChestKPScore() const;
        arrayWithLength<WeighedTreasure*> GetWeightedChestItems() const;
        const PlayerEquipmentInfo& GetPlayerEquipment() const;
    };

    enum EGlideVariant
    {
        eGlideVariant_TimeAttack,
        eGlideVariant_ScoreAttack
    };

    MiniGameDef(const XmlObject& object);
    MiniGameDef(EMiniGameId id = eMiniGameId_Lobby);
    unsigned int GetHostOption(eCustomGameModeHostOption option) const;
    unsigned int GetRoundCount() const;
    int GetFinalRoundIndex() const;
    unsigned int GetGameVariant() const;
    bool HostOption_Bool(eCustomGameModeHostOption option, unsigned int index) const;
    bool HostOption_Bool(eCustomGameModeHostOption option) const;
    bool ArePointsActive() const;
    bool ArePowerupsActive() const;
    bool HasFeatureSupportForInfiniteLives() const;
    static maybe<int> parseTimer(std::wstring name);
    static maybe<EGameModeSound> parseSound(std::wstring name);
    void parseFoodBalance(const XmlObject& object, FoodBalance* parent);
    void parsePermissions(const XmlObject& object);
    void parseChestRefill(const XmlObject& object);
    static maybe<ELootDistributionMethod> parseDistributionMethod(std::wstring name);
    static std::deque<not_null_ptr<ItemInstance> > parseItemInstance(const XmlObject& object, int* slot);
    static void parseKit(std::vector<ChestKitInfo>* kits, const XmlObject& object);
    static void parseEquipment(PlayerEquipmentInfo& equipment, const XmlObject& object);
    static void parseChestItem(std::vector<ChestItemsInfo>* items, const XmlObject& object);
    void parseItemOverrideMap(int itemId, const XmlObject& object);
    void parseMatchmakingFilter(const XmlObject& object);
    void parseHostOptionSettings(const XmlObject& object);
    static unsigned int GetOptionType(eCustomGameModeHostOption option);
    unsigned int GetOptionsDefault(eCustomGameModeHostOption option) const;
    static MiniGameDef* parseMiniGameDef(XmlObject* object);
    static void MakeCRCTable();
    static unsigned int UpdateCRC(unsigned int crc, unsigned char* data, int length);
    static unsigned int CRC(unsigned char* data, int length);
    void setCRC(unsigned int crc);
    static MiniGameDef* parseMiniGameDef(arrayWithLength<unsigned char> data);
    static MiniGameDef* parseMiniGameDef(std::wstring fileName);
    EMiniGameId GetId() const;
    void cleanUp();
    const LootSetDef& getCurrentLootSet() const;
    bool HasLootGroup(const std::wstring& group) const;
    bool PopulateContainers(const std::vector<boost::shared_ptr<Container> >* containers,
                            const std::wstring& group) const;
    unsigned int EquipPlayers(std::vector<boost::shared_ptr<ServerPlayer> >* players) const;
    bool IsCustomGame() const;
    static bool IsCustomGame(EMiniGameId id, const MiniGamePlayerSettings& settings);
    int GetOptionsCount(eCustomGameModeHostOption option) const;
    unsigned int GetGameVariantCount() const;
    int GetLootSetCount() const;
    int GetLootSetId() const;
    void SetLootSet(int id) const;
    std::wstring GetLootSetText(int id) const;
    std::wstring GetLootSetText() const;
    std::wstring GetLootSetDescription(int id) const;
    MiniGamePlayerSettings GetDefaultPlayerSettings() const;
    boost::shared_ptr<Packet> GetPlayerSettingsPacket() const;
    std::wstring getVersionString() const;
    bool IsCRCValid() const;
    bool HasPermission(EBuildMode mode, const std::vector<int>& permissions, int value) const;
    bool MayPlace(int id) const;
    bool MayDestroy(int id) const;
    bool MayUse(int id) const;
    unsigned int MayUseBlock(int id) const;
    bool CanAutoStart() const;
    bool ShowInvulnerableEffect() const;
    bool LockedInventory() const;
    bool ShowdownAnnouncements() const;
    bool AllowPvP() const;
    bool GetTutorialsEnabled() const;
    bool GetShowNametags() const;
    double GetDamageIndicatorDelay() const;
    bool LoadOnlyActiveAreaChunks() const;
    unsigned int GetLivesPerRound() const;
    unsigned int GetTeamCount() const;
    bool IsTeamBasedSpawn() const;
    bool IsCheckpointBasedSpawn() const;
    bool HasSpectatorParticipation() const;
    bool IsSmallInventoryEnabled() const;
    unsigned int GetMaxPlayersForSmallMaps() const;
    bool IsTakeEverythingEnabled() const;
    bool IsChestRefillEnabled() const;
    bool ForceSpectateModeFollow() const;
    bool SupressGameModeMessages() const;
    const FoodBalance* GetFoodBalance() const;
    unsigned int getMaxFood(int index) const;
    float getMaxSaturation(float value) const;
    float getStartSaturation(float value) const;
    float getSaturationFloor(float value) const;
    float getExhaustionDrop(float value) const;
    unsigned int getHealthTickCount(int index) const;
    unsigned int getHealthTickCountSaturated(int index) const;
    unsigned int getHealLevel(int index) const;
    unsigned int getStarveLevel(int index) const;
    unsigned int getSprintLevel(int index) const;
    float getExhaustionHeal(float value) const;
    float getExhaustionJump(float value) const;
    float getExhaustionSprintJump(float value) const;
    float getExhaustionMine(float value) const;
    float getExhaustionAttack(float value) const;
    float getExhaustionDamage(float value) const;
    float getExhaustionWalk(float value) const;
    float getExhaustionSprint(float value) const;
    float getExhaustionSneak(float value) const;
    float getExhaustionSwim(float value) const;
    float getExhaustionIdle(float value) const;
    int getCentralRefillChance() const;
    int getOuterRefillChance() const;
    int getHVRefillChance() const;
    int getNumberOfChestsToRefill() const;
    int getMinPlayerDistanceToEffectChance() const;
    int getMaxPlayerDistanceToEffectChance() const;
    std::wstring getCentralLootGroup() const;
    std::wstring getOuterLootGroup() const;
    std::wstring getHVLootGroup() const;
    not_null_ptr<ItemInstance> getEquipmentForSlot(EquipmentSlot::EIndex slot) const;
    GameCondition* GetStartConditions() const;
    GameCondition* GetEndConditions() const;
    GameCondition* GetPostGameConditions() const;
    GameCondition* GetShowdownConditions() const;
    const std::vector<int>& GetOptionsList(eCustomGameModeHostOption option) const;
    unsigned int HostOption_Category(eCustomGameModeHostOption option, unsigned int index) const;
    unsigned int HostOption_Category(eCustomGameModeHostOption option) const;
    int GetSpectateMode() const;
    unsigned long long GetSpectateEntity() const;
    maybe<int> getDifficulty() const;
    const GameType* getPlayerGameType() const;
    Abilities getPlayerAbilities() const;
    TimerSpec getTimerSpec(int timer) const;
    SoundSpec getSoundSpec(EGameModeSound sound) const;
    double getBaseDamage(int itemId, double defaultValue) const;
    int getNutrition(int itemId, int defaultValue) const;
    float getSaturationModifier(int itemId, float defaultValue) const;
    int getPotionDuration(int effectId, int defaultValue) const;
    int getLingeringDuration(int effectId, int defaultValue) const;
    double getThrowPower(unsigned long long entityType, double defaultValue) const;
    double getThrowAngle(unsigned long long entityType, double defaultValue) const;
    bool shouldTickBlocks() const;
    bool shouldTickBlock(int blockId) const;
    bool canLavaConvertWater() const;
    bool canTntDestroyBlocks() const;
    unsigned char CurrentHostOptionVersion() const;
    unsigned int HostOption_Time(eCustomGameModeHostOption option, unsigned int index) const;
    std::wstring GetValueText(eCustomGameModeHostOption option, int value) const;
    static std::wstring getComparisonImageForGameMode(EMiniGameId id);
    static std::wstring GetVariantIconPath(EMiniGameId id, unsigned int variant);
    void SetHostOption(eCustomGameModeHostOption option, unsigned int value) const;
    unsigned int HostOption_Time(eCustomGameModeHostOption option) const;
    int GetHostOptionIndexFrom_Time(eCustomGameModeHostOption option, int value) const;
    int GetHostOptionIndexFrom_Category(eCustomGameModeHostOption option, int value) const;
    unsigned int GetHostOptionIndexFrom_Bool(eCustomGameModeHostOption option, bool value) const;
    unsigned int GetHostOptionIndexFromValue(eCustomGameModeHostOption option, int value) const;
    unsigned int GetOptionMin(eCustomGameModeHostOption option, bool includeRandom) const;
    int GetOptionMax(eCustomGameModeHostOption option) const;
    unsigned int AllowSmallSkins() const;
    std::wstring GetFoodBalanceIndexText(int index) const;
    std::wstring GetFoodBalanceIndexDescription(int index) const;
    void SetDisabledEditActions(bool disabled) const;
    bool EditActionsDisabled() const;
    int GetItemRespawnTime() const;
    unsigned int IsShowdownEnabled() const;
    bool HasMultipleRounds() const;
    bool HasMultipleMaps() const;
    bool HasMultipleVariants() const;
    unsigned int VotableType() const;
    bool HasRoundLengthOption() const;
    bool HasGameVariants() const;
    bool HasCentralSpawnOption() const;
    bool HasTeamGameOption() const;
    bool IsHUDDisabled() const;
    bool ArePortalsDisabled() const;
    bool IsBatSqueakDisabled() const;
    unsigned int GetLayerCount() const;
    double GetLayerScale(float value) const;
    unsigned int GetMapSizeOverride() const;
    bool HasFeatureLookBack() const;
    bool HasFeatureStartCameraAnimation() const;
    bool HasFeatureFinishLineSoundEffect() const;
    bool HasFeatureCheckpoints() const;
    bool HasFeatureClouds() const;
    bool HasFeatureCustomBeacons() const;
    bool HasFeatureDirectionalFireworks() const;
    bool HasFeatureFairExplosions() const;
    bool HasFeatureReducedKnockback() const;
    bool HasFeatureArrowCanKnockback() const;
    bool HasFeatureSplashLevitatePotions() const;
    bool HasFeatureShorterHitbox() const;
    bool HasFeatureGhostThroughProjectiles() const;
    bool HasFeatureProjectilesBreakBlocks() const;
    bool HasFeatureThrownProjectilesAreFireImmune() const;
    bool HasFeatureAutoElytraFly() const;
    bool HasFeatureInvertedExplodingGrenades() const;
    bool HasFeatureLessExplosionParticles() const;
    bool HasFeatureElytraFlyingOnSpawn() const;
    bool HasFeatureOverrideHandSwing() const;
    bool HasFeatureReducedParticles() const;
    bool HasFeatureSnowballAwards() const;
    bool AllowsUGC() const;
    bool CanCraft() const;
    static void addWeightedChestItem(std::vector<ChestItemsInfo>* items, not_null_ptr<ItemInstance> item,
                                     unsigned int minCount, unsigned int maxCount, unsigned int weight);
    static void addChestItem(std::vector<ChestItemsInfo>* items, not_null_ptr<ItemInstance> item,
                             unsigned int minStackCount, unsigned int maxStackCount, unsigned int minCount,
                             unsigned int maxCount, unsigned int playersForMin);
    unsigned int GetName() const;
    static unsigned int GetHostOptionStringId(eCustomGameModeHostOption option);
    void AddLabelForOption(std::vector<HtmlString>& labels, eCustomGameModeHostOption option, unsigned int value) const;
    void GetGameSettingsDescription(std::vector<HtmlString>& labels, unsigned int flags) const;
    void SetDefaultMatchmakingSearchData(bool joining, LaunchMoreOptionsMenuInitData* data) const;
    bool HasFeatureSeasonalMaps() const;
    bool HasFeatureKeepInventory() const;
    void SetGameSettingsFromMatchmakingFilter(LaunchMoreOptionsMenuInitData* data) const;
    unsigned int CalculateMatchmakingData(bool joining, LaunchMoreOptionsMenuInitData* data, unsigned int** values,
                                          unsigned int** ids) const;
    unsigned int CalculateJoiningMatchmakingData(LaunchMoreOptionsMenuInitData* data, unsigned int** values,
                                                 unsigned int** ids) const;
    unsigned int CalculateHostingMatchmakingData(LaunchMoreOptionsMenuInitData* data, unsigned int** values) const;
    std::wstring GetMapSizeOverrideText(int value) const;
    std::wstring getTextureName() const;
    std::wstring getIcon() const;
    static void SetPlayerSettings(const MiniGamePlayerSettings& settings);
    static void GetPlayerSettings(MiniGamePlayerSettings& settings);
    static MiniGameDef* GetCustomGameModeReferenceById(EMiniGameId id);
    static void StaticCtor();
    static MiniGameDef* GetCustomGameModeById(EMiniGameId id, bool active);
    static void ValidateMaxPlayersOptions();
    bool isCurrentVersion() const;
    static void ReplaceCustomGameModeById(EMiniGameId id, arrayWithLength<unsigned char> data, unsigned int crc);
    static void UpdateServerSideMiniGameInfo(ArchiveFile* file, const std::wstring& name, EMiniGameId id,
                                             unsigned int crc);
    static void UpdateServerSideMiniGameInfo(ArchiveFile* file);
    static bool ValidMiniGameId(EMiniGameId id);
    void getVersion(int& major, int& minor) const;
    int getCRC() const;
    bool AllowCrafting() const;
    bool GetNaturalRegeneration() const;
    int GetTeamMaxSize() const;
    int getProtection(int a, int b) const;
    int getComparisonImageForGameMode() const;
    bool HasHostOption(eCustomGameModeHostOption option) const;
    unsigned int HostOption_Category(unsigned int index, eCustomGameModeHostOption option) const;
    int GetFoodBalanceCount() const;
    bool CanUseSmallSkins() const;
    void SetSmallSkinsEnabled(bool enabled) const;
    int GetRoundLength() const;
    bool GetTeamGame() const;
    bool HasMaxPlayerOption() const;
    bool HasFeatureAlwaysSpawnOnPlatform() const;

private:
    template <class T>
    static T atMaybe(const boost::unordered_map<int, T>& values, int key, T defaultValue);

    unsigned char mHostOptionVersion;
    boost::unordered_map<int, int> mOptionDefaults;
    boost::unordered_map<int, std::vector<int> > mOptionLists;
    std::vector<boost::unordered_map<int, boost::unordered_map<int, int> > > mMatchmakingFilters;
    std::vector<int> mMatchmakingFilterDefaults;
    MiniGamePlayerSettings mPlayerSettings;
    boost::unordered_map<int, TimerSpec> mTimers;
    boost::unordered_map<int, SoundSpec> mSounds;
    EMiniGameId mId;
    GameCondition* mStartConditions;
    GameCondition* mEndConditions;
    GameCondition* mPostGameConditions;
    GameCondition* mShowdownConditions;
    unsigned short mVersionMajor;
    unsigned short mVersionMinor;
    unsigned int mCrc;
    bool mCanAutoStart;
    bool mLoadOnlyActiveAreaChunks;
    bool mEditActionsDisabled;
    maybe<int> mDifficulty;
    maybe<int> mPlayerGameType;
    int mSpectateMode;
    unsigned long long mSpectateEntity;
    bool mAllowCrafting;
    bool mShowInvulnerableEffect;
    bool mLockedInventory;
    bool mShowdownAnnouncements;
    bool mAllowPvP;
    bool mTutorialsEnabled;
    float mDamageIndicatorDelay;
    maybe<bool> mCanLavaConvertWater;
    EBuildMode mPlaceMode;
    EBuildMode mDestroyMode;
    EBuildMode mUseMode;
    EBuildMode mUseBlockMode;
    std::vector<int> mPlacePermissions;
    std::vector<int> mDestroyPermissions;
    std::vector<int> mUsePermissions;
    std::vector<int> mBlockUsePermissions;
    bool mAllowsUGC;
    unsigned char mMaxPlayersForSmallMaps;
    boost::unordered_map<int, FoodBalance*> mFoodBalances;
    ChestRefill mChestRefill;
    BlockData mBlockData;
    ItemBalance mItemBalance;
    std::vector<LootSetDef> mLootSets;
};

#endif
