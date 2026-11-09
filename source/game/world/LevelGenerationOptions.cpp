#include "game/world/LevelGenerationOptions.hpp"
#include "game/core/StringIds.hpp"

#include <climits>

#include <boost/unordered_set.hpp>

#include "game/core/BoundingBox.hpp"
#include "game/core/ByteArrayInputStream.hpp"
#include "game/core/CConsoleMinecraftApp.hpp"
#include "game/core/CMinecraftApp.hpp"
#include "game/core/ConsoleSchematicFile.hpp"
#include "game/core/DLCGameRulesHeader.hpp"
#include "game/core/DLCPack.hpp"
#include "game/core/DataInputStream.hpp"
#include "game/core/GameRulesSource.hpp"
#include "game/core/L10N.hpp"
#include "game/data/GameRuleSaveInterface.hpp"
#include "game/data/StringTable.hpp"
#include "game/minigame/ApplySchematicRuleDefinition.hpp"
#include "game/util/AABB.hpp"
#include "game/util/Random.hpp"
#include "game/world/biome/BiomeOverride.hpp"
#include "game/world/chunk/LevelChunk.hpp"
#include "game/world/generation/ConsoleGenerateStructure.hpp"
#include "platform/wiiu/CProfile.hpp"
#include "platform/wiiu/CStorage.hpp"
#include "game/core/PIXNamedEvents.hpp"
#include "platform/wiiu/NintendoFileSystem_WiiU.hpp"
#include "platform/wiiu/WiiU_FileCache.hpp"

template <typename T>
std::wstring _toString(T value);

template <typename T>
T _fromString(const std::wstring& value);

char* wstringtofilename(const std::wstring& path, char (&filename)[256]);

LevelGenerationOptions::LevelGenerationOptions(DLCPack* pack)
{
    mLoadedData = false;
    mBaseSaveData = NULL;
    mBaseSaveDataSize = 0;
    mHasLevelSeed = false;
    mLevelSeed = 0;
    mUseFlatWorld = false;
    mSpawnPos = NULL;
    mLevelHasBeenInCreative = true;
    mMinYCalculated = false;
    mMinY = INT_MAX;
    mRequiresGameRules = false;
    mRequiredGameRules = NULL;
    mStringTable = NULL;
    mPack = pack;
    mPackMountPending = false;
    mBaseSaveRoot = L"";
    mMapSize = -1;
    mThemeId = -1;
    mCloudHeight = 128;
    mFogDistance = -1;
    mDayTime = 6000;
    mCRCValid = false;
}

LevelGenerationOptions::~LevelGenerationOptions()
{
    clearSchematics();
    delete mSpawnPos;

    for (std::vector<ApplySchematicRuleDefinition*>::iterator it = mSchematicRules.begin(); it != mSchematicRules.end();
         ++it)
    {
        delete *it;
    }

    for (std::vector<ConsoleGenerateStructure*>::iterator it = mStructures.begin(); it != mStructures.end(); ++it)
    {
        delete *it;
    }

    for (std::vector<BiomeOverride*>::iterator it = mBiomeOverrides.begin(); it != mBiomeOverrides.end(); ++it)
    {
        delete *it;
    }

    for (std::vector<StartFeature*>::iterator it = mFeatures.begin(); it != mFeatures.end(); ++it)
    {
        delete *it;
    }

    if (mStringTable != NULL && !isTutorial())
    {
        delete mStringTable;
    }

    if (isFromSave() && mGameRulesSource != NULL)
    {
        delete mGameRulesSource;
    }
}

unsigned int LevelGenerationOptions::getThemeId() const
{
    return mThemeId;
}

unsigned short LevelGenerationOptions::GetThemeFromUniqueId(int uniqueId)
{
    return uniqueId & 0xffff;
}

unsigned int LevelGenerationOptions::CreateUniqueId(int themeId, int mapSize)
{
    return mapSize << 16 | themeId & 0xffff;
}

unsigned int LevelGenerationOptions::getUniqueId()
{
    return CreateUniqueId(mThemeId, mMapSize);
}

