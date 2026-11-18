// Adapted from LCEReborn (GPLv3): source/platform/pc/shaderpack/ShaderPackBridge.cpp
#ifndef PLATFORM_PC_SHADERPACK_SHADERBLOCKIDS_HPP
#define PLATFORM_PC_SHADERPACK_SHADERBLOCKIDS_HPP

#include <map>
#include <string>
#include <vector>

// The ids a shader pack gives blocks, entities and items (block.properties, entity.properties and
// item.properties, read with the pack's current options), as its programs see them in mc_Entity, entityId,
// blockEntityId and heldItemId.
struct ShaderIdMappings
{
    // by block state element id (Block::getElementId), -1 where the pack names none; 65536 entries
    std::vector<int> blocks;
    // by block state element id, the light the block gives off (0 to 15), for Iris's at_midBlock.w
    // (BLOCK_EMISSION_ATTRIBUTE); 65536 entries
    std::vector<int> lights;
    // by qualified name ("minecraft:zombie")
    std::map<std::string, int> entities;
    std::map<std::string, int> items;
};

// From the active pack; empty maps without one. Packs written for 1.13 and later name blocks the modern way
// ("grass_block", "oak_planks"), and this game's 1.12 names and states are matched to those.
void ShaderBuildIdMappings(ShaderIdMappings& mappings);

// "minecraft:" in front of a name without a namespace, lower case
std::string ShaderQualifiedName(const std::string& name);

#endif
