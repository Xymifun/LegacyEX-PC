#include "game/core/RegionFile.hpp"

#include <algorithm>
#include <cstring>

#include "game/core/ByteArrayInputStream.hpp"
#include "game/core/Compression.hpp"
#include "game/core/MemSect.hpp"
#include "game/core/PIXNamedEvents.hpp"
#include "game/core/System.hpp"
#include "game/data/ConsoleSavePath.hpp"
#include "platform/wiiu/TLSStorageCafe.hpp"

static const int SECTOR_BYTES = 0x1000;
static const int SECTOR_INTS = SECTOR_BYTES / 4;
static const int CHUNK_HEADER_SIZE = 8;
static const int MAX_SECTORS_PER_CHUNK = 0x100;
static const unsigned int CHUNK_RLE_FLAG = 0x80000000U;
static const unsigned int CHUNK_COMPRESSED_FLAG = 0x40000000U;
static const unsigned int CHUNK_LENGTH_MASK = 0x3fffffffU;
static const unsigned int FILE_BEGIN = 0;
static const unsigned int FILE_END = 2;

static arrayWithLength<unsigned char> emptySector(SECTOR_BYTES, true);
static unsigned int sThreadStorageIndex = TlsAlloc();

RegionFile::RegionFile(ConsoleSaveFile* saveFile, File* file) : mSaveFile(saveFile), mLastModified(0)
{
    mOffsets = new unsigned int[SECTOR_INTS];
    std::memset(mOffsets, 0, SECTOR_BYTES);
    mTimestamps = new unsigned int[SECTOR_INTS];
    std::memset(mTimestamps, 0, SECTOR_BYTES);
    mFile = file;
    mSizeDelta = 0;

    mFileEntry = mSaveFile->createFile(ConsoleSavePath(file->getPath()));
    mSaveFile->setFilePointer(mFileEntry, 0, NULL, FILE_END);

    if (mFileEntry->m_data.m_fileSize < SECTOR_BYTES)
    {
        mSizeDelta += SECTOR_BYTES * 2;
        mNewFile = true;
    }
    else
    {
        mNewFile = false;
    }

    if ((mFileEntry->m_data.m_fileSize & (SECTOR_BYTES - 1)) != 0)
    {
        unsigned int padding = SECTOR_BYTES - (mFileEntry->m_data.m_fileSize & (SECTOR_BYTES - 1));
        unsigned int bytesWritten = 0;
        unsigned char* zeroes = new unsigned char[padding];
        std::memset(zeroes, 0, padding);
        mSaveFile->writeFile(mFileEntry, zeroes, padding, &bytesWritten);
        delete[] zeroes;
    }

    int sectorCount;
    if (mNewFile)
    {
        sectorCount = 2;
    }
    else
    {
        sectorCount = static_cast<int>(mFileEntry->m_data.m_fileSize) / SECTOR_BYTES;
    }

    mUsedSectors = new std::vector<bool>();
    mUsedSectors->reserve(sectorCount);
    for (int i = 0; i < sectorCount; ++i)
    {
        mUsedSectors->push_back(true);
    }
    mUsedSectors->at(0) = false;
    mUsedSectors->at(1) = false;

    mSaveFile->setFilePointer(mFileEntry, 0, NULL, FILE_BEGIN);

    for (int i = 0; i < SECTOR_INTS; ++i)
    {
        unsigned int offset = 0;
        unsigned int bytesRead = 0;
        if (!mNewFile)
        {
            mSaveFile->readFile(mFileEntry, &offset, 4, &bytesRead);
            if (saveFile->isSaveEndianDifferent())
            {
                System::ReverseULONG(&offset);
            }
        }
        mOffsets[i] = offset;
        if (offset != 0 && (offset >> 8) + (offset & 0xff) <= mUsedSectors->size())
        {
            for (unsigned int sector = 0; sector < (offset & 0xff); ++sector)
            {
                mUsedSectors->at((offset >> 8) + sector) = false;
            }
        }
    }

    for (int i = 0; i < SECTOR_INTS; ++i)
    {
        int timestamp = 0;
        unsigned int bytesRead = 0;
        if (!mNewFile)
        {
            mSaveFile->readFile(mFileEntry, &timestamp, 4, &bytesRead);
            if (saveFile->isSaveEndianDifferent())
            {
                System::ReverseINT(&timestamp);
            }
        }
        mTimestamps[i] = timestamp;
    }
}

