#include "platform/pc/PcShaderPacks.hpp"

#include <algorithm>
#include <cctype>
#include <cmath>
#include <cstdio>
#include <cstring>
#include <map>
#include <string>
#include <set>
#include <vector>

#include "game/client/render/GameRenderer.hpp"
#include "game/client/render/LevelRenderer.hpp"
#include "game/client/render/Renderer.hpp"
#include "game/client/render/Textures.hpp"
#include "game/core/FoodData.hpp"
#include "game/core/GameType.hpp"
#include "game/core/GlStateManager.hpp"
#include "game/core/HumanoidArm.hpp"
#include "game/core/Minecraft.hpp"
#include "game/core/Options.hpp"
#include "game/entity/mob/LivingEntity.hpp"
#include "game/item/Item.hpp"
#include "game/item/ItemInstance.hpp"
#include "game/entity/mob/MobEffects.hpp"
#include "game/util/Vec3.hpp"
#include "game/world/Dimension.hpp"
#include "game/world/DimensionType.hpp"
#include "game/world/Level.hpp"
#include "game/world/LevelSource.hpp"
#include "game/world/biome/Biome.hpp"
#include "game/world/block/Block.hpp"
#include "game/world/block/BlockEntity.hpp"
#include "game/world/block/BlockPos.hpp"
#include "game/world/block/Material.hpp"
#include "game/world/generation/MultiPlayerGameMode.hpp"
#include "game/world/generation/MultiplayerLocalPlayer.hpp"
#include "game/core/PIXNamedEvents.hpp"
#include "platform/pc/PcDisplay.hpp"
#include "platform/pc/PcGl.hpp"
#include "platform/pc/PcProfiler.hpp"
#include "platform/pc/PcSettings.hpp"
#include "platform/pc/shaderpack/ShaderBlockIds.hpp"
#include "platform/pc/shaderpack/ShaderPackLang.hpp"
#include "platform/pc/shaderpack/ShaderPackManager.hpp"
#include "platform/pc/shaderpack/ShaderPackOptions.hpp"
#include "platform/pc/shaderpack/ShaderPackProperties.hpp"
#include "platform/pc/shaderpack/ShaderPipeline.hpp"
#include "platform/pc/shaderpack/ShaderSettings.hpp"

