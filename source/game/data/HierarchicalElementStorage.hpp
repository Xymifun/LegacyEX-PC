#ifndef GAME_DATA_HIERARCHICALELEMENTSTORAGE_HPP
#define GAME_DATA_HIERARCHICALELEMENTSTORAGE_HPP

#include "game/world/biome/arrayWithLength.hpp"

class CompressedBlockStorage;
class DataInputStream;
class DataOutputStream;
class SparseDataStorage;

class HierarchicalElementStorage
{
public:
    enum
    {
        SUBCHUNK_COUNT = 16,
        SUBCHUNK_PAGE_BYTES = 256,
        SUBCHUNK_HEADER_BYTES = 128,
        SECTIONS_PER_SUBCHUNK = 64,
        ELEMENTS_PER_SECTION = 64,
        SECTION_PLANE_BYTES = 8,
        SECTION_TYPE_UNIFORM = 0,
        SECTION_TYPE_MAX_PLANE_COUNT = 4,
        SECTION_TYPE_RAW = 7,
        MAX_PALETTE_ENTRIES = 16,
        UNUSED_PALETTE_ENTRY = 0xFFFF,
        SUBCHUNK_DATA_MAX_BYTES = 0x41000
    };

    class SubChunkInfo
    {
    public:
        void setSizeAndOffset(unsigned int subChunk, unsigned short size, unsigned short offset);

        unsigned short getOffset(unsigned int subChunk);
        unsigned char getSize(unsigned int subChunk);

        void incSize(unsigned int subChunk, unsigned short amount);
        void decSize(unsigned int subChunk, unsigned short amount);
        void incOffset(unsigned int subChunk, unsigned short amount);
        void decOffset(unsigned int subChunk, unsigned short amount);

        int getUsedSize();

        static unsigned int getRawDataBytes();
        void getRawData(unsigned char* rawData);
        void setFromRawData(unsigned char* rawData);

        void write(DataOutputStream* out);
        void read(DataInputStream* in);

    private:
        unsigned short mOffsets[SUBCHUNK_COUNT];
        unsigned char mSizes[SUBCHUNK_COUNT];
    };

    class SubChunkHeader
    {
    public:
        void setData(unsigned int section, unsigned short value);
        unsigned short getData(unsigned int section);

        void setTypeAndOffset(unsigned int section, unsigned short sectionType, unsigned short offset,
                              bool hasTwoIndices);
        void setType(unsigned int section, unsigned short sectionType);
        void setOffset(unsigned int section, unsigned short offset);

        unsigned short getType(unsigned int section);
        bool getHasTwoIndices(unsigned int section);
        unsigned short getOffset(unsigned int section);

        void incOffset(unsigned int section, unsigned short amount);
        void decOffset(unsigned int section, unsigned short amount);

        int getUsedSize();

    private:
        unsigned short mSections[SECTIONS_PER_SUBCHUNK];
    };

    class PalettisedSectionReference
    {
    public:
        PalettisedSectionReference(SubChunkHeader* header, unsigned int section);

        unsigned int getPlaneCount();
        void setPlaneCount(unsigned int planeCount);

        unsigned int getPaletteSize();
        unsigned short getPaletteEntry(unsigned int paletteIndex);
        void setPaletteEntry(unsigned int paletteIndex, unsigned short value);

        unsigned char* getPlaneData();
        unsigned char* getExtraPlaneData();
        unsigned int getPlaneDataSize();

    private:
        unsigned int mPlaneCount;
        bool mHasTwoIndices;
        unsigned short* mPalette;
        unsigned char* mPlaneData;
    };

    class RawSectionReference
    {
    public:
        RawSectionReference(SubChunkHeader* header, unsigned int section);

        unsigned short getElement(unsigned int elementIndex);
        void setElement(unsigned int elementIndex, unsigned short value);

        unsigned short getExtraElement(unsigned int elementIndex);
        void setExtraElement(unsigned int elementIndex, unsigned short value);

    private:
        unsigned short* mElements;
        bool mHasTwoIndices;
    };

    class Palettiser
    {
    public:
        enum SourceType
        {
            SOURCE_BLOCK_ARRAYS = 0,
            SOURCE_BLOCK_ARRAYS_HALF_HEIGHT = 1,
            SOURCE_ELEMENT_ARRAY = 2,
            SOURCE_ELEMENT_ARRAY_HALF_HEIGHT = 3,
            SOURCE_BLOCK_STORAGE = 4
        };

        Palettiser(arrayWithLength<unsigned char> blocks, arrayWithLength<unsigned char> data);
        Palettiser(arrayWithLength<unsigned short> elements);
        Palettiser(CompressedBlockStorage* blocks, CompressedBlockStorage* extraBlocks, SparseDataStorage* data,
                   SparseDataStorage* extraData);

        void initialiseSection(unsigned int sectionBaseIndex);
        void createHeadroom(unsigned int extraEntries);
        bool didPalettise();

        unsigned short getElement(unsigned int elementIndex);

        bool writeSection(SubChunkHeader* header, unsigned int section, unsigned short& offset);

