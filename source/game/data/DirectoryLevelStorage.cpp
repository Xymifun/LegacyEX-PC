#include "game/data/DirectoryLevelStorage.hpp"

#include <algorithm>
#include <cfloat>

#include "game/core/ByteArrayInputStream.hpp"
#include "game/core/ByteArrayOutputStream.hpp"
#include "game/core/CConsoleMinecraftApp.hpp"
#include "game/core/CMinecraftApp.hpp"
#include "game/core/DataInputStream.hpp"
#include "game/core/DataOutputStream.hpp"
#include "game/core/FileEntry.hpp"
#include "game/core/NbtIo.hpp"
#include "game/core/StringConversion.hpp"
#include "game/core/StringUtils.hpp"
#include "game/core/System.hpp"
#include "game/data/CompoundTag.hpp"
#include "game/data/ConsoleSaveFile.hpp"
#include "game/data/ConsoleSaveFileInputStream.hpp"
#include "game/data/ConsoleSaveFileOutputStream.hpp"
#include "game/data/DataFixerUpper.hpp"
#include "game/data/DirectoryLevelStorageSource.hpp"
#include "game/data/DoubleTag.hpp"
#include "game/data/ListTag.hpp"
#include "game/entity/mob/Entity.hpp"
#include "game/world/Dimension.hpp"
#include "game/world/LevelData.hpp"
#include "game/world/NetherDimension.hpp"
#include "game/world/TheEndDimension.hpp"
#include "game/world/chunk/OldChunkStorage.hpp"
#include "game/world/generation/Player.hpp"
#include "game/world/generation/StructureManager.hpp"
#include "platform/wiiu/CStorage.hpp"
#include "vendor/_MapDataMappings.hpp"
#include "vendor/_MapDataMappings_old.hpp"

static const std::wstring PLAYER_DIR = L"players/";
const std::wstring DirectoryLevelStorage::NETHER_FOLDER = L"DIM-1";
const std::wstring DirectoryLevelStorage::ENDER_FOLDER = L"DIM1/";

DirectoryLevelStorage::DirectoryLevelStorage(ConsoleSaveFile* saveFile, File folder, const std::wstring& levelId,
                                             bool createIfMissing, DataFixerUpper* fixer)
    : mFolder(L""),
      mPlayerDir(PLAYER_DIR),
      mDataDir(std::wstring(L"data/")),
      mSessionId(System::processTimeInMilliSecs()),
      mLevelId(levelId)
{
    mFixer = fixer;
    mHasLoadedMapDataMappings = false;
    mSaveFile = saveFile;
#ifdef MINECRAFT_PC
    mUsedMappings = arrayWithLength<unsigned char>(128, true);
#else
    mUsedMappings = arrayWithLength<unsigned char>(32, true);
#endif
    mStructureManager = createIfMissing ? new StructureManager(fixer) : NULL;
}

DirectoryLevelStorage::~DirectoryLevelStorage()
{
    delete mSaveFile;

    for (CachedSaveDataMap::iterator it = mCachedSaveData.begin(); it != mCachedSaveData.end(); ++it)
        delete it->second;

    delete mUsedMappings.data;

    if (mStructureManager != NULL)
    {
        delete mStructureManager;
        mStructureManager = NULL;
    }
}

void DirectoryLevelStorage::dontSaveMapMappingForPlayer(PlayerUID uid)
{
    PlayerMappingsMap::iterator it = mPlayerMappings.find(uid);
    if (it == mPlayerMappings.end())
        return;

    boost::unordered_map<long long, short>& mappings = it->second.mMappings;
    for (boost::unordered_map<long long, short>::iterator mapping = mappings.begin(); mapping != mappings.end();
         ++mapping)
    {
        short id = mapping->second;
        mUsedMappings.data[id / 8] &= ~(1 << (id % 8));
    }
    mPlayerMappings.erase(it);
}

