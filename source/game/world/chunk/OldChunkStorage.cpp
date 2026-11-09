#include "game/world/chunk/OldChunkStorage.hpp"
#ifdef MINECRAFT_PC
#include "platform/pc/PcModContent.hpp"
#endif
#include "game/world/block/BlockId.hpp"

#include <cstdio>
#include <string>

#include <boost/shared_ptr.hpp>
#include <boost/weak_ptr.hpp>

#include "game/data/DataFixerUpper.hpp"
#include "game/core/DataInputStream.hpp"
#include "game/core/DataOutputStream.hpp"
#include "game/core/FileInputStream.hpp"
#include "game/core/Mth.hpp"
#include "game/core/PIXNamedEvents.hpp"
#include "game/core/NbtIo.hpp"
#include "game/core/CriticalSection.hpp"
#include "game/core/TickNextTickData.hpp"
#include "game/data/CompoundTag.hpp"
#include "game/data/ListTag.hpp"
#include "game/entity/mob/EntityIO.hpp"
#include "game/entity/mob/Entity.hpp"
#include "game/world/Level.hpp"
#include "game/world/block/Block.hpp"
#include "game/world/block/BlockState.hpp"
#include "game/world/block/BlockEntity.hpp"
#include "game/world/chunk/ChunkPos.hpp"
#include "game/world/chunk/LevelChunk.hpp"
#include "platform/wiiu/TLSStorageCafe.hpp"

static unsigned int gOldChunkStorageTlsKey = 0;
static OldChunkStorage::ThreadStorage* gOldChunkStorageDefault = NULL;

static const unsigned int THREAD_BLOCK_DATA_SIZE = 16 * 16 * 256;
static const unsigned int THREAD_NIBBLE_DATA_SIZE = THREAD_BLOCK_DATA_SIZE / 2;

OldChunkStorage::ThreadStorage::ThreadStorage()
    : blockData(NULL),
      blockDataSize(0),
      data(NULL),
      dataSize(0),
      skyLightData(NULL),
      skyLightDataSize(0),
      blockLightData(NULL),
      blockLightDataSize(0)
{
    unsigned char* buffer;
    buffer = new unsigned char[THREAD_BLOCK_DATA_SIZE];
    for (unsigned int i = 0; i < THREAD_BLOCK_DATA_SIZE; ++i)
        buffer[i] = 0;
    blockData = buffer;
    blockDataSize = THREAD_BLOCK_DATA_SIZE;
    buffer = new unsigned char[THREAD_NIBBLE_DATA_SIZE];
    for (unsigned int i = 0; i < THREAD_NIBBLE_DATA_SIZE; ++i)
        buffer[i] = 0;
    data = buffer;
    dataSize = THREAD_NIBBLE_DATA_SIZE;
    buffer = new unsigned char[THREAD_NIBBLE_DATA_SIZE];
    for (unsigned int i = 0; i < THREAD_NIBBLE_DATA_SIZE; ++i)
        buffer[i] = 0;
    skyLightData = buffer;
    skyLightDataSize = THREAD_NIBBLE_DATA_SIZE;
    buffer = new unsigned char[THREAD_NIBBLE_DATA_SIZE];
    for (unsigned int i = 0; i < THREAD_NIBBLE_DATA_SIZE; ++i)
        buffer[i] = 0;
    blockLightData = buffer;
    blockLightDataSize = THREAD_NIBBLE_DATA_SIZE;
}

OldChunkStorage::ThreadStorage::~ThreadStorage()
{
    delete[] blockData;
    delete[] data;
    delete[] skyLightData;
    delete[] blockLightData;
}

void OldChunkStorage::CreateNewThreadStorage()
{
    ThreadStorage* storage = new ThreadStorage();
    if (gOldChunkStorageDefault == NULL)
    {
        gOldChunkStorageTlsKey = TlsAlloc();
        gOldChunkStorageDefault = storage;
    }
    TlsSetValue(gOldChunkStorageTlsKey, storage);
}

void OldChunkStorage::UseDefaultThreadStorage()
{
    TlsSetValue(gOldChunkStorageTlsKey, gOldChunkStorageDefault);
}