GameRulesSource* LevelGenerationOptions::info()
{
    return mGameRulesSource;
}

void LevelGenerationOptions::setSrc(eSrc source)
{
    mSource = source;
}

LevelGenerationOptions::eSrc LevelGenerationOptions::getSrc()
{
    return mSource;
}

bool LevelGenerationOptions::isLocal()
{
    return getSrc() == eSrc_Local || getSrc() == eSrc_Tutorial;
}

bool LevelGenerationOptions::isTutorial()
{
    return getSrc() == eSrc_Tutorial;
}

bool LevelGenerationOptions::isFromSave()
{
    return getSrc() == eSrc_Save;
}

bool LevelGenerationOptions::isFromDLC()
{
    return getSrc() == eSrc_DLC;
}

bool LevelGenerationOptions::requiresTexturePack()
{
    return info()->requiresTexturePack();
}

unsigned int LevelGenerationOptions::getTexturePackId()
{
    return info()->getTexturePackId();
}

const wchar_t* LevelGenerationOptions::getString(const std::wstring& key)
{
    const wchar_t* result = L"";
    if (mStringTable != NULL)
    {
        std::wstring value(mStringTable->getString(key));
        if (key == L"IDS_DISPLAY_NAME" && value == L"")
        {
            return mStringTable->getString(std::wstring(L"title"));
        }

        result = mStringTable->getString(key);
    }

    return result;
}

std::wstring LevelGenerationOptions::getDefaultSaveName()
{
    switch (getSrc())
    {
    case eSrc_Save:
        return getString(info()->getDefaultSaveName());
    case eSrc_DLC:
        return getString(info()->getDefaultSaveName());
    case eSrc_Local:
        return getString(info()->getDefaultSaveName());
    case eSrc_Tutorial:
        return L10N::GetString(IDS_TUTORIALSAVENAME);
    default:
        return L"";
    }
}

const wchar_t* LevelGenerationOptions::getWorldName()
{
    switch (getSrc())
    {
    case eSrc_Save:
        return getString(info()->getWorldName());
    case eSrc_DLC:
        return getString(info()->getWorldName());
    case eSrc_Local:
        return getString(info()->getWorldName());
    case eSrc_Tutorial:
        return L10N::GetString(IDS_PLAY_TUTORIAL);
    default:
        return L"";
    }
}

std::wstring LevelGenerationOptions::getWorldNameKey()
{
    switch (getSrc())
    {
    case eSrc_Save:
        return info()->getWorldName();
    case eSrc_DLC:
        return info()->getWorldName();
    case eSrc_Local:
        return info()->getWorldName();
    default:
        return L"";
    }
}

const wchar_t* LevelGenerationOptions::getDisplayName()
{
    switch (getSrc())
    {
    case eSrc_Save:
        return getString(info()->getDisplayName());
    case eSrc_DLC:
        return getString(info()->getDisplayName());
    case eSrc_Local:
        return getString(info()->getDisplayName());
    default:
        return L"";
    }
}

const wchar_t* LevelGenerationOptions::getWorldDescription()
{
    return getString(mWorldDescription);
}

const wchar_t* LevelGenerationOptions::getWorldPreviewImage()
{
    return mWorldPreviewImage.c_str();
}

std::wstring LevelGenerationOptions::getGrfPath()
{
    return info()->getGrfPath();
}

bool LevelGenerationOptions::requiresBaseSave()
{
    return info()->requiresBaseSave();
}

void LevelGenerationOptions::setBaseSaveRoot(const std::wstring& root)
{
    mBaseSaveRoot = root;
}

std::wstring LevelGenerationOptions::getBaseSaveRoot()
{
    return mBaseSaveRoot;
}

std::wstring LevelGenerationOptions::getBaseSavePath()
{
    return info()->getBaseSavePath();
}

bool LevelGenerationOptions::isGameModeLobby()
{
    return info()->isGameModeLobby();
}