namespace
{
std::string s_requestedPack;
bool s_packRequested = false;
bool s_managerReady = false;
bool s_frameEligible = false;
bool s_worldActive = false;
bool s_worldDrawn = false;
unsigned int s_framebuffer = 0;
int s_width = 0;
int s_height = 0;
bool s_optionsChanged = false;
int s_optionsVersion = 0;
ShaderIdMappings s_ids;
const ShaderPack* s_mappedPack = nullptr;
std::string s_mappedOptions;

std::string Lower(std::string text)
{
    std::transform(text.begin(), text.end(), text.begin(),
                   [](unsigned char character) { return static_cast<char>(std::tolower(character)); });
    return text;
}

std::string Narrow(const std::wstring& text)
{
    std::string narrow;
    narrow.reserve(text.size());
    for (wchar_t character : text)
        narrow += character < 0x80 ? static_cast<char>(character) : '?';
    return narrow;
}

GLuint ResolveResourceTexture(const std::string& name)
{
    Textures* textures = Minecraft::GetInstance()->getTextureManager();
    if (textures == nullptr)
        return 0;
    if (name == "atlas/blocks")
        return gRenderer->TextureGlName(textures->loadTexture(BLOCKS_LOCATION.getTexture()));
    static std::set<std::string> reported;
    if (reported.insert(name).second)
        std::printf("[pc] shaders: the pack asks for the game texture %s, which is not available to packs\n",
                    name.c_str());
    return 0;
}

class PassScope
{
public:
    explicit PassScope(const char* name)
    {
        if (PcProfilerSyncGpu())
            PcGl::Finish();
        PIXBeginNamedEvent(0.0f, name);
    }
    ~PassScope()
    {
        if (PcProfilerSyncGpu())
            PcGl::Finish();
        PIXEndNamedEvent();
    }
};

void RefreshIds()
{
    ShaderPackManager& manager = ShaderPackManager::Get();
    const ShaderPack* pack = manager.ActivePack();
    std::string options = manager.Options().SaveValues();
    if (pack == s_mappedPack && options == s_mappedOptions)
        return;
    s_mappedPack = pack;
    s_mappedOptions = options;
    ShaderBuildIdMappings(s_ids);
    if (!s_ids.blocks.empty())
        ShaderPipeline::Get().SetBlockMap(s_ids.blocks, s_ids.lights);
}

int ItemId(const not_null_ptr<ItemInstance>& item, int& light)
{
    light = 0;
    std::string name = "minecraft:air";
    if (!item->isEmpty() && item->getItem() != nullptr)
    {
        Item* definition = item->getItem();
        name = Lower(Narrow(Item::itemRegistry.getKey(definition).toString()));
        Block* block = Block::byItem(definition);
        if (block != nullptr)
            light = block->getLightEmission(block->defaultBlockState());
    }
    std::map<std::string, int>::const_iterator found = s_ids.items.find(name);
    return found != s_ids.items.end() ? found->second : -1;
}

int BiomeCategory(Biome* biome)
{
    const std::string name = Lower(Narrow(biome->mBiomeName));
    const char* const kCategories[][2] = {{"ocean", "11"},  {"river", "13"},  {"beach", "9"},  {"shore", "9"},
                                          {"mushroom", "15"}, {"hell", "16"},  {"nether", "16"}, {"sky", "8"},
                                          {"end", "8"},       {"ice", "7"},    {"frozen", "7"},  {"desert", "12"},
                                          {"mesa", "4"},      {"badlands", "4"}, {"savanna", "6"}, {"jungle", "3"},
                                          {"taiga", "1"},     {"hills", "2"},  {"mountain", "2"}, {"swamp", "14"},
                                          {"forest", "10"},   {"plains", "5"}};
    for (const auto& category : kCategories)
    {
        if (name.find(category[0]) != std::string::npos)
            return std::atoi(category[1]);
    }
    return 0;
}

const char* MODERN_BIOMES[] = {
    "ocean", "deep_ocean", "warm_ocean", "lukewarm_ocean", "deep_lukewarm_ocean", "cold_ocean", "deep_cold_ocean",
    "frozen_ocean", "deep_frozen_ocean", "mushroom_fields", "jagged_peaks", "frozen_peaks", "stony_peaks", "meadow",
    "cherry_grove", "grove", "snowy_slopes", "windswept_hills", "windswept_gravelly_hills", "windswept_forest",
    "forest", "flower_forest", "taiga", "old_growth_pine_taiga", "old_growth_spruce_taiga", "snowy_taiga",
    "birch_forest", "old_growth_birch_forest", "dark_forest", "pale_garden", "jungle", "sparse_jungle",
    "bamboo_jungle", "river", "frozen_river", "swamp", "mangrove_swamp", "beach", "snowy_beach", "stony_shore",
    "plains", "sunflower_plains", "snowy_plains", "ice_spikes", "desert", "savanna", "savanna_plateau",
    "windswept_savanna", "badlands", "wooded_badlands", "eroded_badlands", "deep_dark", "dripstone_caves",
    "lush_caves", "nether_wastes", "warped_forest", "crimson_forest", "soul_sand_valley", "basalt_deltas",
    "the_end", "end_highlands", "end_midlands", "small_end_islands", "end_barrens", "the_void"};
const int MODERN_BIOME_COUNT = (int)(sizeof(MODERN_BIOMES) / sizeof(MODERN_BIOMES[0]));

struct LceBiome
{
    Biome** biome;
    const char* modern;
};

LceBiome LCE_BIOMES[] = {
    {&Biome::OCEAN, "ocean"}, {&Biome::PLAINS, "plains"}, {&Biome::DESERT, "desert"},
    {&Biome::EXTREME_HILLS, "windswept_hills"}, {&Biome::FOREST, "forest"}, {&Biome::TAIGA, "taiga"},
    {&Biome::SWAMPLAND, "swamp"}, {&Biome::RIVER, "river"}, {&Biome::HELL, "nether_wastes"},
    {&Biome::THE_END, "the_end"}, {&Biome::LEGACY_FROZEN_OCEAN, "frozen_ocean"}, {&Biome::FROZEN_RIVER, "frozen_river"},
    {&Biome::ICE_PLAINS, "snowy_plains"}, {&Biome::ICE_MOUNTAINS, "snowy_plains"},
    {&Biome::MUSHROOM_ISLAND, "mushroom_fields"}, {&Biome::MUSHROOM_ISLAND_SHORE, "mushroom_fields"},
    {&Biome::BEACH, "beach"}, {&Biome::DESERT_HILLS, "desert"}, {&Biome::FOREST_HILLS, "forest"},
    {&Biome::TAIGA_HILLS, "taiga"}, {&Biome::EXTREME_HILLS_EDGE, "windswept_hills"}, {&Biome::JUNGLE, "jungle"},
    {&Biome::JUNGLE_HILLS, "jungle"}, {&Biome::JUNGLE_EDGE, "sparse_jungle"}, {&Biome::STONE_BEACH, "stony_shore"},
    {&Biome::COLD_BEACH, "snowy_beach"}, {&Biome::BIRCH_FOREST, "birch_forest"},
    {&Biome::BIRCH_FOREST_HILLS, "birch_forest"}, {&Biome::ROOFED_FOREST, "dark_forest"},
    {&Biome::COLD_TAIGA, "snowy_taiga"}, {&Biome::COLD_TAIGA_HILLS, "snowy_taiga"},
    {&Biome::MEGA_TAIGA, "old_growth_pine_taiga"}, {&Biome::MEGA_TAIGA_HILLS, "old_growth_pine_taiga"},
    {&Biome::EXTREME_HILLS_PLUS_TREES, "windswept_forest"}, {&Biome::SAVANNA, "savanna"},
    {&Biome::SAVANNA_PLATEAU, "savanna_plateau"}, {&Biome::MESA, "badlands"}, {&Biome::MESA_ROCK, "wooded_badlands"},
    {&Biome::MESA_CLEAR_ROCK, "badlands"}, {&Biome::DEEP_OCEAN, "deep_ocean"}, {&Biome::COLD_OCEAN, "cold_ocean"},
    {&Biome::DEEP_COLD_OCEAN, "deep_cold_ocean"}, {&Biome::FROZEN_OCEAN, "frozen_ocean"},
    {&Biome::DEEP_FROZEN_OCEAN, "deep_frozen_ocean"}, {&Biome::LUKEWARM_OCEAN, "lukewarm_ocean"},
    {&Biome::DEEP_LUKEWARM_OCEAN, "deep_lukewarm_ocean"}, {&Biome::WARM_OCEAN, "warm_ocean"},
    {&Biome::DEEP_WARM_OCEAN, "warm_ocean"}, {&Biome::THE_VOID, "the_void"},
    {&Biome::SUNFLOWER_PLAINS, "sunflower_plains"}, {&Biome::DESERT_MUTATED, "desert"},
    {&Biome::EXTREME_HILLS_MUTATED, "windswept_gravelly_hills"}, {&Biome::FLOWER_FOREST, "flower_forest"},
    {&Biome::TAIGA_MUTATED, "taiga"}, {&Biome::SWAMPLAND_MUTATED, "swamp"}, {&Biome::ICE_PLAINS_SPIKES, "ice_spikes"},
    {&Biome::JUNGLE_MUTATED, "jungle"}, {&Biome::JUNGLE_EDGE_MUTATED, "sparse_jungle"},
    {&Biome::BIRCH_FOREST_MUTATED, "old_growth_birch_forest"},
    {&Biome::BIRCH_FOREST_HILLS_MUTATED, "old_growth_birch_forest"}, {&Biome::ROOFED_FOREST_MUTATED, "dark_forest"},
    {&Biome::COLD_TAIGA_MUTATED, "snowy_taiga"}, {&Biome::MEGA_SPRUCE_TAIGA, "old_growth_spruce_taiga"},
    {&Biome::REDWOOD_TAIGA_HILLS_MUTATED, "old_growth_spruce_taiga"},
    {&Biome::EXTREME_HILLS_PLUS_TREES_MUTATED, "windswept_gravelly_hills"},
    {&Biome::SAVANNA_MUTATED, "windswept_savanna"}, {&Biome::SAVANNA_PLATEAU_MUTATED, "windswept_savanna"},
    {&Biome::MESA_BRYCE, "eroded_badlands"}, {&Biome::MESA_ROCK_MUTATED, "wooded_badlands"},
    {&Biome::MESA_CLEAR_ROCK_MUTATED, "badlands"}};
const int LCE_BIOME_COUNT = (int)(sizeof(LCE_BIOMES) / sizeof(LCE_BIOMES[0]));

int ModernBiomeId(const char* name)
{
    for (int i = 0; i < MODERN_BIOME_COUNT; i++)
    {
        if (strcmp(MODERN_BIOMES[i], name) == 0)
            return i;
    }
    return -1;
}

int BiomeUniform(Biome* biome)
{
    for (int i = 0; i < LCE_BIOME_COUNT; i++)
    {
        if (*LCE_BIOMES[i].biome == biome)
            return ModernBiomeId(LCE_BIOMES[i].modern);
    }
    return -1;
}

void AddBiomeNames()
{
    std::map<std::string, float> constants;
    for (int i = 0; i < MODERN_BIOME_COUNT; i++)
    {
        std::string name = std::string("BIOME_") + MODERN_BIOMES[i];
        for (size_t c = 0; c < name.size(); c++)
            name[c] = (char)std::toupper((unsigned char)name[c]);
        constants[name] = (float)i;
        gShaderExtraMacros[name] = std::to_string(i);
    }
    ShaderPipeline::Get().SetBiomeConstants(constants);
}

void EnsureManager()
{
    if (s_managerReady)
        return;
    s_managerReady = true;
    AddBiomeNames();
    ShaderPackManager& manager = ShaderPackManager::Get();
    manager.Initialise();
    if (s_packRequested)
    {
        const std::string lower = Lower(s_requestedPack);
        manager.SelectPack(lower == "off" || lower == "none" ? std::string(ShaderSettings::kPackOff) : s_requestedPack);
    }
    else
    {
        std::vector<std::string> packs = manager.ListPacks();
        const std::string saved = manager.Settings().shaderPack;
        bool gone = manager.Settings().IsPackSelected()
                    && std::find(packs.begin(), packs.end(), saved) == packs.end();
        if (packs.size() == 1 && (!manager.SettingsSaved() || gone))
        {
            if (gone)
                std::printf("[pc] shaders: saved pack %s isn't in %s anymore\n", saved.c_str(),
                            manager.PacksFolder().c_str());
            std::printf("[pc] shaders: %s is the only pack in %s, using it\n", packs[0].c_str(),
                        manager.PacksFolder().c_str());
            manager.UsePack(packs[0]);
        }
    }
    if (!manager.Settings().UsesPack())
        return;
    if (manager.ActivePack() == nullptr)
        std::printf("[pc] shaders: shader pack %s could not be loaded\n", manager.Settings().shaderPack.c_str());
    for (const std::string& error : manager.Errors())
        std::printf("[pc] shaders: %s\n", error.c_str());
    std::fflush(stdout);
}

void FillWorldState(GameRenderer* gameRenderer, float partialTick, Minecraft* minecraft, Entity* camera,
                    ShaderWorldState& state)
{
    Level* level = minecraft->mLevel;
    state.dimension = level->mDimension->getType()->getId();
    state.cameraX = (camera->mX - camera->mXOld) * partialTick + camera->mXOld;
    state.cameraY = (camera->mY - camera->mYOld) * partialTick + camera->mYOld;
    state.cameraZ = (camera->mZ - camera->mZOld) * partialTick + camera->mZOld;
    state.eyeAltitude = static_cast<float>(state.cameraY + camera->getEyeHeight());
    if (gameRenderer->getCameraThirdPerson() == 0)
        state.eyeHeight = (camera->mEyeHeight - camera->mEyeHeightO) * partialTick + camera->mEyeHeightO;

    const float* view = gRenderer->MatrixGet(0);
    const float* projection = gRenderer->MatrixGet(1);
    std::copy(view, view + 16, state.view);
    std::copy(projection, projection + 16, state.projection);

    state.celestialAngle = level->getTimeOfDay(partialTick);
    state.worldTime = level->getDayTime();
    state.moonPhase = level->getMoonPhase();
    state.rainStrength = level->getRainLevel(partialTick);
    state.thunderStrength = level->getThunderLevel(partialTick);
    if (camera->isUnderLiquid(Material::WATER))
        state.isEyeInWater = 1;
    else if (camera->isUnderLiquid(Material::LAVA))
        state.isEyeInWater = 2;

    const BlockPos eye(static_cast<int>(std::floor(state.cameraX)), static_cast<int>(std::floor(state.eyeAltitude)),
                       static_cast<int>(std::floor(state.cameraZ)));
    state.eyeBlockLight = level->getBrightness(LightLayer::BLOCK, eye);
    state.eyeSkyLight = level->getBrightness(LightLayer::SKY, eye);
    if (level->mDimension->isHasSkyLight() && level->hasChunkAt(eye) && level->canSeeSky(eye))
        state.eyeSkyLight = 15;

    Vec3* sky = level->getSkyColor(minecraft->getCameraEntity(), partialTick);
    state.skyColor[0] = static_cast<float>(sky->x);
    state.skyColor[1] = static_cast<float>(sky->y);
    state.skyColor[2] = static_cast<float>(sky->z);
    gameRenderer->PcGetFogColour(state.fogColor);
    state.nearPlane = 0.05f;
    state.farPlane = static_cast<float>(PcSettingsRenderDistance() * 16);
    state.hideGui = minecraft->mOptions->mHideGui;

    if (eTYPE_DERIVED_FROM(EntityType::LIVING_ENTITY, camera->GetType()))
    {
        LivingEntity* living = static_cast<LivingEntity*>(camera);
        if (living->hasEffect(MobEffects::BLINDNESS))
            state.blindness = 1.0f;
        state.entityFlags[ShaderEntity_Child] = living->isBaby();
        state.entityFlags[ShaderEntity_Hurt] = living->mHurtTime > 0;
        state.elytraFlying = living->isFallFlying();
        state.heldItemId = ItemId(living->getMainHandItem(), state.heldBlockLight);
        state.heldItemId2 = ItemId(living->getOffhandItem(), state.heldBlockLight2);
    }
    state.entityFlags[ShaderEntity_Alive] = camera->isAlive();
    state.entityFlags[ShaderEntity_Burning] = camera->isOnFire();
    state.entityFlags[ShaderEntity_InLava] = camera->isInLava();
    state.entityFlags[ShaderEntity_InWater] = camera->isInWater();
    state.entityFlags[ShaderEntity_Invisible] = camera->isInvisible();
    state.entityFlags[ShaderEntity_OnGround] = camera->mOnGround;
    state.entityFlags[ShaderEntity_Ridden] = camera->isVehicle();
    state.entityFlags[ShaderEntity_Riding] = camera->isPassenger();
    state.entityFlags[ShaderEntity_Sneaking] = camera->isSneaking();
    state.entityFlags[ShaderEntity_Sprinting] = camera->isSprinting();
    state.entityFlags[ShaderEntity_Wet] = camera->isInWater() || level->isRainingAt(eye);

    state.eyeX = state.cameraX;
    state.eyeY = state.eyeAltitude;
    state.eyeZ = state.cameraZ;
    Vec3* look = camera->getViewVector(partialTick);
    state.lookVector[0] = static_cast<float>(look->x);
    state.lookVector[1] = static_cast<float>(look->y);
    state.lookVector[2] = static_cast<float>(look->z);
    state.bodyVector[0] = -std::sin(camera->mYRot * 0.017453292f);
    state.bodyVector[2] = std::cos(camera->mYRot * 0.017453292f);
    state.firstPerson = minecraft->mOptions->mThirdPersonView == 0;

    state.cloudHeight = level->mDimension->getCloudHeight();
    state.heightLimit = level->getMaxBuildHeight();
    state.logicalHeightLimit = level->getHeight();

    const BlockPos feet(static_cast<int>(std::floor(state.cameraX)), static_cast<int>(std::floor(state.cameraY)),
                        static_cast<int>(std::floor(state.cameraZ)));
    Biome* biome = level->getBiome(feet);
    if (biome != nullptr)
    {
        state.biome = BiomeUniform(biome);
        state.biomeCategory = BiomeCategory(biome);
        state.temperature = biome->getTemperature(feet);
        state.rainfall = biome->getDownfall();
        if (biome->hasRain() || biome->hasSnow())
            state.biomePrecipitation = state.temperature < 0.15f ? 2 : 1;
    }

    if (minecraft->mTextures != nullptr)
        gRenderer->TextureSize(minecraft->mTextures->loadTexture(BLOCKS_LOCATION.getTexture()), &state.atlasWidth,
                               &state.atlasHeight);
}

int LocalPlayerCount(Minecraft* minecraft)
{
    int count = 0;
    for (int index = 0; index < 4; ++index)
    {
        if (minecraft->mLocalPlayers[index] != nullptr)
            ++count;
    }
    return count;
}
}