void OldChunkStorage::ReleaseThreadStorage()
{
    ThreadStorage* storage = static_cast<ThreadStorage*>(TlsGetValue(gOldChunkStorageTlsKey));
    if (storage != gOldChunkStorageDefault)
        delete storage;
}

OldChunkStorage::OldChunkStorage(File file, bool createValue) : directory()
{
    directory = file;
    create = createValue;
}

File OldChunkStorage::getFile(int x, int z)
{
    wchar_t name[256];
    wchar_t xRadix36[256];
    wchar_t zRadix36[256];
    File file(directory, std::wstring(xRadix36));
    if (!file.exists())
    {
        if (create)
            file.mkdir();
        else
            return File(L"");
    }
    file = File(file, std::wstring(zRadix36));
    if (!file.exists())
    {
        if (create)
            file.mkdir();
        else
            return File(L"");
    }
    file = File(file, std::wstring(name));
    if (!file.exists() && !create)
        return File(L"");
    return file;
}

void OldChunkStorage::swapBlockID(LevelChunk* chunk, int x, int y, int z, int oldId, int newId)
{
    int element = chunk->getElement(x, y, z);
    int id = element >> 4;
    if (id == oldId || id == newId)
    {
        int newBlock = id != newId ? newId : oldId;
        unsigned short newElement = (newBlock << 4) | (element & 0xf);
        chunk->setElement(x, y, z, newElement);
    }
}

void OldChunkStorage::fixBlockIDs(LevelChunk* chunk)
{
    for (int y = 0; y < 256; ++y)
        for (int z = 0; z < 16; ++z)
            for (int x = 0; x < 16; x++)
            {
                swapBlockID(chunk, x, y, z, 0xa1, 0xa2);
                swapBlockID(chunk, x, y, z, 0xba, 0xbb);
                swapBlockID(chunk, x, y, z, 0xbf, 0xc0);
            }
}

boost::shared_ptr<Entity> OldChunkStorage::loadStaticEntity(CompoundTag* tag, Level* level)
{
    return EntityIO::newEntity(tag, level);
}

boost::shared_ptr<Entity> OldChunkStorage::loadAndAddEntity(CompoundTag* tag, Level* level, LevelChunk* chunk)
{
    boost::shared_ptr<Entity> entity = loadStaticEntity(tag, level);
    if (entity == NULL)
        return boost::shared_ptr<Entity>();

    chunk->addEntity(entity);
    if (tag->contains(Entity::RIDING_TAG, Tag::TAG_LIST))
    {
        ListTag* passengers = tag->getList(Entity::RIDING_TAG, Tag::TAG_COMPOUND);
        for (int i = 0; i < passengers->size(); ++i)
        {
            boost::shared_ptr<Entity> passenger = loadAndAddEntity(passengers->getCompound(i), level, chunk);
            if (passenger != NULL)
                passenger->startRiding(entity, true);
        }
    }
    return entity;
}

void OldChunkStorage::loadEntities(LevelChunk* chunk, Level* level, CompoundTag* tag)
{
    if (tag->contains(L"Entities"))
    {
        ListTag* entities = tag->getList(L"Entities", Tag::TAG_COMPOUND);
        if (entities != NULL)
        {
            int i;
            for (i = 0; i < entities->size(); ++i)
            {
                CompoundTag* entityTag = entities->getCompound(i);
                loadAndAddEntity(entityTag, level, chunk);
                chunk->setLastSaveHadEntities(true);
            }
        }
    }
    if (tag->contains(L"TileEntities"))
    {
        ListTag* blockEntities = tag->getList(L"TileEntities", Tag::TAG_COMPOUND);
        if (blockEntities != NULL)
        {
            int i;
            for (i = 0; i < blockEntities->size(); ++i)
            {
                CompoundTag* blockEntityTag = blockEntities->getCompound(i);
                boost::shared_ptr<BlockEntity> blockEntity = BlockEntity::loadStatic(level, blockEntityTag);
                if (blockEntity)
                    chunk->addBlockEntity(blockEntity);
            }
        }
    }
}