unsigned int LevelGenerationOptions::getMapSize() const
{
    return mMapSize;
}

void LevelGenerationOptions::setGrSource(GameRulesSource* source)
{
    mGameRulesSource = source;
}

void LevelGenerationOptions::setRequiresTexturePack(bool required)
{
    info()->setRequiresTexturePack(required);
}

void LevelGenerationOptions::setTexturePackId(unsigned int id)
{
    info()->setTexturePackId(id);
}

void LevelGenerationOptions::setDefaultSaveName(const std::wstring& name)
{
    info()->setDefaultSaveName(name);
}

void LevelGenerationOptions::setWorldName(const std::wstring& name)
{
    info()->setWorldName(name);
}

void LevelGenerationOptions::setDisplayName(const std::wstring& name)
{
    info()->setDisplayName(name);
}

void LevelGenerationOptions::setWorldDescription(const std::wstring& description)
{
    mWorldDescription = description;
}

void LevelGenerationOptions::setWorldPreviewImage(const std::wstring& image)
{
    mWorldPreviewImage = image.substr(0, image.length() - 4);
}

void LevelGenerationOptions::setGrfPath(const std::wstring& path)
{
    info()->setGrfPath(path);
}

void LevelGenerationOptions::setBaseSavePath(const std::wstring& path)
{
    info()->setBaseSavePath(path);
}

void LevelGenerationOptions::setIsGameModeLobby(bool lobby)
{
    info()->setIsGameModeLobby(lobby);
}

bool LevelGenerationOptions::ready()
{
    return info()->ready();
}

void LevelGenerationOptions::reset_finish()
{
    if (isFromDLC())
    {
        mLoadedData = false;
    }
}

bool LevelGenerationOptions::requiresGameRules()
{
    return mRequiresGameRules;
}

LevelRuleset* LevelGenerationOptions::getRequiredGameRules()
{
    return mRequiredGameRules;
}

void LevelGenerationOptions::reset_start()
{
    for (std::vector<ApplySchematicRuleDefinition*>::iterator it = mSchematicRules.begin(); it != mSchematicRules.end();
         ++it)
    {
        (*it)->reset();
    }
}

void LevelGenerationOptions::loadGameRulesHeaders(DLCPack* pack, StringTable* table, LevelGenerationOptions* options)
{
    int headerCount = pack->getDLCItemsCount(DLCManager::eDLCType_GameRulesHeader);
    for (int i = 0; i < headerCount; ++i)
    {
        DLCGameRulesHeader* header = static_cast<DLCGameRulesHeader*>(
            pack->getFile(DLCManager::eDLCType_GameRulesHeader, i, true, true));
        if (header->m_levelGenerationOptions != options || header->getGrfPath().empty())
            continue;

        File grfFile(gApp.getFilePath(pack->GetPackId(), header->getGrfPath(), true, L"WPACK:"));
        if (!grfFile.exists())
            continue;

        char fileName[256];
        int handle = CreateFileA(wstringtofilename(grfFile.getPath(), fileName), 0x80000000, 0, NULL, 3, 0x8000000, 0);
        if (handle != -1)
        {
            unsigned int size = static_cast<unsigned int>(grfFile.length());
            unsigned char* data = new unsigned char[size];
            unsigned int bytesRead;
            if (ReadFile(handle, data, size, &bytesRead, NULL) == 0)
            {
                gApp.FatalLoadError(L"<LevelGenerationOptions@%d> Failed to read %s", 1136, fileName);
            }
            CloseHandle(handle);
            header->setGrfData(data, size, table);
            delete[] data;
            gApp.mGameRuleManager.setLevelGenerationOptions(header->m_levelGenerationOptions);
        }
    }
}