void PcShaderPacksRequestPack(const std::string& name)
{
    s_requestedPack = name;
    s_packRequested = true;
}

std::vector<std::string> PcShaderPacksList()
{
    EnsureManager();
    return ShaderPackManager::Get().ListPacks();
}

std::string PcShaderPacksCurrent()
{
    EnsureManager();
    ShaderPackManager& manager = ShaderPackManager::Get();
    if (!manager.Settings().UsesPack() || manager.ActivePack() == nullptr)
        return std::string();
    return manager.Settings().shaderPack;
}

void PcShaderPacksChoose(const std::string& name)
{
    EnsureManager();
    ShaderPackManager& manager = ShaderPackManager::Get();
    if (s_optionsChanged && manager.ActivePack() != nullptr)
        manager.SaveOptions();
    s_optionsChanged = false;
    manager.SelectPack(name.empty() ? std::string(ShaderSettings::kPackOff) : name);
    if (!name.empty() && manager.ActivePack() == nullptr)
        std::printf("[pc] shaders: shader pack %s could not be loaded\n", name.c_str());
    std::fflush(stdout);
    ShaderPipeline::Get().RequestReload();
}

namespace
{
std::wstring MenuText(const std::string& text)
{
    const std::string plain = ShaderPackLang::StripFormatting(text);
    std::wstring wide;
    wide.reserve(plain.size());
    for (size_t index = 0; index < plain.size();)
    {
        const unsigned char lead = static_cast<unsigned char>(plain[index]);
        int length = lead < 0x80 ? 1 : (lead >> 5) == 0x6 ? 2 : (lead >> 4) == 0xE ? 3 : (lead >> 3) == 0x1E ? 4 : 1;
        if (index + length > plain.size())
            length = 1;
        unsigned int code = length == 1 ? lead : lead & (0x7F >> length);
        for (int part = 1; part < length; ++part)
            code = (code << 6) | (static_cast<unsigned char>(plain[index + part]) & 0x3F);
        wide += code <= 0xFFFF ? static_cast<wchar_t>(code) : L'?';
        index += length;
    }
    return wide;
}

void MarkOptionsChanged()
{
    s_optionsChanged = true;
    ++s_optionsVersion;
}

void AddOptionEntry(const ShaderOption& option, std::vector<PcShaderMenuEntry>& entries)
{
    ShaderPackManager& manager = ShaderPackManager::Get();
    const ShaderPackLang& lang = manager.Lang();
    const std::string current = manager.Options().Value(option.name);
    PcShaderMenuEntry entry;
    entry.key = option.name;
    entry.label = MenuText(lang.OptionLabel(option.name));
    entry.valueLabel = MenuText(lang.ValueLabel(option, current));
    entry.checked = current == "true";
    entry.value = 0;
    entry.count = 0;
    if (option.ambiguous)
        entry.kind = PcShaderMenuEntry::Info;
    else if (option.IsBoolean())
        entry.kind = PcShaderMenuEntry::Toggle;
    else if (option.values.size() > 1)
    {
        entry.kind = PcShaderMenuEntry::Choice;
        entry.count = static_cast<int>(option.values.size());
        std::vector<std::string>::const_iterator found =
            std::find(option.values.begin(), option.values.end(), current);
        entry.value = found != option.values.end() ? static_cast<int>(found - option.values.begin()) : 0;
    }
    else
        entry.kind = PcShaderMenuEntry::Info;
    entries.push_back(entry);
}

void AddProfileEntry(std::vector<PcShaderMenuEntry>& entries)
{
    ShaderPackManager& manager = ShaderPackManager::Get();
    const std::vector<ShaderProfile>& profiles = manager.Properties().Profiles();
    const std::string active = manager.Properties().ActiveProfile(manager.Options());
    PcShaderMenuEntry entry;
    entry.kind = PcShaderMenuEntry::Profile;
    entry.label = L"Profile";
    entry.checked = false;
    entry.count = static_cast<int>(profiles.size()) + 1;
    entry.value = static_cast<int>(profiles.size());
    entry.valueLabel = L"Custom";
    for (size_t index = 0; index < profiles.size(); ++index)
    {
        if (profiles[index].name == active)
        {
            entry.value = static_cast<int>(index);
            entry.valueLabel = MenuText(manager.Lang().ProfileLabel(active));
        }
    }
    entries.push_back(entry);
}
}