LevelChunk* OldChunkStorage::load(Level* level, CompoundTag* tag, LevelChunk* chunk)
{
    int version = 9;
    if (tag->contains(L"version", Tag::TAG_SHORT))
        version = tag->getShort(L"version");
    int x = tag->getInt(L"xPos");
    int z = tag->getInt(L"zPos");
    if (chunk == NULL)
        chunk = new LevelChunk(level, x, z);
    chunk->setChunkVersion(version);
    if (version < 10)
        fixBlockIDs(chunk);
    delete chunk->mElements;
    chunk->mElements = new ElementStorage(false, tag->getByteArray(L"Blocks"), tag->getByteArray(L"Data"), 0);
    chunk->setSkyLightData(tag->getByteArray(L"SkyLight"));
    chunk->setBlockLightData(tag->getByteArray(L"BlockLight"));
    delete[] chunk->mHeightmap.data;
    arrayWithLength<unsigned char> heightmap = tag->getByteArray(L"HeightMap");
    arrayWithLength<unsigned char> copiedHeightmap(heightmap.length, true);
    for (unsigned int i = 0; i < heightmap.length; ++i)
        copiedHeightmap.data[i] = heightmap.data[i];
    chunk->mHeightmap = copiedHeightmap;
    chunk->recalcMinAndMaxHeights();
    if (!tag->get(L"TerrainPopulated"))
    {
        chunk->mChunkFlags = static_cast<unsigned short>(tag->getShort(L"TerrainPopulatedFlags"));
        if ((chunk->mChunkFlags & LevelChunk::TERRAIN_POPULATED_ALL_NEIGHBOURS)
            == LevelChunk::TERRAIN_POPULATED_ALL_NEIGHBOURS)
            chunk->mChunkFlags |= LevelChunk::TERRAIN_POPULATED_FULL;
    }
    else
    {
        chunk->mChunkFlags = static_cast<unsigned short>(tag->getByte(L"TerrainPopulated"));
        if (static_cast<short>(chunk->mChunkFlags) > 0)
            chunk->mChunkFlags = LevelChunk::TERRAIN_POPULATED_ALL;
    }
    if (tag->contains(L"Biomes"))
    {
        arrayWithLength<unsigned char> biomes = tag->getByteArray(L"Biomes");
        arrayWithLength<unsigned char> copiedBiomes(biomes.length, true);
        for (unsigned int i = 0; i < biomes.length; ++i)
            copiedBiomes.data[i] = biomes.data[i];
        chunk->setBiomes(copiedBiomes);
    }
    loadEntities(chunk, level, tag);
    if (tag->contains(L"TileTicks"))
    {
        ListTag* ticks = tag->getList(L"TileTicks", Tag::TAG_COMPOUND);
        if (ticks != NULL)
        {
            for (int i = 0; i < ticks->size(); ++i)
            {
                CompoundTag* tick = ticks->getCompound(i);
#ifdef MINECRAFT_PC
                Block* block = Block::byId(PcModContentLoadBlockId(tick->getInt(L"i")));
                if (block == NULL)
                    continue;
                level->forceAddBlockTick(BlockPos(tick->getInt(L"x"), tick->getInt(L"y"), tick->getInt(L"z")),
                                         block, tick->getInt(L"t"), tick->getInt(L"p"));
#else
                level->forceAddBlockTick(BlockPos(tick->getInt(L"x"), tick->getInt(L"y"), tick->getInt(L"z")),
                                         Block::byId(tick->getInt(L"i")), tick->getInt(L"t"), tick->getInt(L"p"));
#endif
            }
        }
    }
    return chunk;
}