    private:
        unsigned short mPalette[MAX_PALETTE_ENTRIES];
        unsigned char mIndices[ELEMENTS_PER_SECTION];
        unsigned int mPaletteCount;
        SourceType mSourceType;
        arrayWithLength<unsigned char> mBlocks;
        arrayWithLength<unsigned char> mData;
        arrayWithLength<unsigned short> mElements;
        CompressedBlockStorage* mBlockStorage;
        CompressedBlockStorage* mExtraBlockStorage;
        SparseDataStorage* mDataStorage;
        SparseDataStorage* mExtraDataStorage;
        unsigned char mSectionBlocks[ELEMENTS_PER_SECTION];
    };

    HierarchicalElementStorage();
    HierarchicalElementStorage(HierarchicalElementStorage* other);
    ~HierarchicalElementStorage();

    void _Init(arrayWithLength<unsigned char> blocks, arrayWithLength<unsigned char> data, unsigned int headroom);
    void _Init(arrayWithLength<unsigned short> elements, unsigned int headroom);
    void _Init(CompressedBlockStorage* blocks, CompressedBlockStorage* extraBlocks, SparseDataStorage* data,
               SparseDataStorage* extraData);

    void setData(arrayWithLength<unsigned char> blocks, arrayWithLength<unsigned char> data);
    void setData(arrayWithLength<unsigned short> elements);

    void getData(arrayWithLength<unsigned char> blocks, arrayWithLength<unsigned char> data, int minSlice,
                 int maxSlice);
    void getData(arrayWithLength<unsigned short> elements, int minSlice, int maxSlice);
    void getData(arrayWithLength<unsigned short> elements, arrayWithLength<unsigned short> extraElements, int minSlice,
                 int maxSlice);

    unsigned short get(unsigned int x, unsigned int y, unsigned int z, bool* isUniformSection);
    unsigned short getExtraElement(unsigned int x, unsigned int y, unsigned int z);

    void set(unsigned int x, unsigned int y, unsigned int z, unsigned short value);
    void setExtraElement(unsigned int x, unsigned int y, unsigned int z, unsigned short value);

    int setDataRegion(arrayWithLength<unsigned char> data, int x0, int y0, int z0, int x1, int y1, int z1, int offset,
                      void (*changedCallback)(int, int, int, void*, int), void* callbackData, int callbackValue);
    int getDataRegion(arrayWithLength<unsigned char> data, int x0, int y0, int z0, int x1, int y1, int z1, int offset);

    bool isRenderChunkEmpty(int baseY, int* highestNonEmptyY);
    int getHighestNonEmptyY();

    bool sectionContainsElementData(int x, int y, int z, unsigned short value, unsigned short mask);

    void removeCompressionHeadroom();

    int getAllocatedBytes();
    unsigned char* getAllocatedRawData();

    int getRawData(unsigned char* rawData);
    void setFromRawData(unsigned char* rawData, unsigned int rawDataBytes);
    static int getRawDataMaxBytes();

    void write(DataOutputStream* out);
    void read(DataInputStream* in);

    virtual void getReadLockUnconditional(int reason);
    virtual void releaseReadLockUnconditional();
    virtual unsigned char* getTempData();

    static unsigned short swapBytes(unsigned short value);
    static bool CanEncodeElementAs0Bit(unsigned short value);
    static unsigned int SectionTypeToSize(unsigned int sectionType, bool hasTwoIndices);

    static unsigned int GetSectionIndexFromChunkXYZ(unsigned int x, unsigned int y, unsigned int z);
    static unsigned int GetElementIndexFromChunkXYZ(unsigned int x, unsigned int y, unsigned int z);
    static unsigned int GetSectionIDFromXYZ(int x, int y, int z);

    static void* allocData(unsigned int bytes);
    static void freeData(void* data);

private:
    void initWithPalettiser(Palettiser& palettiser, unsigned int headroom);

    void ExtendSubchunkData(unsigned int subChunk);
    void UpdateSectionDataSize(unsigned int subChunk, unsigned int section, unsigned int sectionType,
                               bool hasTwoIndices);
    void ReduceSubchunkDataTo(unsigned int subChunk, unsigned int pages);
    void ReduceSubchunkStorage();
    void checkSubChunkStorageRequirements(unsigned int subChunk);

    void set_subChunkEmpty(unsigned int x, unsigned int y, unsigned int z, unsigned short value, bool isExtra);
    void set_section0Bit(unsigned int x, unsigned int y, unsigned int z, unsigned short value, unsigned int section,
                         SubChunkHeader* header, unsigned int subChunkOffset, bool isExtra);
    void set_sectionRaw(unsigned int x, unsigned int y, unsigned int z, unsigned short value, unsigned int section,
                        SubChunkHeader* header, unsigned int subChunkOffset, bool isExtra);
    void set_sectionPalettised(unsigned int x, unsigned int y, unsigned int z, unsigned short value,
                               unsigned int section, SubChunkHeader* header, unsigned int subChunkOffset,
                               unsigned short planeCount, bool isExtra);

    unsigned int getSingleEntryPaletteValue(PalettisedSectionReference& sectionRef, unsigned int subChunk,
                                            unsigned int section, bool isExtra);
    void removeSingleEntryPaletteSection(PalettisedSectionReference& sectionRef, unsigned int subChunk,
                                         unsigned int section, unsigned short value);
    void checkForSingleEntryPalette(PalettisedSectionReference& sectionRef, unsigned int subChunk, unsigned int section,
                                    bool hasTwoIndices);

    SubChunkInfo mSubChunkInfo;
    unsigned char* mData;
    unsigned int mSubChunkCount;
};

#endif