bool PcShaderPacksHasOptions()
{
    EnsureManager();
    ShaderPackManager& manager = ShaderPackManager::Get();
    return manager.Settings().UsesPack() && manager.ActivePack() != nullptr;
}

std::vector<PcShaderMenuEntry> PcShaderPacksPage(const std::string& page)
{
    std::vector<PcShaderMenuEntry> entries;
    if (!PcShaderPacksHasOptions())
        return entries;
    ShaderPackManager& manager = ShaderPackManager::Get();
    const ShaderPackProperties& properties = manager.Properties();
    const ShaderPackOptions& options = manager.Options();

    std::vector<std::string> items;
    const ShaderScreen* screen = properties.Screen(page);
    if (screen != nullptr)
        items = screen->items;
    else if (page.empty())
        items.push_back("*");

    std::set<std::string> listed;
    for (const ShaderScreen& other : properties.Screens())
        listed.insert(other.items.begin(), other.items.end());

    bool profileShown = false;
    for (const std::string& item : items)
    {
        if (item.empty() || item == "<empty>")
            continue;
        if (item == "<profile>")
        {
            if (!properties.Profiles().empty() && !profileShown)
            {
                AddProfileEntry(entries);
                profileShown = true;
            }
        }
        else if (item.size() > 2 && item[0] == '[' && item[item.size() - 1] == ']')
        {
            const std::string name = item.substr(1, item.size() - 2);
            if (properties.Screen(name) == nullptr)
                continue;
            PcShaderMenuEntry entry;
            entry.kind = PcShaderMenuEntry::Page;
            entry.key = name;
            entry.label = MenuText(manager.Lang().ScreenLabel(name));
            entry.checked = false;
            entry.value = 0;
            entry.count = 0;
            entries.push_back(entry);
        }
        else if (item == "*")
        {
            for (const ShaderOption& option : options.Options())
            {
                if (listed.count(option.name) == 0 && (option.IsBoolean() || option.values.size() > 1))
                    AddOptionEntry(option, entries);
            }
        }
        else
        {
            const ShaderOption* option = options.Find(item);
            if (option != nullptr)
                AddOptionEntry(*option, entries);
        }
    }

    if (page.empty() && !profileShown && !properties.Profiles().empty())
    {
        std::vector<PcShaderMenuEntry> withProfile;
        AddProfileEntry(withProfile);
        entries.insert(entries.begin(), withProfile[0]);
    }
    return entries;
}