LevelChunk* OldChunkStorage::load(Level* level, int x, int z, LevelChunk* chunk)
{
    char buffer[260];
    File file = getFile(x, z);
    bool found = false;
    if (file.getPath().length() != 0 && file.exists())
        found = true;
    if (found)
    {
        FileInputStream input(file);
        CompoundTag* root = NbtIo::readCompressed(&input);
        int hasLevel = root->contains(L"Level", Tag::TAG_COMPOUND);
        if (hasLevel == 1)
        {
            int hasBlocks = root->getCompound(L"Level")->contains(L"Blocks");
            if (hasBlocks == 1)
            {
                LevelChunk* result = load(level, root->getCompound(L"Level"), chunk);
                if (!result->isAt(x, z))
                {
                    sprintf(buffer,
                            "Chunk fileat %d, %d is in the wrong location; relocating. Expected %d, %d, got %d, %d\n",
                            x, z, x, z, result->x, result->z);
                    root->putInt(L"xPos", x);
                    root->putInt(L"zPos", z);
                    result = load(level, root->getCompound(L"Level"), result);
                }
                return result;
            }
            sprintf(buffer, "Chunk file at %d, %d is missing block data, skipping\n", x, z);
            return NULL;
        }
        else
        {
            sprintf(buffer, "Chunk file at %d, %d is missing level data, skipping\n", x, z);
            return NULL;
        }
    }
    return NULL;
}

void OldChunkStorage::save(Level* level, LevelChunk* chunk)
{
    (void)level;
    (void)chunk;
}

bool OldChunkStorage::saveEntities(LevelChunk* chunk, Level* level, CompoundTag* tag)
{
    if (!chunk->mLastSaveHadEntities)
        return false;
    chunk->mLastSaveHadEntities = false;
    ListTag* entitiesTag = new ListTag();
    EnterCriticalSection(&LevelChunk::m_csEntities);
    for (int section = 0; section < chunk->mEntitySectionCount; ++section)
    {
        std::vector<boost::shared_ptr<Entity> >::iterator end = chunk->mEntitySections[section]->end();
        for (std::vector<boost::shared_ptr<Entity> >::iterator entity = chunk->mEntitySections[section]->begin();
             entity != end; ++entity)
        {
            boost::shared_ptr<Entity> savedEntity = *entity;
            chunk->mLastSaveHadEntities = true;
            CompoundTag* entityTag = new CompoundTag();
            if (savedEntity->save(entityTag))
                entitiesTag->add(entityTag);
            else
                delete entityTag;
        }
    }
    LeaveCriticalSection(&LevelChunk::m_csEntities);
    tag->put(L"Entities", entitiesTag);
    return chunk->mLastSaveHadEntities;
}

void OldChunkStorage::save(LevelChunk* chunk, Level* level, DataOutputStream* stream)
{
    stream->writeShort(12);
    stream->writeInt(chunk->x);
    stream->writeInt(chunk->z);
    stream->writeLong(level->getGameTime());
    stream->writeLong(chunk->mInhabitedTime);
    PIXBeginNamedEvent(0.0f, "Getting element data");
#ifdef MINECRAFT_PC
    chunk->WriteCompressedElementsForSave(stream);
#else
    chunk->writeCompressedElements(stream);
#endif
    PIXEndNamedEvent();
    PIXBeginNamedEvent(0.0f, "Getting sky and block light data");
    chunk->writeCompressedSkyLightData(stream);
    chunk->writeCompressedBlockLightData(stream);
    PIXEndNamedEvent();
    stream->write(chunk->getHeightmap());
    stream->writeShort(chunk->mChunkFlags);
    stream->write(chunk->getBiomes());
    PIXBeginNamedEvent(0.0f, "Saving entities");
    CompoundTag* tag = new CompoundTag();
    saveEntities(chunk, level, tag);
    PIXBeginNamedEvent(0.0f, "Saving block entities");
    ListTag* blockEntities = new ListTag();
    LevelChunk::BlockEntityMap::iterator end = chunk->mBlockEntityMap.end();
    for (LevelChunk::BlockEntityMap::iterator it = chunk->mBlockEntityMap.begin(); it != end; ++it)
    {
        boost::shared_ptr<BlockEntity> blockEntity = it->second;
        CompoundTag* blockEntityTag = new CompoundTag();
        blockEntity->save(blockEntityTag);
        blockEntities->add(blockEntityTag);
    }
    tag->put(L"TileEntities", blockEntities);
    PIXEndNamedEvent();
    PIXBeginNamedEvent(0.0f, "Saving block tick data");
    std::vector<TickNextTickData>* ticks = level->fetchTicksInChunk(chunk, false);
    if (ticks != NULL)
    {
        long long gameTime = level->getGameTime();
        ListTag* tickTags = new ListTag();
        unsigned int i;
        for (i = 0; i < ticks->size(); ++i)
        {
            TickNextTickData tick = ticks->at(i);
            CompoundTag* tickTag = new CompoundTag();
#ifdef MINECRAFT_PC
            tickTag->putInt(L"i", PcModContentSaveBlockId(Block::getId(tick.block)));
#else
            tickTag->putInt(L"i", Block::getId(tick.block));
#endif
            tickTag->putInt(L"x", tick.pos.x);
            tickTag->putInt(L"y", tick.pos.y);
            tickTag->putInt(L"z", tick.pos.z);
            tickTag->putInt(L"t", static_cast<int>(tick.triggerTick - gameTime));
            tickTags->add(tickTag);
        }
        tag->put(L"TileTicks", tickTags);
        delete ticks;
    }
    PIXEndNamedEvent();
    NbtIo::write(tag, stream);
    delete tag;
    PIXEndNamedEvent();
}