int LevelGenerationOptions::packMounted(void* data, int mountIndex, unsigned int mounted, unsigned int flags)
{
    LevelGenerationOptions* options = static_cast<LevelGenerationOptions*>(data);
    options->mPackMountPending = false;
    if (mounted == 0)
    {
        loadGameRulesHeaders(options->mPack, options->mStringTable, options);
        if (options->requiresBaseSave() && !options->getBaseSavePath().empty() && !options->hasBaseSaveData())
        {
            options->setBaseSaveData(
                File(gApp.getFilePath(options->mPack->GetPackId(), options->getBaseSavePath(), true, L"WPACK:")));
        }
        gStorage->UnmountInstalledDLC("WPACK");
    }
    options->setLoadedData();
    return 0;
}

void LevelGenerationOptions::loadBaseSaveData()
{
    if (mPack == NULL)
    {
        loadGameRulesHeaders(NULL, mStringTable, this);
    }
    else
    {
        int mountIndex = static_cast<int>(mPack->GetDLCMountIndex());
        if (mountIndex >= 0)
        {
            if (gStorage->MountInstalledDLC(gProfile->GetPrimaryPad(), mountIndex, packMounted, this, "WPACK") == 0x3e5)
            {
                mPackMountPending = true;
                return;
            }
            setLoadedData();
            return;
        }
        loadGameRulesHeaders(mPack, mStringTable, this);
    }

    if (requiresBaseSave() && !getBaseSavePath().empty())
    {
        setBaseSaveData(File(gApp.getFilePath(mPack->GetPackId(), getBaseSavePath(), true, L"WPACK:")));
    }
    setLoadedData();
}

void LevelGenerationOptions::deleteBaseSaveData()
{
    if (mBaseSaveData != NULL)
    {
        delete mBaseSaveData;
    }
    mBaseSaveData = NULL;
    mBaseSaveDataSize = 0;
}

void LevelGenerationOptions::setBaseSaveData(unsigned char* data, unsigned int size)
{
    if (mBaseSaveData != NULL)
    {
        deleteBaseSaveData();
    }

    mBaseSaveData = data;
    mBaseSaveDataSize = size;
}

void LevelGenerationOptions::setBaseSaveData(File file)
{
    if (!file.exists())
    {
        return;
    }

    char fileName[256];
    char* path = wstringtofilename(file.getPath(), fileName);
    int handle = CreateFileA(path, 0x80000000, 0, NULL, 3, 0x8000000, 0);
    if (handle != -1)
    {
        unsigned int size = GetFileSize(handle, NULL);
        unsigned char* data = new unsigned char[size];
        unsigned int bytesRead;
        if (ReadFile(handle, data, size, &bytesRead, NULL) == 0)
        {
            gApp.FatalLoadError(L"<LevelGenerationOptions@%d> Failed to read %s", 1193, path);
        }
        CloseHandle(handle);
        setBaseSaveData(data, size);
    }
}

unsigned char* LevelGenerationOptions::getBaseSaveData(unsigned int& size)
{
    size = mBaseSaveDataSize;
    return mBaseSaveData;
}

bool LevelGenerationOptions::hasBaseSaveData()
{
    return mBaseSaveDataSize != 0 && mBaseSaveData != NULL;
}

bool LevelGenerationOptions::hasLoadedData()
{
    return mLoadedData;
}

void LevelGenerationOptions::setLoadedData()
{
    mLoadedData = true;
}

void LevelGenerationOptions::clearSchematics()
{
    for (SchematicFileMap::iterator it = mSchematicFiles.begin(); it != mSchematicFiles.end(); ++it)
    {
        delete it->second;
    }

    mSchematicFiles.clear();
}

unsigned int LevelGenerationOptions::getParentPackId()
{
    if (mPack != NULL)
    {
        return mPack->GetPackId();
    }

    return 0;
}