RegionFile::~RegionFile()
{
    delete[] mOffsets;
    delete[] mTimestamps;
    delete mUsedSectors;
    mSaveFile->closeHandle(mFileEntry);
}

unsigned int RegionFile::getOffset(int x, int z)
{
    return mOffsets[x + z * 32];
}

bool RegionFile::outOfBounds(int x, int z)
{
    return x < 0 || x >= 32 || z < 0 || z >= 32;
}

DataInputStream* RegionFile::getChunkDataInputStream(int x, int z, bool* compressed)
{
    if (outOfBounds(x, z))
    {
        return NULL;
    }

    int offset = getOffset(x, z);
    if (offset == 0)
    {
        return NULL;
    }

    int sectorNumber = offset >> 8;
    int numSectors = offset & 0xff;
    if (static_cast<unsigned int>(sectorNumber + numSectors) > mUsedSectors->size())
    {
        return NULL;
    }

    mSaveFile->LockSaveAccess();
    mSaveFile->setFilePointer(mFileEntry, sectorNumber * SECTOR_BYTES, NULL, FILE_BEGIN);

    unsigned int length;
    unsigned int decompressedLength;
    unsigned int bytesRead = 0;
    mSaveFile->readFile(mFileEntry, &length, 4, &bytesRead);
    if (mSaveFile->isSaveEndianDifferent())
    {
        System::ReverseULONG(&length);
    }

    bool rle = (length & CHUNK_RLE_FLAG) != 0;
    if (compressed != NULL)
    {
        *compressed = (length & CHUNK_COMPRESSED_FLAG) != 0;
    }
    length &= CHUNK_LENGTH_MASK;

    mSaveFile->readFile(mFileEntry, &decompressedLength, 4, &bytesRead);
    if (mSaveFile->isSaveEndianDifferent())
    {
        System::ReverseULONG(&decompressedLength);
    }

    if (length > static_cast<unsigned int>(numSectors * SECTOR_BYTES))
    {
        mSaveFile->ReleaseSaveAccess();
        return NULL;
    }

    MemSect(50);
    unsigned char* data = new unsigned char[length];
    unsigned char* decompressed = new unsigned char[decompressedLength];
    MemSect(0);

    unsigned int decompressedSize = decompressedLength;
    arrayWithLength<unsigned char> chunkData;
    chunkData.data = decompressed;
    mSaveFile->readFile(mFileEntry, data, length, &bytesRead);
    mSaveFile->ReleaseSaveAccess();

    Compression::getCompression()->SetDecompressionType(mSaveFile->getSavePlatform());
    if (rle)
    {
        Compression::getCompression()->DecompressLZXRLE(decompressed, &decompressedSize, data, length);
    }
    else
    {
        Compression::getCompression()->Decompress(decompressed, &decompressedSize, data, length);
    }
    Compression::getCompression()->SetDecompressionType(eSavePlatform_WIIU);
    delete[] data;

    chunkData.length = decompressedSize;
    return new DataInputStream(new ByteArrayInputStream(chunkData));
}

DataOutputStream* RegionFile::getChunkDataOutputStream(int x, int z, bool compressed)
{
    return new DataOutputStream(new ChunkBuffer(x, this, z, compressed));
}

void* RegionFile::GetThreadCompressedDataStorage()
{
    return TlsGetValue(sThreadStorageIndex);
}

