#include "game/world/block/MaterialTable.hpp"

#include <cstring>

#include "game/core/ByteArrayInputStream.hpp"
#include "game/core/DataInputStream.hpp"
#include "game/world/biome/arrayWithLength.hpp"

static const MobNameToInstanceMap sMobNameToInstanceMap = MaterialTable::generateMobNameToInstanceMap();
static const MaterialLookUpMap sMaterialLookUpMap = MaterialTable::generateMaterialLookUPMap();

MaterialShaders MaterialTable::getMaterialShadersFromDataName(const std::wstring& materialName)
{
    MaterialLookUpMap::const_iterator material = sMaterialLookUpMap.find(materialName);

    if (material == sMaterialLookUpMap.end())
    {
        return sMaterialLookUpMap.at(L"entity");
    }

    return material->second;
}

void MaterialTable::resetMaterials()
{
    mMaterialShaders[eMinecraftEntityMaterial_Bat] = getMaterialShadersFromDataName(L"entity_alphatest");
    mMaterialShaders[eMinecraftEntityMaterial_EnderDragon] = getMaterialShadersFromDataName(L"entity_emissive_alpha");
    mMaterialShaders[eMinecraftEntityMaterial_BlazeHead] = getMaterialShadersFromDataName(L"entity_emissive_alpha");
    mMaterialShaders[eMinecraftEntityMaterial_Drowned] = getMaterialShadersFromDataName(L"entity_emissive_alpha");
    mMaterialShaders[eMinecraftEntityMaterial_Enderman] = getMaterialShadersFromDataName(L"entity_emissive_alpha");
    mMaterialShaders[eMinecraftEntityMaterial_EndermanInvisible] = getMaterialShadersFromDataName(
        L"entity_emissive_alpha_only");
    mMaterialShaders[eMinecraftEntityMaterial_Ghast] = getMaterialShadersFromDataName(L"entity_emissive_alpha");
    mMaterialShaders[eMinecraftEntityMaterial_Guardian] = getMaterialShadersFromDataName(L"entity_alphatest");
    mMaterialShaders[eMinecraftEntityMaterial_MagmaCube] = getMaterialShadersFromDataName(L"entity_emissive_alpha");
    mMaterialShaders[eMinecraftEntityMaterial_ZombiePigman] = getMaterialShadersFromDataName(L"entity");
    mMaterialShaders[eMinecraftEntityMaterial_Phantom] = getMaterialShadersFromDataName(L"entity_emissive_alpha");
    mMaterialShaders[eMinecraftEntityMaterial_PhantomInvisible] = getMaterialShadersFromDataName(
        L"entity_emissive_alpha_only");
    mMaterialShaders[eMinecraftEntityMaterial_Sheep] = getMaterialShadersFromDataName(L"entity_alphatest_change_color");
    mMaterialShaders[eMinecraftEntityMaterial_Shulker] = getMaterialShadersFromDataName(
        L"entity_alphatest_change_color");
    mMaterialShaders[eMinecraftEntityMaterial_Skeleton] = getMaterialShadersFromDataName(L"entity_alphatest");
    mMaterialShaders[eMinecraftEntityMaterial_Spider] = getMaterialShadersFromDataName(L"entity_emissive_alpha");
    mMaterialShaders[eMinecraftEntityMaterial_SpiderInvisible] = getMaterialShadersFromDataName(
        L"entity_emissive_alpha_only");
    mMaterialShaders[eMinecraftEntityMaterial_Stray] = getMaterialShadersFromDataName(L"entity_alphatest");
    mMaterialShaders[eMinecraftEntityMaterial_IronGolem] = getMaterialShadersFromDataName(L"entity_alphatest");
    mMaterialShaders[eMinecraftEntityMaterial_WitherBoss] = getMaterialShadersFromDataName(L"entity_alphatest");
    mMaterialShaders[eMinecraftEntityMaterial_WitherSkeleton] = getMaterialShadersFromDataName(L"entity_alphatest");
    mMaterialShaders[eMinecraftEntityMaterial_Wolf] = getMaterialShadersFromDataName(L"entity_alphatest_change_color");
}

MaterialTable::MaterialTable()
{
    resetMaterials();
}

MaterialTable::MaterialTable(unsigned char* materialData, unsigned int materialDataLength)
{
    resetMaterials();
    loadMaterialData(materialData, materialDataLength);
}

eMinecraftEntityMaterial MaterialTable::getMobMaterialFromName(const std::wstring& mobName)
{
    MobNameToInstanceMap::const_iterator mob = sMobNameToInstanceMap.find(mobName);

    if (mob == sMobNameToInstanceMap.end())
    {
        return eMinecraftEntityMaterial_Invalid;
    }

    return mob->second;
}

void MaterialTable::loadMaterialData(unsigned char* materialData, unsigned int materialDataLength)
{
    arrayWithLength<unsigned char> buffer;
    buffer.data = materialData;
    buffer.length = materialDataLength;

    ByteArrayInputStream byteStream(buffer);
    DataInputStream stream((InputStream*)&byteStream);

    stream.readInt();

    int materialCount = stream.readInt();

    for (int i = 0; i < materialCount; i++)
    {
        std::wstring mobName = stream.readUTF();
        std::wstring materialName = stream.readUTF();

        eMinecraftEntityMaterial material = getMobMaterialFromName(mobName);

        if (material == eMinecraftEntityMaterial_Invalid)
        {
            continue;
        }

        mMaterialShaders[material] = getMaterialShadersFromDataName(materialName);
    }
    byteStream.reset();
}