boost::shared_ptr<Entity> OldChunkStorage::loadEntity(CompoundTag* tag, Level* level, double x, double y, double z,
                                                      bool addToWorld)
{
    boost::shared_ptr<Entity> entity = loadStaticEntity(tag, level);
    if (entity == NULL)
        return boost::shared_ptr<Entity>();

    entity->moveTo(x, y, z, entity->mYRot, entity->mXRot);

    if (addToWorld && !level->addEntity(entity))
        return boost::shared_ptr<Entity>();

    if (tag->contains(Entity::RIDING_TAG, Tag::TAG_LIST))
    {
        ListTag* passengers = tag->getList(Entity::RIDING_TAG, Tag::TAG_COMPOUND);
        for (int i = 0; i < passengers->size(); ++i)
        {
            boost::shared_ptr<Entity> passenger = loadEntity(passengers->getCompound(i), level, x, y, z, addToWorld);
            if (passenger != NULL)
                passenger->startRiding(entity, true);
        }
    }

    return entity;
}

void OldChunkStorage::addEntityToWorld(boost::shared_ptr<Entity> entity, Level* level)
{
    if (level->addEntity(entity))
    {
        if (entity->isVehicle())
        {
            std::vector<boost::weak_ptr<Entity> >* passengers = entity->getPassengers();
            std::vector<boost::weak_ptr<Entity> >::iterator it;
            for (it = passengers->begin(); it != passengers->end(); ++it)
            {
                boost::shared_ptr<Entity> passenger = it->lock();
                addEntityToWorld(passenger, level);
            }
        }
    }
}

boost::shared_ptr<Entity> OldChunkStorage::loadEntity(CompoundTag* tag, Level* level, bool addToWorld)
{
    boost::shared_ptr<Entity> entity = loadStaticEntity(tag, level);
    if (entity == NULL)
        return boost::shared_ptr<Entity>();

    if (addToWorld && !level->addEntity(entity))
        return boost::shared_ptr<Entity>();

    if (tag->contains(Entity::RIDING_TAG, Tag::TAG_LIST))
    {
        ListTag* passengers = tag->getList(Entity::RIDING_TAG, Tag::TAG_COMPOUND);
        for (int i = 0; i < passengers->size(); ++i)
        {
            boost::shared_ptr<Entity> passenger = loadEntity(passengers->getCompound(i), level, addToWorld);
            if (passenger != NULL)
                passenger->startRiding(entity, true);
        }
    }

    return entity;
}

void OldChunkStorage::fixCauldronData(LevelChunk* chunk)
{
    for (int x = 0; x < 16; ++x)
        for (int z = 0; z < 16; ++z)
            for (int y = 0; y < 256; ++y)
            {
                const BlockState* state = chunk->getBlockState(x, y, z);
                if (Block::getId(state->getBlock()) == BlockId::CAULDRON)
                {
                    int data = chunk->getData(x, y, z);
                    bool* changed = new bool(false);
                    int fixedData = Mth::clamp(data << 1, 0, 6);
                    chunk->setData(x, y, z, fixedData, 0, changed);
                    delete changed;
                }
            }
}