void PcShaderPacksSetToggle(const std::string& option, bool on)
{
    if (!PcShaderPacksHasOptions())
        return;
    ShaderPackManager& manager = ShaderPackManager::Get();
    const std::string value = on ? "true" : "false";
    if (manager.Options().Value(option) != value && manager.SetOption(option, value))
        MarkOptionsChanged();
}

void PcShaderPacksSetChoice(const std::string& option, int position)
{
    if (!PcShaderPacksHasOptions())
        return;
    ShaderPackManager& manager = ShaderPackManager::Get();
    const ShaderOption* found = manager.Options().Find(option);
    if (found == nullptr || position < 0 || position >= static_cast<int>(found->values.size()))
        return;
    const std::string& value = found->values[static_cast<size_t>(position)];
    if (manager.Options().Value(option) != value && manager.SetOption(option, value))
        MarkOptionsChanged();
}

void PcShaderPacksSetProfile(int position)
{
    if (!PcShaderPacksHasOptions())
        return;
    ShaderPackManager& manager = ShaderPackManager::Get();
    const std::vector<ShaderProfile>& profiles = manager.Properties().Profiles();
    if (position < 0 || position >= static_cast<int>(profiles.size()))
        return;
    const std::string before = manager.Options().SaveValues();
    manager.ApplyProfile(profiles[static_cast<size_t>(position)].name);
    if (manager.Options().SaveValues() != before)
        MarkOptionsChanged();
}