void LevelGenerationOptions::writeAttributes(GameRuleSaveInterface* saveInterface, unsigned int extraAttributeCount)
{
    unsigned int attributeCount = 5;
    if (mSpawnPos != NULL)
    {
        attributeCount += 3;
    }
    if (mHasLevelSeed)
    {
        ++attributeCount;
    }

    GameRuleDefinition::writeAttributes(saveInterface, extraAttributeCount + attributeCount);

    if (mSpawnPos != NULL)
    {
        saveInterface->saveAttribute(ConsoleGameRules::eGameRuleAttr_spawnX, _toString(mSpawnPos->x));
        saveInterface->saveAttribute(ConsoleGameRules::eGameRuleAttr_spawnY, _toString(mSpawnPos->y));
        saveInterface->saveAttribute(ConsoleGameRules::eGameRuleAttr_spawnZ, _toString(mSpawnPos->z));
    }
    if (mHasLevelSeed)
    {
        saveInterface->saveAttribute(ConsoleGameRules::eGameRuleAttr_seed, _toString(mLevelSeed));
    }

    saveInterface->saveAttribute(ConsoleGameRules::eGameRuleAttr_flatworld, _toString(mUseFlatWorld));
    saveInterface->saveAttribute(ConsoleGameRules::eGameRuleAttr_hasBeenInCreative, _toString(mLevelHasBeenInCreative));
    saveInterface->saveAttribute(ConsoleGameRules::eGameRuleAttr_cloudHeight, _toString(mCloudHeight));
    saveInterface->saveAttribute(ConsoleGameRules::eGameRuleAttr_fogDistance, _toString(mFogDistance));
    saveInterface->saveAttribute(ConsoleGameRules::eGameRuleAttr_dayTime, _toString(mDayTime));
}

void LevelGenerationOptions::getChildren(std::vector<GameRuleDefinition*>* children)
{
    for (std::vector<ConsoleGenerateStructure*>::iterator it = mStructures.begin(); it != mStructures.end(); ++it)
    {
        children->push_back(*it);
    }

    for (std::vector<ApplySchematicRuleDefinition*>::iterator it = mSchematicRules.begin(); it != mSchematicRules.end();
         ++it)
    {
        if (!(*it)->isComplete())
        {
            children->push_back(*it);
        }
    }

    for (std::vector<BiomeOverride*>::iterator it = mBiomeOverrides.begin(); it != mBiomeOverrides.end(); ++it)
    {
        children->push_back(*it);
    }

    for (std::vector<StartFeature*>::iterator it = mFeatures.begin(); it != mFeatures.end(); ++it)
    {
        children->push_back(*it);
    }
}

GameRuleDefinition* LevelGenerationOptions::addChild(ConsoleGameRules::EGameRuleType type)
{
    GameRuleDefinition* child = NULL;

    if (type == ConsoleGameRules::eGameRuleType_ApplySchematic)
    {
        child = new ApplySchematicRuleDefinition(this);
        mSchematicRules.push_back(static_cast<ApplySchematicRuleDefinition*>(child));
    }
    else if (type == ConsoleGameRules::eGameRuleType_GenerateStructure)
    {
        child = new ConsoleGenerateStructure();
        mStructures.push_back(static_cast<ConsoleGenerateStructure*>(child));
    }
    else if (type == ConsoleGameRules::eGameRuleType_BiomeOverride)
    {
        child = new BiomeOverride();
        mBiomeOverrides.push_back(static_cast<BiomeOverride*>(child));
    }
    else if (type == ConsoleGameRules::eGameRuleType_StartFeature)
    {
        child = new StartFeature();
        mFeatures.push_back(static_cast<StartFeature*>(child));
    }

    return child;
}

