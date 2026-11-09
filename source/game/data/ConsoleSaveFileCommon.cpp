#include "game/data/ConsoleSaveFileCommon.hpp"

#include <cstdio>
#include <cstring>

#include "game/core/CConsoleMinecraftApp.hpp"
#include "game/core/CMinecraftApp.hpp"
#include "game/core/Compression.hpp"
#include "game/core/DataInputStream.hpp"
#include "game/core/DataOutputStream.hpp"
#include "game/core/File.hpp"
#include "game/core/RegionFile.hpp"
#include "game/core/RegionFileCache.hpp"
#include "game/core/System.hpp"
#include "platform/wiiu/CStorage.hpp"
#include "game/core/XMem.hpp"
#include "game/core/PIXNamedEvents.hpp"

static const unsigned int SAVE_PAGE_SIZE = 0x1000;

ConsoleSaveFileCommon::ConsoleSaveFileCommon() : mFileHeader()
{
    InitializeCriticalSectionAndSpinCount(&mSaveAccessMutex, 0x1400);
    mFilename = L"";
}

ConsoleSaveFileCommon::~ConsoleSaveFileCommon()
{
    DeleteCriticalSection(&mSaveAccessMutex);
}

void ConsoleSaveFileCommon::_initSaveFromData(const std::wstring& filename, unsigned int dataSize, void* data,
                                              ESavePlatform platform)
{
    mFilename = filename;
    AllocInitialQuota(dataSize);
    if (dataSize == 0)
    {
        mFileHeader.WriteHeader(mData);
        return;
    }

    if (data == NULL)
    {
        unsigned int savedSize;
        gStorage->GetSaveData(mData, &savedSize);
        dataSize = savedSize;
    }
    else
        std::memcpy(mData, data, dataSize);

    void* saveData = data;
    if (saveData == NULL)
        saveData = mData;

    unsigned int* compressedHeader = static_cast<unsigned int*>(saveData);
    if (compressedHeader[0] == 0)
    {
        unsigned int uncompressedSize = compressedHeader[1];
        if (isLocalEndianDifferent(platform))
            System::ReverseULONG(&uncompressedSize);

        if (uncompressedSize == 0)
        {
            std::memset(compressedHeader, 0, dataSize);
            mFileHeader.WriteHeader(compressedHeader);
            mFileHeader.ReadHeader(mData, platform);
            return;
        }

        ReAlloc(uncompressedSize);
        unsigned char* uncompressedData = new unsigned char[uncompressedSize];
        if (platform == eSavePlatform_PSV)
        {
            Compression::VitaVirtualDecompress(uncompressedData, &uncompressedSize, compressedHeader + 2, dataSize - 8);
            std::memcpy(mData, uncompressedData, uncompressedSize);
        }
        else
        {
            Compression::getCompression()->SetDecompressionType(platform);
            int result = Compression::getCompression()->Decompress(uncompressedData, &uncompressedSize,
                                                                   compressedHeader + 2, dataSize - 8);
            Compression::getCompression()->SetDecompressionType(eSavePlatform_WIIU);
            if (result == 0)
                std::memcpy(mData, uncompressedData, uncompressedSize);
            else
            {
                std::memset(mData, 0, dataSize);
                mFileHeader.WriteHeader(mData);
            }
        }
        delete[] uncompressedData;
    }

    mFileHeader.ReadHeader(mData, platform);
}

void ConsoleSaveFileCommon::_deleteFile(FileEntry* entry)
{
    if (entry == NULL)
        return;
    LockSaveAccess();
    unsigned char buffer[SAVE_PAGE_SIZE];
    unsigned char* destination = static_cast<unsigned char*>(mData) + entry->m_data.m_startOffset;
    unsigned char* source = destination + entry->m_data.m_fileSize;
    unsigned char* end = static_cast<unsigned char*>(mData) + mFileHeader.GetStartOfNextData();
    for (;;)
    {
        unsigned int count = SAVE_PAGE_SIZE;
        if (end < source + SAVE_PAGE_SIZE)
        {
            count = static_cast<unsigned int>(end - source);
            if (count == 0)
                break;
        }
        std::memcpy(buffer, source, count);
        source += count;
        std::memcpy(destination, buffer, count);
        destination += count;
    }
    mFileHeader.RemoveFile(entry);
    finalizeWrite();
    ReleaseSaveAccess();
}

