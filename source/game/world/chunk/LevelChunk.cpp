#include "game/world/chunk/LevelChunk.hpp"
#ifdef MINECRAFT_PC
#include "platform/pc/PcModContent.hpp"
#endif
#include "game/core/MemSect.hpp"
#include "game/world/block/BlockId.hpp"
#include "game/core/EntityType.hpp"
#include "game/world/chunk/ChunkPos.hpp"
#include "game/world/chunk/ChunkPrimer.hpp"
#include "game/world/chunk/ChunkSource.hpp"
#include "game/world/chunk/ChunkGenerator.hpp"
#include "game/world/Level.hpp"
#include "game/world/Dimension.hpp"
#include "game/world/block/Block.hpp"
#include "game/world/block/BlockEntity.hpp"
#include "game/world/block/BlockState.hpp"
#include "game/world/block/EntityBlock.hpp"
#include "game/world/block/MutableBlockPos.hpp"
#include "game/world/block/CommandBlock.hpp"
#include "game/world/block/Blocks.hpp"
#include "game/world/block/ChestBlock.hpp"
#include "game/world/biome/BiomeSource.hpp"
#include "game/world/biome/Biome.hpp"
#include "game/world/block/CompressedBlockStorage.hpp"
#include "game/world/block/Material.hpp"
#include "game/world/block/LiquidBlock.hpp"
#include "game/data/SparseDataStorage.hpp"
#include "game/data/CompoundTag.hpp"
#include "game/data/ListTag.hpp"
#include "game/entity/mob/Entity.hpp"
#include "game/entity/mob/EntityIO.hpp"
#include "game/entity/mob/AbstractHorse.hpp"
#include "game/entity/mob/AbstractSkeleton.hpp"
#include "game/entity/mob/Animal.hpp"
#include "game/entity/mob/ItemEntity.hpp"
#include "game/entity/mob/LivingEntity.hpp"
#include "game/entity/mob/Mob.hpp"
#include "game/entity/mob/Monster.hpp"
#include "game/entity/mob/Zombie.hpp"
#include "game/entity/projectile/Arrow.hpp"
#include "game/core/Minecart.hpp"
#include "game/core/Mth.hpp"
#include "game/world/generation/Player.hpp"
#include "game/core/TickNextTickData.hpp"
#include "game/core/Minecraft.hpp"
#include "game/util/AABB.hpp"
#include "game/util/Random.hpp"
#include "game/client/render/GameRenderer.hpp"
#include "game/world/DimensionType.hpp"
#include "game/server/MinecraftServer.hpp"
#include "game/server/ServerLevel.hpp"
#include "game/core/CConsoleMinecraftApp.hpp"
#include "game/core/CMinecraftApp.hpp"
#include "game/core/XMem.hpp"
#include "game/core/PIXNamedEvents.hpp"
#include "platform/wiiu/TLSStorageCafe.hpp"
#include <algorithm>
#include <cstdio>
#include <vector>
#include <cstring>
#include <cstdlib>

extern MinecraftServer* gMinecraftServer;

static unsigned int sLevelChunkThreadStorage = TlsAlloc();
static bool sEntityRemovalFlag = true;
OSFastMutex LevelChunk::m_csLighting;
OSFastMutex LevelChunk::m_csEntities;
OSFastMutex LevelChunk::m_csTileEntities;
bool LevelChunk::touchedSky = false;

LevelChunk::LevelChunk(Level* level, int x, int z) : mEntitySectionCount(16)
{
    init(level, x, z);
    mServerChunkFlags = NULL;
    mBlockLight = NULL;
    mSkyLight = NULL;
    mSkyLightUpper = new SparseLightStorage(true, true);
    mBlockLightUpper = new SparseLightStorage(false, true);
    mElements = new ElementStorage(level->mIsClientSide);
}
LevelChunk::LevelChunk(Level* level, ChunkPrimer* primer, int x, int z) : mEntitySectionCount(16)
{
    init(level, x, z);
    arrayWithLength<unsigned short> elements = primer->getElementIds();
    mEmissiveMaxHeight = primer->getEmissiveMaxHeight();
    if (elements.data == NULL)
    {
        mSkyLight = new SparseLightStorage(true, true);
        mBlockLight = new SparseLightStorage(false, true);
        mElements = new ElementStorage(level->mIsClientSide);
        mSkyLightUpper = new SparseLightStorage(true, true);
        mBlockLightUpper = new SparseLightStorage(false, true);
        mServerChunkFlags = NULL;
    }
    else
    {
        mSkyLight = new SparseLightStorage(true);
        mBlockLight = new SparseLightStorage(false);
        mElements = new ElementStorage(level->mIsClientSide, elements, 4);
        mSkyLightUpper = new SparseLightStorage(true, true);
        mBlockLightUpper = new SparseLightStorage(false, true);
        mServerChunkFlags = NULL;
    }
}
LevelChunk::LevelChunk(Level* level, int x, int z, LevelChunk* source) : mEntitySectionCount(16)
{
    init(level, x, z);
    mElements = new ElementStorage(level->mIsClientSide, source->mElements);
    mSkyLight = new SparseLightStorage(source->mSkyLight);
    mBlockLight = new SparseLightStorage(source->mBlockLight);
    mSkyLightUpper = new SparseLightStorage(source->mSkyLightUpper);
    mBlockLightUpper = new SparseLightStorage(source->mBlockLightUpper);
    delete[] mBiomes.data;
    mBiomes.data = source->mBiomes.data;
    mOwnsBiomes = true;
    mServerChunkFlags = &source->mChunkFlags;
}
LevelChunk::~LevelChunk()
{
    delete mElements;
    if (!mOwnsBiomes)
        delete[] mBiomes.data;
    delete mSkyLight;
    delete mBlockLight;
    delete mSkyLightUpper;
    delete mBlockLightUpper;
    delete[] mHeightmap.data;
    EnterCriticalSection(&m_csEntities);
    for (int i = 0; i < mEntitySectionCount; ++i)
    {
        mEntitySections[i]->clear();
        delete mEntitySections[i];
    }
    delete[] mEntitySections;
    mEntitySections = NULL;
    LeaveCriticalSection(&m_csEntities);
    delete mPendingBlockEntityTag;
}
short LevelChunk::BlockChange::GetPackedPos() const
{
    return mPackedPos;
}
unsigned int LevelChunk::BlockChange::GetElement() const
{
    return (unsigned int)mElement;
}
BlockPos LevelChunk::BlockChange::GetPos() const
{
    return LevelChunk::UnpackBlockPos((short)mPackedPos);
}
const BlockState* LevelChunk::BlockChange::GetState() const
{
#ifdef MINECRAFT_PC
    return Block::blockStateRegistry->byId(mElement & 0xFFFF);
#else
    return Block::blockStateRegistry->byId(mElement & 255);
#endif
}
const BlockState* LevelChunk::BlockChange::GetExtraState() const
{
#ifdef MINECRAFT_PC
    return Block::blockStateRegistry->byId((mElement >> 16) & 0xFFFF);
#else
    return Block::blockStateRegistry->byId((mElement >> 16) & 255);
#endif
}
LevelChunk::ThreadStorage::ThreadStorage() : elementStorage(NULL), extraElementStorage(NULL), tempLightingStorage(NULL)
{
}

bool LevelChunk::isReady()
{
    return mLightPopulated && (mChunkFlags & TERRAIN_POPULATED_FULL) != 0 && mPostProcess;
}
int LevelChunk::getBlockId(int x, int y, int z)
{
    return (int)(mElements->get(x, y & 0xff, z, NULL) >> 4);
}
unsigned int LevelChunk::getLowestHeightmap()
{
    return mLowestHeightmap;
}
unsigned short LevelChunk::getElement(int x, int y, int z)
{
    return (unsigned short)mElements->get(x, y & 0xff, z, NULL);
}
bool LevelChunk::sectionContainsBlockId(int x, int y, int z, int blockId)
{
    return mElements->sectionContainsElementData(x, y & 0xff, z, (unsigned short)((blockId & 0xfff) << 4), 0x7ff0);
}
int LevelChunk::getExtraBlockId(int x, int y, int z)
{
    return (int)(mElements->getExtraElement(x, y & 0xff, z) >> 4);
}
void LevelChunk::getBlockAndExtraId(int x, int y, int z, int* blockId, int* extraBlockId)
{
    unsigned short element = 0;
    unsigned short extra = 0;
    mElements->getElementAndExtraElement(x, y & 0xff, z, &element, &extra);
    *blockId = element >> 4;
    *extraBlockId = extra >> 4;
}
unsigned int LevelChunk::getData(int x, int y, int z)
{
    return mElements->get(x, y & 0xff, z, NULL) & 15;
}
unsigned int LevelChunk::getExtraBlockData(int x, int y, int z)
{
    return mElements->getExtraElement(x, y & 0xff, z) & 15;
}
unsigned short LevelChunk::getElement(int x, int y, int z, bool* present)
{
    return (unsigned short)mElements->get(x, y & 0xff, z, present);
}
int LevelChunk::getHighestSectionPosition()
{
    for (int section = 15; section >= 0; --section)
    {
        int highest = 0;
        if (!mElements->isRenderChunkEmpty(section * 16, &highest))
            return section * 16 + 15;
    }
    return 0;
}
BlockPos LevelChunk::getTopRainBlock(const BlockPos& pos)
{
    int columnIndex = (pos.x & 0xf) | ((pos.z & 0xf) << 4);
    BlockPos current(pos.x, mPrecipitationHeightmap[columnIndex], pos.z);
    if (current.y == 255)
    {
        int top = getHighestSectionPosition();
        current = BlockPos(pos.x, top + 15, pos.z);
        int found = -1;
        while (current.y >= 0 && found == -1)
        {
            if (current.y < 256)
            {
                const BlockState* state = getBlockState(current);
                Material* material = state->getMaterial();
                const BlockState* extraState = getExtraBlockState(current);
                Material* extraMaterial = extraState->getMaterial();
                if (!material->blocksMotion() && !material->isLiquid() && !extraMaterial->isLiquid())
                    current = current.below();
                else
                    found = current.y + 1;
            }
            else
            {
                current = current.below();
            }
        }
        if (found == 255)
            found = 254;
        mPrecipitationHeightmap[columnIndex] = (unsigned char)found;
    }
    return BlockPos(pos.x, mPrecipitationHeightmap[columnIndex], pos.z);
}
void LevelChunk::updateBiomeFlags(int x, int z)
{
    unsigned int index = (x >> 1) | (z << 3);
    int shift = (x & 1) << 2;
    if ((mBiomeFlags[index] & (2 << shift)) != 0)
        return;
    Biome* biome = level->getBiome(BlockPos(this->x * 16 + x, 0, this->z * 16 + z));
    if (biome->hasRain())
        mBiomeFlags[index] |= (unsigned char)(8 << shift);
    if (biome->hasSnow())
        mBiomeFlags[index] |= (unsigned char)((4 << shift) | (2 << shift));
    else
        mBiomeFlags[index] |= (unsigned char)(2 << shift);
}
bool LevelChunk::biomeHasRain(int x, int z)
{
    updateBiomeFlags(x, z);
    return (mBiomeFlags[((x >> 1) | (z << 3)) & 127] & (8 << ((x & 1) << 2))) != 0;
}
bool LevelChunk::biomeHasSnow(int x, int z)
{
    updateBiomeFlags(x, z);
    return (mBiomeFlags[((x >> 1) | (z << 3)) & 127] & (4 << ((x & 1) << 2))) != 0;
}
long long LevelChunk::getInhabitedTime()
{
    return mInhabitedTime;
}
bool LevelChunk::isTerrainPopulated()
{
    return (mChunkFlags & TERRAIN_POPULATED_FULL) != 0;
}
bool LevelChunk::isRenderChunkEmpty(int section)
{
    if (Minecraft::InMiniGame(eMiniGameId_Any, true))
        return false;

    bool renderChunkEmpty = true;
    if (!isEmpty())
        renderChunkEmpty = mElements->isRenderChunkEmpty(section, NULL);

    return renderChunkEmpty;
}
void LevelChunk::getBlockData(arrayWithLength<unsigned char> data)
{
    arrayWithLength<unsigned char> extra;
    mElements->getData(data, extra, 0, 64);
}
void LevelChunk::setAllElements(arrayWithLength<unsigned char> elements, arrayWithLength<unsigned char> data)
{
    mElements->setData(elements, data);
}
void LevelChunk::setAllElements(arrayWithLength<unsigned short> elements, arrayWithLength<unsigned short> data)
{
    mElements->setData(elements, data);
}
#ifdef MINECRAFT_PC
static const int BLOCK_ID_COUNT = 4096;
static const int CHUNK_ELEMENTS = 0x10000;

static bool IsKnownBlock(int id)
{
    static signed char known[BLOCK_ID_COUNT];
    if (known[id] == 0)
        known[id] = (id == 0 || Block::byId(id) != NULL) ? 1 : -1;
    return known[id] > 0;
}