void DirectoryLevelStorage::save(boost::shared_ptr<Player> player)
{
    PlayerUID uid = player->mXuid;
    bool keep = !player->mIsGuest;
#ifdef MINECRAFT_PC
    keep = keep || (uid.flags & PlayerUID::FlagUuid) != 0;
#endif
    if (uid != PlayerUID(0) && keep)
    {
        CompoundTag* tag = new CompoundTag();
        player->saveWithoutId(tag);

        ConsoleSavePath file(std::wstring(mPlayerDir) + player->getXuid().toString() + L".dat");
        if (gStorage->GetSaveDisabled())
        {
            ByteArrayOutputStream* bytes = new ByteArrayOutputStream();
            NbtIo::writeCompressed(tag, bytes);

            CachedSaveDataMap::iterator it = mCachedSaveData.find(file.getPath());
            if (it != mCachedSaveData.end())
                delete it->second;
            mCachedSaveData[file.getPath()] = bytes;
        }
        else
        {
            ConsoleSaveFileOutputStream output(mSaveFile, file);
            NbtIo::writeCompressed(tag, &output);
        }

        delete tag;
    }
    else if (uid != PlayerUID(0))
    {
        dontSaveMapMappingForPlayer(uid);
    }
}

CompoundTag* DirectoryLevelStorage::loadPlayerDataTag(PlayerUID uid)
{
    ConsoleSavePath file(std::wstring(mPlayerDir) + uid.toString() + L".dat");

    std::wstring path = file.getPath();
    CachedSaveDataMap::iterator it = mCachedSaveData.find(path);
    if (it != mCachedSaveData.end())
    {
        ByteArrayOutputStream* bytes = it->second;
        ByteArrayInputStream input(bytes->mBuffer, 0, bytes->mCount);
        CompoundTag* tag = NbtIo::readCompressed(&input);
        input.reset();
        return tag;
    }
    else if (mSaveFile->doesFileExist(file))
    {
        ConsoleSaveFileInputStream input(mSaveFile, file);
        return NbtIo::readCompressed(&input);
    }
    return NULL;
}

CompoundTag* DirectoryLevelStorage::load(boost::shared_ptr<Player> player)
{
    CompoundTag* tag = loadPlayerDataTag(player->mXuid);
    if (tag != NULL)
        player->load(mFixer->updateData(DataFixTypes::eDataFixTypes_Player, tag));
    return tag;
}

void DirectoryLevelStorage::deleteMapFilesForPlayer(PlayerUID uid)
{
    PlayerMappingsMap::iterator it = mPlayerMappings.find(uid);
    if (it == mPlayerMappings.end())
        return;

    boost::unordered_map<long long, short>& mappings = it->second.mMappings;
    for (boost::unordered_map<long long, short>::iterator mapping = mappings.begin(); mapping != mappings.end();
         ++mapping)
    {
        std::wstring name = std::wstring(L"map_") + _toString<short>(mapping->second);
        ConsoleSavePath file = getDataFile(name);
        if (mSaveFile->doesFileExist(file))
        {
            if (gStorage->GetSaveDisabled())
                mMapFilesToDelete.push_back(mapping->second);
            else
                mSaveFile->deleteFile(mSaveFile->createFile(file));
        }

        short id = mapping->second;
        mUsedMappings.data[id / 8] &= ~(1 << (id % 8));
    }
    mPlayerMappings.erase(it);
}

void DirectoryLevelStorage::clearOldPlayerFiles()
{
    if (gStorage->GetSaveDisabled())
        return;

    std::vector<FileEntry*>* files = mSaveFile->getFilesWithPrefix(std::wstring(mPlayerDir));
    if (files == NULL)
        return;

    if (files->size() > MAX_PLAYER_DATA_SAVES)
    {
        std::sort(files->begin(), files->end(), FileEntry::newestFirst);

        for (unsigned int i = MAX_PLAYER_DATA_SAVES; i < files->size(); ++i)
        {
            FileEntry* file = files->at(i);
            std::wstring xuidString = replaceAll(replaceAll(file->m_fileName, std::wstring(mPlayerDir), L""), L".dat",
                                                 L"");
            PlayerUID xuid(xuidString);
            deleteMapFilesForPlayer(xuid);
            mSaveFile->deleteFile(files->at(i));
        }
    }
    delete files;
}