void PcShaderPacksResetOptions()
{
    if (!PcShaderPacksHasOptions())
        return;
    ShaderPackManager& manager = ShaderPackManager::Get();
    const std::string before = manager.Options().SaveValues();
    manager.ResetOptions();
    if (manager.Options().SaveValues() != before)
        MarkOptionsChanged();
}

void PcShaderPacksApplyOptions()
{
    if (!s_optionsChanged)
        return;
    s_optionsChanged = false;
    if (!PcShaderPacksHasOptions())
        return;
    ShaderPackManager::Get().SaveOptions();
    ShaderPipeline::Get().RequestReload();
}

int PcShaderPacksOptionsVersion()
{
    return s_optionsVersion;
}

void PcShaderPacksBeginFrame(bool eligible)
{
    Minecraft* minecraft = Minecraft::GetInstance();
    s_frameEligible = eligible && minecraft != nullptr && LocalPlayerCount(minecraft) <= 1;
    s_worldActive = false;
    s_worldDrawn = false;
}

bool PcShaderPacksBeginWorld(GameRenderer* gameRenderer, float partialTick)
{
    s_worldActive = false;
    if (!s_frameEligible)
        return false;
    s_frameEligible = false;
    if (!PcShaderPackTarget(&s_framebuffer, &s_width, &s_height))
        return false;
    EnsureManager();
    ShaderPackManager& manager = ShaderPackManager::Get();
    if (!manager.Settings().UsesPack() || manager.ActivePack() == nullptr)
    {
        if (ShaderPipeline::Get().Enabled())
            ShaderPipeline::Get().Shutdown();
        return false;
    }

    Minecraft* minecraft = Minecraft::GetInstance();
    boost::shared_ptr<Entity> camera = minecraft->getCameraEntity();
    if (minecraft->mLevel == nullptr || camera == nullptr)
        return false;
    RefreshIds();

    ShaderWorldState state;
    FillWorldState(gameRenderer, partialTick, minecraft, camera.get(), state);
    ShaderPipeline::Get().SetResourceResolver(&ResolveResourceTexture);
    if (minecraft->mPlayer != nullptr)
        ShaderPipeline::Get().SetLightmapTexture(gRenderer->TextureGlName(
            static_cast<int>(gameRenderer->getLightTexture(minecraft->mPlayer->mUserIndex, minecraft->mLevel))));
    s_worldActive = ShaderPipeline::Get().BeginWorld(state, s_width, s_height);
    s_worldDrawn = s_worldActive;
    PcShaderPackRestore();
    return s_worldActive;
}