void LevelGenerationOptions::addAttribute(const std::wstring& name, const std::wstring& value)
{
    if (name == L"seed")
    {
        mHasLevelSeed = true;
        mLevelSeed = _fromString<long long>(value);
        return;
    }

    if (name == L"spawnX")
    {
        BlockPos spawn;
        if (mSpawnPos != NULL)
        {
            spawn = *mSpawnPos;
            delete mSpawnPos;
        }
        mSpawnPos = new BlockPos(_fromString<int>(value), spawn.y, spawn.z);
        return;
    }

    if (name == L"spawnY")
    {
        BlockPos spawn;
        if (mSpawnPos != NULL)
        {
            spawn = *mSpawnPos;
            delete mSpawnPos;
        }
        mSpawnPos = new BlockPos(spawn.x, _fromString<int>(value), spawn.z);
        return;
    }

    if (name == L"spawnZ")
    {
        BlockPos spawn;
        if (mSpawnPos != NULL)
        {
            spawn = *mSpawnPos;
            delete mSpawnPos;
        }
        mSpawnPos = new BlockPos(spawn.x, spawn.y, _fromString<int>(value));
        return;
    }

    if (name == L"flatworld")
    {
        if (value == L"true")
            mUseFlatWorld = true;
        return;
    }
    if (name == L"saveName")
    {
        std::wstring saveName(value);
        setDefaultSaveName(saveName);
        return;
    }
    if (name == L"worldName")
    {
        std::wstring worldName(value);
        setWorldName(worldName);
        return;
    }
    if (name == L"displayName")
    {
        std::wstring displayName(value);
        setDisplayName(displayName);
        return;
    }
    if (name == L"worldDescription")
    {
        std::wstring worldDescription(value);
        setWorldDescription(worldDescription);
        return;
    }
    if (name == L"worldPreviewImage")
    {
        std::wstring previewImage(value);
        setWorldPreviewImage(previewImage);
        return;
    }
    if (name == L"texturePackId")
    {
        setTexturePackId(_fromString<unsigned int>(value));
        setRequiresTexturePack(true);
        return;
    }
    if (name == L"texturePackRequired")
    {
        if (value == L"false")
            setRequiresTexturePack(false);
        return;
    }
    if (name == L"isTutorial")
    {
        if (value == L"true")
            setSrc(eSrc_Tutorial);
        return;
    }
    if (name == L"baseSaveName")
    {
        setBaseSavePath(value);
        return;
    }
    if (name == L"hasBeenInCreative")
    {
        mLevelHasBeenInCreative = _fromString<bool>(value);
        return;
    }
    if (name == L"mapSize")
    {
        mMapSize = _fromString<int>(value);
        return;
    }
    if (name == L"themeId")
    {
        mThemeId = _fromString<int>(value);
        return;
    }
    if (name == L"cloudHeight")
    {
        mCloudHeight = _fromString<int>(value);
        return;
    }
    if (name == L"fogDistance")
    {
        mFogDistance = _fromString<int>(value);
        return;
    }
    if (name == L"dayTime")
    {
        mDayTime = _fromString<int>(value);
        return;
    }

    GameRuleDefinition::addAttribute(name, value);
}

bool LevelGenerationOptions::hasLevelSeed()
{
    return mHasLevelSeed;
}

long long LevelGenerationOptions::getLevelSeed()
{
    return mLevelSeed;
}

bool LevelGenerationOptions::getLevelHasBeenInCreative()
{
    return mLevelHasBeenInCreative;
}

BlockPos* LevelGenerationOptions::getSpawnPos()
{
    return mSpawnPos;
}

bool LevelGenerationOptions::getuseFlatWorld()
{
    return mUseFlatWorld;
}

int LevelGenerationOptions::getCloudHeight()
{
    return mCloudHeight;
}

int LevelGenerationOptions::getFogDistance()
{
    return mFogDistance;
}

int LevelGenerationOptions::getDayTime()
{
    return mDayTime;
}

unsigned short LevelGenerationOptions::GetMapSizeFromUniqueId(int uniqueId)
{
    return static_cast<unsigned int>(uniqueId) >> 16;
}

ConsoleSchematicFile* LevelGenerationOptions::getSchematicFile(const std::wstring& name) const
{
    SchematicFileMap::const_iterator it = mSchematicFiles.find(name);
    return it != mSchematicFiles.end() ? it->second : NULL;
}