void DirectoryLevelStorage::saveMapIdLookup()
{
    if (gStorage->GetSaveDisabled())
        return;

    ConsoleSavePath file = getDataFile(L"largeMapDataMappings");
    if (!file.getPath().empty())
    {
        FileEntry* entry = mSaveFile->createFile(file);
        mSaveFile->setFilePointer(entry, 0, NULL, 0);

        ByteArrayOutputStream bytes;
        DataOutputStream output(&bytes);
        output.writeInt(mPlayerMappings.size());
        for (PlayerMappingsMap::iterator it = mPlayerMappings.begin(); it != mPlayerMappings.end(); ++it)
        {
            output.writePlayerUID(it->first);
            it->second.writeMappings(&output);
        }
        output.write(mUsedMappings);

        unsigned int bytesWritten;
        mSaveFile->writeFile(entry, bytes.mBuffer.data, bytes.mCount, &bytesWritten);
    }
}

void DirectoryLevelStorage::deleteMapFilesForPlayer(boost::shared_ptr<Player> player)
{
    PlayerUID uid = player->mXuid;
    if (uid != PlayerUID(0))
        deleteMapFilesForPlayer(uid);
}

void DirectoryLevelStorage::saveAllCachedData()
{
    if (gStorage->GetSaveDisabled())
        return;

    for (CachedSaveDataMap::iterator it = mCachedSaveData.begin(); it != mCachedSaveData.end(); ++it)
    {
        ByteArrayOutputStream* bytes = it->second;
        ConsoleSavePath file(it->first);
        ConsoleSaveFileOutputStream output(mSaveFile, file);
        output.write(bytes->mBuffer, 0, bytes->mCount);
        delete bytes;
    }
    mCachedSaveData.clear();

    for (std::vector<short>::iterator it = mMapFilesToDelete.begin(); it != mMapFilesToDelete.end(); ++it)
    {
        std::wstring name = std::wstring(L"map_") + _toString<short>(*it);
        ConsoleSavePath file = getDataFile(name);
        if (mSaveFile->doesFileExist(file))
            mSaveFile->deleteFile(mSaveFile->createFile(file));
    }
    mMapFilesToDelete.clear();
}

void DirectoryLevelStorage::flushSaveFile(bool force)
{
    mSaveFile->Flush(force, true);
}

LevelData* DirectoryLevelStorage::prepareLevel()
{
    ConsoleSavePath mapFile = getDataFile(L"largeMapDataMappings");
    bool legacyMappings = false;
    if (!mHasLoadedMapDataMappings && !getSaveFile()->doesFileExist(mapFile))
    {
        mapFile = getDataFile(L"mapDataMappings");
        legacyMappings = true;
    }

    if (!mHasLoadedMapDataMappings && getSaveFile()->doesFileExist(mapFile))
    {
        FileEntry* entry = getSaveFile()->createFile(mapFile);
        getSaveFile()->setFilePointer(entry, 0, NULL, 0);

        unsigned int bytesRead;
        if (legacyMappings)
        {
            if (getSaveFile()->getSaveVersion() < 5)
            {
                _MapDataMappings_old oldMapDataMappings;
                getSaveFile()->readFile(entry, &oldMapDataMappings, sizeof(oldMapDataMappings), &bytesRead);
                for (unsigned int id = 0; id < 256; ++id)
                {
                    PlayerUID xuid = oldMapDataMappings.xuids[id];
                    if (xuid != PlayerUID(0))
                    {
                        mPlayerMappings[xuid].addMapping(id, 0, 0, oldMapDataMappings.getDimension(id), 3);
                        mUsedMappings.data[id >> 3] |= 1 << (id & 7);
                    }
                }
            }
            else
            {
                _MapDataMappings mapDataMappings;
                getSaveFile()->readFile(entry, &mapDataMappings, sizeof(mapDataMappings), &bytesRead);
                for (unsigned int id = 0; id < 256; ++id)
                {
                    PlayerUID xuid = mapDataMappings.xuids[id];
                    if (xuid != PlayerUID(0))
                    {
                        mPlayerMappings[xuid].addMapping(id, 0, 0, mapDataMappings.getDimension(id), 3);
                        mUsedMappings.data[id >> 3] |= 1 << (id & 7);
                    }
                }
            }
        }
        else
        {
            arrayWithLength<unsigned char> data(entry->m_data.m_fileSize, true);
            getSaveFile()->readFile(entry, data.data, entry->m_data.m_fileSize, &bytesRead);

            ByteArrayInputStream bytes(data);
            DataInputStream input(&bytes);
            int count = input.readInt();
            for (int i = 0; i < count; ++i)
            {
                PlayerUID uid = input.readPlayerUID();
                mPlayerMappings[uid].readMappings(&input);
            }
#ifdef MINECRAFT_PC
            for (unsigned int i = 0; i < mUsedMappings.length && bytes.mPosition < bytes.mCount; ++i)
                mUsedMappings.data[i] = static_cast<unsigned char>(input.readByte());
#else
            input.readFully(mUsedMappings);
#endif
        }

        if (getSaveFile()->getSaveVersion() < 5)
            saveMapIdLookup();

        mHasLoadedMapDataMappings = true;
    }

    ConsoleSavePath levelFile(L"level.dat");
    if (mSaveFile->doesFileExist(levelFile))
    {
        LevelData* levelData = DirectoryLevelStorageSource::getLevelData(mSaveFile, levelFile, mFixer);
        if (levelData != NULL)
            return levelData;
    }
    return NULL;
}