static unsigned short KnownElement(unsigned short element)
{
    int id = element >> 4;
    if (id >= kPcModFirstBlockId)
    {
        const int mapped = PcModContentLoadBlockId(id);
        if (mapped == 0)
            return 0;
        if (mapped != id)
            return static_cast<unsigned short>((mapped << 4) | (element & 0xF));
    }
    if (IsKnownBlock(id))
        return element;
    static bool reported[BLOCK_ID_COUNT];
    if (!reported[id])
    {
        reported[id] = true;
        printf("[pc] chunk: block id %d is not in this edition; loaded as air\n", id);
    }
    return 0;
}

void LevelChunk::ReplaceUnknownBlocks()
{
    if (mElements == NULL)
        return;
    std::vector<unsigned short> elements(CHUNK_ELEMENTS);
    std::vector<unsigned short> extraElements(CHUNK_ELEMENTS);
    arrayWithLength<unsigned short> elementArray(&elements[0], CHUNK_ELEMENTS);
    arrayWithLength<unsigned short> extraArray(&extraElements[0], CHUNK_ELEMENTS);
    mElements->getData(elementArray, extraArray, 0, 64);
    bool changed = false;
    for (int i = 0; i < CHUNK_ELEMENTS; ++i)
    {
        unsigned short element = KnownElement(elements[i]);
        unsigned short extra = KnownElement(extraElements[i]);
        if (element != elements[i] || extra != extraElements[i])
        {
            elements[i] = element;
            extraElements[i] = extra;
            changed = true;
        }
    }
    if (changed)
        mElements->setData(elementArray, extraArray);
}