void PcShaderPacksRenderShadows(LevelRenderer* levelRenderer, float partialTick)
{
    ShaderPipeline& pipeline = ShaderPipeline::Get();
    if (!s_worldActive)
        return;
    PassScope scope("Shader pack shadow pass");
    if (!pipeline.BeginShadow())
        return;
    PcShaderPackRestore();

    float view[16];
    float projection[16];
    pipeline.ShadowMatrices(view, projection);
    GlStateManager::matrixMode(1);
    GlStateManager::pushMatrix();
    GlStateManager::loadIdentity();
    GlStateManager::glMultMatrixf(projection);
    GlStateManager::matrixMode(0);
    GlStateManager::pushMatrix();
    GlStateManager::loadIdentity();
    GlStateManager::glMultMatrixf(view);

    const float distance = pipeline.ShadowRenderDistance();
    if (pipeline.ShadowTerrain())
        levelRenderer->renderShadowChunks(0, partialTick, distance, view, projection);
    const bool player = pipeline.ShadowEntities() || pipeline.ShadowPlayer();
    if (pipeline.ShadowEntities() || player || pipeline.ShadowBlockEntities())
        levelRenderer->renderShadowEntities(partialTick, distance, view, projection, pipeline.ShadowEntities(),
                                            player, pipeline.ShadowBlockEntities());
    pipeline.EndShadowOpaque();
    PcShaderPackRestore();
    if (pipeline.ShadowTerrain() && pipeline.ShadowTranslucent())
    {
        levelRenderer->renderShadowChunks(2, partialTick, distance, view, projection);
        levelRenderer->renderShadowChunks(1, partialTick, distance, view, projection);
    }

    GlStateManager::matrixMode(1);
    GlStateManager::popMatrix();
    GlStateManager::matrixMode(0);
    GlStateManager::popMatrix();
    pipeline.EndShadow();
    PcShaderPackRestore();
}