void DirectoryLevelStorage::checkSession()
{
}

ChunkStorage* DirectoryLevelStorage::createChunkStorage(Dimension* dimension)
{
    if (dynamic_cast<NetherDimension*>(dimension) != NULL)
    {
        File netherFolder(mFolder, NETHER_FOLDER);
        return new OldChunkStorage(netherFolder, true);
    }
    if (dynamic_cast<TheEndDimension*>(dimension) != NULL)
    {
        File enderFolder(mFolder, ENDER_FOLDER);
        return new OldChunkStorage(enderFolder, true);
    }
    return new OldChunkStorage(mFolder, true);
}

void DirectoryLevelStorage::saveLevelData(LevelData* levelData, std::vector<boost::shared_ptr<Player> >* players)
{
    CompoundTag* dataTag = levelData->createTag(players);
    CompoundTag* root = new CompoundTag();
    root->put(L"Data", dataTag);

    ConsoleSavePath file(std::wstring(L"level.dat"));
    ConsoleSaveFileOutputStream output(mSaveFile, file);
    NbtIo::writeCompressed(root, &output);
    delete root;
}

void DirectoryLevelStorage::saveLevelData(LevelData* levelData)
{
    CompoundTag* dataTag = levelData->createTag();
    CompoundTag* root = new CompoundTag();
    root->put(L"Data", dataTag);

    ConsoleSavePath file(std::wstring(L"level.dat"));
    ConsoleSaveFileOutputStream output(mSaveFile, file);
    NbtIo::writeCompressed(root, &output);
    delete root;
}

PlayerIO* DirectoryLevelStorage::getPlayerIO()
{
    return this;
}

void DirectoryLevelStorage::closeAll()
{
}

File DirectoryLevelStorage::getFolder()
{
    return mFolder;
}

ConsoleSavePath DirectoryLevelStorage::getDataFile(const std::wstring& name)
{
    return ConsoleSavePath(std::wstring(mDataDir) + name + L".dat");
}

std::wstring DirectoryLevelStorage::getLevelId()
{
    return mLevelId;
}

StructureManager* DirectoryLevelStorage::getStructureManager()
{
    return mStructureManager;
}

ConsoleSaveFile* DirectoryLevelStorage::getSaveFile()
{
    return mSaveFile;
}

