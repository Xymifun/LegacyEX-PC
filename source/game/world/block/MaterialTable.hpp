#ifndef GAME_WORLD_BLOCK_MATERIALTABLE_HPP
#define GAME_WORLD_BLOCK_MATERIALTABLE_HPP

#include <string>

#include <boost/unordered_map.hpp>

#include "game/world/block/MaterialShaders.hpp"

enum eMinecraftEntityMaterial
{
    eMinecraftEntityMaterial_Invalid = -1,
    eMinecraftEntityMaterial_Bat,
    eMinecraftEntityMaterial_EnderDragon,
    eMinecraftEntityMaterial_BlazeHead,
    eMinecraftEntityMaterial_Drowned,
    eMinecraftEntityMaterial_Enderman,
    eMinecraftEntityMaterial_EndermanInvisible,
    eMinecraftEntityMaterial_Ghast,
    eMinecraftEntityMaterial_Guardian,
    eMinecraftEntityMaterial_MagmaCube,
    eMinecraftEntityMaterial_ZombiePigman,
    eMinecraftEntityMaterial_Phantom,
    eMinecraftEntityMaterial_PhantomInvisible,
    eMinecraftEntityMaterial_Sheep,
    eMinecraftEntityMaterial_Shulker,
    eMinecraftEntityMaterial_Skeleton,
    eMinecraftEntityMaterial_Spider,
    eMinecraftEntityMaterial_SpiderInvisible,
    eMinecraftEntityMaterial_Stray,
    eMinecraftEntityMaterial_IronGolem,
    eMinecraftEntityMaterial_WitherBoss,
    eMinecraftEntityMaterial_WitherSkeleton,
    eMinecraftEntityMaterial_Wolf,
    eMinecraftEntityMaterial_Count
};

typedef boost::unordered_map<std::wstring, MaterialShaders> MaterialLookUpMap;
typedef boost::unordered_map<std::wstring, eMinecraftEntityMaterial> MobNameToInstanceMap;

class MaterialTable
{
public:
    MaterialTable();
    MaterialTable(unsigned char* materialData, unsigned int materialDataLength);
    MaterialTable(const MaterialTable& materialTable, unsigned char* materialData, unsigned int materialDataLength);

    void resetMaterials();
    void loadMaterialData(unsigned char* materialData, unsigned int materialDataLength);

    MaterialShaders& getMaterialShadersFor(eMinecraftEntityMaterial material);
    MaterialShaders getMaterialShadersFromDataName(const std::wstring& materialName);
    eMinecraftEntityMaterial getMobMaterialFromName(const std::wstring& mobName);

    static MaterialLookUpMap generateMaterialLookUPMap();
    static MobNameToInstanceMap generateMobNameToInstanceMap();

private:
    MaterialShaders mMaterialShaders[eMinecraftEntityMaterial_Count];
};

#endif