LevelChunk* OldChunkStorage::load(Level* level, DataInputStream* stream, DataFixerUpper* fixer, LevelChunk* chunk)
{
    PIXBeginNamedEvent(0.0f, "Loading chunk");
    int version = stream->readShort();
    int x = stream->readInt();
    int z = stream->readInt();
    stream->readLong();
    if (chunk == NULL)
    {
        PIXBeginNamedEvent(0.0f, "Creating LevelChunk");
        chunk = new LevelChunk(level, x, z);
        PIXEndNamedEvent();
    }
    if (version >= 9)
        chunk->setInhabitedTime(stream->readLong());
    chunk->setChunkVersion(version);
    PIXBeginNamedEvent(0.0f, "Reading compressed data");
    if (version < 12)
        chunk->readCompressedElementsFromBlockAndData(stream);
    else
        chunk->readCompressedElements(stream);
#ifdef MINECRAFT_PC
    chunk->ReplaceUnknownBlocks();
#endif
    chunk->readCompressedSkyLightData(stream);
    chunk->readCompressedBlockLightData(stream);
    PIXEndNamedEvent();
    if (version < 10)
    {
        PIXBeginNamedEvent(0.0f, "fixBlockIDs");
        fixBlockIDs(chunk);
        PIXEndNamedEvent();
        fixCauldronData(chunk);
    }
    else if (version < 11)
        fixCauldronData(chunk);
    PIXBeginNamedEvent(0.0f, "Read heightmap");
    stream->readFully(chunk->mHeightmap);
    chunk->recalcMinAndMaxHeights();
    PIXEndNamedEvent();
    short flags = stream->readShort();
    if ((flags & LevelChunk::TERRAIN_POPULATED_ALL_NEIGHBOURS) == LevelChunk::TERRAIN_POPULATED_ALL_NEIGHBOURS)
        chunk->mChunkFlags = flags | LevelChunk::TERRAIN_POPULATED_FULL;
    else
        chunk->mChunkFlags = flags;
    stream->readFully(chunk->mBiomes);
    chunk->updateBiomePresentFlags();
    CompoundTag* tag = fixer->updateData(DataFixTypes::eDataFixTypes_Chunk, NbtIo::read(stream));
    PIXBeginNamedEvent(0.0f, "Load entities");
    loadEntities(chunk, level, tag);
    PIXEndNamedEvent();
    if (tag->contains(L"TileTicks"))
    {
        PIXBeginNamedEvent(0.0f, "Loading BlockTicks");
        ListTag* ticks = tag->getList(L"TileTicks", Tag::TAG_COMPOUND);
        if (ticks != NULL)
        {
            for (int i = 0; i < ticks->size(); ++i)
            {
                CompoundTag* tick = ticks->getCompound(i);
#ifdef MINECRAFT_PC
                Block* block = Block::byId(PcModContentLoadBlockId(tick->getInt(L"i")));
                if (block == NULL)
                    continue;
                level->forceAddBlockTick(BlockPos(tick->getInt(L"x"), tick->getInt(L"y"), tick->getInt(L"z")),
                                         block, tick->getInt(L"t"), tick->getInt(L"p"));
#else
                level->forceAddBlockTick(BlockPos(tick->getInt(L"x"), tick->getInt(L"y"), tick->getInt(L"z")),
                                         Block::byId(tick->getInt(L"i")), tick->getInt(L"t"), tick->getInt(L"p"));
#endif
            }
        }
        PIXEndNamedEvent();
    }
    delete tag;
    PIXEndNamedEvent();
    return chunk;
}

void OldChunkStorage::tick()
{
}

void OldChunkStorage::flush()
{
}

void OldChunkStorage::saveEntities(Level* level, LevelChunk* chunk)
{
    (void)level;
    (void)chunk;
}