void RegionFile::write(int sector, unsigned char* data, int length, unsigned int compressedLength, bool rle,
                       bool compressed, bool deleteData)
{
    unsigned int header = compressedLength;
    unsigned int bytesWritten = 0;
    unsigned int decompressedLength = length;

    mSaveFile->setFilePointer(mFileEntry, sector * SECTOR_BYTES, NULL, FILE_BEGIN);

    if (rle)
    {
        header |= CHUNK_RLE_FLAG;
    }
    else
    {
        header &= ~CHUNK_RLE_FLAG;
    }
    if (compressed)
    {
        header |= CHUNK_COMPRESSED_FLAG;
    }

    mSaveFile->writeFile(mFileEntry, &header, 4, &bytesWritten);
    header &= CHUNK_LENGTH_MASK;
    mSaveFile->writeFile(mFileEntry, &decompressedLength, 4, &bytesWritten);
    mSaveFile->writeFile(mFileEntry, data, header, &bytesWritten);

    if (deleteData)
    {
        delete[] data;
    }
}

void RegionFile::zero(int sector, int length)
{
    unsigned int bytesWritten = 0;
    mSaveFile->setFilePointer(mFileEntry, sector * SECTOR_BYTES, NULL, FILE_BEGIN);
    mSaveFile->zeroFile(mFileEntry, length, &bytesWritten);
}

void RegionFile::insertInitialSectors()
{
    mSaveFile->setFilePointer(mFileEntry, 0, NULL, FILE_BEGIN);

    unsigned int bytesWritten = 0;
    unsigned char zeroes[SECTOR_BYTES];
    std::memset(zeroes, 0, SECTOR_BYTES);
    mSaveFile->writeFile(mFileEntry, zeroes, SECTOR_BYTES, &bytesWritten);
    mSaveFile->writeFile(mFileEntry, zeroes, SECTOR_BYTES, &bytesWritten);

    mNewFile = false;
}

void RegionFile::setOffset(int x, int z, int offset)
{
    if (mNewFile)
    {
        insertInitialSectors();
    }

    mOffsets[x + z * 32] = offset;
    unsigned int bytesWritten = 0;
    mSaveFile->setFilePointer(mFileEntry, (x + z * 32) * 4, NULL, FILE_BEGIN);
    mSaveFile->writeFile(mFileEntry, &offset, 4, &bytesWritten);
}

void RegionFile::setTimestamp(int x, int z, int timestamp)
{
    if (mNewFile)
    {
        insertInitialSectors();
    }

    mTimestamps[x + z * 32] = timestamp;
    unsigned int bytesWritten = 0;
    mSaveFile->setFilePointer(mFileEntry, (x + z * 32) * 4 + SECTOR_BYTES, NULL, FILE_BEGIN);
    mSaveFile->writeFile(mFileEntry, &timestamp, 4, &bytesWritten);
}