void PcShaderPacksEndWorld()
{
    if (!s_worldActive)
        return;
    PassScope scope("Shader pack composite and final");
    s_worldActive = false;
    ShaderPipeline::Get().EndWorld(s_framebuffer, s_width, s_height);
    PcShaderPackRestore();
}

bool PcShaderPacksActive()
{
    return s_worldActive;
}

bool PcShaderPacksDrewWorld()
{
    return s_worldDrawn;
}

void PcShaderPacksSetStage(int stage)
{
    if (s_worldActive)
        ShaderPipeline::Get().SetStage(stage);
}

void PcShaderPacksBeginTranslucent()
{
    if (!s_worldActive)
        return;
    PassScope scope("Shader pack deferred");
    ShaderPipeline::Get().BeginTranslucent();
    PcShaderPackRestore();
}

void PcShaderPacksBeginHand()
{
    if (!s_worldActive)
        return;
    ShaderPipeline::Get().BeginHand();
    PcShaderPackRestore();
}

void PcShaderPacksEndHand()
{
    if (s_worldActive)
        ShaderPipeline::Get().EndHand();
}

bool PcShaderPacksRenderHorizon()
{
    return s_worldActive && ShaderPipeline::Get().RenderSkyDisc();
}

bool PcShaderPacksRenderSun()
{
    return !s_worldActive || ShaderPipeline::Get().RenderSun();
}

bool PcShaderPacksRenderMoon()
{
    return !s_worldActive || ShaderPipeline::Get().RenderMoon();
}

int PcShaderPacksCloudMode()
{
    return s_worldActive ? ShaderPipeline::Get().CloudMode() : -1;
}

bool PcShaderPacksWeatherDepth()
{
    return s_worldActive && ShaderPipeline::Get().RainDepth();
}

bool PcShaderPacksVignette()
{
    return !s_worldDrawn || ShaderPipeline::Get().Vignette();
}

bool PcShaderPacksUnderwaterOverlay()
{
    return !s_worldDrawn || ShaderPipeline::Get().UnderwaterOverlay();
}

void PcShaderPacksSetEntity(Entity* entity)
{
    if (!s_worldActive)
        return;
    int id = -1;
    if (entity != nullptr)
    {
        Minecraft* minecraft = Minecraft::GetInstance();
        std::string name = Lower(Narrow(entity->getEncodeId()));
        if (minecraft->mPlayer != nullptr && entity == static_cast<Entity*>(minecraft->mPlayer.get())
            && s_ids.entities.count("minecraft:current_player") != 0)
            name = "minecraft:current_player";
        else if (eTYPE_DERIVED_FROM(EntityType::PLAYER, entity->GetType()))
            name = "minecraft:player";
        std::map<std::string, int>::const_iterator found = s_ids.entities.find(name);
        if (found != s_ids.entities.end())
            id = found->second;
    }
    ShaderPipeline::Get().SetEntityId(id);
}

void PcShaderPacksSetBlockEntity(BlockEntity* blockEntity)
{
    if (!s_worldActive)
        return;
    int id = -1;
    if (blockEntity != nullptr && blockEntity->mLevel != nullptr && !s_ids.blocks.empty())
    {
        const BlockState* state = blockEntity->mLevel->getBlockState(blockEntity->getBlockPos());
        int element = state != nullptr ? Block::getElementId(state) : 0;
        if (element > 0 && element < static_cast<int>(s_ids.blocks.size()))
            id = s_ids.blocks[static_cast<size_t>(element)];
    }
    ShaderPipeline::Get().SetBlockEntityId(id);
}

void PcShaderPacksClearEntity()
{
    if (!s_worldActive)
        return;
    ShaderPipeline::Get().SetEntityId(-1);
    ShaderPipeline::Get().SetBlockEntityId(-1);
}