void LevelChunk::WriteCompressedElementsForSave(DataOutputStream* stream)
{
    if (PcModContentSaveIsIdentity() || mElements == NULL)
    {
        mElements->write(stream);
        return;
    }
    std::vector<unsigned short> elements(CHUNK_ELEMENTS);
    std::vector<unsigned short> extraElements(CHUNK_ELEMENTS);
    arrayWithLength<unsigned short> elementArray(&elements[0], CHUNK_ELEMENTS);
    arrayWithLength<unsigned short> extraArray(&extraElements[0], CHUNK_ELEMENTS);
    mElements->getData(elementArray, extraArray, 0, 64);
    for (int i = 0; i < CHUNK_ELEMENTS; ++i)
    {
        const int id = elements[i] >> 4;
        const int extraId = extraElements[i] >> 4;
        if (id >= kPcModFirstBlockId)
            elements[i] = static_cast<unsigned short>((PcModContentSaveBlockId(id) << 4) | (elements[i] & 0xF));
        if (extraId >= kPcModFirstBlockId)
            extraElements[i] =
                static_cast<unsigned short>((PcModContentSaveBlockId(extraId) << 4) | (extraElements[i] & 0xF));
    }
    ElementStorage copy(false, mElements);
    copy.setData(elementArray, extraArray);
    copy.write(stream);
}
#endif
void LevelChunk::getAllElements(arrayWithLength<unsigned char> elements, arrayWithLength<unsigned char> data)
{
    mElements->getData(elements, data, 0, 64);
}
void LevelChunk::getElementRange(arrayWithLength<unsigned short> elements, int start, int end)
{
    if (mElements != NULL)
        mElements->getData(elements, start, end);
}
void LevelChunk::getElementRange(arrayWithLength<unsigned short> elements, arrayWithLength<unsigned short> data,
                                 int start, int end)
{
    if (mElements != NULL)
        mElements->getData(elements, data, start, end);
}
int LevelChunk::getRawDataMaxBytes()
{
    return ElementStorage::getRawDataMaxBytes() + (int)SparseLightStorage::getRawDataMaxBytes() * 4 + 0x10c;
}
arrayWithLength<unsigned char> LevelChunk::getRawData()
{
    unsigned char* buffer = (unsigned char*)malloc(getRawDataMaxBytes());
    unsigned char* cursor = buffer + 12;
    unsigned int elementBytes = mElements->getRawData(cursor);
    *(unsigned int*)buffer = elementBytes;
    cursor += elementBytes;
    if (mBlockLight != NULL)
    {
        unsigned short bytes = mBlockLight->getRawData(cursor);
        cursor += bytes;
        *(unsigned short*)(buffer + 4) = bytes;
    }
    if (mBlockLightUpper != NULL)
    {
        unsigned short bytes = mBlockLightUpper->getRawData(cursor);
        cursor += bytes;
        *(unsigned short*)(buffer + 6) = bytes;
    }
    if (mSkyLight != NULL)
    {
        unsigned short bytes = mSkyLight->getRawData(cursor);
        cursor += bytes;
        *(unsigned short*)(buffer + 8) = bytes;
    }
    if (mSkyLightUpper != NULL)
    {
        unsigned short bytes = mSkyLightUpper->getRawData(cursor);
        cursor += bytes;
        *(unsigned short*)(buffer + 10) = bytes;
    }
    memcpy(cursor, mBiomes.data, 256);
    cursor += 256;
    unsigned int totalBytes = (unsigned int)(cursor - buffer);
    arrayWithLength<unsigned char> result(totalBytes, false);
    memcpy(result.data, buffer, totalBytes);
    free(buffer);
    return result;
}
void LevelChunk::setFromRawData(arrayWithLength<unsigned char> data)
{
    unsigned int elementBytes = *(unsigned int*)data.data;
    mElements->setFromRawData(data.data + 12, elementBytes);
    unsigned char* cursor = data.data + 12 + elementBytes;
    unsigned short blockLightBytes = *(unsigned short*)(data.data + 4);
    if (mBlockLight == NULL)
        mBlockLight = new SparseLightStorage(false);
    mBlockLight->setFromRawData(cursor, blockLightBytes);
    cursor += blockLightBytes;
    unsigned short blockLightUpperBytes = *(unsigned short*)(data.data + 6);
    if (mBlockLightUpper == NULL)
        mBlockLightUpper = new SparseLightStorage(false, true);
    mBlockLightUpper->setFromRawData(cursor, blockLightUpperBytes);
    cursor += blockLightUpperBytes;
    unsigned short skyLightBytes = *(unsigned short*)(data.data + 8);
    if (mSkyLight == NULL)
        mSkyLight = new SparseLightStorage(true);
    mSkyLight->setFromRawData(cursor, skyLightBytes);
    cursor += skyLightBytes;
    unsigned short skyLightUpperBytes = *(unsigned short*)(data.data + 10);
    if (mSkyLightUpper == NULL)
        mSkyLightUpper = new SparseLightStorage(true, true);
    mSkyLightUpper->setFromRawData(cursor, skyLightUpperBytes);
    cursor += skyLightUpperBytes;
    std::memcpy(mBiomes.data, cursor, 0x100);
    recalcHeightmapOnly();
    for (BlockEntityMap::iterator it = mBlockEntityMap.begin(); it != mBlockEntityMap.end(); ++it)
        it->second->clearCache();
    mChunkFlags = TERRAIN_POPULATED_FULL;
    mPostProcess = true;
}
void LevelChunk::getDataData(arrayWithLength<unsigned char> data)
{
    arrayWithLength<unsigned char> elements;
    mElements->getData(elements, data, 0, 64);
}
void LevelChunk::getSkyLightData(arrayWithLength<unsigned char> data)
{
    mSkyLight->getData(data, 0);
    if (data.length > LIGHT_SECTION_BYTES)
        mSkyLightUpper->getData(data, LIGHT_SECTION_BYTES);
}
void LevelChunk::getBlockLightData(arrayWithLength<unsigned char> data)
{
    mBlockLight->getData(data, 0);
    if (data.length > LIGHT_SECTION_BYTES)
        mBlockLightUpper->getData(data, LIGHT_SECTION_BYTES);
}
void LevelChunk::setSkyLightData(arrayWithLength<unsigned char> data)
{
    if (mSkyLight == NULL)
        mSkyLight = new SparseLightStorage(true);
    if (mSkyLightUpper == NULL)
        mSkyLightUpper = new SparseLightStorage(true, true);
    mSkyLight->setData(data, 0);
    if (data.length > LIGHT_SECTION_BYTES)
        mSkyLightUpper->setData(data, LIGHT_SECTION_BYTES);
}
void LevelChunk::setBlockLightData(arrayWithLength<unsigned char> data)
{
    if (mBlockLight == NULL)
        mBlockLight = new SparseLightStorage(false);
    if (mBlockLightUpper == NULL)
        mBlockLightUpper = new SparseLightStorage(false, true);
    mBlockLight->setData(data, 0);
    if (data.length > LIGHT_SECTION_BYTES)
        mBlockLightUpper->setData(data, LIGHT_SECTION_BYTES);
}
void LevelChunk::setSkyLightDataAllBright()
{
    mSkyLight->setAllBright();
    mSkyLightUpper->setAllBright();
}
bool LevelChunk::isLowerBlockLightStorageCompressed()
{
    return mBlockLight->isCompressed();
}
void LevelChunk::writeCompressedElements(DataOutputStream* stream)
{
    mElements->write(stream);
}
void LevelChunk::readCompressedElements(DataInputStream* stream)
{
    mElements->read(stream);
}
void LevelChunk::readCompressedElementsFromBlockAndData(DataInputStream* stream)
{
    CompressedBlockStorage* blocks = new CompressedBlockStorage();
    CompressedBlockStorage* extraBlocks = new CompressedBlockStorage();
    SparseDataStorage* data = new SparseDataStorage();
    SparseDataStorage* extraData = new SparseDataStorage();
    blocks->read(stream);
    extraBlocks->read(stream);
    data->read(stream);
    extraData->read(stream);
    delete mElements;
    mElements = new ElementStorage(level->mIsClientSide, blocks, extraBlocks, data, extraData);
    delete blocks;
    delete extraBlocks;
    delete data;
    delete extraData;
}
void LevelChunk::writeCompressedSkyLightData(DataOutputStream* stream)
{
    mSkyLight->write(stream);
    mSkyLightUpper->write(stream);
}
void LevelChunk::writeCompressedBlockLightData(DataOutputStream* stream)
{
    mBlockLight->write(stream);
    mBlockLightUpper->write(stream);
}
unsigned char* LevelChunk::GetTempLightingStorage()
{
    ThreadStorage* storage = static_cast<ThreadStorage*>(TlsGetValue(sLevelChunkThreadStorage));
    return storage == NULL ? NULL : storage->tempLightingStorage;
}
void LevelChunk::readCompressedSkyLightData(DataInputStream* stream)
{
    if (mSkyLight == NULL)
        mSkyLight = new SparseLightStorage(true);
    if (mSkyLightUpper == NULL)
        mSkyLightUpper = new SparseLightStorage(true, true);
    unsigned char* buffer = GetTempLightingStorage();
    mSkyLight->read(stream, buffer);
    mSkyLightUpper->read(stream, buffer);
}
void LevelChunk::readCompressedBlockLightData(DataInputStream* stream)
{
    if (mBlockLight == NULL)
        mBlockLight = new SparseLightStorage(false);
    if (mBlockLightUpper == NULL)
        mBlockLightUpper = new SparseLightStorage(false, true);
    unsigned char* buffer = GetTempLightingStorage();
    mBlockLight->read(stream, buffer);
    mBlockLightUpper->read(stream, buffer);
}
unsigned int LevelChunk::getSparseBlockAndDataMemoryAllocated()
{
    return mElements == NULL ? 0 : mElements->getAllocatedBytes();
}
bool LevelChunk::getHasGapsToCheck()
{
    return mHasGapsToCheck;
}
unsigned int LevelChunk::getBiomePresentFlag()
{
    return mBiomePresent;
}
void LevelChunk::reSyncLighting()
{
    EnterCriticalSection(&m_csLighting);
    if (!isEmpty())
    {
        const DimensionType* dimensionType = level->mDimension->getType();
        ServerLevel* otherLevel = gMinecraftServer->getLevel(dimensionType->getId());
        ChunkSource* chunkSource = otherLevel->getChunkSource();
        LevelChunk* otherChunk = chunkSource->getChunkLoadedOrUnloaded(x, z);
        if (!otherChunk->isEmpty())
        {
            GameRenderer::AddForDelete(mSkyLight);
            mSkyLight = new SparseLightStorage(otherChunk->mSkyLight);
            GameRenderer::FinishedReassigning();
            GameRenderer::AddForDelete(mBlockLight);
            mBlockLight = new SparseLightStorage(otherChunk->mBlockLight);
            GameRenderer::FinishedReassigning();
            GameRenderer::AddForDelete(mSkyLightUpper);
            mSkyLightUpper = new SparseLightStorage(otherChunk->mSkyLightUpper);
            GameRenderer::FinishedReassigning();
            GameRenderer::AddForDelete(mBlockLightUpper);
            mBlockLightUpper = new SparseLightStorage(otherChunk->mBlockLightUpper);
            GameRenderer::FinishedReassigning();
            GameRenderer::AddForDelete(mElements);
            mElements = new ElementStorage(level->mIsClientSide, otherChunk->mElements);
            GameRenderer::FinishedReassigning();
            level->setBlocksDirty(BlockPos(x * 16 - 1, 0, z * 16 - 1), BlockPos(x * 16 + 16, 256, z * 16 + 16));
            LeaveCriticalSection(&m_csLighting);
            mLightingDropped = false;
            return;
        }
    }
    LeaveCriticalSection(&m_csLighting);
}
void LevelChunk::reSyncLightingNearby()
{
    reSyncLighting();
    if (level->mDimension->getType() == DimensionType::THE_END)
    {
        level->getChunk(x - 1, z - 1)->reSyncLighting();
        level->getChunk(x, z - 1)->reSyncLighting();
        level->getChunk(x + 1, z - 1)->reSyncLighting();
        level->getChunk(x - 1, z)->reSyncLighting();
        level->getChunk(x + 1, z)->reSyncLighting();
        level->getChunk(x - 1, z + 1)->reSyncLighting();
        level->getChunk(x, z + 1)->reSyncLighting();
        level->getChunk(x + 1, z + 1)->reSyncLighting();
    }
}
void LevelChunk::dropLighting()
{
    EnterCriticalSection(&m_csLighting);
    if (isEmpty())
    {
        LeaveCriticalSection(&m_csLighting);
        return;
    }

    GameRenderer::AddForDelete(mSkyLight);
    mSkyLight = new SparseLightStorage(true, true);
    GameRenderer::FinishedReassigning();
    GameRenderer::AddForDelete(mBlockLight);
    mBlockLight = new SparseLightStorage(false, true);
    GameRenderer::FinishedReassigning();
    GameRenderer::AddForDelete(mSkyLightUpper);
    mSkyLightUpper = new SparseLightStorage(true, true);
    GameRenderer::FinishedReassigning();
    GameRenderer::AddForDelete(mBlockLightUpper);
    mBlockLightUpper = new SparseLightStorage(false, true);
    GameRenderer::FinishedReassigning();

    LeaveCriticalSection(&m_csLighting);
    mLightingDropped = true;
}
void LevelChunk::SetInvalid(bool invalid)
{
    mInvalid = invalid;
    if (invalid)
    {
        EnterCriticalSection(&m_csEntities);
        for (int i = 0; i < mEntitySectionCount; ++i)
            mEntitySections[i]->clear();
        LeaveCriticalSection(&m_csEntities);
        EnterCriticalSection(&m_csTileEntities);
        for (BlockEntityMap::iterator it = mBlockEntityMap.begin(); it != mBlockEntityMap.end(); ++it)
            level->markForRemoval(it->second);
        mBlockEntityMap.clear();
        LeaveCriticalSection(&m_csTileEntities);
        delete mPendingBlockEntityTag;
        mPendingBlockEntityTag = NULL;
        mUnloaded = true;
    }
}
bool LevelChunk::hasLightingBeenDropped()
{
    return mLightingDropped;
}
void LevelChunk::init(Level* level, int x, int z)
{
    mPostProcessFlags = 0;
    mBiomes = arrayWithLength<unsigned char>(256, true);
    for (int i = 0; i < 256; ++i)
        mBiomes[i] = 255;
    mOwnsBiomes = false;
    EnterCriticalSection(&m_csEntities);
    mEntitySections = new std::vector<boost::shared_ptr<Entity> >*[mEntitySectionCount]();
    for (int i = 0; i < mEntitySectionCount; ++i)
        mEntitySections[i] = new std::vector<boost::shared_ptr<Entity> >;
    LeaveCriticalSection(&m_csEntities);
    mUnsaved = false;
    mLightingDropped = false;
    mPostProcess = false;
    mMinHeight = 0;
    mLightPopulated = false;
    mLastSaveHadEntities = false;
    this->level = level;
    this->z = z;
    mLoaded = false;
    mDontSave = false;
    mChunkFlags = 0;
    mCheckAllLight = 0;
    mLastSaveTime = 0;
    mHasGapsToCheck = false;
    this->x = x;
    mMaxHeight = 255;
    mReservedFlag = true;
    MemSect(1);
    mHeightmap = arrayWithLength<unsigned char>(256, true);
    MemSect(0);
    mLowestHeightmap = 256;
    mInhabitedTime = 0;
    for (int i = 0; i < 256; ++i)
        mPrecipitationHeightmap[i] = 255;
    for (int i = 0; i < 128; ++i)
        mBiomeFlags[i] = 0;
    mBiomePresent = 0;
    mEmissiveMaxHeight = 0;
    mPendingBlockEntityTag = NULL;
    mUnloaded = false;
    mChunkVersion = CURRENT_CHUNK_VERSION;
    mInvalid = false;

    Random random(level->getSeed() + (long long)(x * x * 4987142) + (long long)(x * 5947611)
                      + (long long)(z * z * 4392871) + (long long)(z * 389711)
                  ^ 987234911);
    if (random.nextInt(10) == 0)
        mBiomePresent |= BIOME_PRESENT_SLIME_CHUNK;
}
bool LevelChunk::isAt(int x, int z)
{
    return x == this->x && z == this->z;
}
bool LevelChunk::isLoaded()
{
    return mLoaded;
}
void LevelChunk::setLoaded(bool loaded)
{
    mLoaded = loaded;
}
int LevelChunk::getHeightmap(const BlockPos& pos)
{
    return getHeightmap(pos.x & 15, pos.z & 15);
}
unsigned char LevelChunk::getHeightmap(int x, int z)
{
    return mHeightmap.data[(z << 4) | x];
}
void LevelChunk::recalcBlockLights()
{
}
int LevelChunk::getHighestNonEmptyY()
{
    return mElements->getHighestNonEmptyY();
}
unsigned short* LevelChunk::GetElementStorage()
{
    ThreadStorage* storage = static_cast<ThreadStorage*>(TlsGetValue(sLevelChunkThreadStorage));
    return storage == NULL ? NULL : storage->elementStorage;
}
unsigned short* LevelChunk::GetExtraElementStorage()
{
    ThreadStorage* storage = static_cast<ThreadStorage*>(TlsGetValue(sLevelChunkThreadStorage));
    return storage == NULL ? NULL : storage->extraElementStorage;
}
void LevelChunk::setUnsaved(bool unsaved)
{
    mUnsaved = unsaved;
}
void LevelChunk::recalcHeightmapOnly()
{
    PIXBeginNamedEvent(0.0f, "LevelChunk::recalcHeightmapOnly - Begin");
    PIXBeginNamedEvent(0.0f, "LevelChunk::recalcHeightmapOnly - getHighestNonEmptyY");
    int highestY = getHighestNonEmptyY();
    PIXEndNamedEvent();
    int rangeEnd = highestY / 4;
    if (highestY & 3)
        ++rangeEnd;
    PIXBeginNamedEvent(0.0f, "LevelChunk::recalcHeightmapOnly - getElements");
    unsigned short* elements = GetElementStorage();
    unsigned short* extraElements = GetExtraElementStorage();
    bool ownsBuffers = (elements == NULL);
    if (ownsBuffers)
    {
        elements = new unsigned short[0x10000];
        extraElements = new unsigned short[0x10000];
    }
    XMemSet(elements, 0, 0x20000);
    XMemSet(extraElements, 0, 0x20000);
    getElementRange(arrayWithLength<unsigned short>(elements, 0x10000),
                    arrayWithLength<unsigned short>(extraElements, 0x10000), 0, rangeEnd);
    PIXEndNamedEvent();
    PIXBeginNamedEvent(0.0f, "LevelChunk::recalcHeightmapOnly - rangeLoop");
    int runningMin = 255;
    int runningMax = 0;
    for (int localX = 0; localX < 16; ++localX)
    {
        for (int localZ = 0; localZ < 16; ++localZ)
        {
            mPrecipitationHeightmap[(localZ << 4) | localX] = 255;
            int height = highestY;
            if (highestY > 0)
            {
                int lightBlock;
                do
                {
                    int blockY = height - 1;
                    int index = ((blockY & 0x80) << 8) | (localX << 11) | (localZ << 7) | (blockY & 0x7f);
                    Block* block = Block::byId(elements[index] >> 4);
                    Block* extraBlock = Block::byId(extraElements[index] >> 4);
                    lightBlock = block->defaultBlockState()->getLightBlock();
                    int extraLightBlock = extraBlock->defaultBlockState()->getLightBlock();
                    if (extraLightBlock > lightBlock)
                        lightBlock = extraLightBlock;
                } while (lightBlock == 0 && --height > 0);
            }
            mHeightmap.data[(localZ << 4) | localX] = (unsigned char)height;
            if (height < runningMin)
                runningMin = height;
            if (runningMax < height)
                runningMax = height;
        }
    }
    PIXEndNamedEvent();
    mMaxHeight = runningMax;
    mMinHeight = runningMin;
    setUnsaved(true);
    if (ownsBuffers)
    {
        delete[] elements;
        delete[] extraElements;
    }
    PIXEndNamedEvent();
}
void LevelChunk::lightGaps(int x, int z)
{
    int index = (x >> 1) | (z << 3);
    mBiomeFlags[index] |= (unsigned char)(1 << ((x & 1) << 2));
    mHasGapsToCheck = true;
}
void LevelChunk::recalcHeightmap()
{
    PIXBeginNamedEvent(0.0f, "LevelChunk::recalcHeightmap - Begin");
    PIXBeginNamedEvent(0.0f, "LevelChunk::recalcHeightmap - getHighestNonEmptyY");
    int highestY = getHighestNonEmptyY();
    PIXEndNamedEvent();
    int rangeEnd = highestY / 4;
    if (highestY & 3)
        ++rangeEnd;
    PIXBeginNamedEvent(0.0f, "LevelChunk::recalcHeightmap - getElementRange");
    unsigned short* elements = GetElementStorage();
    unsigned short* extraElements = GetExtraElementStorage();
    bool ownsBuffers = (elements == NULL);
    if (ownsBuffers)
    {
        elements = new unsigned short[0x10000];
        extraElements = new unsigned short[0x10000];
    }
    XMemSet(elements, 0, 0x20000);
    XMemSet(extraElements, 0, 0x20000);
    getElementRange(arrayWithLength<unsigned short>(elements, 0x10000),
                    arrayWithLength<unsigned short>(extraElements, 0x10000), 0, rangeEnd);
    PIXEndNamedEvent();
    int runningMin = 255;
    int runningMax = 0;
    mLowestHeightmap = 0x7fffffff;
    PIXBeginNamedEvent(0.0f, "LevelChunk::recalcHeightmap - RangedLoop");
    for (int localX = 0; localX < 16; ++localX)
    {
        for (int localZ = 0; localZ < 16; ++localZ)
        {
            PIXBeginNamedEvent(0.0f, "LevelChunk::recalcHeightmap - RangedLoop - findY");
            int height = highestY;
            if (highestY > 0)
            {
                int blockY = highestY;
                int lightBlock;
                do
                {
                    --blockY;
                    int index = ((blockY & 0x80) << 8) | (localX << 11) | (localZ << 7) | (blockY & 0x7f);
                    Block* block = Block::byId(elements[index] >> 4);
                    Block* extraBlock = Block::byId(extraElements[index] >> 4);
                    lightBlock = block->defaultBlockState()->getLightBlock();
                    int extraLightBlock = extraBlock->defaultBlockState()->getLightBlock();
                    if (extraLightBlock > lightBlock)
                        lightBlock = extraLightBlock;
                } while (lightBlock == 0 && --height > 0);
            }
            mHeightmap.data[(localZ << 4) | localX] = (unsigned char)height;
            if (height < runningMin)
                runningMin = height;
            if (runningMax < height)
                runningMax = height;
            if (height < mLowestHeightmap)
                mLowestHeightmap = height;
            PIXEndNamedEvent();
            PIXBeginNamedEvent(0.0f, "LevelChunk::recalcHeightmap - RangedLoop - setSkyLight");
            if (level->mDimension->isHasSkyLight())
            {
                SparseLightStorage* storage = mSkyLightUpper;
                int y = 255;
                int light = 15;
                for (;;)
                {
                    int lightBlock;
                    if (highestY < y)
                    {
                        lightBlock = (light != 15) ? 1 : 0;
                    }
                    else
                    {
                        int index = ((y & 0x80) << 8) | (localX << 11) | (localZ << 7) | (y & 0x7f);
                        Block* block = Block::byId(elements[index] >> 4);
                        Block* extraBlock = Block::byId(extraElements[index] >> 4);
                        lightBlock = block->defaultBlockState()->getLightBlock();
                        int extraLightBlock = extraBlock->defaultBlockState()->getLightBlock();
                        if (extraLightBlock > lightBlock)
                            lightBlock = extraLightBlock;
                        if (lightBlock == 0)
                            lightBlock = (light != 15) ? 1 : 0;
                    }
                    light -= lightBlock;
                    if (light > 0)
                        storage->set(localX, y & 0x7f, localZ, light);
                    --y;
                    if (y < 128)
                    {
                        storage = mSkyLight;
                        if (y < 1 || light < 1)
                            break;
                    }
                    else
                    {
                        storage = mSkyLightUpper;
                        if (light < 1)
                            break;
                    }
                }
            }
            PIXEndNamedEvent();
        }
    }
    PIXEndNamedEvent();
    mMaxHeight = runningMax;
    mMinHeight = runningMin;
    PIXBeginNamedEvent(0.0f, "LevelChunk::recalcHeightmap - lightGaps");
    for (int lx = 0; lx < 16; ++lx)
        for (int lz = 0; lz < 16; ++lz)
            lightGaps(lx, lz);
    PIXEndNamedEvent();
    setUnsaved(true);
    if (ownsBuffers)
    {
        delete elements;
        delete extraElements;
    }
    PIXEndNamedEvent();
}
Block* LevelChunk::getChunkBlock(int x, int y, int z)
{
    return Block::byId(getBlockId(x, y, z));
}
void LevelChunk::lightLava()
{
    if (mEmissiveMaxHeight != 0)
    {
        MutableBlockPos pos(0, 0, 0);
        for (int localX = 0; localX < 16; ++localX)
        {
            for (int localZ = 0; localZ < 16; ++localZ)
            {
                for (int localY = 0; localY < (int)mEmissiveMaxHeight; ++localY)
                {
                    Block* block = getChunkBlock(localX, localY, localZ);
                    int lightEmission = block->defaultBlockState()->getLightEmission();
                    if (0 < lightEmission)
                    {
                        pos.set(localX, localY, localZ);
                        int lightBlock = getBrightness(LightLayer::BLOCK, pos);
                        if (lightBlock < lightEmission)
                        {
                            pos.set(x * 16 + localX, localY, z * 16 + localZ);
                            level->checkLight(LightLayer::BLOCK, pos, true, false);
                        }
                    }
                }
            }
        }
        if (level->mDimension->getType() == DimensionType::THE_END)
        {
            bool east = level->hasChunk(x + 1, z, true);
            bool west = level->hasChunk(x - 1, z, true);
            bool south = level->hasChunk(x, z + 1, true);
            bool north = level->hasChunk(x, z - 1, true);
            if (west || east || north || south)
            {
                int worldX = x * 16;
                int worldZ = z * 16;
                for (int worldY = 0; worldY < 256; ++worldY)
                {
                    if (west || east)
                    {
                        for (int offset = 0; offset < 16; ++offset)
                        {
                            if (west)
                            {
                                MutableBlockPos westPos(worldX, worldY, worldZ + offset);
                                level->checkLight(LightLayer::BLOCK, westPos, true, false);
                            }
                            if (east)
                            {
                                MutableBlockPos eastPos(worldX + 15, worldY, worldZ + offset);
                                level->checkLight(LightLayer::BLOCK, eastPos, true, false);
                            }
                        }
                    }
                    if (north || south)
                    {
                        for (int offset = 0; offset < 16; ++offset)
                        {
                            if (north)
                            {
                                MutableBlockPos northPos(worldX + offset, worldY, worldZ);
                                level->checkLight(LightLayer::BLOCK, northPos, true, false);
                            }
                            if (south)
                            {
                                MutableBlockPos southPos(worldX + offset, worldY, worldZ + 15);
                                level->checkLight(LightLayer::BLOCK, southPos, true, false);
                            }
                        }
                    }
                }
            }
        }
        mEmissiveMaxHeight = 0;
    }
}
void LevelChunk::lightGap(int x, int z, int fromY, int toY)
{
    if (fromY < toY && level->hasChunksAt(BlockPos(x, 128, z), 16))
    {
        MutableBlockPos pos(0, 0, 0);
        for (int y = fromY; y < toY; ++y)
        {
            pos.set(x, y, z);
            level->checkLight(LightLayer::SKY, pos, false, false);
        }
        setUnsaved(true);
    }
}
void LevelChunk::lightGap(int x, int z, int targetHeight)
{
    int height = level->getHeightmapPos(BlockPos(x, 0, z)).y;
    if (height < targetHeight)
        lightGap(x, z, height, targetHeight + 1);
    else if (targetHeight < height)
        lightGap(x, z, targetHeight, height + 1);
}
void LevelChunk::recheckGaps(bool onlyOne)
{
    if (!isEmpty())
    {
        int minPos = (level->mDimension->getXZSize() * -16) >> 1;
        int maxPos = ((level->mDimension->getXZSize() << 4) >> 1) - 1;
        if (level->hasChunksAt(BlockPos(x * 16 + 8, 128, z * 16 + 8), 16))
        {
            for (int localX = 0; localX < 16; ++localX)
            {
                for (int localZ = 0; localZ < 16; ++localZ)
                {
                    int index = (localX >> 1) | (localZ << 3);
                    unsigned char mask = (unsigned char)(1 << ((localX & 1) << 2));
                    unsigned char flags = mBiomeFlags[index];
                    if (onlyOne || (flags & mask) != 0)
                    {
                        mBiomeFlags[index] = (unsigned char)(flags & ~mask);
                        unsigned char height = getHeightmap(localX, localZ);
                        int worldX = x * 16 + localX;
                        int worldZ = z * 16 + localZ;
                        BlockPos pos(worldX, 0, worldZ);
                        int minHeight = level->getHeightmapPos(pos).y;
                        int westX = worldX - 1;
                        if (minPos <= westX)
                        {
                            int westHeight = level->getHeightmapPos(pos.west()).y;
                            if (westHeight < minHeight)
                                minHeight = westHeight;
                        }
                        int eastX = worldX + 1;
                        if (eastX <= maxPos)
                        {
                            int eastHeight = level->getHeightmapPos(pos.east()).y;
                            if (eastHeight < minHeight)
                                minHeight = eastHeight;
                        }
                        int northZ = worldZ - 1;
                        if (minPos <= northZ)
                        {
                            int northHeight = level->getHeightmapPos(pos.north()).y;
                            if (northHeight < minHeight)
                                minHeight = northHeight;
                        }
                        int southZ = worldZ + 1;
                        if (southZ <= maxPos)
                        {
                            int southHeight = level->getHeightmapPos(pos.south()).y;
                            if (southHeight < minHeight)
                                minHeight = southHeight;
                        }
                        lightGap(worldX, worldZ, minHeight);
                        if (!onlyOne)
                        {
                            if (minPos <= westX)
                                lightGap(westX, worldZ, height);
                            if (eastX <= maxPos)
                                lightGap(eastX, worldZ, height);
                            if (minPos <= northZ)
                                lightGap(worldX, northZ, height);
                            if (southZ <= maxPos)
                                lightGap(worldX, southZ, height);
                        }
                        mHasGapsToCheck = false;
                    }
                }
            }
        }
    }
}
void LevelChunk::recalcHeight(int localX, int y, int localZ)
{
    int index = (localZ << 4) | localX;
    int oldHeight = mHeightmap.data[index];
    int height = y > oldHeight ? y : oldHeight;
    while (height > 0)
    {
        Block* block = Block::byId(getBlockId(localX, height - 1, localZ));
        Block* extraBlock = Block::byId(getExtraBlockId(localX, height - 1, localZ));
        int lightBlock = block->defaultBlockState()->getLightBlock();
        int extraLightBlock = extraBlock->defaultBlockState()->getLightBlock();
        if (lightBlock < extraLightBlock)
            lightBlock = extraLightBlock;
        if (lightBlock != 0)
            break;
        --height;
    }
    if (height == oldHeight)
        return;
    mHeightmap.data[index] = (unsigned char)height;
    bool recalculate = true;
    if (height < mMinHeight)
    {
        recalculate = false;
        mMinHeight = height;
    }
    int maxHeight = mMaxHeight;
    bool aboveMax = height > maxHeight;
    if (aboveMax)
        height = maxHeight;
    if (!aboveMax || recalculate)
    {
        int runningMin = 255;
        int runningMax = 0;
        for (int columnX = 0; columnX < 16; ++columnX)
        {
            for (int columnZ = 0; columnZ < 16; ++columnZ)
            {
                int columnHeight = mHeightmap.data[(columnZ << 4) | columnX];
                if (columnHeight < runningMin)
                    runningMin = columnHeight;
                if (runningMax < columnHeight)
                    runningMax = columnHeight;
            }
        }
        mMaxHeight = runningMax;
        mMinHeight = runningMin;
    }
    int worldX = x * 16 + localX;
    int worldZ = z * 16 + localZ;
    if (level->mDimension->isHasSkyLight())
    {
        if (oldHeight < height)
        {
            for (int lightY = oldHeight; lightY < height; ++lightY)
            {
                if (lightY < 128)
                    mSkyLight->set(localX, lightY % 128, localZ, 0);
                else
                    mSkyLightUpper->set(localX, lightY % 128, localZ, 0);
            }
        }
        else if (height < oldHeight)
        {
            for (int lightY = height; lightY < oldHeight; ++lightY)
            {
                if (lightY < 128)
                    mSkyLight->set(localX, lightY % 128, localZ, 15);
                else
                    mSkyLightUpper->set(localX, lightY % 128, localZ, 15);
            }
        }
        int remaining = 15;
        while (height > 0 && remaining > 0)
        {
            --height;
            SparseLightStorage* storage = height < 128 ? mSkyLight : mSkyLightUpper;
            Block* block = Block::byId(getBlockId(localX, height, localZ));
            Block* extraBlock = Block::byId(getExtraBlockId(localX, height, localZ));
            int lightBlock = block->defaultBlockState()->getLightBlock();
            int extraLightBlock = extraBlock->defaultBlockState()->getLightBlock();
            if (lightBlock < extraLightBlock)
                lightBlock = extraLightBlock;
            if (lightBlock == 0)
                lightBlock = 1;
            remaining -= lightBlock;
            if (remaining < 0)
                remaining = 0;
            storage->set(localX, height % 128, localZ, remaining);
        }
    }
    level->lightColumnChanged(worldX, worldZ, height, oldHeight);
    int newHeight = mHeightmap.data[index];
    int lowY = newHeight < oldHeight ? newHeight : oldHeight;
    int highY = newHeight < oldHeight ? oldHeight : newHeight;
    if (newHeight < mLowestHeightmap)
        mLowestHeightmap = newHeight;
    if (level->mDimension->isHasSkyLight())
    {
        PIXBeginNamedEvent(0.0f, "Light gaps");
        lightGap(worldX - 1, worldZ, lowY, highY);
        lightGap(worldX + 1, worldZ, lowY, highY);
        lightGap(worldX, worldZ - 1, lowY, highY);
        lightGap(worldX, worldZ + 1, lowY, highY);
        lightGap(worldX, worldZ, lowY, highY);
        PIXEndNamedEvent();
    }
    setUnsaved(true);
}
int LevelChunk::getBlockLightBlock(const BlockPos& pos)
{
    int block = getBlock(pos)->defaultBlockState()->getLightBlock();
    int extra = getExtraBlock(pos)->defaultBlockState()->getLightBlock();
    return block > extra ? block : extra;
}
int LevelChunk::getChunkBlockLightBlock(int x, int y, int z)
{
    return getBlockLightBlock(BlockPos(x, y, z));
}
Block* LevelChunk::getBlock(const BlockPos& pos)
{
    return getChunkBlock(pos.x & 15, pos.y, pos.z & 15);
}
Block* LevelChunk::getExtraBlock(const BlockPos& pos)
{
    return Block::byId(getExtraBlockId(pos.x, pos.y, pos.z));
}
const BlockState* LevelChunk::getBlockState(int x, int y, int z)
{
    if (isEmpty())
        return Blocks::air->defaultBlockState();
    if (y < 0)
        return Block::getStateByElementId(0);
    return Block::getStateByElementId((unsigned short)mElements->get(x & 15, y & 0xff, z & 15, NULL));
}
const BlockState* LevelChunk::getBlockState(const BlockPos& pos)
{
    return getBlockState(pos.x, pos.y, pos.z);
}
const BlockState* LevelChunk::getExtraBlockState(const BlockPos& pos)
{
    if (isEmpty())
        return Blocks::air->defaultBlockState();
    if (pos.y < 0)
        return Block::getStateByElementId(0);
    return Block::getStateByElementId((unsigned short)mElements->getExtraElement(pos.x & 15, pos.y & 0xff, pos.z & 15));
}
void LevelChunk::getBlockAndExtraId_NoLock(int x, int y, int z, int* blockId, int* extraBlockId)
{
    unsigned short element = 0;
    unsigned short extra = 0;
    mElements->getElementAndExtraElement_NoLock(x, y & 0xff, z, &element, &extra);
    *blockId = element >> 4;
    *extraBlockId = extra >> 4;
}
void LevelChunk::getElementReadLock()
{
    mElements->getReadLockExternal();
}
void LevelChunk::releaseElementReadLock()
{
    mElements->releaseReadLockExternal();
}
unsigned int LevelChunk::getElementFrameLastUpdated()
{
    return mElements->getFrameLastUpdated();
}
void LevelChunk::setElement(int x, int y, int z, unsigned short element)
{
    mElements->set(x, y, z, element);
    const BlockState* state = Block::getStateByElementId(element);
    if (state != NULL && state->getLightEmission() > 0 && mEmissiveMaxHeight < y + 1)
        mEmissiveMaxHeight = static_cast<unsigned short>(y + 1);
}
unsigned short LevelChunk::getChunkBlockData(int x, int y, int z)
{
    const BlockState* state = getBlockState(x, y, z);
    return state->getBlock()->convertBlockStateToLegacyData(state);
}
int LevelChunk::getData(const BlockPos& pos)
{
    return getChunkBlockData(pos.x & 15, pos.y, pos.z & 15);
}
bool LevelChunk::setData(int x, int y, int z, int data, int dataMask, bool* maskedChange)
{
    setUnsaved(true);
    unsigned int old = mElements->get(x, y & 0xff, z, NULL);
    *maskedChange = (dataMask & (old ^ data)) != 0;
    mElements->set(x, y & 0xff, z, static_cast<unsigned short>((old & 0xff0) | data));

    Block* block = Block::byId(old >> 4);
    if (block->mBlockEntity)
    {
        boost::shared_ptr<BlockEntity> blockEntity = getBlockEntity(BlockPos(x, y, z), NO_CREATE);
        if (blockEntity != NULL)
        {
            blockEntity->clearCache();
            blockEntity->setData(data);
        }
    }
    return true;
}
const BlockState* LevelChunk::setBlock(const BlockPos& pos, const BlockState* state)
{
    return internalSetBlock(false, pos, state);
}
const BlockState* LevelChunk::setExtraBlock(const BlockPos& pos, const BlockState* state)
{
    return internalSetBlock(true, pos, state);
}
const BlockState* LevelChunk::internalSetBlock(bool extra, const BlockPos& pos, const BlockState* state)
{
    int y = pos.y;
    int localX = pos.x & 15;
    int localZ = pos.z & 15;
    int columnIndex = (localZ << 4) | localX;
    if ((int)(mPrecipitationHeightmap[columnIndex] - 1) <= y)
        mPrecipitationHeightmap[columnIndex] = 255;
    int existingHeight = mHeightmap.data[columnIndex];
    unsigned short oldValue = extra ? mElements->getExtraElement(localX, y, localZ)
                                    : mElements->get(localX, y, localZ, NULL);
    const BlockState* oldState = Block::getStateByElementId(oldValue);
    if (oldState == state)
    {
        boost::shared_ptr<BlockEntity> blockEntity = getBlockEntity(pos, NO_CREATE);
        if (blockEntity)
            blockEntity->clearCache();
        return NULL;
    }
    Block* newBlock = state->getBlock();
    int newBlockId = Block::getId(newBlock);
    int data = newBlock->convertBlockStateToLegacyData(state);
    Block* oldBlock = oldState->getBlock();
    int oldLightBlock = getBlockState(pos)->getLightBlock();
    int oldExtraLightBlock = getExtraBlockState(pos)->getLightBlock();
    if (oldLightBlock < oldExtraLightBlock)
        oldLightBlock = oldExtraLightBlock;
    bool commandBlockChange = false;
    if (!extra)
    {
        mElements->set(localX, y, localZ, (unsigned short)((newBlockId << 4 | data) & 0xffff));
        if (oldBlock != newBlock)
        {
            bool oldWasWaterSource = oldBlock->mMaterial == Material::WATER
                                     && oldState->getValue(LiquidBlock::LEVEL) == 0;
            if (!newBlock->canContainLiquid(state))
            {
                mElements->setExtraElement(localX, y, localZ, 0);
            }
            else if (newBlock->mMaterial == Material::AIR)
            {
                unsigned short extraValue = mElements->getExtraElement(localX, y, localZ);
                if (extraValue != 0)
                {
                    mElements->set(localX, y, localZ, extraValue);
                    mElements->setExtraElement(localX, y, localZ, 0);
                }
            }
            else if (oldBlock->mMaterial->isLiquid() && oldWasWaterSource && !newBlock->mMaterial->isLiquid())
            {
                mElements->setExtraElement(localX, y, localZ, oldValue);
            }
            if (0 < newBlock->defaultBlockState()->getLightEmission() && mEmissiveMaxHeight < y + 1)
                mEmissiveMaxHeight = (unsigned short)(y + 1);
            if (level->mIsClientSide)
            {
                if (oldBlock->mBlockEntity)
                {
                    if (CommandBlock::isCommandBlock(oldBlock) && CommandBlock::isCommandBlock(newBlock))
                        commandBlockChange = true;
                    else
                        level->removeBlockEntity(pos);
                }
            }
            else
            {
                oldBlock->onRemove(level, pos, oldState);
            }
        }
    }
    else if (newBlockId == BlockId::AIR
             || (newBlock->mMaterial->isLiquid() && newBlock->mMaterial == Material::WATER
                 && newBlock->mMaterial != Material::AIR && newBlock->mMaterial != Material::LAVA))
    {
        mElements->setExtraElement(localX, y, localZ, (unsigned short)((newBlockId << 4 | data) & 0xffff));
    }
    PIXBeginNamedEvent(0.0f, "Updating lighting");
    int newLightBlock = getBlockState(pos)->getLightBlock();
    int newExtraLightBlock = getExtraBlockState(pos)->getLightBlock();
    newLightBlock = newLightBlock < newExtraLightBlock ? newExtraLightBlock : newLightBlock;
    if (0 < newLightBlock)
    {
        if (existingHeight <= y)
        {
            PIXBeginNamedEvent(0.0f, "Recalc height 1");
            recalcHeight(localX, y + 1, localZ);
            PIXEndNamedEvent();
        }
    }
    else if (y == existingHeight - 1)
    {
        PIXBeginNamedEvent(0.0f, "Recalc height 2");
        recalcHeight(localX, y, localZ);
        PIXEndNamedEvent();
    }
    if (newLightBlock < oldLightBlock
        || (oldLightBlock < newLightBlock
            && (getBrightness(LightLayer::SKY, pos) > 0 || getBrightness(LightLayer::BLOCK, pos) > 0)))
    {
        PIXBeginNamedEvent(0.0f, "Lighting gaps");
        lightGaps(localX, localZ);
        PIXEndNamedEvent();
    }
    if (oldBlock->mBlockEntity && !commandBlockChange)
    {
        boost::shared_ptr<BlockEntity> blockEntity = getBlockEntity(pos, NO_CREATE);
        if (blockEntity)
            blockEntity->clearCache();
    }
    PIXEndNamedEvent();
    if (!level->mIsClientSide && oldBlock != newBlock)
    {
        newBlock->onPlace(level, pos, state);
    }
    else if (newBlock == Blocks::fire && !newBlock->mayPlace(level, pos))
    {
        mElements->set(localX, y & 0xff, localZ, 0);
    }
    if (newBlock->mBlockEntity && !commandBlockChange)
    {
        boost::shared_ptr<BlockEntity> blockEntity = getBlockEntity(pos, NO_CREATE);
        if (!blockEntity)
        {
            EntityBlock* entityBlock = dynamic_cast<EntityBlock*>(newBlock);
            blockEntity = entityBlock->newBlockEntity(level, newBlock->convertBlockStateToLegacyData(state));
            level->setBlockEntity(pos, blockEntity);
        }
        if (blockEntity)
            blockEntity->clearCache();
    }
    setUnsaved(true);
    return oldState;
}