void RegionFile::write(int x, int z, unsigned char* data, int length, bool compressed)
{
    unsigned char* buffer = NULL;
    unsigned int compressedLength = length + 0x800;
    bool ownsBuffer = false;

    if (compressedLength < THREAD_STORAGE_SIZE)
    {
        buffer = static_cast<unsigned char*>(GetThreadCompressedDataStorage());
    }
    if (buffer == NULL)
    {
        ownsBuffer = true;
        buffer = new unsigned char[compressedLength];
    }

    bool rle = true;
    if (Compression::getCompression()->CompressLZXRLE(buffer, &compressedLength, data, length, NULL) != 0)
    {
        Compression::getCompression()->Compress(buffer, &compressedLength, data, length);
        rle = false;
    }

    int sectorsNeeded = ((compressedLength + CHUNK_HEADER_SIZE) / SECTOR_BYTES) + 1;
    if (sectorsNeeded >= MAX_SECTORS_PER_CHUNK)
    {
        return;
    }

    mSaveFile->LockSaveAccess();

    int offset = getOffset(x, z);
    int sectorNumber = offset >> 8;
    int sectorsAllocated = offset & 0xff;

    if (sectorNumber == 0 || sectorsAllocated != sectorsNeeded)
    {
        for (int i = 0; i < sectorsAllocated; ++i)
        {
            mUsedSectors->at(sectorNumber + i) = true;
        }
        zero(sectorNumber, sectorsAllocated * SECTOR_BYTES);

        PIXBeginNamedEvent(0.0f, "Scanning for free space\n");
        unsigned int runStart = std::find(mUsedSectors->begin(), mUsedSectors->end(), true) - mUsedSectors->begin();
        int runLength = 0;
        if (runStart != static_cast<unsigned int>(-1))
        {
            for (unsigned int i = runStart; i < mUsedSectors->size(); ++i)
            {
                if (runLength != 0)
                {
                    if (mUsedSectors->at(i))
                    {
                        ++runLength;
                    }
                    else
                    {
                        runLength = 0;
                    }
                }
                else if (mUsedSectors->at(i))
                {
                    runStart = i;
                    runLength = 1;
                }
                if (runLength >= sectorsNeeded)
                {
                    break;
                }
            }
        }
        PIXEndNamedEvent();

        if (runLength >= sectorsNeeded)
        {
            sectorNumber = runStart;
            setOffset(x, z, (sectorNumber << 8) | sectorsNeeded);
            for (int i = 0; i < sectorsNeeded; ++i)
            {
                mUsedSectors->at(sectorNumber + i) = false;
            }
        }
        else
        {
            PIXBeginNamedEvent(0.0f, "Expanding storage for %d sectors\n", sectorsNeeded);
            mSaveFile->setFilePointer(mFileEntry, 0, NULL, FILE_END);
            sectorNumber = mUsedSectors->size();
            unsigned int bytesWritten = 0;
            for (int i = 0; i < sectorsNeeded; ++i)
            {
                mSaveFile->writeFile(mFileEntry, emptySector.data, SECTOR_BYTES, &bytesWritten);
                mUsedSectors->push_back(false);
            }
            mSizeDelta += sectorsNeeded * SECTOR_BYTES;

            write(sectorNumber, buffer, length, compressedLength, rle, compressed, ownsBuffer);
            setOffset(x, z, (sectorNumber << 8) | sectorsNeeded);
            PIXEndNamedEvent();
            setTimestamp(x, z, static_cast<int>(System::processTimeInSeconds()));
            mSaveFile->ReleaseSaveAccess();
            return;
        }
    }

    write(sectorNumber, buffer, length, compressedLength, rle, compressed, ownsBuffer);
    setTimestamp(x, z, static_cast<int>(System::processTimeInSeconds()));
    mSaveFile->ReleaseSaveAccess();
}

int RegionFile::fjCountSavedChunks() const
{
    int count = 0;
    for (int i = 0; i < SECTOR_INTS; ++i)
    {
        if (mOffsets[i] != 0)
        {
            ++count;
        }
    }
    return count;
}

void RegionFile::writeAllOffsets()
{
    if (mNewFile)
    {
        return;
    }

    mSaveFile->LockSaveAccess();

    unsigned int bytesWritten = 0;
    mSaveFile->setFilePointer(mFileEntry, 0, NULL, FILE_BEGIN);
    mSaveFile->writeFile(mFileEntry, mOffsets, SECTOR_BYTES, &bytesWritten);

    bytesWritten = 0;
    mSaveFile->setFilePointer(mFileEntry, SECTOR_BYTES, NULL, FILE_BEGIN);
    mSaveFile->writeFile(mFileEntry, mTimestamps, SECTOR_BYTES, &bytesWritten);

    mSaveFile->ReleaseSaveAccess();
}

void RegionFile::close()
{
    mSaveFile->closeHandle(mFileEntry);
}

void RegionFile::CreateNewThreadStorage()
{
    TlsSetValue(sThreadStorageIndex, new unsigned char[THREAD_STORAGE_SIZE]);
}

long long RegionFile::lastModified()
{
    return mLastModified;
}

int RegionFile::getSizeDelta()
{
    int sizeDelta = mSizeDelta;
    mSizeDelta = 0;
    return sizeDelta;
}

bool RegionFile::hasChunk(int x, int z)
{
    return getOffset(x, z) != 0;
}

void RegionFile::ReleaseThreadStorage()
{
    unsigned char* storage = static_cast<unsigned char*>(TlsGetValue(sThreadStorageIndex));
    if (storage != NULL)
    {
        delete[] storage;
        TlsSetValue(sThreadStorageIndex, NULL);
    }
}

void RegionFile::ChunkBuffer::close()
{
    mRegion->write(mX, mZ, mBuffer.data, mCount, mCompressed);
}
