// Adapted from LCEReborn (GPLv3): source/platform/pc/shaderpack/ShaderPackBridge.cpp
#include "platform/pc/shaderpack/ShaderBlockIds.hpp"

#include <algorithm>
#include <cctype>
#include <cstdlib>
#include <string>

#include "game/world/block/Block.hpp"
#include "game/world/block/BlockState.hpp"
#include "game/world/block/IdMapper.hpp"
#include "game/world/block/Property.hpp"
#include "platform/pc/shaderpack/ShaderPackManager.hpp"
#include "platform/pc/shaderpack/ShaderPackProperties.hpp"
#include "platform/pc/shaderpack/ShaderPreprocessor.hpp"

namespace
{
struct BlockRule
{
    int id;
    std::vector<std::pair<std::string, std::vector<std::string>>> properties;
    std::vector<int> metas;
};

std::string Narrow(const std::wstring& text)
{
    std::string result;
    result.reserve(text.size());
    for (wchar_t character : text)
        result.push_back(character < 128 ? static_cast<char>(character) : '?');
    return result;
}

std::string Lower(std::string text)
{
    for (char& character : text)
        character = static_cast<char>(std::tolower(static_cast<unsigned char>(character)));
    return text;
}

bool IsNumber(const std::string& text)
{
    if (text.empty())
        return false;
    for (char character : text)
    {
        if (!std::isdigit(static_cast<unsigned char>(character)))
            return false;
    }
    return true;
}

std::vector<std::string> SplitOn(const std::string& text, char separator)
{
    std::vector<std::string> parts;
    size_t start = 0;
    for (;;)
    {
        size_t end = text.find(separator, start);
        parts.push_back(text.substr(start, end == std::string::npos ? std::string::npos : end - start));
        if (end == std::string::npos)
            break;
        start = end + 1;
    }
    return parts;
}

ShaderPropertyList LoadMappingFile(const char* file)
{
    ShaderPackManager& manager = ShaderPackManager::Get();
    ShaderPack* pack = manager.ActivePack();
    std::string text;
    if (pack == nullptr || !pack->ReadFile(file, text))
        return ShaderPropertyList();
    ShaderMacros macros = manager.StandardMacros();
    for (const auto& option : manager.Options().Macros())
        macros[option.first] = option.second;
    return ShaderParseProperties(ShaderPreprocessConditionals(text, macros));
}

std::vector<std::string> ModernNames(const std::string& name, std::map<std::string, std::string>& values)
{
    static const std::map<std::string, std::string> kRenames = {
        {"grass", "grass_block"}, {"deadbush", "dead_bush"}, {"yellow_flower", "dandelion"}, {"snow", "snow_block"},
        {"snow_layer", "snow"}, {"brick_block", "bricks"}, {"nether_brick", "nether_bricks"},
        {"red_nether_brick", "red_nether_bricks"}, {"hardened_clay", "terracotta"}, {"reeds", "sugar_cane"},
        {"waterlily", "lily_pad"}, {"web", "cobweb"}, {"portal", "nether_portal"}, {"magma", "magma_block"},
        {"mob_spawner", "spawner"}, {"lit_pumpkin", "jack_o_lantern"}, {"pumpkin", "carved_pumpkin"},
        {"melon_block", "melon"}, {"quartz_ore", "nether_quartz_ore"}, {"lit_furnace", "furnace"},
        {"lit_redstone_ore", "redstone_ore"}, {"lit_redstone_lamp", "redstone_lamp"},
        {"unlit_redstone_torch", "redstone_torch"}, {"powered_repeater", "repeater"}, {"unpowered_repeater", "repeater"},
        {"powered_comparator", "comparator"}, {"unpowered_comparator", "comparator"}, {"wooden_door", "oak_door"},
        {"fence", "oak_fence"}, {"fence_gate", "oak_fence_gate"}, {"wooden_pressure_plate", "oak_pressure_plate"},
        {"wooden_button", "oak_button"}, {"trapdoor", "oak_trapdoor"}, {"standing_sign", "oak_sign"},
        {"wall_sign", "oak_wall_sign"}, {"noteblock", "note_block"}, {"golden_rail", "powered_rail"},
        {"slime", "slime_block"}, {"flowing_water", "water"}, {"flowing_lava", "lava"},
        {"stone_stairs", "cobblestone_stairs"}, {"grass_path", "dirt_path"}, {"double_stone_slab", "stone_slab"},
        {"double_stone_slab2", "stone_slab2"}, {"double_wooden_slab", "wooden_slab"}, {"purpur_double_slab", "purpur_slab"},
        {"silver_glazed_terracotta", "light_gray_glazed_terracotta"}, {"silver_shulker_box", "light_gray_shulker_box"}};
    static const std::map<std::string, std::string> kVariants = {
        {"tallgrass|tall_grass", "short_grass"}, {"tallgrass|fern", "fern"}, {"tallgrass|dead_bush", "dead_bush"},
        {"red_flower|houstonia", "azure_bluet"}, {"double_plant|syringa", "lilac"},
        {"double_plant|double_grass", "tall_grass"}, {"double_plant|double_fern", "large_fern"},
        {"double_plant|double_rose", "rose_bush"}, {"double_plant|paeonia", "peony"},
        {"stone|smooth_granite", "polished_granite"}, {"stone|smooth_diorite", "polished_diorite"},
        {"stone|smooth_andesite", "polished_andesite"}, {"sandstone|smooth_sandstone", "cut_sandstone"},
        {"red_sandstone|smooth_red_sandstone", "cut_red_sandstone"}, {"stonebrick|stonebrick", "stone_bricks"},
        {"stonebrick|mossy_stonebrick", "mossy_stone_bricks"}, {"stonebrick|cracked_stonebrick", "cracked_stone_bricks"},
        {"stonebrick|chiseled_stonebrick", "chiseled_stone_bricks"}, {"quartz_block|default", "quartz_block"},
        {"quartz_block|chiseled", "chiseled_quartz_block"}, {"quartz_block|lines_x", "quartz_pillar"},
        {"quartz_block|lines_y", "quartz_pillar"}, {"quartz_block|lines_z", "quartz_pillar"},
        {"stone_slab|stone", "smooth_stone_slab"}, {"stone_slab|sand", "sandstone_slab"},
        {"stone_slab|wood_old", "petrified_oak_slab"}, {"stone_slab|cobblestone", "cobblestone_slab"},
        {"stone_slab|brick", "brick_slab"}, {"stone_slab|stone_brick", "stone_brick_slab"},
        {"stone_slab|nether_brick", "nether_brick_slab"}, {"stone_slab|quartz", "quartz_slab"},
        {"stone_slab2|red_sandstone", "red_sandstone_slab"}, {"brown_mushroom_block|stem", "mushroom_stem"},
        {"red_mushroom_block|stem", "mushroom_stem"}, {"cobblestone_wall|normal", "cobblestone_wall"},
        {"cobblestone_wall|mossy", "mossy_cobblestone_wall"}, {"prismarine|rough", "prismarine"},
        {"prismarine|bricks", "prismarine_bricks"}, {"prismarine|dark", "dark_prismarine"},
        {"monster_egg|stone", "infested_stone"}, {"monster_egg|cobblestone", "infested_cobblestone"},
        {"monster_egg|stone_brick", "infested_stone_bricks"}, {"monster_egg|mossy_brick", "infested_mossy_stone_bricks"},
        {"monster_egg|cracked_brick", "infested_cracked_stone_bricks"},
        {"monster_egg|chiseled_brick", "infested_chiseled_stone_bricks"}};

    if (name.compare(0, 4, "lit_") == 0 || name == "redstone_torch")
        values.insert(std::make_pair("lit", "true"));
    else if (name == "furnace" || name == "redstone_ore" || name == "redstone_lamp" || name == "unlit_redstone_torch")
        values.insert(std::make_pair("lit", "false"));
    if (name.compare(0, 8, "powered_") == 0)
        values.insert(std::make_pair("powered", "true"));
    else if (name.compare(0, 10, "unpowered_") == 0)
        values.insert(std::make_pair("powered", "false"));

    std::vector<std::string> names;
    auto facing = values.find("facing");
    bool wall = facing != values.end() && facing->second != "up";
    if (name == "torch" && wall)
        names.push_back("wall_torch");
    if ((name == "redstone_torch" || name == "unlit_redstone_torch") && wall)
        names.push_back("redstone_wall_torch");
    if (name == "sponge")
    {
        auto wet = values.find("wet");
        if (wet != values.end() && wet->second == "true")
            names.push_back("wet_sponge");
    }

    std::string variant;
    for (const char* key : {"variant", "type", "color"})
    {
        auto found = values.find(key);
        if (found != values.end())
        {
            variant = found->second == "silver" ? "light_gray" : found->second;
            break;
        }
    }
    auto renamed = kRenames.find(name);
    std::string canonical = renamed != kRenames.end() && name.compare(0, 7, "double_") == 0 ? renamed->second : name;
    if (!variant.empty())
    {
        auto special = kVariants.find(canonical + "|" + variant);
        if (special != kVariants.end())
        {
            names.push_back(special->second);
        }
        else
        {
            std::string base = canonical;
            while (!base.empty() && std::isdigit(static_cast<unsigned char>(base.back())))
                base.pop_back();
            if (base == "stained_hardened_clay")
                base = "terracotta";
            else if (base == "wooden_slab")
                base = "slab";
            auto axis = values.find("axis");
            if (base == "log" && axis != values.end() && axis->second == "none")
                names.push_back(variant + "_wood");
            names.push_back(variant + "_" + base);
            names.push_back(variant);
        }
    }
    if (name == "tallgrass")
    {
        names.push_back("tallgrass");
        names.push_back("grass");
    }
    if (renamed != kRenames.end())
        names.push_back(renamed->second);
    if (name != "grass" && name != "snow")
        names.push_back(name);
    return names;
}

bool UsesModernNames(const std::map<std::string, std::vector<BlockRule>>& named)
{
    static const char* const kMarkers[] = {"grass_block", "short_grass", "oak_log",   "oak_planks", "oak_leaves",
                                           "stone_bricks", "white_wool", "dandelion", "poppy",      "sugar_cane",
                                           "lily_pad",    "cobweb",     "bricks",    "terracotta", "granite"};
    for (const char* marker : kMarkers)
    {
        if (named.count(std::string("minecraft:") + marker) != 0)
            return true;
    }
    return false;
}

void BuildBlockMap(std::vector<int>& blocks, std::vector<int>& lights)
{
    std::map<std::string, std::vector<BlockRule>> named;
    std::map<int, std::vector<BlockRule>> numbered;
    for (const auto& entry : LoadMappingFile("block.properties"))
    {
        if (entry.first.compare(0, 6, "block.") != 0)
            continue;
        int id = std::atoi(entry.first.c_str() + 6);
        for (const std::string& token : ShaderSplitList(entry.second))
        {
            std::vector<std::string> parts = SplitOn(token, ':');
            BlockRule rule;
            rule.id = id;
            if (IsNumber(parts[0]))
            {
                for (size_t index = 1; index < parts.size(); ++index)
                {
                    for (const std::string& meta : SplitOn(parts[index], ','))
                    {
                        if (IsNumber(meta))
                            rule.metas.push_back(std::atoi(meta.c_str()));
                    }
                }
                numbered[std::atoi(parts[0].c_str())].push_back(rule);
                continue;
            }
            size_t first = 1;
            std::string name = "minecraft:" + Lower(parts[0]);
            if (parts.size() >= 2 && parts[1].find('=') == std::string::npos && !IsNumber(SplitOn(parts[1], ',')[0]))
            {
                name = Lower(parts[0]) + ":" + Lower(parts[1]);
                first = 2;
            }
            for (size_t index = first; index < parts.size(); ++index)
            {
                size_t equals = parts[index].find('=');
                if (equals != std::string::npos)
                {
                    rule.properties.push_back(std::make_pair(Lower(parts[index].substr(0, equals)),
                                                             SplitOn(Lower(parts[index].substr(equals + 1)), ',')));
                    continue;
                }
                for (const std::string& meta : SplitOn(parts[index], ','))
                {
                    if (IsNumber(meta))
                        rule.metas.push_back(std::atoi(meta.c_str()));
                }
            }
            named[name].push_back(rule);
        }
    }

    bool modern = UsesModernNames(named);
    blocks.assign(65536, -1);
    lights.assign(65536, 0);
    for (int element = 1; element < IdMapper<const BlockState*>::BLOCK_STATE_COUNT; ++element)
    {
        if (Block::byId(element >> 4) == nullptr)
            continue;
        const BlockState* state = Block::getStateByElementId(static_cast<unsigned short>(element));
        if (state == nullptr || state->getBlock() == nullptr)
            continue;
        Block* block = state->getBlock();
        lights[element] = block->getLightEmission(state);
        int blockId = element >> 4;
        int meta = element & 15;
        std::string name = Lower(Narrow(Block::blockRegistry->getKey(block).toString()));

        std::map<std::string, std::string> values;
        for (const Property* property : state->getProperties())
        {
            Boxed* value = state->getBoxedValue(property);
            if (value != nullptr)
                values[Lower(Narrow(property->getName()))] = Lower(Narrow(property->getName(value)));
        }

        auto matches = [&](const BlockRule& rule) {
            if (!rule.metas.empty() && std::find(rule.metas.begin(), rule.metas.end(), meta) == rule.metas.end())
                return false;
            for (const auto& filter : rule.properties)
            {
                auto found = values.find(filter.first);
                if (found == values.end())
                    return false;
                if (std::find(filter.second.begin(), filter.second.end(), found->second) == filter.second.end())
                    return false;
            }
            return true;
        };

        std::vector<std::string> candidates;
        if (modern && name.compare(0, 10, "minecraft:") == 0)
        {
            for (const std::string& candidate : ModernNames(name.substr(10), values))
                candidates.push_back("minecraft:" + candidate);
        }
        else
        {
            candidates.push_back(name);
        }

        int mapped = -1;
        for (const std::string& candidate : candidates)
        {
            auto byName = named.find(candidate);
            if (byName == named.end())
                continue;
            for (const BlockRule& rule : byName->second)
            {
                if (matches(rule))
                {
                    mapped = rule.id;
                    break;
                }
            }
            if (mapped >= 0)
                break;
        }
        if (mapped < 0)
        {
            auto byNumber = numbered.find(blockId);
            if (byNumber != numbered.end())
            {
                for (const BlockRule& rule : byNumber->second)
                {
                    if (matches(rule))
                    {
                        mapped = rule.id;
                        break;
                    }
                }
            }
        }
        blocks[static_cast<size_t>(element)] = mapped;
    }
}

void BuildNameMap(const char* file, const char* prefix, std::map<std::string, int>& map)
{
    map.clear();
    size_t prefixLength = std::char_traits<char>::length(prefix);
    for (const auto& entry : LoadMappingFile(file))
    {
        if (entry.first.compare(0, prefixLength, prefix) != 0)
            continue;
        int id = std::atoi(entry.first.c_str() + prefixLength);
        for (const std::string& token : ShaderSplitList(entry.second))
        {
            std::string name = ShaderQualifiedName(token);
            if (map.find(name) == map.end())
                map[name] = id;
        }
    }
}
}

std::string ShaderQualifiedName(const std::string& name)
{
    std::string lower = Lower(name);
    return lower.find(':') == std::string::npos ? "minecraft:" + lower : lower;
}

void ShaderBuildIdMappings(ShaderIdMappings& mappings)
{
    mappings.blocks.clear();
    mappings.lights.clear();
    mappings.entities.clear();
    mappings.items.clear();
    if (ShaderPackManager::Get().ActivePack() == nullptr)
        return;
    BuildBlockMap(mappings.blocks, mappings.lights);
    BuildNameMap("entity.properties", "entity.", mappings.entities);
    BuildNameMap("item.properties", "item.", mappings.items);
}