void LevelGenerationOptions::processSchematics(LevelChunk* chunk)
{
    PIXBeginNamedEvent(0.0f, "Processing schematics for chunk (%d,%d)", chunk->x, chunk->z);

    int minX = chunk->x * 16;
    int minZ = chunk->z * 16;
    AABB* chunkBox = AABB::newTemp(minX, 0.0, minZ, minX + 16, 256.0, minZ + 16);

    for (std::vector<ApplySchematicRuleDefinition*>::iterator it = mSchematicRules.begin(); it != mSchematicRules.end();
         ++it)
    {
        (*it)->processSchematic(chunkBox, chunk);
    }

    int chunkMinX = chunk->x * 16;
    int chunkMinZ = chunk->z * 16;
    for (std::vector<ConsoleGenerateStructure*>::iterator it = mStructures.begin(); it != mStructures.end(); ++it)
    {
        ConsoleGenerateStructure* structure = *it;
        if (structure->getBoundingBox()->intersects(chunkMinX, chunkMinZ, chunkMinX + 15, chunkMinZ + 15))
        {
            Random random;
            BoundingBox* box = new BoundingBox(chunkMinX, chunkMinZ, chunkMinX + 15, chunkMinZ + 15);
            structure->postProcess(chunk->level, random, box);
            delete box;
        }
    }

    PIXEndNamedEvent();
}

void LevelGenerationOptions::processSchematicsLighting(LevelChunk* chunk)
{
    PIXBeginNamedEvent(0.0f, "Processing schematics (lighting) for chunk (%d,%d)", chunk->x, chunk->z);

    int minX = chunk->x * 16;
    int minZ = chunk->z * 16;
    AABB* chunkBox = AABB::newTemp(minX, 0.0, minZ, minX + 16, 256.0, minZ + 16);

    for (std::vector<ApplySchematicRuleDefinition*>::iterator it = mSchematicRules.begin(); it != mSchematicRules.end();
         ++it)
    {
        (*it)->processSchematicLighting(chunkBox, chunk);
    }

    PIXEndNamedEvent();
}

bool LevelGenerationOptions::checkIntersects(const BlockPos& min, const BlockPos& max)
{
    PIXBeginNamedEvent(0.0f, "Check Intersects");

    if (!mMinYCalculated)
    {
        for (std::vector<ApplySchematicRuleDefinition*>::iterator it = mSchematicRules.begin();
             it != mSchematicRules.end(); ++it)
        {
            int minY = (*it)->getMinY();
            if (minY < mMinY)
                mMinY = minY;
        }
        for (std::vector<ConsoleGenerateStructure*>::iterator it = mStructures.begin(); it != mStructures.end(); ++it)
        {
            int minY = (*it)->getMinY();
            if (minY < mMinY)
                mMinY = minY;
        }
        mMinYCalculated = true;
    }

    if (mMinY > max.y)
    {
        return false;
    }

    bool intersects = false;
    for (std::vector<ApplySchematicRuleDefinition*>::iterator it = mSchematicRules.begin(); it != mSchematicRules.end();
         ++it)
    {
        intersects = (*it)->checkIntersects(min, max);
        if (intersects)
        {
            break;
        }
    }
    if (!intersects)
    {
        for (std::vector<ConsoleGenerateStructure*>::iterator it = mStructures.begin(); it != mStructures.end(); ++it)
        {
            intersects = (*it)->checkIntersects(min, max);
            if (intersects)
            {
                break;
            }
        }
    }

    PIXEndNamedEvent();
    return intersects;
}

void LevelGenerationOptions::setCRCIsValid(bool valid)
{
    mCRCValid = valid;
}

bool LevelGenerationOptions::isCRCValid()
{
    return mCRCValid;
}

ConsoleSchematicFile* LevelGenerationOptions::loadSchematicFile(const std::wstring& name, unsigned char* data,
                                                                unsigned int size)
{
    SchematicFileMap::iterator found = mSchematicFiles.find(name);
    if (found != mSchematicFiles.end())
    {
        ++found->second->mRefCount;
        return found->second;
    }

    arrayWithLength<unsigned char> bytes;
    bytes.data = data;
    bytes.length = size;
    ByteArrayInputStream byteStream(bytes);
    DataInputStream input(&byteStream);
    ConsoleSchematicFile* schematic = new ConsoleSchematicFile();
    schematic->load(&input);
    mSchematicFiles[name] = schematic;
    byteStream.reset();
    return schematic;
}