bool LevelChunk::setBlockAndData(int x, int y, int z, int blockId, int data, bool update, bool extra)
{
    int localX = x & 15;
    int localZ = z & 15;
    BlockPos pos(x, y, z);
    int columnIndex = (localZ << 4) | localX;
    if ((int)(mPrecipitationHeightmap[columnIndex] - 1) <= y)
        mPrecipitationHeightmap[columnIndex] = 255;
    if (extra)
    {
        if (mElements != NULL)
            mElements->set(localX, y, localZ, (unsigned short)((blockId << 4 | data) & 0xffff));
        return true;
    }
    int existingHeight = mHeightmap.data[columnIndex];
    int oldValue = mElements->get(localX, y & 0xff, localZ, NULL);
    int oldBlockId = oldValue >> 4;
    const BlockState* oldState = Block::getStateByElementId(oldValue);
    if (oldBlockId == blockId && (oldValue & 0xf) == data)
    {
        boost::shared_ptr<BlockEntity> blockEntity = getBlockEntity(pos, NO_CREATE);
        if (blockEntity)
            blockEntity->clearCache();
        return false;
    }
    Block* oldBlock = Block::byId(oldBlockId);
    int oldLightBlock = getBlockState(pos)->getLightBlock();
    int oldExtraLightBlock = getExtraBlockState(pos)->getLightBlock();
    if (oldLightBlock < oldExtraLightBlock)
        oldLightBlock = oldExtraLightBlock;
    Block* newBlock = Block::byId(blockId);
    bool commandBlockChange = false;
    if (oldBlockId != BlockId::AIR)
    {
        if (level->mIsClientSide)
        {
            if (oldBlock->mBlockEntity && oldBlockId != blockId)
            {
                if (CommandBlock::isCommandBlock(oldBlock) && CommandBlock::isCommandBlock(newBlock))
                    commandBlockChange = true;
                else
                    level->removeBlockEntity(pos);
            }
        }
        else
        {
            oldBlock->onRemove(level, pos, oldState);
        }
    }
    if (mElements != NULL)
        mElements->set(localX, y, localZ, (unsigned short)((blockId << 4 | data) & 0xffff));
    bool oldWasWater = oldBlock->mMaterial == Material::WATER;
    if (!newBlock->canContainLiquid(newBlock->getBlockState(data)))
    {
        mElements->setExtraElement(localX, y, localZ, 0);
    }
    else if (blockId == BlockId::AIR)
    {
        unsigned short extraValue = mElements->getExtraElement(localX, y, localZ);
        if (extraValue != 0)
        {
            mElements->set(localX, y, localZ, extraValue);
            mElements->setExtraElement(localX, y, localZ, 0);
        }
    }
    else if (oldBlock->mMaterial->isLiquid() && oldWasWater && !newBlock->mMaterial->isLiquid())
    {
        mElements->setExtraElement(localX, y, localZ, oldValue);
    }
    int newLightEmission = newBlock->defaultBlockState()->getLightEmission();
    if (0 < newLightEmission && mEmissiveMaxHeight < y + 1)
        mEmissiveMaxHeight = (unsigned short)(y + 1);
    PIXBeginNamedEvent(0.0f, "Updating lighting");
    int newLightBlock = getBlockState(pos)->getLightBlock();
    int newExtraLightBlock = getExtraBlockState(pos)->getLightBlock();
    if (newLightBlock < newExtraLightBlock)
        newLightBlock = newExtraLightBlock;
    if (update && newLightBlock != oldLightBlock)
    {
        if (level->mDimension->isHasSkyLight())
        {
            if (newLightBlock == 0)
            {
                if (y == existingHeight - 1)
                {
                    PIXBeginNamedEvent(0.0f, "Recalc height 2");
                    recalcHeight(localX, y, localZ);
                    PIXEndNamedEvent();
                }
            }
            else if (existingHeight <= y)
            {
                PIXBeginNamedEvent(0.0f, "Recalc height 1");
                recalcHeight(localX, y + 1, localZ);
                PIXEndNamedEvent();
            }
        }
        PIXBeginNamedEvent(0.0f, "Lighting gaps");
        lightGaps(localX, localZ);
        PIXEndNamedEvent();
    }
    PIXEndNamedEvent();
    if (blockId >= BlockId::AIR)
    {
        if (blockId < 1)
        {
            if (0 < oldBlockId && newBlock->mBlockEntity)
            {
                boost::shared_ptr<BlockEntity> blockEntity = getBlockEntity(pos, NO_CREATE);
                if (blockEntity)
                    blockEntity->clearCache();
            }
        }
        else if (newBlock->mBlockEntity && !commandBlockChange)
        {
            boost::shared_ptr<BlockEntity> blockEntity = getBlockEntity(pos, NO_CREATE);
            if (!blockEntity)
            {
                EntityBlock* entityBlock = dynamic_cast<EntityBlock*>(newBlock);
                blockEntity = entityBlock->newBlockEntity(level, data);
                level->setBlockEntity(pos, blockEntity);
            }
            if (blockEntity)
                blockEntity->clearCache();
        }
    }
    setUnsaved(true);
    return true;
}
void LevelChunk::setExtraElement(int x, int y, int z, unsigned short element)
{
    if (mElements != NULL)
        mElements->setExtraElement(x, y, z, element);
}
unsigned short LevelChunk::getExtraElement(int x, int y, int z)
{
    if (mElements == NULL)
        return 0;
    return mElements->getExtraElement(x, y, z);
}
void LevelChunk::getElementAndExtraElement(int x, int y, int z, unsigned short* element, unsigned short* extraElement)
{
    if (mElements != NULL)
    {
        mElements->getElementAndExtraElement(x, y, z, element, extraElement);
        return;
    }
    *element = 0;
    *extraElement = 0;
}
int LevelChunk::getBrightness(LightLayer::variety layer, const BlockPos& pos)
{
    if (layer == LightLayer::SKY)
    {
        if (!level->mDimension->isHasSkyLight())
            return 0;

        SparseLightStorage* storage = pos.y < 128 ? mSkyLight : mSkyLightUpper;
        return storage == NULL ? 0 : storage->get(pos.x & 15, pos.y & 127, pos.z & 15);
    }

    if (layer == LightLayer::BLOCK)
    {
        SparseLightStorage* storage = pos.y < 128 ? mBlockLight : mBlockLightUpper;
        return storage == NULL ? 0 : storage->get(pos.x & 15, pos.y & 127, pos.z & 15);
    }

    return 0;
}
void LevelChunk::getNeighbourBrightnesses(int* brightnesses, LightLayer::variety layer, int x, int y, int z)
{
    SparseLightStorage* storage;
    if (layer == LightLayer::SKY)
        storage = y < 128 ? mSkyLight : mSkyLightUpper;
    else
        storage = y < 128 ? mBlockLight : mBlockLightUpper;

    if (storage != NULL)
    {
        int localY = y - (((y >> 31) & 0x7F) + y & ~0x7F);
        brightnesses[0] = storage->get(x - 1, localY, z);
        brightnesses[1] = storage->get(x + 1, localY, z);
        brightnesses[4] = storage->get(x, localY, z - 1);
        brightnesses[5] = storage->get(x, localY, z + 1);
    }

    int belowY = y - 1;
    if (layer == LightLayer::SKY)
        storage = belowY < 128 ? mSkyLight : mSkyLightUpper;
    else
        storage = belowY < 128 ? mBlockLight : mBlockLightUpper;

    if (storage != NULL)
    {
        int localY = belowY - (((belowY >> 31) & 0x7F) + belowY & ~0x7F);
        brightnesses[2] = storage->get(x, localY, z);
    }

    int aboveY = y + 1;
    if (layer == LightLayer::SKY)
        storage = aboveY < 128 ? mSkyLight : mSkyLightUpper;
    else
        storage = aboveY < 128 ? mBlockLight : mBlockLightUpper;

    if (storage != NULL)
    {
        int localY = aboveY - (((aboveY >> 31) & 0x7F) + aboveY & ~0x7F);
        brightnesses[3] = storage->get(x, localY, z);
    }
}
void LevelChunk::setBrightness(LightLayer::variety layer, const BlockPos& pos, int brightness)
{
    setUnsaved(true);

    if (layer == LightLayer::SKY)
    {
        if (level->mDimension->isHasSkyLight())
        {
            SparseLightStorage* storage = pos.y >= 128 ? mSkyLightUpper : mSkyLight;
            storage->set(pos.x & 15, pos.y % 128, pos.z & 15, brightness);
        }
    }
    else if (layer == LightLayer::BLOCK)
    {
        SparseLightStorage* storage = pos.y >= 128 ? mBlockLightUpper : mBlockLight;
        storage->set(pos.x & 15, pos.y % 128, pos.z & 15, brightness);
    }
}
int LevelChunk::getRawBrightness(const BlockPos& pos, int skyDarken)
{
    int localX = pos.x & 15;
    int y = pos.y;
    int localZ = pos.z & 15;
    SparseLightStorage* skyStorage = y >= 128 ? mSkyLightUpper : mSkyLight;
    int brightness;
    if (!level->mDimension->isHasSkyLight())
    {
        brightness = 0;
    }
    else
    {
        brightness = skyStorage->get(localX, y % 128, localZ);
        if (brightness > 0)
            touchedSky = true;
    }
    brightness -= skyDarken;

    SparseLightStorage* blockStorage = y >= 128 ? mBlockLightUpper : mBlockLight;
    int block = blockStorage->get(localX, y % 128, localZ);
    if (block > brightness)
        brightness = block;
    return brightness;
}
void LevelChunk::addEntity(boost::shared_ptr<Entity> entity)
{
    mLastSaveHadEntities = true;
    int chunkX = Mth::floor(entity->mX / 16.0);
    int chunkZ = Mth::floor(entity->mZ / 16.0);
    if (chunkX != x || chunkZ != z)
        entity->remove();
    int section = Mth::floor(entity->mY / 16.0);
    if (section < 0)
        section = 0;
    if (section >= mEntitySectionCount)
        section = mEntitySectionCount - 1;
    entity->mInChunk = true;
    entity->mChunkX = x;
    entity->mChunkY = section;
    entity->mChunkZ = z;
    EnterCriticalSection(&m_csEntities);
    mEntitySections[section]->push_back(entity);
    LeaveCriticalSection(&m_csEntities);
}
void LevelChunk::removeEntity(boost::shared_ptr<Entity> entity)
{
    removeEntity(entity, entity->mChunkY);
}
void LevelChunk::removeEntity(boost::shared_ptr<Entity> entity, int section)
{
    if (section < 0)
        section = 0;
    if (section >= mEntitySectionCount)
        section = mEntitySectionCount - 1;
    EnterCriticalSection(&m_csEntities);
    std::vector<boost::shared_ptr<Entity> >& entities = *mEntitySections[section];
    std::vector<boost::shared_ptr<Entity> >::iterator it = std::find(entities.begin(), entities.end(), entity);
    if (it != entities.end())
    {
        entities.erase(it);
        MemSect(0x1f);
        if (sEntityRemovalFlag)
            sEntityRemovalFlag = false;
        MemSect(0);
    }
    LeaveCriticalSection(&m_csEntities);
}
bool LevelChunk::isSkyLit(const BlockPos& pos)
{
    return pos.y >= mHeightmap.data[((pos.z & 15) << 4) | (pos.x & 15)];
}
boost::shared_ptr<BlockEntity> LevelChunk::createBlockEntity(const BlockPos& pos)
{
    const BlockState* state = getBlockState(pos);
    Block* block = state->getBlock();
    if (!block->mBlockEntity)
        return boost::shared_ptr<BlockEntity>();
    EntityBlock* entityBlock = dynamic_cast<EntityBlock*>(block);
    return entityBlock->newBlockEntity(level, state->getBlock()->convertBlockStateToLegacyData(state));
}
boost::shared_ptr<BlockEntity> LevelChunk::getBlockEntity(const BlockPos& pos, EntityCreationType type)
{
    boost::shared_ptr<BlockEntity> result;
    EnterCriticalSection(&m_csTileEntities);
    BlockEntityMap::iterator it = mBlockEntityMap.find(pos);
    if (it == mBlockEntityMap.end())
    {
        LeaveCriticalSection(&m_csTileEntities);
        if (level->mScheduledUpdatesAreImmediate)
            return boost::shared_ptr<BlockEntity>();
        if (type == IMMEDIATE)
        {
            result = createBlockEntity(pos);
            level->setBlockEntity(pos, result);
        }
        else if (type == QUEUED)
        {
            mPendingBlockEntities.push_back(pos);
        }
        EnterCriticalSection(&m_csTileEntities);
        it = mBlockEntityMap.find(pos);
    }
    if (it != mBlockEntityMap.end())
        result = it->second;
    LeaveCriticalSection(&m_csTileEntities);
    if (result && result->isRemoved())
    {
        EnterCriticalSection(&m_csTileEntities);
        mBlockEntityMap.erase(pos);
        LeaveCriticalSection(&m_csTileEntities);
        return boost::shared_ptr<BlockEntity>();
    }
    return result;
}
void LevelChunk::addBlockEntity(boost::shared_ptr<BlockEntity> entity)
{
    setBlockEntity(entity->getBlockPos(), entity);
    if (mLoaded)
    {
        EnterCriticalSection(&level->mBlockEntityMutex);
        level->addBlockEntity(entity);
        LeaveCriticalSection(&level->mBlockEntityMutex);
    }
}
void LevelChunk::setBlockEntity(const BlockPos& pos, boost::shared_ptr<BlockEntity> entity)
{
    entity->setLevel(level);
    entity->setPosition(pos);
    Block* block = getBlock(pos);
    if (block->mBlockEntity)
    {
        BlockEntityMap::iterator old = mBlockEntityMap.find(pos);
        if (old != mBlockEntityMap.end())
            old->second->setRemoved();
        entity->clearRemoved();
        EnterCriticalSection(&m_csTileEntities);
        mBlockEntityMap[pos] = entity;
        LeaveCriticalSection(&m_csTileEntities);
    }
}
void LevelChunk::removeBlockEntity(const BlockPos& pos)
{
    if (mLoaded)
    {
        EnterCriticalSection(&m_csTileEntities);
        BlockEntityMap::iterator it = mBlockEntityMap.find(pos);
        if (it != mBlockEntityMap.end())
        {
            boost::shared_ptr<BlockEntity> removed = mBlockEntityMap[pos];
            mBlockEntityMap.erase(pos);
            if (removed.get() != NULL)
                removed->setRemoved();
        }
        LeaveCriticalSection(&m_csTileEntities);
    }
}
void LevelChunk::load(bool blockEntitiesOnly)
{
    mLoaded = true;
    SetInvalid(false);
    if (level->mIsClientSide && !blockEntitiesOnly)
    {
        mUnloaded = false;
        return;
    }
    if (mUnloaded && mPendingBlockEntityTag != NULL)
    {
        ListTag* entities = mPendingBlockEntityTag->getList(L"Entities", Tag::TAG_COMPOUND);
        if (entities != NULL)
        {
            for (int i = 0; i < entities->size(); ++i)
            {
                CompoundTag* tag = entities->getCompound(i);
                boost::shared_ptr<Entity> entity = EntityIO::newEntity(tag, level);
                if (entity)
                {
                    entity->onLoadedFromSave();
                    addEntity(entity);
                    if (tag->contains(Entity::RIDING_TAG, Tag::TAG_LIST))
                    {
                        ListTag* riders = tag->getList(Entity::RIDING_TAG, Tag::TAG_COMPOUND);
                        for (int j = 0; j < riders->size(); ++j)
                        {
                            boost::shared_ptr<Entity> rider = EntityIO::newEntity(riders->getCompound(j), level);
                            if (rider)
                            {
                                addEntity(rider);
                                rider->startRiding(entity, true);
                            }
                        }
                    }
                }
            }
        }
        ListTag* blockEntities = mPendingBlockEntityTag->getList(L"TileEntities", Tag::TAG_COMPOUND);
        if (blockEntities != NULL)
        {
            for (int i = 0; i < blockEntities->size(); ++i)
            {
                boost::shared_ptr<BlockEntity> entity = BlockEntity::loadStatic(level, blockEntities->getCompound(i));
                if (entity)
                    addBlockEntity(entity);
            }
        }
        ListTag* ticks = mPendingBlockEntityTag->getList(L"TileTicks", Tag::TAG_COMPOUND);
        if (ticks != NULL)
        {
            for (int i = 0; i < ticks->size(); ++i)
            {
                CompoundTag* tag = ticks->getCompound(i);
                Block* block = tag->contains(L"i", Tag::TAG_STRING) ? Block::byString(tag->getString(L"i"))
                                                                    : Block::byId(tag->getInt(L"i"));
                const int tickX = tag->getInt(L"x");
                const int tickY = tag->getInt(L"y");
                const int tickZ = tag->getInt(L"z");
                const BlockPos pos(tickX, tickY, tickZ);
                const int delay = tag->getInt(L"t");
                const int priority = tag->getInt(L"p");
                level->forceAddBlockTick(pos, block, delay, priority);
            }
        }
        delete mPendingBlockEntityTag;
        mPendingBlockEntityTag = NULL;
    }
    mUnloaded = false;
    std::vector<boost::shared_ptr<BlockEntity> > blockEntities;
    EnterCriticalSection(&m_csTileEntities);
    for (BlockEntityMap::iterator it = mBlockEntityMap.begin(); it != mBlockEntityMap.end(); ++it)
        blockEntities.push_back(it->second);
    LeaveCriticalSection(&m_csTileEntities);
    level->addAllPendingBlockEntities(blockEntities);
    if (!blockEntitiesOnly)
    {
        EnterCriticalSection(&m_csEntities);
        for (int i = 0; i < mEntitySectionCount; ++i)
            level->addEntities(mEntitySections[i]);
        LeaveCriticalSection(&m_csEntities);
    }
}
void LevelChunk::unload(bool removeBlockEntities, bool removeEntities)
{
    mLoaded = false;
    if (removeBlockEntities)
    {
        EnterCriticalSection(&m_csTileEntities);
        for (BlockEntityMap::iterator it = mBlockEntityMap.begin(); it != mBlockEntityMap.end(); ++it)
            level->markForRemoval(it->second);
        LeaveCriticalSection(&m_csTileEntities);
    }
    if (removeEntities)
    {
        EnterCriticalSection(&m_csEntities);
        for (int i = 0; i < mEntitySectionCount; ++i)
            level->removeEntities(mEntitySections[i]);
        LeaveCriticalSection(&m_csEntities);
    }
    if (!mUnloaded)
    {
        mUnloaded = true;
        if (!level->mIsClientSide)
        {
            delete mPendingBlockEntityTag;
            mPendingBlockEntityTag = new CompoundTag();
            PIXBeginNamedEvent(0.0f, "Saving entities");
            ListTag* entities = new ListTag();
            EnterCriticalSection(&m_csEntities);
            for (int i = 0; i < mEntitySectionCount; ++i)
            {
                std::vector<boost::shared_ptr<Entity> >& section = *mEntitySections[i];
                for (std::vector<boost::shared_ptr<Entity> >::iterator it = section.begin(); it != section.end(); ++it)
                {
                    boost::shared_ptr<Entity> entity = *it;
                    CompoundTag* tag = new CompoundTag();
                    if (entity->save(tag))
                        entities->add(tag);
                }
                section.clear();
            }
            LeaveCriticalSection(&m_csEntities);
            mPendingBlockEntityTag->put(L"Entities", entities);
            PIXEndNamedEvent();
            PIXBeginNamedEvent(0.0f, "Saving block entities");
            ListTag* blockEntities = new ListTag();
            for (BlockEntityMap::iterator it = mBlockEntityMap.begin(); it != mBlockEntityMap.end(); ++it)
            {
                boost::shared_ptr<BlockEntity> entity = it->second;
                CompoundTag* tag = new CompoundTag();
                entity->save(tag);
                blockEntities->add(tag);
            }
            mPendingBlockEntityTag->put(L"TileEntities", blockEntities);
            PIXEndNamedEvent();
            PIXBeginNamedEvent(0.0f, "Saving block tick data");
            std::vector<TickNextTickData>* ticks = level->fetchTicksInChunk(this, false);
            if (ticks != NULL)
            {
                const long long gameTime = level->getGameTime();
                ListTag* savedTicks = new ListTag();
                for (std::size_t i = 0; i < ticks->size(); ++i)
                {
                    TickNextTickData tick = ticks->at(i);
                    CompoundTag* tag = new CompoundTag();
                    tag->putInt(L"i", Block::getId(tick.getBlock()));
                    tag->putInt(L"x", tick.pos.x);
                    tag->putInt(L"y", tick.pos.y);
                    tag->putInt(L"z", tick.pos.z);
                    tag->putInt(L"t", static_cast<int>(tick.triggerTick - gameTime));
                    tag->putInt(L"p", tick.priorityTilt);
                    savedTicks->add(tag);
                }
                mPendingBlockEntityTag->put(L"TileTicks", savedTicks);
                delete ticks;
            }
            PIXEndNamedEvent();
        }
        if (removeBlockEntities)
            mBlockEntityMap.clear();
    }
}
bool LevelChunk::containsPlayer()
{
    EnterCriticalSection(&m_csEntities);
    for (int section = 0; section < mEntitySectionCount; ++section)
    {
        std::vector<boost::shared_ptr<Entity> >* entities = mEntitySections[section];
        for (unsigned int index = 0; index < entities->size(); ++index)
        {
            boost::shared_ptr<Entity>& entity = entities->at(index);
            if (entity->GetType() == EntityType::SERVER_PLAYER)
            {
                LeaveCriticalSection(&m_csEntities);
                return true;
            }
        }
    }
    LeaveCriticalSection(&m_csEntities);
    return false;
}
bool LevelChunk::isUnloaded()
{
    return mUnloaded;
}
void LevelChunk::markUnsaved()
{
    setUnsaved(true);
}
void LevelChunk::getEntities(boost::shared_ptr<Entity> except, const AABB* box,
                             std::vector<boost::shared_ptr<Entity> >& entities,
                             const Predicate<boost::shared_ptr<Entity> >* predicate)
{
    int first = Mth::floor((box->mMinY - 2.0) / 16.0);
    int last = Mth::floor((box->mMaxY + 2.0) / 16.0);
    first = Mth::clamp(first, 0, mEntitySectionCount - 1);
    last = Mth::clamp(last, 0, mEntitySectionCount - 1);
    for (int section = first; section <= last; ++section)
    {
        std::vector<boost::shared_ptr<Entity> >* sectionEntities = mEntitySections[section];
        if (sectionEntities->empty())
            continue;
        std::vector<boost::shared_ptr<Entity> >::iterator end = sectionEntities->end();
        for (std::vector<boost::shared_ptr<Entity> >::iterator it = sectionEntities->begin(); it != end; it++)
        {
            boost::shared_ptr<Entity>& entity = *it;
            if (entity->mBoundingBox->intersects(box) && entity != except)
            {
                if (predicate == NULL || predicate->apply(entity))
                    entities.push_back(entity);
                std::vector<boost::shared_ptr<Entity> >* subEntities = entity->getSubEntities();
                if (subEntities != NULL)
                {
                    for (unsigned int i = 0; i < subEntities->size(); ++i)
                    {
                        boost::shared_ptr<Entity>& subEntity = subEntities->at(i);
                        if (subEntity != except && subEntity->mBoundingBox->intersects(box)
                            && (predicate == NULL || predicate->apply(subEntity)))
                            entities.push_back(subEntity);
                    }
                }
            }
        }
    }
}
void LevelChunk::getEntitiesOfClass(const std::type_info& type, const AABB* box,
                                    std::vector<boost::shared_ptr<Entity> >& entities,
                                    const Predicate<boost::shared_ptr<Entity> >* predicate,
                                    bool excludeInvisiblePlayers)
{
    int first = Mth::floor((box->mMinY - 2.0) / 16.0);
    int last = Mth::floor((box->mMaxY + 2.0) / 16.0);
    first = Mth::clamp(first, 0, mEntitySectionCount - 1);
    last = Mth::clamp(last, 0, mEntitySectionCount - 1);

    unsigned long long typeMask = 0;
    if (type == typeid(Player))
        typeMask = EntityType::PLAYER;
    else if (type == typeid(Entity))
        typeMask = EntityType::ENTITY;
    else if (type == typeid(Mob))
        typeMask = EntityType::MOB;
    else if (type == typeid(LivingEntity))
        typeMask = EntityType::LIVING_ENTITY;
    else if (type == typeid(ItemEntity))
        typeMask = EntityType::ITEM_ENTITY;
    else if (type == typeid(Minecart))
        typeMask = EntityType::MINECART;
    else if (type == typeid(Monster))
        typeMask = EntityType::MONSTER;
    else if (type == typeid(Zombie))
        typeMask = EntityType::ZOMBIE;
    else if (type == typeid(Animal))
        typeMask = EntityType::ANIMAL;
    else if (type == typeid(Arrow))
        typeMask = EntityType::ARROW;
    else if (type == typeid(AbstractHorse))
        typeMask = EntityType::ABSTRACT_HORSE;
    else if (type == typeid(AbstractSkeleton))
        typeMask = EntityType::ABSTRACT_SKELETON;

    for (int section = first; section <= last; ++section)
    {
        std::vector<boost::shared_ptr<Entity> >* sectionEntities = mEntitySections[section];
        std::vector<boost::shared_ptr<Entity> >::iterator end = sectionEntities->end();
        for (std::vector<boost::shared_ptr<Entity> >::iterator it = sectionEntities->begin(); it != end; it++)
        {
            boost::shared_ptr<Entity> entity = *it;
            bool matches = false;
            if (typeMask != 0)
            {
                if (eTYPE_DERIVED_FROM(typeMask, entity->GetType()))
                    matches = true;
            }
            else if (entity != NULL && type == typeid(*entity.get()))
                matches = true;
            if (matches && entity->mBoundingBox->intersects(box))
            {
                if (predicate == NULL || predicate->apply(entity))
                {
                    if (type == typeid(Player) && excludeInvisiblePlayers)
                    {
                        boost::shared_ptr<Player> player = boost::dynamic_pointer_cast<Player>(entity);
                        if (!player->hasInvisiblePrivilege())
                            entities.push_back(entity);
                    }
                    else
                    {
                        entities.push_back(entity);
                    }
                }
            }
        }
    }
}
int LevelChunk::countEntities()
{
    int count = 0;
    EnterCriticalSection(&m_csEntities);
    for (int section = 0; section < mEntitySectionCount; ++section)
        count += (int)mEntitySections[section]->size();
    LeaveCriticalSection(&m_csEntities);
    return count;
}
bool LevelChunk::shouldSave(bool force)
{
    if (mDontSave)
        return false;
    if (force)
    {
        if ((mLastSaveHadEntities && level->getGameTime() != mLastSaveTime) || mUnsaved)
            return true;
    }
    else if (mLastSaveHadEntities && level->getGameTime() >= mLastSaveTime + 600)
        return true;
    return mUnsaved;
}
int LevelChunk::getBlocksAndData(arrayWithLength<unsigned char>* output, int x, int y, int z, int width, int height,
                                 int depth, int offset, bool includeLight)
{
    int next = offset + mElements->getDataRegion(*output, x, y, z, width, height, depth, offset);
    if (includeLight)
    {
        if (y < 128)
            next += mBlockLight->getDataRegion(*output, x, y, z, width, height < 128 ? height : 128, depth, next);
        if (height > 128)
            next += mBlockLightUpper->getDataRegion(*output, x, y < 128 ? 0 : y - 128, z, width, height - 128, depth,
                                                    next);
        if (y < 128)
            next += mSkyLight->getDataRegion(*output, x, y, z, width, height < 128 ? height : 128, depth, next);
        if (height > 128)
            next += mSkyLightUpper->getDataRegion(*output, x, y < 128 ? 0 : y - 128, z, width, height - 128, depth,
                                                  next);
    }
    return next;
}
void LevelChunk::blockUpdatedCallback(int x, int y, int z, void* data, int value)
{
    LevelChunk* chunk = static_cast<LevelChunk*>(data);
    chunk->level->checkLight(BlockPos(chunk->x * 16 + x, y + value, chunk->z * 16 + z), false, false);
}
int LevelChunk::setBlocksAndData(arrayWithLength<unsigned char> input, int x, int y, int z, int width, int height,
                                 int depth, int offset, bool includeLight)
{
    if (includeLight)
    {
        GameRenderer::AddForDelete(mSkyLight);
        mSkyLight = new SparseLightStorage(true, false);
        GameRenderer::FinishedReassigning();
        GameRenderer::AddForDelete(mBlockLight);
        mBlockLight = new SparseLightStorage(false, false);
        GameRenderer::FinishedReassigning();
    }
    GameRenderer::EnterDeleteThreadCS();
    offset += mElements->setDataRegion(input, x, y, z, width, height, depth, offset,
                                       includeLight ? NULL : blockUpdatedCallback, this, 0);
    recalcHeightmapOnly();
    GameRenderer::LeaveDeleteThreadCS();
    if (includeLight)
    {
        if (y < 128)
            offset += mBlockLight->setDataRegion(input, x, y, z, width, height < 128 ? height : 128, depth, offset);
        if (height > 128)
            offset += mBlockLightUpper->setDataRegion(input, x, y < 128 ? 0 : y - 128, z, width, height - 128, depth,
                                                      offset);
        if (y < 128)
            offset += mSkyLight->setDataRegion(input, x, y, z, width, height < 128 ? height : 128, depth, offset);
        if (height > 128)
            offset += mSkyLightUpper->setDataRegion(input, x, y < 128 ? 0 : y - 128, z, width, height - 128, depth,
                                                    offset);
        std::memcpy(mBiomes.data, input.data + offset, mBiomes.length);
        offset += mBiomes.length;
    }
    for (BlockEntityMap::iterator it = mBlockEntityMap.begin(); it != mBlockEntityMap.end(); ++it)
        it->second->clearCache();
    if (includeLight)
    {
        compressLighting();
        compressBlocks();
        compressData();
    }
    mChunkFlags = TERRAIN_POPULATED_FULL;
    mPostProcess = true;
    return offset;
}
Random* LevelChunk::getRandom(long long seed)
{
    return new Random(level->getSeed() + (long long)(x * x * 0x4c1906) + (long long)(x * 0x5ac0db)
                          + (long long)(z * z * 0x4307a7) + (long long)(z * 0x5f24f)
                      ^ seed);
}
bool LevelChunk::isEmpty()
{
    return false;
}
void LevelChunk::staticCtor()
{
    InitializeCriticalSection(&m_csLighting);
    InitializeCriticalSection(&m_csEntities);
    InitializeCriticalSection(&m_csTileEntities);
}
void LevelChunk::postProcess()
{
    mPostProcess = 1;
}
void LevelChunk::flagPostProcessComplete(short value)
{
    mChunkFlags |= value;
    if ((mChunkFlags
         & (TERRAIN_POPULATED_SELF | TERRAIN_POPULATED_WEST | TERRAIN_POPULATED_NORTH | TERRAIN_POPULATED_NORTH_WEST))
        == (TERRAIN_POPULATED_SELF | TERRAIN_POPULATED_WEST | TERRAIN_POPULATED_NORTH | TERRAIN_POPULATED_NORTH_WEST))
    {
        PIXBeginNamedEvent(0.0f, "Compressing lighting/blocks");
        mElements->removeCompressionHeadroom();
        if (!isLowerBlockLightStorageCompressed())
            compressLighting();
        PIXEndNamedEvent();
    }
    if (mChunkFlags == TERRAIN_POPULATED_ALL_NEIGHBOURS)
    {
        gApp.processSchematicsLighting(this);
        PIXBeginNamedEvent(0.0f, "Recheck gaps");
        if (level->mDimension->getType() != DimensionType::THE_END)
            recheckGaps(true);
        PIXEndNamedEvent();
        PIXBeginNamedEvent(0.0f, "Light lava (this)");
        lightLava();
        PIXEndNamedEvent();
        mChunkFlags |= TERRAIN_POPULATED_FULL;
    }
}
void LevelChunk::doPostProcess(ChunkSource* source, ChunkGenerator* generator)
{
    if ((mChunkFlags & TERRAIN_POPULATED_SELF) == 0)
    {
        PIXBeginNamedEvent(0.0f, "Main post processing");
        postProcess();
        generator->postProcess(x, z);
        PIXEndNamedEvent();
        markUnsaved();
        mChunkFlags |= TERRAIN_POPULATED_SELF;
        if (!level->mDimension->containsChunk(x - 1, z))
            mChunkFlags |= TERRAIN_POPULATED_WEST | TERRAIN_POPULATED_NORTH_WEST | TERRAIN_POPULATED_SOUTH_WEST;
        if (!level->mDimension->containsChunk(x + 1, z))
            mChunkFlags |= TERRAIN_POPULATED_SOUTH_EAST | TERRAIN_POPULATED_EAST | TERRAIN_POPULATED_NORTH_EAST;
        if (!level->mDimension->containsChunk(x, z + 1))
            mChunkFlags |= TERRAIN_POPULATED_NORTH | TERRAIN_POPULATED_NORTH_WEST | TERRAIN_POPULATED_NORTH_EAST;
        if (!level->mDimension->containsChunk(x, z - 1))
            mChunkFlags |= TERRAIN_POPULATED_SOUTH_WEST | TERRAIN_POPULATED_SOUTH | TERRAIN_POPULATED_SOUTH_EAST;
        flagPostProcessComplete(0);
        source->flagPostProcessComplete(TERRAIN_POPULATED_WEST, x + 1, z);
        source->flagPostProcessComplete(TERRAIN_POPULATED_NORTH_WEST, x + 1, z + 1);
        source->flagPostProcessComplete(TERRAIN_POPULATED_NORTH, x, z + 1);
        source->flagPostProcessComplete(TERRAIN_POPULATED_NORTH_EAST, x - 1, z + 1);
        source->flagPostProcessComplete(TERRAIN_POPULATED_EAST, x - 1, z);
        source->flagPostProcessComplete(TERRAIN_POPULATED_SOUTH_EAST, x - 1, z - 1);
        source->flagPostProcessComplete(TERRAIN_POPULATED_SOUTH, x, z - 1);
        source->flagPostProcessComplete(TERRAIN_POPULATED_SOUTH_WEST, x + 1, z - 1);
    }
    else if (generator->postProcessLoadedChunk(this, x, z))
        markUnsaved();
}
void LevelChunk::checkPostProcess(ChunkSource* source, ChunkGenerator* generator)
{
    LevelChunk* north = source->getChunkIfLoaded(x, z - 1);
    LevelChunk* east = source->getChunkIfLoaded(x + 1, z);
    LevelChunk* south = source->getChunkIfLoaded(x, z + 1);
    LevelChunk* west = source->getChunkIfLoaded(x - 1, z);
    if (east != NULL && south != NULL && source->getChunkIfLoaded(x + 1, z + 1) != NULL)
        doPostProcess(source, generator);
    if (west != NULL && south != NULL && source->getChunkIfLoaded(x - 1, z + 1) != NULL)
        west->doPostProcess(source, generator);
    if (north != NULL)
    {
        if (east != NULL && source->getChunkIfLoaded(x + 1, z - 1) != NULL)
            north->doPostProcess(source, generator);
        if (west != NULL)
        {
            LevelChunk* northWest = source->getChunkIfLoaded(x - 1, z - 1);
            if (northWest != NULL)
                northWest->doPostProcess(source, generator);
        }
    }
}
void LevelChunk::checkChests(ChunkSource* source, int chunkX, int chunkZ)
{
    if (mChunkVersion >= 10)
        return;
    LevelChunk* chunk = source->getChunk(chunkX, chunkZ);
    PIXBeginNamedEvent(0.0f, "LevelChunk::checkChests");
    MutableBlockPos gatePos(0, 0, 0);
    MutableBlockPos pos(0, 0, 0);
    for (int localX = 0; localX < 16; ++localX)
    {
        int worldX = chunkX * 16 + localX;
        for (int localZ = 0; localZ < 16; ++localZ)
        {
            int worldZ = chunkZ * 16 + localZ;
            for (int y = 0; y < 128; ++y)
            {
                const BlockState* state = chunk->getBlockState(localX, y, localZ);
                if (state->getBlock() == Blocks::chest && chunk->getData(gatePos) == 0)
                {
                    ChestBlock* chest = static_cast<ChestBlock*>(Blocks::chest);
                    pos.set(worldX, y, worldZ);
                    PIXBeginNamedEvent(0.0f, "Recalc lock dir");
                    chest->recalcLockDir(level, pos, state);
                    PIXEndNamedEvent();
                    PIXBeginNamedEvent(0.0f, "Check light");
                    level->checkLight(pos, true, false);
                    PIXEndNamedEvent();
                }
            }
        }
    }
    PIXEndNamedEvent();
}
void LevelChunk::tick(bool update)
{
    if (mHasGapsToCheck && level->mDimension->isHasSkyLight() && !update)
    {
        PIXBeginNamedEvent(0.0f, "Check for gaps");
        recheckGaps(level->mIsClientSide);
        PIXEndNamedEvent();
    }
    mLightPopulated = true;
    if (!mPostProcess && (mChunkFlags & TERRAIN_POPULATED_FULL) != 0)
    {
        PIXBeginNamedEvent(0.0f, "postProcess");
        postProcess();
        PIXEndNamedEvent();
    }
    PIXBeginNamedEvent(0.0f, "Process block entities");
    while (!mPendingBlockEntities.empty())
    {
        BlockPos pos = mPendingBlockEntities.front();
        mPendingBlockEntities.pop_front();
        if (!getBlockEntity(pos, NO_CREATE) && getBlock(pos)->mBlockEntity)
        {
            boost::shared_ptr<BlockEntity> entity = createBlockEntity(pos);
            level->setBlockEntity(pos, entity);
            level->setBlocksDirty(pos, pos);
        }
    }
    PIXEndNamedEvent();
}
ChunkPos LevelChunk::getPos()
{
    return ChunkPos(x, z);
}
bool LevelChunk::isYSpaceEmpty(int y0, int y1)
{
    return false;
}
void LevelChunk::reloadBiomes()
{
    BiomeSource* source = level->mDimension->mBiomeSource;
    for (int localX = 0; localX < 16; ++localX)
    {
        for (int localZ = 0; localZ < 16; ++localZ)
        {
            Biome* biome = source->getBiome(BlockPos(x * 16 + localX, 0, z * 16 + localZ), Biome::PLAINS);
            mBiomes.data[(localZ << 4) | localX] = (unsigned char)biome->id;
        }
    }
}
Biome* LevelChunk::getBiome(const BlockPos& pos, BiomeSource* source)
{
    int columnIndex = (pos.x & 15) | ((pos.z & 15) << 4);
    int biomeId = mBiomes.data[columnIndex];
    if (!level->mIsClientSide && biomeId == 255)
    {
        biomeId = source->getBiome(pos, Biome::PLAINS)->id;
        mBiomes.data[columnIndex] = (unsigned char)biomeId;
        switch (biomeId)
        {
        case Biome::eBiomeIDs_Ocean:
            mBiomePresent |= BIOME_PRESENT_OCEAN;
            break;
        case Biome::eBiomeIDs_Swampland:
            mBiomePresent |= BIOME_PRESENT_SWAMP;
            break;
        case Biome::eBiomeIDs_DeepOcean:
            mBiomePresent |= BIOME_PRESENT_DEEP_OCEAN;
            break;
        }
    }
    Biome* result = Biome::getBiome(biomeId);
    if (result == NULL)
    {
        int dimensionId = level->mDimension->getType()->getId();
        if (dimensionId == 0)
            return Biome::PLAINS;
        if (dimensionId == 1)
            return Biome::THE_END;
        if (dimensionId == -1)
            return Biome::HELL;
    }
    return result;
}
arrayWithLength<unsigned char> LevelChunk::getBiomes()
{
    return mBiomes;
}
void LevelChunk::updateBiomePresentFlags()
{
    if (mBiomes.length == 0)
        return;
    for (unsigned int i = 0; i < mBiomes.length; ++i)
    {
        unsigned int biome = mBiomes.data[i];
        if (biome == 0)
            mBiomePresent |= BIOME_PRESENT_OCEAN;
        else if (biome == 6)
            mBiomePresent |= BIOME_PRESENT_SWAMP;
        else if (biome == 24)
            mBiomePresent |= BIOME_PRESENT_DEEP_OCEAN;
        else if (biome >= 40 && biome < 48)
            mBiomePresent |= (biome & 1) == 0 ? BIOME_PRESENT_OCEAN : BIOME_PRESENT_DEEP_OCEAN;
    }
}
void LevelChunk::setBiomes(arrayWithLength<unsigned char> biomes)
{
    if (mBiomes.data != NULL)
        delete[] mBiomes.data;
    mBiomes = biomes;
    updateBiomePresentFlags();
}
void LevelChunk::setCheckAllLight()
{
    mCheckAllLight = 1;
}
bool LevelChunk::postProcessAt(int x, int z)
{
    PIXBeginNamedEvent(0.0f, "LevelChunk::postProcessAt %d,%d", x, z);
    MutableBlockPos pos(this->x * 16 + x, 0, this->z * 16 + z);
    int y = getHighestSectionPosition() + 15;
    bool foundLight = false;
    bool belowLimit = false;
    PIXBeginNamedEvent(0.0f, "Checking full height");
    for (;;)
    {
        int seaLevel = level->getSeaLevel();
        if (y <= seaLevel && (y < 1 || belowLimit))
        {
            PIXEndNamedEvent();
            PIXBeginNamedEvent(0.0f, "Checking light emissions");
            for (int checkY = pos.y; checkY > 0; --checkY)
            {
                pos.y = checkY;
                if (getBlock(pos)->defaultBlockState()->getLightEmission() > 0)
                    level->checkLight(pos, false, false);
            }
            PIXEndNamedEvent();
            PIXEndNamedEvent();
            return true;
        }
        PIXBeginNamedEvent(0.0f, "Checking at y %d", y);
        pos.y = y;
        int lightBlock = getBlockLightBlock(pos);
        if (lightBlock == 0xff && y < 0x3f)
            belowLimit = true;
        if (foundLight)
        {
            if (lightBlock == 0)
            {
                PIXBeginNamedEvent(0.0f, "First check light");
                if (!level->checkLight(pos, false, false))
                {
                    PIXEndNamedEvent();
                    PIXEndNamedEvent();
                    PIXEndNamedEvent();
                    PIXEndNamedEvent();
                    return false;
                }
                PIXEndNamedEvent();
            }
            PIXEndNamedEvent();
        }
        else
        {
            if (lightBlock >= 1)
                foundLight = true;
            PIXEndNamedEvent();
        }
        --y;
    }
}
Level* LevelChunk::getLevel()
{
    return level;
}
arrayWithLength<unsigned char> LevelChunk::getHeightmap()
{
    return mHeightmap;
}
void LevelChunk::recalcMinAndMaxHeights()
{
    int minHeight = 255;
    int maxHeight = 0;
    for (unsigned int i = 0; i < mHeightmap.length; ++i)
    {
        int height = mHeightmap.data[i];
        if (height < minHeight)
            minHeight = height;
        if (maxHeight < height)
            maxHeight = height;
    }
    mMinHeight = minHeight;
    mMaxHeight = maxHeight;
}
LevelChunk::BlockEntityMap* LevelChunk::getBlockEntities()
{
    return &mBlockEntityMap;
}
void LevelChunk::setLastSaveHadEntities(bool value)
{
    mLastSaveHadEntities = value;
}
void LevelChunk::setLastSaveTime(long long value)
{
    mLastSaveTime = value;
}
void LevelChunk::setInhabitedTime(long long value)
{
    mInhabitedTime = value;
}
void LevelChunk::compressLighting()
{
    mSkyLight->compress();
    mSkyLightUpper->compress();
    mBlockLight->compress();
    mBlockLightUpper->compress();
}
void LevelChunk::compressBlocks()
{
    if (mElements != NULL)
        mElements->removeCompressionHeadroom();
}
void LevelChunk::compressData()
{
}
void LevelChunk::setChunkVersion(int version)
{
    mChunkVersion = version;
}
void LevelChunk::swapEntities(LevelChunk* chunk)
{
    EnterCriticalSection(&m_csEntities);
    std::vector<boost::shared_ptr<Entity> >** sections = chunk->mEntitySections;
    chunk->mEntitySections = mEntitySections;
    mEntitySections = sections;
    LeaveCriticalSection(&m_csEntities);
}
int LevelChunk::PackBlockPos(const BlockPos& pos)
{
    return (short)(((pos.x & 15) << 12) | ((pos.z & 15) << 8) | (pos.y & 255));
}
BlockPos LevelChunk::UnpackBlockPos(short packed)
{
    unsigned short value = (unsigned short)packed;
    return BlockPos((value >> 12) & 15, value & 255, (value >> 8) & 15);
}
void LevelChunk::CreateNewThreadStorage()
{
    ThreadStorage* storage = new ThreadStorage();
    storage->elementStorage = new unsigned short[0x10000];
    storage->extraElementStorage = new unsigned short[0x10000];
    storage->tempLightingStorage = new unsigned char[0x4080];
    TlsSetValue(sLevelChunkThreadStorage, storage);
}
void LevelChunk::ReleaseThreadStorage()
{
    ThreadStorage* storage = static_cast<ThreadStorage*>(TlsGetValue(sLevelChunkThreadStorage));
    delete[] storage->elementStorage;
    delete[] storage->extraElementStorage;
    delete[] storage->tempLightingStorage;
    delete storage;
}