void ConsoleSaveFileCommon::MoveDataBeyond(FileEntry* entry, unsigned int size)
{
    ReAlloc(mFileHeader.GetFileSize() + size);
    unsigned char* dataStart = static_cast<unsigned char*>(mData) + entry->m_data.m_startOffset
                               + entry->m_data.m_fileSize;
    unsigned char* source = static_cast<unsigned char*>(mData) + mFileHeader.GetStartOfNextData();
    unsigned char* destination = source + size;
    unsigned char* destinationEnd = destination;

    if ((size & 0xfff) == 0)
    {
        if (size != 0)
        {
            uintptr_t lowPage = reinterpret_cast<uintptr_t>(dataStart) & ~static_cast<uintptr_t>(0xfff);
            uintptr_t highPage = reinterpret_cast<uintptr_t>(source - 1) & ~static_cast<uintptr_t>(0xfff);
            uintptr_t page;
            for (page = highPage; page >= lowPage; page -= SAVE_PAGE_SIZE)
            {
                unsigned char* from = reinterpret_cast<unsigned char*>(page);
                if (from < dataStart)
                    from = dataStart;
                unsigned char* to = reinterpret_cast<unsigned char*>(page + SAVE_PAGE_SIZE);
                if (source < to)
                    to = source;
                XMemCpy(from + size, from, static_cast<unsigned int>(to - from));
            }
        }
    }
    else
    {
        static unsigned char moveBuffer[SAVE_PAGE_SIZE];
        static unsigned char previousBuffer[SAVE_PAGE_SIZE];
        unsigned int previousCount = 0;
        unsigned int count;
        do
        {
            std::memcpy(previousBuffer, moveBuffer, previousCount);
            count = SAVE_PAGE_SIZE;
            if (source - SAVE_PAGE_SIZE < dataStart)
                count = static_cast<unsigned int>(source - dataStart);
            source -= count;
            std::memcpy(moveBuffer, source, count);
            destination -= previousCount;
            if (destination + previousCount <= destinationEnd)
                std::memcpy(destination, previousBuffer, previousCount);
            previousCount = count;
        } while (count != 0);
    }
    mFileHeader.AdjustStartOffsets(entry, size, false);
}

void ConsoleSaveFileCommon::PrepareForWrite(FileEntry* entry, unsigned int size)
{
    int delta = static_cast<int>(entry->m_filePointer - entry->m_data.m_startOffset + size - entry->m_data.m_fileSize);
    if (delta > 0)
    {
        PIXBeginNamedEvent(0.0f, "Growing file by %d bytes", delta);
        MoveDataBeyond(entry, static_cast<unsigned int>(delta));
        PIXEndNamedEvent();
        entry->m_data.m_fileSize += delta;
        finalizeWrite();
    }
}

bool ConsoleSaveFileCommon::_writeFile(FileEntry* entry, const void* buffer, unsigned int size, unsigned int* written)
{
    if (mData == NULL)
        return false;
    LockSaveAccess();
    PrepareForWrite(entry, size);
    std::memcpy(static_cast<unsigned char*>(mData) + entry->m_filePointer, buffer, size);
    *written = size;
    entry->m_filePointer += size;
    entry->m_data.m_timestamp = System::currentRealTimeMillis();
    ReleaseSaveAccess();
    return true;
}

bool ConsoleSaveFileCommon::_zeroFile(FileEntry* entry, unsigned int size, unsigned int* written)
{
    if (mData == NULL)
        return false;
    LockSaveAccess();
    PrepareForWrite(entry, size);
    std::memset(static_cast<unsigned char*>(mData) + entry->m_filePointer, 0, size);
    *written = size;
    entry->m_filePointer += size;
    entry->m_data.m_timestamp = System::currentRealTimeMillis();
    ReleaseSaveAccess();
    return true;
}

