#ifndef GAME_DATA_GAMERULESAVEINTERFACE_HPP
#define GAME_DATA_GAMERULESAVEINTERFACE_HPP

#include <deque>
#include <string>

class ConsoleGameRules
{
public:
    enum EGameRuleType
    {
        eGameRuleType_Root = 0,
        eGameRuleType_MapOptions = 1,
        eGameRuleType_ApplySchematic = 2,
        eGameRuleType_GenerateStructure = 3,
        eGameRuleType_GenerateBox = 4,
        eGameRuleType_PlaceBlock = 5,
        eGameRuleType_PlaceContainer = 6,
        eGameRuleType_PlaceSpawner = 7,
        eGameRuleType_BiomeOverride = 8,
        eGameRuleType_StartFeature = 9,
        eGameRuleType_AddItem = 10,
        eGameRuleType_AddEnchantment = 11,
        eGameRuleType_WeighedTreasureItem = 12,
        eGameRuleType_RandomItemSet = 13,
        eGameRuleType_DistributeItems = 14,
        eGameRuleType_WorldPosition = 15,
        eGameRuleType_LevelRules = 16,
        eGameRuleType_NamedArea = 17,
        eGameRuleType_ActiveChunkArea = 18,
        eGameRuleType_TargetArea = 19,
        eGameRuleType_ScoreRing = 20,
        eGameRuleType_ThermalArea = 21,
        eGameRuleType_PlayerBoundsVolume = 22,
        eGameRuleType_Killbox = 23,
        eGameRuleType_BlockLayer = 24,
        eGameRuleType_UseBlock = 25,
        eGameRuleType_CollectItem = 26,
        eGameRuleType_CompleteAll = 27,
        eGameRuleType_UpdatePlayer = 28,
        eGameRuleType_OnGameStartSpawnPositions = 29,
        eGameRuleType_OnInitialiseWorld = 30,
        eGameRuleType_SpawnPositionSet = 31,
        eGameRuleType_PopulateContainer = 32,
        eGameRuleType_DegradationSequence = 33,
        eGameRuleType_RandomDissolveDegrade = 34,
        eGameRuleType_DirectionalDegrade = 35,
        eGameRuleType_GrantPermissions = 36,
        eGameRuleType_AllowIn = 37,
        eGameRuleType_LayerGeneration = 38,
        eGameRuleType_LayerAsset = 39,
        eGameRuleType_AnyCombinationOf = 40,
        eGameRuleType_CombinationDefinition = 41,
        eGameRuleType_Variations = 42,
        eGameRuleType_BlockDef = 43,
        eGameRuleType_LayerSize = 44,
        eGameRuleType_UniformSize = 45,
        eGameRuleType_RandomizeSize = 46,
        eGameRuleType_LinearBlendSize = 47,
        eGameRuleType_LayerShape = 48,
        eGameRuleType_BasicShape = 49,
        eGameRuleType_StarShape = 50,
        eGameRuleType_PatchyShape = 51,
        eGameRuleType_RingShape = 52,
        eGameRuleType_SpiralShape = 53,
        eGameRuleType_LayerFill = 54,
        eGameRuleType_BasicLayerFill = 55,
        eGameRuleType_CurvedLayerFill = 56,
        eGameRuleType_WarpedLayerFill = 57,
        eGameRuleType_LayerTheme = 58,
        eGameRuleType_NullTheme = 59,
        eGameRuleType_FilterTheme = 60,
        eGameRuleType_ShaftsTheme = 61,
        eGameRuleType_BasicPatchesTheme = 62,
        eGameRuleType_BlockStackTheme = 63,
        eGameRuleType_RainbowTheme = 64,
        eGameRuleType_TerracottaTheme = 65,
        eGameRuleType_FunctionPatchesTheme = 66,
        eGameRuleType_SimplePatchesTheme = 67,
        eGameRuleType_CarpetTrapTheme = 68,
        eGameRuleType_MushroomBlockTheme = 69,
        eGameRuleType_TextureTheme = 70,
        eGameRuleType_SchematicTheme = 71,
        eGameRuleType_BlockCollisionException = 72,
        eGameRuleType_Powerup = 73,
        eGameRuleType_Checkpoint = 74,
        eGameRuleType_CustomBeacon = 75,
        eGameRuleType_ActiveViewArea = 76,
        eGameRuleType_Count = 77
    };