void LevelGenerationOptions::setRequiredGameRules(LevelRuleset* ruleset)
{
    mRequiredGameRules = ruleset;
    mRequiresGameRules = true;
}

void LevelGenerationOptions::getBiomeOverride(int biome, Biome::BlockPair* top, Biome::BlockPair* filler)
{
    std::vector<BiomeOverride*>::iterator end = mBiomeOverrides.end();
    for (std::vector<BiomeOverride*>::iterator it = mBiomeOverrides.begin(); it != end; ++it)
    {
        BiomeOverride* biomeOverride = *it;
        if (biomeOverride->isBiome(biome))
        {
            biomeOverride->getBlockValues(top, filler);
            return;
        }
    }
}

bool LevelGenerationOptions::isFeatureChunk(int chunkX, int chunkZ, StructureFeature::EFeatureTypes type,
                                            const Direction** direction)
{
    bool found = false;
    for (std::vector<StartFeature*>::iterator it = mFeatures.begin(); it != mFeatures.end(); ++it)
    {
        if ((*it)->isFeatureChunk(chunkX, chunkZ, type, direction))
        {
            found = true;
            break;
        }
    }
    return found;
}

bool LevelGenerationOptions::hasFeature(StructureFeature::EFeatureTypes type)
{
    bool found = false;
    std::vector<StartFeature*>::iterator end = mFeatures.end();
    for (std::vector<StartFeature*>::iterator it = mFeatures.begin(); it != end; ++it)
    {
        if ((*it)->getFeatureType() == type)
        {
            found = true;
            break;
        }
    }
    return found;
}

BlockPos* LevelGenerationOptions::getClosestFeature(StructureFeature::EFeatureTypes type, const BlockPos& pos)
{
    int chunkX = pos.x >> 4;
    int chunkZ = pos.z >> 4;
    int closestX = 0;
    int closestZ = 0;
    int closestDistance = INT_MAX;

    std::vector<StartFeature*>::iterator end = mFeatures.end();
    for (std::vector<StartFeature*>::iterator it = mFeatures.begin(); it != end; ++it)
    {
        StartFeature* feature = *it;
        if (feature->getFeatureType() == type)
        {
            int featureChunkX = feature->getChunkX();
            int xDistance = chunkX - featureChunkX;
            int featureChunkZ = feature->getChunkZ();
            int zDistance = chunkZ - featureChunkZ;
            int distance = xDistance * xDistance + zDistance * zDistance;
            if (distance < closestDistance)
            {
                closestX = featureChunkX;
                closestZ = featureChunkZ;
                closestDistance = distance;
            }
        }
    }

    return new BlockPos(closestX * 16 + 8, 64, closestZ * 16 + 8);
}

void LevelGenerationOptions::loadStringTable(StringTable* table)
{
    mStringTable = table;
}

void LevelGenerationOptions::copyStringToTable(const std::wstring& value, StringTable* table)
{
    if (mStringTable != NULL)
    {
        mStringTable->copyStringToTable(value, table);
    }
}

LevelGenerationOptions::SchematicFileMap* LevelGenerationOptions::getUnfinishedSchematicFiles()
{
    boost::unordered_set<std::wstring> names;
    for (std::vector<ApplySchematicRuleDefinition*>::iterator it = mSchematicRules.begin(); it != mSchematicRules.end();
         ++it)
    {
        if (!(*it)->isComplete())
        {
            names.insert((*it)->getSchematicName());
        }
    }

    SchematicFileMap* files = new SchematicFileMap();
    for (boost::unordered_set<std::wstring>::iterator it = names.begin(); it != names.end(); ++it)
    {
        (*files)[*it] = getSchematicFile(*it);
    }
    return files;
}

ConsoleGameRules::EGameRuleType LevelGenerationOptions::getActionType()
{
    return ConsoleGameRules::eGameRuleType_MapOptions;
}