bool ConsoleSaveFileCommon::_readFile(FileEntry* entry, void* buffer, unsigned int size, unsigned int* read)
{
    if (mData == NULL)
        return false;
    LockSaveAccess();
    unsigned int filePointer = entry->m_filePointer;
    unsigned int end = entry->m_data.m_startOffset + entry->m_data.m_fileSize;
    if (end < filePointer + size)
        size = end - filePointer;
    std::memcpy(buffer, static_cast<unsigned char*>(mData) + filePointer, size);
    *read = size;
    entry->m_filePointer += size;
    ReleaseSaveAccess();
    return true;
}

bool ConsoleSaveFileCommon::closeHandle(FileEntry*)
{
    LockSaveAccess();
    finalizeWrite();
    ReleaseSaveAccess();
    return true;
}

void ConsoleSaveFileCommon::finalizeWrite()
{
    LockSaveAccess();
    mFileHeader.WriteHeader(mData);
    ReleaseSaveAccess();
}

bool ConsoleSaveFileCommon::doesFileExist(ConsoleSavePath path)
{
    LockSaveAccess();
    bool result = mFileHeader.fileExists(path.getPath());
    ReleaseSaveAccess();
    return result;
}

unsigned int ConsoleSaveFileCommon::getSizeOnDisk()
{
    return mFileHeader.GetFileSize();
}
std::wstring ConsoleSaveFileCommon::getFilename()
{
    return mFilename;
}
std::vector<FileEntry*>* ConsoleSaveFileCommon::getFilesWithPrefix(const std::wstring& prefix)
{
    return mFileHeader.getFilesWithPrefix(prefix);
}
short ConsoleSaveFileCommon::getSaveVersion()
{
    return mFileHeader.m_saveVersion;
}
short ConsoleSaveFileCommon::getOriginalSaveVersion()
{
    return mFileHeader.m_originalSaveVersion;
}
void ConsoleSaveFileCommon::LockSaveAccess()
{
    EnterCriticalSection(&mSaveAccessMutex);
}
void ConsoleSaveFileCommon::ReleaseSaveAccess()
{
    LeaveCriticalSection(&mSaveAccessMutex);
}
ESavePlatform ConsoleSaveFileCommon::getSavePlatform()
{
    return mFileHeader.m_savePlatform;
}
bool ConsoleSaveFileCommon::isSaveEndianDifferent()
{
    return mFileHeader.m_saveEndian != kLocalByteOrder;
}
void ConsoleSaveFileCommon::setLocalPlatform()
{
    mFileHeader.m_saveEndian = kLocalByteOrder;
    mFileHeader.m_savePlatform = kLocalSavePlatform;
}
void ConsoleSaveFileCommon::setPlatform(ESavePlatform platform)
{
    mFileHeader.m_savePlatform = platform;
}
ByteOrder ConsoleSaveFileCommon::getSaveEndian()
{
    return mFileHeader.m_saveEndian;
}
ByteOrder ConsoleSaveFileCommon::getLocalEndian()
{
    return kLocalByteOrder;
}
void ConsoleSaveFileCommon::setEndian(ByteOrder endian)
{
    mFileHeader.m_saveEndian = endian;
}
bool ConsoleSaveFileCommon::isLocalEndianDifferent(ESavePlatform platform)
{
    return getLocalEndian() != FileHeader::getEndian(platform);
}

void ConsoleSaveFileCommon::ConvertRegionFile(File regionFile)
{
    RegionFile region(this, &regionFile);
    bool compressed = true;
    int x;
    for (x = 0; x < 32; ++x)
    {
        int z;
        for (z = 0; z < 32; ++z)
        {
            DataInputStream* input = region.getChunkDataInputStream(x, z, &compressed);
            if (input != NULL)
            {
                arrayWithLength<unsigned char> data(0x100000, true);
                int length = input->read(data);
                input->close();
                input->deleteChildStream();
                delete input;
                DataOutputStream* output = region.getChunkDataOutputStream(x, z, compressed);
                output->write(data, 0, length);
                output->close();
                output->deleteChildStream();
                delete output;
                delete data.data;
            }
        }
    }
    region.writeAllOffsets();
}