    enum EGameRuleAttr
    {
        eGameRuleAttr_descriptionName = 0,
        eGameRuleAttr_promptName = 1,
        eGameRuleAttr_dataTag = 2,
        eGameRuleAttr_enchantmentId = 3,
        eGameRuleAttr_enchantmentLevel = 4,
        eGameRuleAttr_itemId = 5,
        eGameRuleAttr_quantity = 6,
        eGameRuleAttr_auxValue = 7,
        eGameRuleAttr_slot = 8,
        eGameRuleAttr_name = 9,
        eGameRuleAttr_food = 10,
        eGameRuleAttr_health = 11,
        eGameRuleAttr_blockId = 12,
        eGameRuleAttr_useCoords = 13,
        eGameRuleAttr_seed = 14,
        eGameRuleAttr_flatworld = 15,
        eGameRuleAttr_filename = 16,
        eGameRuleAttr_rot = 17,
        eGameRuleAttr_data = 18,
        eGameRuleAttr_block = 19,
        eGameRuleAttr_entity = 20,
        eGameRuleAttr_facing = 21,
        eGameRuleAttr_edgeBlock = 22,
        eGameRuleAttr_fillBlock = 23,
        eGameRuleAttr_skipAir = 24,
        eGameRuleAttr_x = 25,
        eGameRuleAttr_x0 = 26,
        eGameRuleAttr_x1 = 27,
        eGameRuleAttr_y = 28,
        eGameRuleAttr_y0 = 29,
        eGameRuleAttr_y1 = 30,
        eGameRuleAttr_z = 31,
        eGameRuleAttr_z0 = 32,
        eGameRuleAttr_z1 = 33,
        eGameRuleAttr_chunkX = 34,
        eGameRuleAttr_chunkZ = 35,
        eGameRuleAttr_yRot = 36,
        eGameRuleAttr_xRot = 37,
        eGameRuleAttr_spawnX = 38,
        eGameRuleAttr_spawnY = 39,
        eGameRuleAttr_spawnZ = 40,
        eGameRuleAttr_orientation = 41,
        eGameRuleAttr_dimension = 42,
        eGameRuleAttr_topblockId = 43,
        eGameRuleAttr_biomeId = 44,
        eGameRuleAttr_feature = 45,
        eGameRuleAttr_minCount = 46,
        eGameRuleAttr_maxCount = 47,
        eGameRuleAttr_weight = 48,
        eGameRuleAttr_id = 49,
        eGameRuleAttr_probability = 50,
        eGameRuleAttr_method = 51,
        eGameRuleAttr_hasBeenInCreative = 52,
        eGameRuleAttr_cloudHeight = 53,
        eGameRuleAttr_fogDistance = 54,
        eGameRuleAttr_dayTime = 55,
        eGameRuleAttr_target = 56,
        eGameRuleAttr_speed = 57,
        eGameRuleAttr_dir = 58,
        eGameRuleAttr_type = 59,
        eGameRuleAttr_pass = 60,
        eGameRuleAttr_for = 61,
        eGameRuleAttr_random = 62,
        eGameRuleAttr_blockAux = 63,
        eGameRuleAttr_size = 64,
        eGameRuleAttr_scale = 65,
        eGameRuleAttr_freq = 66,
        eGameRuleAttr_func = 67,
        eGameRuleAttr_upper = 68,
        eGameRuleAttr_lower = 69,
        eGameRuleAttr_dY = 70,
        eGameRuleAttr_thickness = 71,
        eGameRuleAttr_points = 72,
        eGameRuleAttr_holeSize = 73,
        eGameRuleAttr_variant = 74,
        eGameRuleAttr_startHeight = 75,
        eGameRuleAttr_pattern = 76,
        eGameRuleAttr_colour = 77,
        eGameRuleAttr_primary = 78,
        eGameRuleAttr_laps = 79,
        eGameRuleAttr_liftForceModifier = 80,
        eGameRuleAttr_staticLift = 81,
        eGameRuleAttr_targetHeight = 82,
        eGameRuleAttr_speedBoost = 83,
        eGameRuleAttr_boostDirection = 84,
        eGameRuleAttr_condition_type = 85,
        eGameRuleAttr_condition_value_0 = 86,
        eGameRuleAttr_condition_value_1 = 87,
        eGameRuleAttr_beam_length = 88,
        eGameRuleAttr_beam_direction = 89,
        eGameRuleAttr_string_IDS = 90,
        eGameRuleAttr_Count = 91
    };
};

class GameRuleSaveInterface
{
public:
    virtual void startTag(ConsoleGameRules::EGameRuleType type);
    virtual void startAttributes(int attributeCount);
    virtual void saveAttribute(ConsoleGameRules::EGameRuleAttr attribute, const std::wstring& value);
    virtual void startChildren(int childCount);
    virtual void finishTag();

protected:
    struct Partials
    {
        int mDepth;
        ConsoleGameRules::EGameRuleType mType;
        int mAttributeCount;
        int mAttributesSaved;
        int mChildCount;
        int mChildrenFinished;
        bool mAttributesStarted;
        bool mChildrenStarted;
    };

    std::deque<Partials> mPartials;
};

#endif