MaterialTable::MaterialTable(const MaterialTable& materialTable, unsigned char* materialData,
                             unsigned int materialDataLength)
{
    memcpy(mMaterialShaders, materialTable.mMaterialShaders, sizeof(mMaterialShaders));

    loadMaterialData(materialData, materialDataLength);
}

MaterialShaders& MaterialTable::getMaterialShadersFor(eMinecraftEntityMaterial material)
{
    return mMaterialShaders[material];
}

MaterialLookUpMap MaterialTable::generateMaterialLookUPMap()
{
    MaterialLookUpMap materialLookUpMap(11);

    MaterialShaders& entityEmissiveAlpha = materialLookUpMap[L"entity_emissive_alpha"];
    entityEmissiveAlpha.mVertexType = C4JRender::eVertexType_Entity;
    entityEmissiveAlpha.mPixelShaderType = C4JRender::ePixelShaderType_EntityEmissiveAlpha;

    MaterialShaders& entityEmissiveAlphaOnly = materialLookUpMap[L"entity_emissive_alpha_only"];
    entityEmissiveAlphaOnly.mVertexType = C4JRender::eVertexType_Entity;
    entityEmissiveAlphaOnly.mPixelShaderType = C4JRender::ePixelShaderType_EntityEmissiveAlphaOnly;

    MaterialShaders& entityChangeColor = materialLookUpMap[L"entity_change_color"];
    entityChangeColor.mVertexType = C4JRender::eVertexType_Entity;
    entityChangeColor.mPixelShaderType = C4JRender::ePixelShaderType_EntityChangeColour;

    MaterialShaders& entityAlphaTestChangeColor = materialLookUpMap[L"entity_alphatest_change_color"];
    entityAlphaTestChangeColor.mVertexType = C4JRender::eVertexType_Entity;
    entityAlphaTestChangeColor.mPixelShaderType = C4JRender::ePixelShaderType_EntityChangeColour;

    MaterialShaders& entity = materialLookUpMap[L"entity"];
    entity.mVertexType = C4JRender::eVertexType_Entity;
    entity.mPixelShaderType = C4JRender::ePixelShaderType_Default;

    MaterialShaders& entityAlphaTest = materialLookUpMap[L"entity_alphatest"];
    entityAlphaTest.mVertexType = C4JRender::eVertexType_Entity;
    entityAlphaTest.mPixelShaderType = C4JRender::ePixelShaderType_Default;

    return materialLookUpMap;
}

MobNameToInstanceMap MaterialTable::generateMobNameToInstanceMap()
{
    MobNameToInstanceMap mobNameToInstanceMap(11);

    mobNameToInstanceMap[L"bat"] = eMinecraftEntityMaterial_Bat;
    mobNameToInstanceMap[L"ender_dragon"] = eMinecraftEntityMaterial_EnderDragon;
    mobNameToInstanceMap[L"blaze_head"] = eMinecraftEntityMaterial_BlazeHead;
    mobNameToInstanceMap[L"drowned"] = eMinecraftEntityMaterial_Drowned;
    mobNameToInstanceMap[L"enderman"] = eMinecraftEntityMaterial_Enderman;
    mobNameToInstanceMap[L"enderman_invisible"] = eMinecraftEntityMaterial_EndermanInvisible;
    mobNameToInstanceMap[L"ghast"] = eMinecraftEntityMaterial_Ghast;
    mobNameToInstanceMap[L"guardian"] = eMinecraftEntityMaterial_Guardian;
    mobNameToInstanceMap[L"magma_cube"] = eMinecraftEntityMaterial_MagmaCube;
    mobNameToInstanceMap[L"zombie_pigman"] = eMinecraftEntityMaterial_ZombiePigman;
    mobNameToInstanceMap[L"phantom"] = eMinecraftEntityMaterial_Phantom;
    mobNameToInstanceMap[L"phantom_invisible"] = eMinecraftEntityMaterial_PhantomInvisible;
    mobNameToInstanceMap[L"sheep"] = eMinecraftEntityMaterial_Sheep;
    mobNameToInstanceMap[L"shulker"] = eMinecraftEntityMaterial_Shulker;
    mobNameToInstanceMap[L"skeleton"] = eMinecraftEntityMaterial_Skeleton;
    mobNameToInstanceMap[L"spider"] = eMinecraftEntityMaterial_Spider;
    mobNameToInstanceMap[L"spider_invisible"] = eMinecraftEntityMaterial_SpiderInvisible;
    mobNameToInstanceMap[L"stray"] = eMinecraftEntityMaterial_Stray;
    mobNameToInstanceMap[L"iron_golem"] = eMinecraftEntityMaterial_IronGolem;
    mobNameToInstanceMap[L"wither_boss"] = eMinecraftEntityMaterial_WitherBoss;
    mobNameToInstanceMap[L"wither_skeleton"] = eMinecraftEntityMaterial_WitherSkeleton;
    mobNameToInstanceMap[L"wolf"] = eMinecraftEntityMaterial_Wolf;

    return mobNameToInstanceMap;
}