#ifdef MINECRAFT_PC
ESavePlatform ConsoleSaveFileCommon::PcBaseSavePlatform(const void* data, unsigned int size)
{
    const unsigned char* bytes = static_cast<const unsigned char*>(data);
    if (size < 8 || bytes[0] != 0 || bytes[1] != 0 || bytes[2] != 0 || bytes[3] != 0)
        return eSavePlatform_WIIU;
    unsigned int bigEndian = ((unsigned int)bytes[4] << 24) | ((unsigned int)bytes[5] << 16)
                             | ((unsigned int)bytes[6] << 8) | bytes[7];
    unsigned int littleEndian = ((unsigned int)bytes[7] << 24) | ((unsigned int)bytes[6] << 16)
                                | ((unsigned int)bytes[5] << 8) | bytes[4];
    if (bigEndian > 0x4000000 && littleEndian != 0 && littleEndian <= 0x4000000)
    {
        printf("[pc] save: base save is little endian; read as a PlayStation 4 save\n");
        return eSavePlatform_PS4;
    }
    return eSavePlatform_WIIU;
}

#endif
void ConsoleSaveFileCommon::ConvertToLocalPlatform()
{
    if (getSavePlatform() == kLocalSavePlatform)
        return;
    std::vector<FileEntry*>* files = getFilesWithPrefix(L"");
    std::vector<FileEntry*>::iterator it;
    for (it = files->begin(); it < files->end(); ++it)
    {
        FileEntry* entry = *it;
        std::wstring name = entry->m_fileName;
        std::wstring extension = L".mcr";
        if (name.compare(name.size() - extension.size(), extension.size(), extension) == 0)
            ConvertRegionFile(File(entry->m_fileName));
    }
    setLocalPlatform();
}

unsigned int ConsoleSaveFileCommon::fjGetSaveThumbnailData(unsigned char* data, unsigned int size)
{
    std::memset(data, 0, size);
    CMinecraftApp::PngTagData imageData;
    gApp.CurrentImageTextData(&imageData);
    if ((imageData.flags & CMinecraftApp::PngTagData::eFlag_NumLoads) == 0)
    {
        imageData.numLoads = 0;
        imageData.flags |= CMinecraftApp::PngTagData::eFlag_NumLoads;
    }
    imageData.flags |= CMinecraftApp::PngTagData::eFlag_ExtraData;
    gApp.SetExtraGameData(imageData.extraData, eGameExtraData_SaveBuild, EXTRA_DATA_SAVE_BUILD);
    gApp.SetExtraGameData(imageData.extraData, eGameExtraData_SaveVersion, EXTRA_DATA_SAVE_VERSION);

    unsigned int imageDataSize = gApp.CreateImageTextData(data, &imageData);

    CMinecraftApp::PngTagData parsedData;
    gApp.ParseImageTextData(data, imageDataSize, &parsedData);
    return imageDataSize;
}

int ConsoleSaveFileCommon::fjCountSavedChunks(int dimension)
{
    const std::wstring dimensions[] = {L"", L"DIM-1", L"DIM1/"};
    const std::wstring& folder = dimensions[dimension];
    int savedChunks = 0;
    int regionX;
    for (regionX = -4; regionX < 4; ++regionX)
    {
        int regionZ;
        for (regionZ = -128; regionZ < 128; regionZ += 32)
        {
            File regionPath = RegionFileCache::fjGetFilepath(this, folder, regionX << 5, regionZ);
            if (fjHasRegionFile(regionPath))
            {
                RegionFile* regionFile = gRegionFileCache._getRegionFile(this, folder, regionX << 5, regionZ);
                if (regionFile != NULL)
                    savedChunks += regionFile->fjCountSavedChunks();
            }
        }
    }
    return savedChunks;
}
bool ConsoleSaveFileCommon::fjHasRegionFile(const File& file)
{
    return mFileHeader.fileExists(file.getPath());
}