int DirectoryLevelStorage::getAuxValueForMap(PlayerUID uid, int dimension, int centreX, int centreZ, int scale)
{
    int id = -1;
    PlayerMappingsMap::iterator it = mPlayerMappings.find(uid);
    if (it == mPlayerMappings.end() || !it->second.getMapping(id, centreX, centreZ, dimension, scale))
    {
        for (unsigned int i = 0; i < mUsedMappings.length; ++i)
        {
            if (mUsedMappings.data[i] < 0xff)
            {
                int bit;
                for (bit = 0; bit < 8; ++bit)
                {
                    if ((mUsedMappings.data[i] & (1 << bit)) == 0)
                        break;
                }

                id = i * 8 + bit;
                mPlayerMappings[uid].addMapping(id, centreX, centreZ, dimension, scale);
                mUsedMappings.data[i] |= 1 << bit;
                break;
            }
        }
    }
    return id;
}

void DirectoryLevelStorage::resetPlayerPositionsInDimension(int dimension)
{
    std::vector<FileEntry*>* files = mSaveFile->getFilesWithPrefix(std::wstring(mPlayerDir));
    if (files == NULL)
        return;

    for (std::vector<FileEntry*>::iterator it = files->begin(); it != files->end(); ++it)
    {
        FileEntry* file = *it;
        ConsoleSaveFileInputStream input(mSaveFile, file);
        CompoundTag* tag = NbtIo::readCompressed(&input);
        if (tag == NULL)
            continue;

        if (tag->contains(L"Dimension") && tag->getInt(L"Dimension") == dimension)
        {
            if (tag->contains(L"Pos"))
            {
                ListTag* pos = tag->getList(L"Pos", Tag::TAG_DOUBLE);
                static_cast<DoubleTag*>(pos->get(1))->mValue = DBL_MAX;

                ConsoleSaveFileOutputStream output(mSaveFile, file);
                NbtIo::writeCompressed(tag, &output);
            }
        }
        delete tag;
    }
    delete files;
}

void DirectoryLevelStorage::resetNetherPlayerPositions()
{
    if (gApp.GetResetNether())
        resetPlayerPositionsInDimension(-1);
}

void DirectoryLevelStorage::resetEndPlayerPositions()
{
    if (gApp.GetResetEnd())
        resetPlayerPositionsInDimension(1);
}

void DirectoryLevelStorage::PlayerMappings::writeMappings(DataOutputStream* output)
{
    output->writeInt(mMappings.size());
    for (boost::unordered_map<long long, short>::iterator it = mMappings.begin(); it != mMappings.end(); ++it)
    {
        output->writeLong(it->first);
        output->writeInt(it->second);
    }
}

void DirectoryLevelStorage::PlayerMappings::addMapping(int id, int centreX, int centreZ, int dimension, int scale)
{
    long long index = ((long long)(centreZ & COORDINATE_MASK) << (HIGH_WORD_SHIFT + COORDINATE_Z_SHIFT))
                      | ((long long)(centreX & COORDINATE_MASK) << COORDINATE_X_SHIFT)
                      | (long long)((scale & SCALE_MASK) << SCALE_SHIFT) | (long long)(dimension & DIMENSION_MASK);
    mMappings[index] = id;
}

void DirectoryLevelStorage::PlayerMappings::readMappings(DataInputStream* input)
{
    int count = input->readInt();
    for (int i = 0; i < count; ++i)
    {
        long long index = input->readLong();
        int id = input->readInt();
        mMappings[index] = id;
    }
}

bool DirectoryLevelStorage::PlayerMappings::getMapping(int& id, int centreX, int centreZ, int dimension, int scale)
{
    long long index = ((long long)(centreZ & COORDINATE_MASK) << (HIGH_WORD_SHIFT + COORDINATE_Z_SHIFT))
                      | ((long long)(centreX & COORDINATE_MASK) << COORDINATE_X_SHIFT)
                      | (long long)((scale & SCALE_MASK) << SCALE_SHIFT) | (long long)(dimension & DIMENSION_MASK);
    boost::unordered_map<long long, short>::iterator it = mMappings.find(index);
    if (it != mMappings.end())
    {
        id = it->second;
        return true;
    }
    return false;
}
