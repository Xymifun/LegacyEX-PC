#include "game/data/HierarchicalElementStorage.hpp"

#include <cstdlib>
#include <cstring>

#include "game/core/DataInputStream.hpp"
#include "game/core/DataOutputStream.hpp"
#include "game/data/SparseDataStorage.hpp"
#include "game/world/block/CompressedBlockStorage.hpp"

static int sTotalSubChunkBytes = 0;
static unsigned char* sTempData = 0;
static int sPalettisedChunkCount = 0;

static const unsigned int sSectionTypeSizes[8] = {0, 3, 6, 10, 16, 0, 0, 32};

static const unsigned int sSectionTypeSizesTwoIndices[8] = {0, 5, 10, 16, 24, 0, 0, 64};

unsigned short HierarchicalElementStorage::swapBytes(unsigned short value)
{
#if __BYTE_ORDER__ == __ORDER_BIG_ENDIAN__
    return (unsigned short)((value << 8) | (value >> 8));
#else
    return value;
#endif
}

bool HierarchicalElementStorage::CanEncodeElementAs0Bit(unsigned short value)
{
    return (value & 0xF000) == 0;
}

unsigned int HierarchicalElementStorage::SectionTypeToSize(unsigned int sectionType, bool hasTwoIndices)
{
    if (hasTwoIndices)
    {
        return sSectionTypeSizesTwoIndices[sectionType];
    }

    return sSectionTypeSizes[sectionType];
}

unsigned int HierarchicalElementStorage::GetSectionIndexFromChunkXYZ(unsigned int x, unsigned int y, unsigned int z)
{
    return (((x & 0xC) << 2) | (z & 0xC)) | ((y >> 2) & 3);
}

unsigned int HierarchicalElementStorage::GetElementIndexFromChunkXYZ(unsigned int x, unsigned int y, unsigned int z)
{
    return ((x & 3) << 4) | ((z & 3) << 2) | (y & 3);
}

unsigned int HierarchicalElementStorage::GetSectionIDFromXYZ(int x, int y, int z)
{
    return (((unsigned int)y & 0xFC) << 24) | (((unsigned int)x & 0x3FFC) << 10) | (((unsigned int)z & 0x3FFC) >> 2);
}

void* HierarchicalElementStorage::allocData(unsigned int bytes)
{
    return malloc(bytes);
}

void HierarchicalElementStorage::freeData(void* data)
{
    free(data);
}

void HierarchicalElementStorage::SubChunkInfo::setSizeAndOffset(unsigned int subChunk, unsigned short size,
                                                                unsigned short offset)
{
    mOffsets[subChunk] = swapBytes(offset);
    mSizes[subChunk] = (unsigned char)size;
}

unsigned short HierarchicalElementStorage::SubChunkInfo::getOffset(unsigned int subChunk)
{
    return swapBytes(mOffsets[subChunk]);
}

unsigned char HierarchicalElementStorage::SubChunkInfo::getSize(unsigned int subChunk)
{
    return mSizes[subChunk];
}

void HierarchicalElementStorage::SubChunkInfo::incSize(unsigned int subChunk, unsigned short amount)
{
    mSizes[subChunk] = (unsigned char)(mSizes[subChunk] + amount);
}

void HierarchicalElementStorage::SubChunkInfo::decSize(unsigned int subChunk, unsigned short amount)
{
    mSizes[subChunk] = (unsigned char)(mSizes[subChunk] - amount);
}

void HierarchicalElementStorage::SubChunkInfo::incOffset(unsigned int subChunk, unsigned short amount)
{
    mOffsets[subChunk] = (unsigned short)(mOffsets[subChunk] + swapBytes(amount));
}

void HierarchicalElementStorage::SubChunkInfo::decOffset(unsigned int subChunk, unsigned short amount)
{
    mOffsets[subChunk] = (unsigned short)(mOffsets[subChunk] - swapBytes(amount));
}

int HierarchicalElementStorage::SubChunkInfo::getUsedSize()
{
    return getOffset(SUBCHUNK_COUNT - 1) + getSize(SUBCHUNK_COUNT - 1);
}

unsigned int HierarchicalElementStorage::SubChunkInfo::getRawDataBytes()
{
    return sizeof(unsigned short) * SUBCHUNK_COUNT + sizeof(unsigned char) * SUBCHUNK_COUNT;
}

void HierarchicalElementStorage::SubChunkInfo::getRawData(unsigned char* rawData)
{
    memcpy(rawData, mOffsets, sizeof(mOffsets));
    memcpy(rawData + sizeof(mOffsets), mSizes, sizeof(mSizes));
}

void HierarchicalElementStorage::SubChunkInfo::setFromRawData(unsigned char* rawData)
{
    memcpy(mOffsets, rawData, sizeof(mOffsets));
    memcpy(mSizes, rawData + sizeof(mOffsets), sizeof(mSizes));
}

void HierarchicalElementStorage::SubChunkInfo::write(DataOutputStream* out)
{
    arrayWithLength<unsigned char> offsetBytes((unsigned char*)mOffsets, sizeof(mOffsets));
    out->write(offsetBytes);

    arrayWithLength<unsigned char> sizeBytes(mSizes, sizeof(mSizes));
    out->write(sizeBytes);
}

void HierarchicalElementStorage::SubChunkInfo::read(DataInputStream* in)
{
    arrayWithLength<unsigned char> offsetBytes;
    offsetBytes.data = (unsigned char*)mOffsets;
    offsetBytes.length = sizeof(mOffsets);
    in->readFully(offsetBytes);

    arrayWithLength<unsigned char> sizeBytes;
    sizeBytes.data = mSizes;
    sizeBytes.length = sizeof(mSizes);
    in->readFully(sizeBytes);
}

void HierarchicalElementStorage::SubChunkHeader::setData(unsigned int section, unsigned short value)
{
    mSections[section] = (unsigned short)(swapBytes((unsigned short)(SECTION_TYPE_UNIFORM << 13)) | swapBytes(value));
}

unsigned short HierarchicalElementStorage::SubChunkHeader::getData(unsigned int section)
{
    if (getType(section) != SECTION_TYPE_UNIFORM)
    {
        return 0;
    }

    return (unsigned short)(swapBytes(mSections[section]) & 0xFFF);
}

void HierarchicalElementStorage::SubChunkHeader::setTypeAndOffset(unsigned int section, unsigned short sectionType,
                                                                  unsigned short offset, bool hasTwoIndices)
{
    unsigned short packed = (unsigned short)((sectionType << 13) | offset);

    if (hasTwoIndices)
    {
        packed = (unsigned short)(packed | 0x1000);
    }

    mSections[section] = swapBytes(packed);
}

void HierarchicalElementStorage::SubChunkHeader::setType(unsigned int section, unsigned short sectionType)
{
    mSections[section] = (unsigned short)((mSections[section] & swapBytes(0xFFF))
                                          | (mSections[section] & swapBytes(0x1000))
                                          | swapBytes((unsigned short)((sectionType & 7) << 13)));
}

unsigned short HierarchicalElementStorage::SubChunkHeader::getType(unsigned int section)
{
    return swapBytes(mSections[section]) >> 13;
}

bool HierarchicalElementStorage::SubChunkHeader::getHasTwoIndices(unsigned int section)
{
    return ((swapBytes(mSections[section]) >> 12) & 1) != 0;
}

unsigned short HierarchicalElementStorage::SubChunkHeader::getOffset(unsigned int section)
{
    if (section == 0)
    {
        return 0;
    }

    if (getType(section) != SECTION_TYPE_UNIFORM)
    {
        return (unsigned short)(swapBytes(mSections[section]) & 0xFFF);
    }

    while (section != 1)
    {
        --section;

        unsigned int previousType = getType(section);

        if (previousType != SECTION_TYPE_UNIFORM)
        {
            return (unsigned short)((swapBytes(mSections[section]) & 0xFFF)
                                    + SectionTypeToSize(previousType, getHasTwoIndices(section)));
        }
    }

    return (unsigned short)SectionTypeToSize(getType(0), getHasTwoIndices(0));
}

void HierarchicalElementStorage::SubChunkHeader::incOffset(unsigned int section, unsigned short amount)
{
    if (getType(section) != SECTION_TYPE_UNIFORM)
    {
        mSections[section] = swapBytes((unsigned short)(swapBytes(mSections[section]) + amount));
    }
}

void HierarchicalElementStorage::SubChunkHeader::decOffset(unsigned int section, unsigned short amount)
{
    if (getType(section) != SECTION_TYPE_UNIFORM)
    {
        mSections[section] = swapBytes((unsigned short)(swapBytes(mSections[section]) - amount));
    }
}

int HierarchicalElementStorage::SubChunkHeader::getUsedSize()
{
    unsigned int lastSection = SECTIONS_PER_SUBCHUNK - 1;

    return (int)(SectionTypeToSize(getType(lastSection), getHasTwoIndices(lastSection)) + getOffset(lastSection)
                 + SUBCHUNK_HEADER_BYTES / 4);
}

HierarchicalElementStorage::PalettisedSectionReference::PalettisedSectionReference(SubChunkHeader* header,
                                                                                   unsigned int section)
{
    unsigned int offset = header->getOffset(section);

    mPlaneCount = header->getType(section);
    mHasTwoIndices = header->getHasTwoIndices(section);
    mPalette = (unsigned short*)((unsigned char*)header + SUBCHUNK_HEADER_BYTES + offset * 4);
    mPlaneData = (unsigned char*)mPalette + getPaletteSize() * sizeof(unsigned short);
}

unsigned int HierarchicalElementStorage::PalettisedSectionReference::getPlaneCount()
{
    return mPlaneCount;
}

unsigned int HierarchicalElementStorage::PalettisedSectionReference::getPaletteSize()
{
    return 1u << mPlaneCount;
}

void HierarchicalElementStorage::PalettisedSectionReference::setPlaneCount(unsigned int planeCount)
{
    mPlaneCount = planeCount;
    unsigned int paletteSize = 1 << planeCount;
    mPlaneData = (unsigned char*)(mPalette + paletteSize);
}

unsigned short HierarchicalElementStorage::PalettisedSectionReference::getPaletteEntry(unsigned int paletteIndex)
{
    return swapBytes(mPalette[paletteIndex]);
}

void HierarchicalElementStorage::PalettisedSectionReference::setPaletteEntry(unsigned int paletteIndex,
                                                                             unsigned short value)
{
    mPalette[paletteIndex] = swapBytes(value);
}

unsigned char* HierarchicalElementStorage::PalettisedSectionReference::getPlaneData()
{
    return mPlaneData;
}

unsigned char* HierarchicalElementStorage::PalettisedSectionReference::getExtraPlaneData()
{
    return mPlaneData + mPlaneCount * SECTION_PLANE_BYTES;
}

unsigned int HierarchicalElementStorage::PalettisedSectionReference::getPlaneDataSize()
{
    return mPlaneCount * SECTION_PLANE_BYTES;
}

HierarchicalElementStorage::RawSectionReference::RawSectionReference(SubChunkHeader* header, unsigned int section)
{
    unsigned int offset = header->getOffset(section);

    mElements = (unsigned short*)((unsigned char*)header + SUBCHUNK_HEADER_BYTES + offset * 4);
    mHasTwoIndices = header->getHasTwoIndices(section);
}

unsigned short HierarchicalElementStorage::RawSectionReference::getElement(unsigned int elementIndex)
{
    return swapBytes(mElements[elementIndex]);
}

void HierarchicalElementStorage::RawSectionReference::setElement(unsigned int elementIndex, unsigned short value)
{
    mElements[elementIndex] = swapBytes(value);
}

unsigned short HierarchicalElementStorage::RawSectionReference::getExtraElement(unsigned int elementIndex)
{
    if (!mHasTwoIndices)
    {
        return 0;
    }

    return swapBytes(mElements[ELEMENTS_PER_SECTION + elementIndex]);
}

void HierarchicalElementStorage::RawSectionReference::setExtraElement(unsigned int elementIndex, unsigned short value)
{
    mElements[ELEMENTS_PER_SECTION + elementIndex] = swapBytes(value);
}

HierarchicalElementStorage::Palettiser::Palettiser(arrayWithLength<unsigned char> blocks,
                                                   arrayWithLength<unsigned char> data)
{
    if (blocks.length == 16 * 16 * 128)
    {
        if (data.length == (16 * 16 * 128) / 2)
        {
            mSourceType = SOURCE_BLOCK_ARRAYS_HALF_HEIGHT;
        }
    }
    else if (blocks.length == 16 * 16 * 256)
    {
        if (data.length == (16 * 16 * 256) / 2)
        {
            mSourceType = SOURCE_BLOCK_ARRAYS;
        }
    }

    mBlocks = blocks;
    mData = data;
}

HierarchicalElementStorage::Palettiser::Palettiser(arrayWithLength<unsigned short> elements)
{
    if (elements.length == 16 * 16 * 128)
    {
        mSourceType = SOURCE_ELEMENT_ARRAY_HALF_HEIGHT;
    }
    else if (elements.length == 16 * 16 * 256)
    {
        mSourceType = SOURCE_ELEMENT_ARRAY;
    }

    mElements = elements;
}

HierarchicalElementStorage::Palettiser::Palettiser(CompressedBlockStorage* blocks, CompressedBlockStorage* extraBlocks,
                                                   SparseDataStorage* data, SparseDataStorage* extraData)
{
    mSourceType = SOURCE_BLOCK_STORAGE;
    mBlockStorage = blocks;
    mExtraBlockStorage = extraBlocks;
    mDataStorage = data;
    mExtraDataStorage = extraData;
}

unsigned short HierarchicalElementStorage::Palettiser::getElement(unsigned int elementIndex)
{
    if ((mSourceType == SOURCE_ELEMENT_ARRAY_HALF_HEIGHT || mSourceType == SOURCE_BLOCK_ARRAYS_HALF_HEIGHT)
        && elementIndex >= 0x8000)
    {
        return 0;
    }

    if (mSourceType == SOURCE_ELEMENT_ARRAY || mSourceType == SOURCE_ELEMENT_ARRAY_HALF_HEIGHT)
    {
        return mElements.data[elementIndex];
    }

    if (mSourceType == SOURCE_BLOCK_ARRAYS || mSourceType == SOURCE_BLOCK_ARRAYS_HALF_HEIGHT)
    {
        unsigned short block = (unsigned short)(mBlocks.data[elementIndex] << 4);

        if ((elementIndex & 1) == 0)
        {
            return (unsigned short)(block | (mData.data[elementIndex >> 1] & 0xF));
        }

        return (unsigned short)(block | (mData.data[elementIndex >> 1] >> 4));
    }

    if (mSourceType == SOURCE_BLOCK_STORAGE)
    {
        unsigned int sectionElement = (elementIndex & 3) | ((elementIndex >> 5) & 0xC) | ((elementIndex >> 7) & 0x30);
        unsigned short block = (unsigned short)(mSectionBlocks[sectionElement] << 4);

        if ((elementIndex & 0x8000) == 0)
        {
            return (unsigned short)(block
                                    | mDataStorage->get((elementIndex >> 11) & 0xF, elementIndex & 0x7F,
                                                        (elementIndex >> 7) & 0xF));
        }

        return (unsigned short)(block
                                | mExtraDataStorage->get((elementIndex >> 11) & 0xF, elementIndex & 0x7F,
                                                         (elementIndex >> 7) & 0xF));
    }

    return 0;
}

void HierarchicalElementStorage::Palettiser::initialiseSection(unsigned int sectionBaseIndex)
{
    mPaletteCount = 0;

    if (mSourceType == SOURCE_BLOCK_STORAGE)
    {
        if ((sectionBaseIndex & 0x8000) == 0)
        {
            mBlockStorage->getDataSection(mSectionBlocks, (sectionBaseIndex >> 13) & 3, (sectionBaseIndex >> 2) & 0x1F,
                                          (sectionBaseIndex >> 9) & 3);
        }
        else
        {
            mExtraBlockStorage->getDataSection(mSectionBlocks, (sectionBaseIndex >> 13) & 3,
                                               (sectionBaseIndex >> 2) & 0x1F, (sectionBaseIndex >> 9) & 3);
        }
    }

    for (unsigned int element = 0; element < ELEMENTS_PER_SECTION; ++element)
    {
        sectionBaseIndex = (sectionBaseIndex & 0xE67C) | ((element & 0x30) << 7) | ((element & 0xC) << 5)
                           | (element & 3);
        unsigned short value = getElement(sectionBaseIndex);

        unsigned int paletteCount = mPaletteCount;
        int paletteIndex = MAX_PALETTE_ENTRIES;

        for (unsigned int i = 0; i < paletteCount; ++i)
        {
            if (mPalette[i] == value)
            {
                paletteIndex = i;
                break;
            }
        }

        if (paletteIndex == MAX_PALETTE_ENTRIES)
        {
            if (paletteCount >= MAX_PALETTE_ENTRIES)
            {
                mPaletteCount = 0;
                return;
            }

            mPaletteCount = paletteCount + 1;
            mPalette[paletteCount] = value;
            mIndices[element] = (unsigned char)paletteCount;
        }
        else
        {
            mIndices[element] = (unsigned char)paletteIndex;
        }
    }
}

void HierarchicalElementStorage::Palettiser::createHeadroom(unsigned int extraEntries)
{
    if (mPaletteCount == 0)
    {
        return;
    }

    if (mPaletteCount == 1 && CanEncodeElementAs0Bit(mPalette[0]))
    {
        return;
    }

    unsigned int newCount = mPaletteCount + extraEntries;

    if (newCount > MAX_PALETTE_ENTRIES)
    {
        newCount = MAX_PALETTE_ENTRIES;
    }

    for (unsigned int i = mPaletteCount; i < newCount; ++i)
    {
        mPalette[i] = UNUSED_PALETTE_ENTRY;
    }

    mPaletteCount = newCount;
}

bool HierarchicalElementStorage::Palettiser::didPalettise()
{
    return mPaletteCount != 0;
}

bool HierarchicalElementStorage::Palettiser::writeSection(SubChunkHeader* header, unsigned int section,
                                                          unsigned short& offset)
{
    if (mPaletteCount == 1 && CanEncodeElementAs0Bit(mPalette[0]))
    {
        header->setData(section, mPalette[0]);
        return mPalette[0] == 0;
    }

    unsigned int planeCount;

    if (mPaletteCount < 3)
    {
        planeCount = 1;
    }
    else if (mPaletteCount < 5)
    {
        planeCount = 2;
    }
    else if (mPaletteCount < 9)
    {
        planeCount = 3;
    }
    else
    {
        planeCount = 4;
    }

    header->setTypeAndOffset(section, (unsigned short)planeCount, offset, false);

    PalettisedSectionReference sectionRef(header, section);

    unsigned int paletteSize = sectionRef.getPaletteSize();

    for (unsigned int entry = 0; entry < paletteSize; ++entry)
    {
        sectionRef.setPaletteEntry(entry,
                                   entry < mPaletteCount ? mPalette[entry] : (unsigned short)UNUSED_PALETTE_ENTRY);
    }

    unsigned char* planeData = sectionRef.getPlaneData();
    unsigned int sourceIndex = 0;

    for (unsigned int planeByte = 0; planeByte < SECTION_PLANE_BYTES; ++planeByte)
    {
        for (unsigned int plane = 0; plane < planeCount; ++plane)
        {
            planeData[plane * SECTION_PLANE_BYTES + planeByte] = 0;
        }

        for (unsigned int bit = 0; bit < 8; ++bit)
        {
            unsigned int index = mIndices[sourceIndex];
            ++sourceIndex;

            for (unsigned int plane = 0; plane < planeCount; ++plane)
            {
                unsigned char* target = &planeData[plane * SECTION_PLANE_BYTES + planeByte];
                *target = (unsigned char)((*target << 1) | (index & 1));
                index >>= 1;
            }
        }
    }

    offset = (unsigned short)(offset + SectionTypeToSize(planeCount, false));

    return false;
}

HierarchicalElementStorage::HierarchicalElementStorage()
{
    for (unsigned int subChunk = 0; subChunk < SUBCHUNK_COUNT; ++subChunk)
    {
        mSubChunkInfo.setSizeAndOffset(subChunk, 0, 0);
    }

    mData = NULL;
    mSubChunkCount = 0;
}

HierarchicalElementStorage::HierarchicalElementStorage(HierarchicalElementStorage* other)
{
    other->getReadLockUnconditional(200);

    mSubChunkInfo = other->mSubChunkInfo;
    mSubChunkCount = other->mSubChunkCount;
    mData = (unsigned char*)allocData(mSubChunkCount * SUBCHUNK_PAGE_BYTES);
    memcpy(mData, other->mData, mSubChunkCount * SUBCHUNK_PAGE_BYTES);
    sTotalSubChunkBytes += mSubChunkCount * SUBCHUNK_PAGE_BYTES;

    other->releaseReadLockUnconditional();
}

HierarchicalElementStorage::~HierarchicalElementStorage()
{
    sTotalSubChunkBytes -= mSubChunkCount * SUBCHUNK_PAGE_BYTES;
    freeData(mData);
}

void HierarchicalElementStorage::getReadLockUnconditional(int reason)
{
}

void HierarchicalElementStorage::releaseReadLockUnconditional()
{
}

unsigned char* HierarchicalElementStorage::getTempData()
{
    if (sTempData == NULL)
    {
        sTempData = (unsigned char*)malloc(getRawDataMaxBytes());
    }

    return sTempData;
}

void HierarchicalElementStorage::initWithPalettiser(Palettiser& palettiser, unsigned int headroom)
{
    unsigned char* tempData = getTempData();
    unsigned int usedPages = 0;

    for (unsigned int subChunk = 0; subChunk < SUBCHUNK_COUNT; ++subChunk)
    {
        SubChunkHeader* header = (SubChunkHeader*)(tempData + usedPages * SUBCHUNK_PAGE_BYTES);
        unsigned int elementIndex = ((subChunk & 7) << 4) | ((subChunk & 8) << 12);
        unsigned short offset = 0;
        unsigned int emptySections = 0;

        for (unsigned int section = 0; section < SECTIONS_PER_SUBCHUNK; ++section)
        {
            elementIndex = (elementIndex & 0x99F3) | ((section & 0x30) << 9) | ((section & 0xC) << 7)
                           | ((section & 3) << 2);

            palettiser.initialiseSection(elementIndex);

            if (headroom != 0)
            {
                palettiser.createHeadroom(headroom);
            }

            if (palettiser.didPalettise())
            {
                if (palettiser.writeSection(header, section, offset))
                {
                    ++emptySections;
                }
            }
            else
            {
                header->setTypeAndOffset(section, SECTION_TYPE_RAW, offset, false);

                RawSectionReference sectionRef(header, section);

                for (unsigned int element = 0; element < ELEMENTS_PER_SECTION; ++element)
                {
                    elementIndex = (elementIndex & 0xE67C) | ((element & 0x30) << 7) | ((element & 0xC) << 5)
                                   | (element & 3);
                    sectionRef.setElement(element, palettiser.getElement(elementIndex));
                }

                offset = (unsigned short)(offset + SectionTypeToSize(SECTION_TYPE_RAW, false));
            }
        }

        unsigned int pages = 0;

        if (emptySections < SECTIONS_PER_SUBCHUNK)
        {
            pages = (offset * 4 + SUBCHUNK_HEADER_BYTES + (SUBCHUNK_PAGE_BYTES - 1)) / SUBCHUNK_PAGE_BYTES;
        }

        mSubChunkInfo.setSizeAndOffset(subChunk, (unsigned short)pages, (unsigned short)usedPages);
        usedPages += pages;
    }

    sTotalSubChunkBytes -= mSubChunkCount * SUBCHUNK_PAGE_BYTES;
    mSubChunkCount = usedPages;
    freeData(mData);
    mData = (unsigned char*)allocData(mSubChunkCount * SUBCHUNK_PAGE_BYTES);
    sTotalSubChunkBytes += mSubChunkCount * SUBCHUNK_PAGE_BYTES;
    memcpy(mData, tempData, mSubChunkCount * SUBCHUNK_PAGE_BYTES);
}

void HierarchicalElementStorage::_Init(arrayWithLength<unsigned char> blocks, arrayWithLength<unsigned char> data,
                                       unsigned int headroom)
{
    ++sPalettisedChunkCount;

    mData = NULL;
    mSubChunkCount = 0;

    Palettiser palettiser(blocks, data);
    initWithPalettiser(palettiser, headroom);
}

void HierarchicalElementStorage::_Init(arrayWithLength<unsigned short> elements, unsigned int headroom)
{
    ++sPalettisedChunkCount;

    mSubChunkCount = 0;
    mData = NULL;

    Palettiser palettiser(elements);
    initWithPalettiser(palettiser, headroom);
}

void HierarchicalElementStorage::_Init(CompressedBlockStorage* blocks, CompressedBlockStorage* extraBlocks,
                                       SparseDataStorage* data, SparseDataStorage* extraData)
{
    mData = NULL;
    mSubChunkCount = 0;

    Palettiser palettiser(blocks, extraBlocks, data, extraData);
    initWithPalettiser(palettiser, 0);
}

void HierarchicalElementStorage::setData(arrayWithLength<unsigned char> blocks, arrayWithLength<unsigned char> data)
{
    Palettiser palettiser(blocks, data);
    initWithPalettiser(palettiser, 0);
}

void HierarchicalElementStorage::setData(arrayWithLength<unsigned short> elements)
{
    Palettiser palettiser(elements);
    initWithPalettiser(palettiser, 0);
}

void HierarchicalElementStorage::ExtendSubchunkData(unsigned int subChunk)
{
    unsigned char* newData = (unsigned char*)allocData(mSubChunkCount * SUBCHUNK_PAGE_BYTES + SUBCHUNK_PAGE_BYTES);
    unsigned int pages = mSubChunkInfo.getSize(subChunk);
    unsigned int firstPage = mSubChunkInfo.getOffset(subChunk);
    unsigned int endPage = firstPage + pages;
    unsigned int keptBytes = endPage * SUBCHUNK_PAGE_BYTES;

    memcpy(newData, mData, keptBytes);
    memset(newData + keptBytes, 0, SUBCHUNK_PAGE_BYTES);
    memcpy(newData + keptBytes + SUBCHUNK_PAGE_BYTES, mData + keptBytes,
           (mSubChunkCount - endPage) * SUBCHUNK_PAGE_BYTES);

    mSubChunkInfo.incSize(subChunk, 1);

    for (unsigned int following = subChunk + 1; following < SUBCHUNK_COUNT; ++following)
    {
        mSubChunkInfo.incOffset(following, 1);
    }

    freeData(mData);
    ++mSubChunkCount;
    mData = newData;
    sTotalSubChunkBytes += SUBCHUNK_PAGE_BYTES;
}

void HierarchicalElementStorage::UpdateSectionDataSize(unsigned int subChunk, unsigned int section,
                                                       unsigned int sectionType, bool hasTwoIndices)
{
    unsigned int newSize = SectionTypeToSize(sectionType, hasTwoIndices);
    unsigned int subChunkPages = mSubChunkInfo.getSize(subChunk);
    unsigned int subChunkOffset = mSubChunkInfo.getOffset(subChunk);
    SubChunkHeader* header = (SubChunkHeader*)(mData + subChunkOffset * SUBCHUNK_PAGE_BYTES);
    unsigned int oldType = header->getType(section);
    unsigned int oldSize = SectionTypeToSize(oldType, header->getHasTwoIndices(section));

    if (oldSize < newSize)
    {
        unsigned int growBy = newSize - oldSize;
        unsigned int usedSize = header->getUsedSize();
        unsigned int usedData = usedSize - SUBCHUNK_HEADER_BYTES / 4;

        if (usedSize + growBy > subChunkPages * (SUBCHUNK_PAGE_BYTES / 4))
        {
            ExtendSubchunkData(subChunk);
            header = (SubChunkHeader*)(mData + subChunkOffset * SUBCHUNK_PAGE_BYTES);
        }

        if (section < SECTIONS_PER_SUBCHUNK - 1)
        {
            unsigned int nextOffset = header->getOffset(section + 1);
            unsigned char* moveFrom = (unsigned char*)(header + 1) + nextOffset * 4;

            memmove(moveFrom + growBy * 4, moveFrom, (usedData - nextOffset) * 4);
            header->setTypeAndOffset(section, (unsigned short)sectionType, (unsigned short)(nextOffset - oldSize),
                                     hasTwoIndices);

            for (unsigned int following = section + 1; following < SECTIONS_PER_SUBCHUNK; ++following)
            {
                header->incOffset(following, (unsigned short)growBy);
            }
        }
        else
        {
            header->setTypeAndOffset(section, (unsigned short)sectionType, header->getOffset(section), hasTwoIndices);
        }
    }
    else if (newSize < oldSize)
    {
        unsigned int shrinkBy = oldSize - newSize;
        unsigned int usedSize = header->getUsedSize();
        unsigned int usedData = usedSize - SUBCHUNK_HEADER_BYTES / 4;

        if (section < SECTIONS_PER_SUBCHUNK - 1)
        {
            unsigned int nextOffset = header->getOffset(section + 1);
            unsigned char* moveFrom = (unsigned char*)(header + 1) + nextOffset * 4;

            memmove(moveFrom - shrinkBy * 4, moveFrom, (usedData - nextOffset) * 4);
            header->setType(section, (unsigned short)sectionType);

            for (unsigned int following = section + 1; following < SECTIONS_PER_SUBCHUNK; ++following)
            {
                header->decOffset(following, (unsigned short)shrinkBy);
            }
        }
        else
        {
            header->setType(section, (unsigned short)sectionType);
        }
    }
}

void HierarchicalElementStorage::ReduceSubchunkDataTo(unsigned int subChunk, unsigned int pages)
{
    unsigned int currentPages = mSubChunkInfo.getSize(subChunk);

    if (pages == currentPages)
    {
        return;
    }

    unsigned int subChunkOffset = mSubChunkInfo.getOffset(subChunk);
    unsigned int usedPages = mSubChunkInfo.getUsedSize();
    unsigned int removedPages = currentPages - pages;
    unsigned int tailPage = subChunkOffset + pages + removedPages;

    memmove(mData + (subChunkOffset + pages) * SUBCHUNK_PAGE_BYTES, mData + tailPage * SUBCHUNK_PAGE_BYTES,
            (usedPages - tailPage) * SUBCHUNK_PAGE_BYTES);

    mSubChunkInfo.decSize(subChunk, (unsigned short)removedPages);

    for (unsigned int following = subChunk + 1; following < SUBCHUNK_COUNT; ++following)
    {
        mSubChunkInfo.decOffset(following, (unsigned short)removedPages);
    }
}

void HierarchicalElementStorage::ReduceSubchunkStorage()
{
    unsigned int usedPages = mSubChunkInfo.getUsedSize();

    if (usedPages < mSubChunkCount)
    {
        unsigned char* newData = (unsigned char*)allocData(usedPages * SUBCHUNK_PAGE_BYTES);

        memcpy(newData, mData, usedPages * SUBCHUNK_PAGE_BYTES);
        freeData(mData);
        mData = newData;
        sTotalSubChunkBytes = sTotalSubChunkBytes - mSubChunkCount * SUBCHUNK_PAGE_BYTES
                              + usedPages * SUBCHUNK_PAGE_BYTES;
        mSubChunkCount = usedPages;
    }
}

void HierarchicalElementStorage::removeCompressionHeadroom()
{
    int subChunk = SUBCHUNK_COUNT;

    do
    {
        --subChunk;

        if (mSubChunkInfo.getSize(subChunk) != 0)
        {
            SubChunkHeader* header = (SubChunkHeader*)(mData + mSubChunkInfo.getOffset(subChunk) * SUBCHUNK_PAGE_BYTES);
            int emptySections = 0;
            int section = SECTIONS_PER_SUBCHUNK;

            do
            {
                --section;

                unsigned int planeCount = header->getType(section);

                if (planeCount == SECTION_TYPE_UNIFORM)
                {
                    if (header->getData(section) == 0)
                    {
                        ++emptySections;
                    }

                    continue;
                }

                if (planeCount > SECTION_TYPE_MAX_PLANE_COUNT)
                {
                    continue;
                }

                PalettisedSectionReference sectionRef(header, section);

                unsigned int paletteIndex = 1;

                for (unsigned int plane = 0; plane < planeCount; ++plane)
                {
                    if (sectionRef.getPaletteEntry(paletteIndex) == UNUSED_PALETTE_ENTRY)
                    {
                        if (plane == 0)
                        {
                            if (!header->getHasTwoIndices(section))
                            {
                                unsigned short value = sectionRef.getPaletteEntry(0);

                                if (CanEncodeElementAs0Bit(value))
                                {
                                    UpdateSectionDataSize(subChunk, section, SECTION_TYPE_UNIFORM, false);
                                    header->setData(section, value);

                                    if (value == 0)
                                    {
                                        ++emptySections;
                                    }

                                    break;
                                }
                            }
                        }
                        else
                        {
                            unsigned char* oldPlaneData = sectionRef.getPlaneData();
                            unsigned char* oldExtraPlaneData = NULL;

                            if (header->getHasTwoIndices(section))
                            {
                                oldExtraPlaneData = sectionRef.getExtraPlaneData();
                            }

                            sectionRef.setPlaneCount(plane);

                            unsigned char* newPlaneData = sectionRef.getPlaneData();
                            unsigned int planeDataSize = sectionRef.getPlaneDataSize();

                            memmove(newPlaneData, oldPlaneData, planeDataSize);

                            if (header->getHasTwoIndices(section))
                            {
                                memmove(sectionRef.getExtraPlaneData(), oldExtraPlaneData, planeDataSize);
                            }

                            UpdateSectionDataSize(subChunk, section, plane, header->getHasTwoIndices(section));
                            break;
                        }
                    }

                    paletteIndex <<= 1;
                }
            } while (section != 0);

            if (emptySections == SECTIONS_PER_SUBCHUNK)
            {
                ReduceSubchunkDataTo(subChunk, 0);
            }
            else
            {
                unsigned int pages = ((unsigned int)header->getUsedSize() * 4 + (SUBCHUNK_PAGE_BYTES - 1))
                                     / SUBCHUNK_PAGE_BYTES;

                ReduceSubchunkDataTo(subChunk, pages);
            }
        }
    } while (subChunk != 0);

    ReduceSubchunkStorage();
}

bool HierarchicalElementStorage::isRenderChunkEmpty(int baseY, int* highestNonEmptyY)
{
    int subChunk = baseY / (int)SUBCHUNK_COUNT;

    if (mSubChunkInfo.getSize(subChunk) == 0)
    {
        return true;
    }

    SubChunkHeader* header = (SubChunkHeader*)(mData + mSubChunkInfo.getOffset(subChunk) * SUBCHUNK_PAGE_BYTES);

    for (int ySection = 3; ySection >= 0; --ySection)
    {
        for (unsigned int column = 0; column < SECTIONS_PER_SUBCHUNK / 4; ++column)
        {
            unsigned int section = (unsigned int)ySection | (column * 4);
            unsigned int planeCount = header->getType(section);
            unsigned short value;

            if (planeCount == SECTION_TYPE_UNIFORM)
            {
                value = header->getData(section);
            }
            else if (planeCount == SECTION_TYPE_RAW)
            {
                RawSectionReference sectionRef(header, section);

                for (unsigned int element = 0; element < ELEMENTS_PER_SECTION; ++element)
                {
                    if (sectionRef.getElement(element) != 0)
                    {
                        if (highestNonEmptyY != NULL)
                        {
                            *highestNonEmptyY = baseY + ySection * 4 + 4;
                        }

                        return false;
                    }
                }

                continue;
            }
            else
            {
                PalettisedSectionReference sectionRef(header, section);
                unsigned int sectionPlanes = sectionRef.getPlaneCount();
                unsigned char* planeData = sectionRef.getPlaneData();

                for (unsigned int plane = 0; plane < sectionPlanes; ++plane)
                {
                    unsigned char* bytes = planeData + plane * SECTION_PLANE_BYTES;
                    unsigned char anySet = 0;
                    unsigned char allSet = 0xFF;

                    for (unsigned int byteIndex = 0; byteIndex < SECTION_PLANE_BYTES; ++byteIndex)
                    {
                        anySet = (unsigned char)(anySet | bytes[byteIndex]);
                        allSet = (unsigned char)(allSet & bytes[byteIndex]);
                    }

                    if (anySet != 0 && allSet != 0xFF)
                    {
                        if (highestNonEmptyY != NULL)
                        {
                            *highestNonEmptyY = baseY + ySection * 4 + 4;
                        }

                        return false;
                    }
                }

                unsigned int paletteIndex = 0;

                for (unsigned int plane = 0; plane < sectionPlanes; ++plane)
                {
                    if (planeData[plane * SECTION_PLANE_BYTES] == 0xFF)
                    {
                        paletteIndex |= 1u << plane;
                    }
                }

                value = sectionRef.getPaletteEntry(paletteIndex);
            }

            if (value != 0)
            {
                if (highestNonEmptyY != NULL)
                {
                    *highestNonEmptyY = baseY + ySection * 4 + 4;
                }

                return false;
            }
        }
    }

    return true;
}

int HierarchicalElementStorage::getHighestNonEmptyY()
{
    int highestNonEmptyY = -1;
    int baseY = 240;

    do
    {
        isRenderChunkEmpty(baseY, &highestNonEmptyY);
        baseY -= 16;

        if (highestNonEmptyY != -1)
        {
            break;
        }
    } while (baseY >= 0);

    if (highestNonEmptyY >= 0)
    {
        return highestNonEmptyY;
    }

    return 0;
}

void HierarchicalElementStorage::getData(arrayWithLength<unsigned char> blocks, arrayWithLength<unsigned char> data,
                                         int minSlice, int maxSlice)
{
    if ((blocks.length == 16 * 16 * 128 || data.length == (16 * 16 * 128) / 2) && maxSlice > 32)
    {
        maxSlice = 32;
    }

    bool writeBlocks = blocks.length != 0;
    bool writeData = data.length != 0;

    for (int slice = minSlice; slice < maxSlice; ++slice)
    {
        int subChunk = slice / 4;
        unsigned int sliceBaseIndex = (((unsigned int)slice & 0x1F) << 2) | (((unsigned int)slice & 0x20) << 10);

        if (mSubChunkInfo.getSize(subChunk) == 0)
        {
            for (unsigned int column = 0; column < 256; ++column)
            {
                for (unsigned int y = 0; y < 4; ++y)
                {
                    unsigned int index = sliceBaseIndex | (column << 7) | y;

                    if (writeBlocks)
                    {
                        blocks.data[index] = 0;
                    }

                    if (writeData && (index & 1) == 0)
                    {
                        data.data[index >> 1] = 0;
                    }
                }
            }

            continue;
        }

        SubChunkHeader* header = (SubChunkHeader*)(mData + mSubChunkInfo.getOffset(subChunk) * SUBCHUNK_PAGE_BYTES);

        for (unsigned int xSection = 0; xSection < 4; ++xSection)
        {
            for (unsigned int zSection = 0; zSection < 4; ++zSection)
            {
                unsigned int section = (xSection << 4) | (zSection << 2) | (unsigned int)(slice - subChunk * 4);
                unsigned int sectionBaseIndex = sliceBaseIndex | (xSection << 13) | (zSection << 9);
                unsigned int planeCount = header->getType(section);

                if (planeCount == SECTION_TYPE_UNIFORM)
                {
                    unsigned short value = header->getData(section);

                    for (unsigned int element = 0; element < ELEMENTS_PER_SECTION; ++element)
                    {
                        unsigned int index = sectionBaseIndex | ((element & 0x30) << 7) | ((element & 0xC) << 5)
                                             | (element & 3);

                        if (writeBlocks)
                        {
                            blocks.data[index] = (unsigned char)(value >> 4);
                        }

                        if (writeData)
                        {
                            if ((index & 1) != 0)
                            {
                                data.data[index >> 1] = (unsigned char)(data.data[index >> 1] | (value << 4));
                            }
                            else
                            {
                                data.data[index >> 1] = (unsigned char)(value & 0xF);
                            }
                        }
                    }
                }
                else if (planeCount == SECTION_TYPE_RAW)
                {
                    RawSectionReference sectionRef(header, section);

                    for (unsigned int element = 0; element < ELEMENTS_PER_SECTION; ++element)
                    {
                        unsigned int index = sectionBaseIndex | ((element & 0x30) << 7) | ((element & 0xC) << 5)
                                             | (element & 3);
                        unsigned short value = sectionRef.getElement(element);

                        if (writeBlocks)
                        {
                            blocks.data[index] = (unsigned char)(value >> 4);
                        }

                        if (writeData)
                        {
                            if ((index & 1) != 0)
                            {
                                data.data[index >> 1] = (unsigned char)(data.data[index >> 1] | (value << 4));
                            }
                            else
                            {
                                data.data[index >> 1] = (unsigned char)(value & 0xF);
                            }
                        }
                    }
                }
                else
                {
                    PalettisedSectionReference sectionRef(header, section);
                    unsigned char* planeData = sectionRef.getPlaneData();

                    for (unsigned int element = 0; element < ELEMENTS_PER_SECTION; ++element)
                    {
                        unsigned char bit = (unsigned char)(0x80 >> (element & 7));
                        unsigned int paletteIndex = 0;

                        for (unsigned int plane = 0; plane < planeCount; ++plane)
                        {
                            if ((planeData[plane * SECTION_PLANE_BYTES + (element >> 3)] & bit) != 0)
                            {
                                paletteIndex |= 1u << plane;
                            }
                        }

                        unsigned short value = sectionRef.getPaletteEntry(paletteIndex);
                        unsigned int index = sectionBaseIndex | ((element & 0x30) << 7) | ((element & 0xC) << 5)
                                             | (element & 3);

                        if (writeBlocks)
                        {
                            blocks.data[index] = (unsigned char)(value >> 4);
                        }

                        if (writeData)
                        {
                            if ((index & 1) != 0)
                            {
                                data.data[index >> 1] = (unsigned char)(data.data[index >> 1] | (value << 4));
                            }
                            else
                            {
                                data.data[index >> 1] = (unsigned char)(value & 0xF);
                            }
                        }
                    }
                }
            }
        }
    }
}

void HierarchicalElementStorage::getData(arrayWithLength<unsigned short> elements, int minSlice, int maxSlice)
{
    for (int slice = minSlice; slice < maxSlice; ++slice)
    {
        int subChunk = slice / 4;
        unsigned int sliceBaseIndex = (((unsigned int)slice & 0x20) << 10) | (((unsigned int)slice & 0x1F) << 2);

        int sliceInSubChunk = slice - subChunk * 4;

        if (mSubChunkInfo.getSize(subChunk) == 0)
        {
            for (int column = 0; column < 256; ++column)
            {
                for (int y = 0; y < 4; ++y)
                {
                    elements.data[sliceBaseIndex | (column << 7) | y] = 0;
                }
            }

            continue;
        }

        SubChunkHeader* header = (SubChunkHeader*)(mData + mSubChunkInfo.getOffset(subChunk) * SUBCHUNK_PAGE_BYTES);

        for (int xSection = 0; xSection < 4; ++xSection)
        {
            for (int zSection = 0; zSection < 4; ++zSection)
            {
                unsigned int section = (xSection << 4) | (zSection << 2) | (unsigned int)sliceInSubChunk;
                unsigned int sectionBaseIndex = sliceBaseIndex | (xSection << 13) | (zSection << 9);
                unsigned int planeCount = header->getType(section);

                if (planeCount == SECTION_TYPE_UNIFORM)
                {
                    unsigned short value = header->getData(section);

                    for (unsigned int element = 0; element < ELEMENTS_PER_SECTION; ++element)
                    {
                        elements.data[sectionBaseIndex | ((element & 0x30) << 7) | ((element & 0xC) << 5)
                                      | (element & 3)] = value;
                    }
                }
                else if (planeCount == SECTION_TYPE_RAW)
                {
                    RawSectionReference sectionRef(header, section);

                    for (unsigned int element = 0; element < ELEMENTS_PER_SECTION; ++element)
                    {
                        elements.data[sectionBaseIndex | ((element & 0x30) << 7) | ((element & 0xC) << 5)
                                      | (element & 3)] = sectionRef.getElement(element);
                    }
                }
                else
                {
                    PalettisedSectionReference sectionRef(header, section);
                    unsigned char* planeData = sectionRef.getPlaneData();

                    for (unsigned int element = 0; element < ELEMENTS_PER_SECTION; ++element)
                    {
                        unsigned char bit = (unsigned char)(0x80 >> (element & 7));
                        unsigned int paletteIndex = 0;
                        unsigned char paletteBit = 1;

                        for (unsigned int plane = 0; plane < planeCount; ++plane)
                        {
                            if ((planeData[plane * SECTION_PLANE_BYTES + (element >> 3)] & bit) != 0)
                            {
                                paletteIndex |= paletteBit;
                            }

                            paletteBit <<= 1;
                        }

                        elements.data[sectionBaseIndex | ((element & 0x30) << 7) | ((element & 0xC) << 5)
                                      | (element & 3)] = sectionRef.getPaletteEntry(paletteIndex);
                    }
                }
            }
        }
    }
}

void HierarchicalElementStorage::getData(arrayWithLength<unsigned short> elements,
                                         arrayWithLength<unsigned short> extraElements, int minSlice, int maxSlice)
{
    for (int slice = minSlice; slice < maxSlice; ++slice)
    {
        int subChunk = slice / 4;
        unsigned int sliceBaseIndex = (((unsigned int)slice & 0x20) << 10) | (((unsigned int)slice & 0x1F) << 2);

        if (mSubChunkInfo.getSize(subChunk) == 0)
        {
            for (int column = 0; column < 256; ++column)
            {
                for (int y = 0; y < 4; ++y)
                {
                    elements.data[sliceBaseIndex | (column << 7) | y] = 0;
                }
            }

            continue;
        }

        SubChunkHeader* header = (SubChunkHeader*)(mData + mSubChunkInfo.getOffset(subChunk) * SUBCHUNK_PAGE_BYTES);

        for (int xSection = 0; xSection < 4; ++xSection)
        {
            for (int zSection = 0; zSection < 4; ++zSection)
            {
                unsigned int section = (xSection << 4) | (zSection << 2) | (unsigned int)(slice - subChunk * 4);
                unsigned int sectionBaseIndex = sliceBaseIndex | (xSection << 13) | (zSection << 9);
                unsigned int planeCount = header->getType(section);

                if (planeCount == SECTION_TYPE_UNIFORM)
                {
                    unsigned short value = header->getData(section);

                    for (unsigned int element = 0; element < ELEMENTS_PER_SECTION; ++element)
                    {
                        unsigned int index = sectionBaseIndex | ((element & 0x30) << 7) | ((element & 0xC) << 5)
                                             | (element & 3);
                        elements.data[index] = value;
                        extraElements.data[index] = 0;
                    }
                }
                else if (planeCount == SECTION_TYPE_RAW)
                {
                    RawSectionReference sectionRef(header, section);

                    for (unsigned int element = 0; element < ELEMENTS_PER_SECTION; ++element)
                    {
                        unsigned int index = sectionBaseIndex | ((element & 0x30) << 7) | ((element & 0xC) << 5)
                                             | (element & 3);
                        elements.data[index] = sectionRef.getElement(element);
                        extraElements.data[index] = sectionRef.getExtraElement(element);
                    }
                }
                else
                {
                    PalettisedSectionReference sectionRef(header, section);
                    unsigned char* planeData = sectionRef.getPlaneData();
                    unsigned char* extraPlaneData = NULL;

                    if (header->getHasTwoIndices(section))
                    {
                        extraPlaneData = sectionRef.getExtraPlaneData();
                    }

                    for (unsigned int element = 0; element < ELEMENTS_PER_SECTION; ++element)
                    {
                        unsigned char bit = (unsigned char)(0x80 >> (element & 7));
                        unsigned int paletteIndex = 0;
                        unsigned int extraPaletteIndex = 0;

                        unsigned char paletteBit = 1;

                        for (unsigned int plane = 0; plane < planeCount; ++plane)
                        {
                            if ((planeData[plane * SECTION_PLANE_BYTES + (element >> 3)] & bit) != 0)
                            {
                                paletteIndex |= paletteBit;
                            }

                            if (extraPlaneData != NULL
                                && (extraPlaneData[plane * SECTION_PLANE_BYTES + (element >> 3)] & bit) != 0)
                            {
                                extraPaletteIndex |= paletteBit;
                            }

                            paletteBit <<= 1;
                        }

                        unsigned short extraValue = 0;

                        if (extraPlaneData != NULL)
                        {
                            extraValue = sectionRef.getPaletteEntry(extraPaletteIndex);
                        }

                        unsigned int index = sectionBaseIndex | ((element & 0x30) << 7) | ((element & 0xC) << 5)
                                             | (element & 3);
                        elements.data[index] = sectionRef.getPaletteEntry(paletteIndex);
                        extraElements.data[index] = extraValue;
                    }
                }
            }
        }
    }
}

unsigned short HierarchicalElementStorage::get(unsigned int x, unsigned int y, unsigned int z, bool* isUniformSection)
{
    unsigned int subChunk = y >> 4;

    if (mSubChunkInfo.getSize(subChunk) == 0)
    {
        if (isUniformSection != NULL)
        {
            *isUniformSection = true;
        }

        return 0;
    }

    unsigned int subChunkOffset = mSubChunkInfo.getOffset(subChunk);
    SubChunkHeader* header = (SubChunkHeader*)(mData + subChunkOffset * SUBCHUNK_PAGE_BYTES);
    unsigned int section = GetSectionIndexFromChunkXYZ(x, y, z);
    unsigned int planeCount = header->getType(section);

    if (planeCount == SECTION_TYPE_UNIFORM)
    {
        unsigned short value = header->getData(section);

        if (isUniformSection != NULL)
        {
            *isUniformSection = true;
        }

        return value;
    }

    if (planeCount == SECTION_TYPE_RAW)
    {
        RawSectionReference sectionRef(header, section);
        unsigned short value = sectionRef.getElement(((x & 3) << 4) | ((z & 3) << 2) | (y & 3));

        if (isUniformSection != NULL)
        {
            *isUniformSection = false;
        }

        return value;
    }

    unsigned int elementIndex = GetElementIndexFromChunkXYZ(x, y, z);
    unsigned char bit = (unsigned char)(0x80 >> (elementIndex & 7));
    unsigned int byteIndex = elementIndex >> 3;

    PalettisedSectionReference sectionRef(header, section);
    unsigned char* planeData = sectionRef.getPlaneData();
    unsigned int paletteIndex = 0;

    unsigned char paletteBit = 1;

    for (int plane = 0; plane < (int)planeCount; ++plane)
    {
        if ((planeData[plane * SECTION_PLANE_BYTES + byteIndex] & bit) != 0)
        {
            paletteIndex |= paletteBit;
        }

        paletteBit <<= 1;
    }

    unsigned short value = sectionRef.getPaletteEntry(paletteIndex);

    if (isUniformSection != NULL)
    {
        *isUniformSection = false;
    }

    return value;
}

unsigned short HierarchicalElementStorage::getExtraElement(unsigned int x, unsigned int y, unsigned int z)
{
    unsigned int subChunk = y >> 4;

    if (mSubChunkInfo.getSize(subChunk) == 0)
    {
        return 0;
    }

    SubChunkHeader* header = (SubChunkHeader*)(mData + mSubChunkInfo.getOffset(subChunk) * SUBCHUNK_PAGE_BYTES);
    unsigned int section = GetSectionIndexFromChunkXYZ(x, y, z);
    unsigned int planeCount = header->getType(section);

    if (!header->getHasTwoIndices(section) || planeCount == SECTION_TYPE_UNIFORM)
    {
        return 0;
    }

    if (planeCount == SECTION_TYPE_RAW)
    {
        RawSectionReference sectionRef(header, section);
        return sectionRef.getExtraElement(((x & 3) << 4) | ((z & 3) << 2) | (y & 3));
    }

    unsigned int elementIndex = GetElementIndexFromChunkXYZ(x, y, z);
    unsigned char bit = (unsigned char)(0x80 >> (elementIndex & 7));
    unsigned int byteIndex = elementIndex >> 3;

    PalettisedSectionReference sectionRef(header, section);
    unsigned char* extraPlaneData = sectionRef.getExtraPlaneData();
    unsigned int paletteIndex = 0;

    unsigned char paletteBit = 1;

    for (int plane = 0; plane < (int)planeCount; ++plane)
    {
        if ((extraPlaneData[plane * SECTION_PLANE_BYTES + byteIndex] & bit) != 0)
        {
            paletteIndex |= paletteBit;
        }

        paletteBit <<= 1;
    }

    return sectionRef.getPaletteEntry(paletteIndex);
}

void HierarchicalElementStorage::set_subChunkEmpty(unsigned int x, unsigned int y, unsigned int z, unsigned short value,
                                                   bool isExtra)
{
    ExtendSubchunkData(y >> 4);

    SubChunkHeader* header = (SubChunkHeader*)(mData + mSubChunkInfo.getOffset(y >> 4) * SUBCHUNK_PAGE_BYTES);
    unsigned int targetSection = GetSectionIndexFromChunkXYZ(x, y, z);
    unsigned short offset = 0;
    unsigned int elementIndex = GetElementIndexFromChunkXYZ(x, y, z);

    for (unsigned int section = 0; section < SECTIONS_PER_SUBCHUNK; ++section)
    {
        if (section == targetSection)
        {
            header->setTypeAndOffset(section, 1, offset, isExtra);

            unsigned char bit = (unsigned char)(0x80 >> (elementIndex & 7));
            PalettisedSectionReference sectionRef(header, section);

            if (isExtra)
            {
                unsigned char* extraPlaneData = sectionRef.getExtraPlaneData();

                extraPlaneData[elementIndex >> 3] = bit;
            }
            else
            {
                unsigned char* planeData = sectionRef.getPlaneData();

                planeData[elementIndex >> 3] = bit;
            }

            sectionRef.setPaletteEntry(1, value);
            offset = offset + SectionTypeToSize(1, isExtra);
        }
        else
        {
            header->setData(section, 0);
        }
    }
}

void HierarchicalElementStorage::set_section0Bit(unsigned int x, unsigned int y, unsigned int z, unsigned short value,
                                                 unsigned int section, SubChunkHeader* header,
                                                 unsigned int subChunkOffset, bool isExtra)
{
    unsigned short existing = header->getData(section);

    if (isExtra)
    {
        if (value == 0)
        {
            return;
        }
    }
    else if (value == existing)
    {
        return;
    }

    unsigned int planeCount = 1;

    if (isExtra && existing != 0)
    {
        planeCount = 2;
    }

    UpdateSectionDataSize(y >> 4, section, planeCount, isExtra);

    unsigned int elementIndex = GetElementIndexFromChunkXYZ(x, y, z);
    unsigned int elementByte = elementIndex >> 3;
    unsigned char bit = (unsigned char)(0x80 >> (elementIndex & 7));

    PalettisedSectionReference sectionRef((SubChunkHeader*)(mData + subChunkOffset * SUBCHUNK_PAGE_BYTES), section);
    unsigned char* planeData = sectionRef.getPlaneData();

    if (!isExtra)
    {
        for (unsigned int byteIndex = 0; byteIndex < SECTION_PLANE_BYTES; ++byteIndex)
        {
            planeData[byteIndex] = 0;
        }

        planeData[elementByte] = bit;

        sectionRef.setPaletteEntry(0, existing);
        sectionRef.setPaletteEntry(1, value);
    }
    else
    {
        unsigned char* extraPlaneData = sectionRef.getExtraPlaneData();

        for (unsigned int byteIndex = 0; byteIndex < SECTION_PLANE_BYTES; ++byteIndex)
        {
            planeData[byteIndex] = 0;
            extraPlaneData[byteIndex] = 0;
        }

        extraPlaneData[elementByte] = bit;

        if (planeCount == 2)
        {
            for (unsigned int byteIndex = SECTION_PLANE_BYTES; byteIndex < 2 * SECTION_PLANE_BYTES; ++byteIndex)
            {
                planeData[byteIndex] = 0;
                extraPlaneData[byteIndex] = 0xFF;
            }

            extraPlaneData[elementByte + SECTION_PLANE_BYTES] = (unsigned char)~bit;
        }

        sectionRef.setPaletteEntry(0, existing);
        sectionRef.setPaletteEntry(1, value);

        if (planeCount == 2)
        {
            sectionRef.setPaletteEntry(2, 0);
            sectionRef.setPaletteEntry(3, UNUSED_PALETTE_ENTRY);
        }
    }
}

void HierarchicalElementStorage::set_sectionRaw(unsigned int x, unsigned int y, unsigned int z, unsigned short value,
                                                unsigned int section, SubChunkHeader* header,
                                                unsigned int subChunkOffset, bool isExtra)
{
    if (isExtra && !header->getHasTwoIndices(section))
    {
        UpdateSectionDataSize(y >> 4, section, SECTION_TYPE_RAW, isExtra);

        header = (SubChunkHeader*)(mData + subChunkOffset * SUBCHUNK_PAGE_BYTES);

        RawSectionReference clearedRef(header, section);

        for (unsigned int element = 0; element < ELEMENTS_PER_SECTION; ++element)
        {
            clearedRef.setExtraElement(element, 0);
        }
    }

    RawSectionReference sectionRef(header, section);
    unsigned int elementIndex = GetElementIndexFromChunkXYZ(x, y, z);

    if (isExtra)
    {
        sectionRef.setExtraElement(elementIndex, value);
    }
    else
    {
        sectionRef.setElement(elementIndex, value);
    }
}

unsigned int HierarchicalElementStorage::getSingleEntryPaletteValue(PalettisedSectionReference& sectionRef,
                                                                    unsigned int subChunk, unsigned int section,
                                                                    bool isExtra)
{
    unsigned int planeCount = sectionRef.getPlaneCount();
    unsigned char* planeData = isExtra ? sectionRef.getExtraPlaneData() : sectionRef.getPlaneData();

    for (unsigned int plane = 0; plane < planeCount; ++plane)
    {
        unsigned char* bytes = planeData + plane * SECTION_PLANE_BYTES;
        unsigned char anySet = 0;
        unsigned char allSet = 0xFF;

        for (unsigned int byteIndex = 0; byteIndex < SECTION_PLANE_BYTES; ++byteIndex)
        {
            anySet = (unsigned char)(anySet | bytes[byteIndex]);
            allSet = (unsigned char)(allSet & bytes[byteIndex]);
        }

        if (anySet != 0 && allSet != 0xFF)
        {
            return 0xFFFFFFFF;
        }
    }

    unsigned int paletteIndex = 0;

    for (unsigned int plane = 0; plane < planeCount; ++plane)
    {
        if (planeData[plane * SECTION_PLANE_BYTES] == 0xFF)
        {
            paletteIndex |= 1u << plane;
        }
    }

    return sectionRef.getPaletteEntry(paletteIndex);
}

void HierarchicalElementStorage::removeSingleEntryPaletteSection(PalettisedSectionReference& sectionRef,
                                                                 unsigned int subChunk, unsigned int section,
                                                                 unsigned short value)
{
    UpdateSectionDataSize(subChunk, section, SECTION_TYPE_UNIFORM, false);

    SubChunkHeader* header = (SubChunkHeader*)(mData + mSubChunkInfo.getOffset(subChunk) * SUBCHUNK_PAGE_BYTES);
    header->setData(section, value);
}

void HierarchicalElementStorage::checkSubChunkStorageRequirements(unsigned int subChunk)
{
    SubChunkHeader* header = (SubChunkHeader*)(mData + mSubChunkInfo.getOffset(subChunk) * SUBCHUNK_PAGE_BYTES);
    unsigned int usedBytes = header->getUsedSize() * 4;
    unsigned int pages = (usedBytes + (SUBCHUNK_PAGE_BYTES - 1)) / SUBCHUNK_PAGE_BYTES;

    if (usedBytes == SUBCHUNK_HEADER_BYTES)
    {
        bool empty = true;

        for (unsigned int section = 0; section < SECTIONS_PER_SUBCHUNK; ++section)
        {
            if (header->getData(section) != 0)
            {
                empty = false;
                break;
            }
        }

        if (empty)
        {
            pages = 0;
        }
    }

    if (pages < mSubChunkInfo.getSize(subChunk))
    {
        ReduceSubchunkDataTo(subChunk, pages);
        ReduceSubchunkStorage();
    }
}

void HierarchicalElementStorage::checkForSingleEntryPalette(PalettisedSectionReference& sectionRef,
                                                            unsigned int subChunk, unsigned int section,
                                                            bool hasTwoIndices)
{
    bool removedSecondIndex = false;

    if (hasTwoIndices)
    {
        if (getSingleEntryPaletteValue(sectionRef, subChunk, section, true) != 0)
        {
            return;
        }

        SubChunkHeader* header = (SubChunkHeader*)(mData + mSubChunkInfo.getOffset(subChunk) * SUBCHUNK_PAGE_BYTES);
        unsigned int sectionType = header->getType(section);
        unsigned short offset = header->getOffset(section);

        UpdateSectionDataSize(subChunk, section, sectionType, false);
        header->setTypeAndOffset(section, (unsigned short)sectionType, offset, false);

        removedSecondIndex = true;
    }

    unsigned int value = getSingleEntryPaletteValue(sectionRef, subChunk, section, false);

    if (value != 0xFFFFFFFF && CanEncodeElementAs0Bit((unsigned short)value))
    {
        removeSingleEntryPaletteSection(sectionRef, subChunk, section, (unsigned short)value);
        checkSubChunkStorageRequirements(subChunk);
    }
    else if (removedSecondIndex)
    {
        checkSubChunkStorageRequirements(subChunk);
    }
}

void HierarchicalElementStorage::set_sectionPalettised(unsigned int x, unsigned int y, unsigned int z,
                                                       unsigned short value, unsigned int section,
                                                       SubChunkHeader* header, unsigned int subChunkOffset,
                                                       unsigned short planeCount, bool isExtra)
{
    unsigned int subChunk = y >> 4;
    bool addedSecondIndex = false;

    if (isExtra && !header->getHasTwoIndices(section))
    {
        if (value == 0)
        {
            return;
        }

        UpdateSectionDataSize(subChunk, section, header->getType(section), true);
        addedSecondIndex = true;
        header = (SubChunkHeader*)(mData + subChunkOffset * SUBCHUNK_PAGE_BYTES);
    }

    PalettisedSectionReference sectionRef(header, section);
    unsigned int paletteSize = sectionRef.getPaletteSize();

    unsigned int valueIndex = 0xFFFF;

    for (unsigned int entry = 0; entry < paletteSize; ++entry)
    {
        unsigned short current = sectionRef.getPaletteEntry(entry);

        if (current == UNUSED_PALETTE_ENTRY)
        {
            sectionRef.setPaletteEntry(entry, value);
            current = sectionRef.getPaletteEntry(entry);
        }

        if (current == value)
        {
            valueIndex = entry;
            break;
        }
    }

    unsigned int zeroIndex = 0xFFFF;

    if (addedSecondIndex)
    {
        for (unsigned int entry = 0; entry < paletteSize; ++entry)
        {
            unsigned short current = sectionRef.getPaletteEntry(entry);

            if (current == UNUSED_PALETTE_ENTRY)
            {
                sectionRef.setPaletteEntry(entry, 0);
                current = sectionRef.getPaletteEntry(entry);
            }

            if (current == 0)
            {
                zeroIndex = entry;
                break;
            }
        }
    }

    if (valueIndex != 0xFFFF && (!addedSecondIndex || zeroIndex != 0xFFFF))
    {
        unsigned int elementIndex = GetElementIndexFromChunkXYZ(x, y, z);
        unsigned char bit = (unsigned char)(0x80 >> (elementIndex & 7));
        unsigned char* planeData = isExtra ? sectionRef.getExtraPlaneData() : sectionRef.getPlaneData();

        if (addedSecondIndex)
        {
            for (unsigned int plane = 0; plane < planeCount; ++plane)
            {
                unsigned char fill = (zeroIndex & (1u << plane)) != 0 ? (unsigned char)0xFF : (unsigned char)0;

                for (unsigned int byteIndex = 0; byteIndex < SECTION_PLANE_BYTES; ++byteIndex)
                {
                    planeData[plane * SECTION_PLANE_BYTES + byteIndex] = fill;
                }
            }
        }

        bool uniformPlanes = true;

        for (unsigned int plane = 0; plane < planeCount; ++plane)
        {
            unsigned char* target = &planeData[plane * SECTION_PLANE_BYTES + (elementIndex >> 3)];
            unsigned char updated = (unsigned char)(*target & ~bit);

            if ((valueIndex & (1u << plane)) != 0)
            {
                updated = (unsigned char)(updated | bit);
            }

            *target = updated;

            if (updated != 0 && updated != 0xFF)
            {
                uniformPlanes = false;
            }
        }

        if (uniformPlanes)
        {
            checkForSingleEntryPalette(sectionRef, subChunk, section, header->getHasTwoIndices(section));
        }

        return;
    }

    if (planeCount == SECTION_TYPE_MAX_PLANE_COUNT)
    {
        bool hasTwoIndices = header->getHasTwoIndices(section);
        unsigned int baseX = x & 0xC;
        unsigned int baseY = y & 0xFC;
        unsigned int baseZ = z & 0xC;
        unsigned short elements[ELEMENTS_PER_SECTION];
        unsigned short extraElements[ELEMENTS_PER_SECTION];

        for (unsigned int element = 0; element < ELEMENTS_PER_SECTION; ++element)
        {
            unsigned int elementX = baseX + (element >> 4);
            unsigned int elementY = baseY + ((element >> 2) & 3);
            unsigned int elementZ = baseZ + (element & 3);

            elements[element] = get(elementX, elementY, elementZ, NULL);

            if (hasTwoIndices)
            {
                extraElements[element] = addedSecondIndex ? (unsigned short)0
                                                          : getExtraElement(elementX, elementY, elementZ);
            }
        }

        UpdateSectionDataSize(subChunk, section, SECTION_TYPE_RAW, hasTwoIndices);

        for (unsigned int element = 0; element < ELEMENTS_PER_SECTION; ++element)
        {
            unsigned int elementX = baseX + (element >> 4);
            unsigned int elementY = baseY + ((element >> 2) & 3);
            unsigned int elementZ = baseZ + (element & 3);

            set(elementX, elementY, elementZ, elements[element]);

            if (hasTwoIndices)
            {
                setExtraElement(elementX, elementY, elementZ, extraElements[element]);
            }
        }
    }
    else
    {
        bool hasTwoIndices = header->getHasTwoIndices(section);
        unsigned int newPlaneCount = planeCount + 1;

        UpdateSectionDataSize(subChunk, section, newPlaneCount, hasTwoIndices);

        SubChunkHeader* newHeader = (SubChunkHeader*)(mData + subChunkOffset * SUBCHUNK_PAGE_BYTES);
        PalettisedSectionReference newSectionRef(newHeader, section);

        unsigned char* newPlaneData = newSectionRef.getPlaneData();
        unsigned char* newExtraPlaneData = NULL;

        if (newHeader->getHasTwoIndices(section))
        {
            newExtraPlaneData = newSectionRef.getExtraPlaneData();
        }

        unsigned int newPaletteSize = newSectionRef.getPaletteSize();

        newSectionRef.setPlaneCount(newPlaneCount - 1);

        unsigned int oldPaletteSize = newSectionRef.getPaletteSize();
        unsigned int oldPlaneDataSize = newSectionRef.getPlaneDataSize();
        unsigned char* oldPlaneData = newSectionRef.getPlaneData();

        if (newHeader->getHasTwoIndices(section))
        {
            memmove(newExtraPlaneData, newSectionRef.getExtraPlaneData(), oldPlaneDataSize);
        }

        memmove(newPlaneData, oldPlaneData, oldPlaneDataSize);

        newSectionRef.setPlaneCount(newPlaneCount);

        for (unsigned int entry = oldPaletteSize; entry < newPaletteSize; ++entry)
        {
            newSectionRef.setPaletteEntry(entry, UNUSED_PALETTE_ENTRY);
        }

        for (unsigned int byteIndex = 0; byteIndex < SECTION_PLANE_BYTES; ++byteIndex)
        {
            newPlaneData[(newPlaneCount - 1) * SECTION_PLANE_BYTES + byteIndex] = 0;
        }

        if (addedSecondIndex)
        {
            for (unsigned int entry = 0; entry < newPaletteSize; ++entry)
            {
                unsigned short current = newSectionRef.getPaletteEntry(entry);

                if (current == UNUSED_PALETTE_ENTRY)
                {
                    newSectionRef.setPaletteEntry(entry, 0);
                    current = newSectionRef.getPaletteEntry(entry);
                }

                if (current == 0)
                {
                    zeroIndex = entry;
                    break;
                }
            }

            for (unsigned int plane = 0; plane < newPlaneCount; ++plane)
            {
                unsigned char fill = (zeroIndex & (1u << plane)) != 0 ? (unsigned char)0xFF : (unsigned char)0;

                for (unsigned int byteIndex = 0; byteIndex < SECTION_PLANE_BYTES; ++byteIndex)
                {
                    newExtraPlaneData[plane * SECTION_PLANE_BYTES + byteIndex] = fill;
                }
            }
        }
        else if (newHeader->getHasTwoIndices(section))
        {
            for (unsigned int byteIndex = 0; byteIndex < SECTION_PLANE_BYTES; ++byteIndex)
            {
                newExtraPlaneData[(newPlaneCount - 1) * SECTION_PLANE_BYTES + byteIndex] = 0;
            }
        }
    }

    if (isExtra)
    {
        setExtraElement(x, y, z, value);
    }
    else
    {
        set(x, y, z, value);
    }
}

void HierarchicalElementStorage::set(unsigned int x, unsigned int y, unsigned int z, unsigned short value)
{
    unsigned int subChunk = y >> 4;

    if (mSubChunkInfo.getSize(subChunk) == 0)
    {
        if (value != 0)
        {
            set_subChunkEmpty(x, y, z, value, false);
        }

        return;
    }

    unsigned int subChunkOffset = mSubChunkInfo.getOffset(subChunk);
    SubChunkHeader* header = (SubChunkHeader*)(mData + subChunkOffset * SUBCHUNK_PAGE_BYTES);
    unsigned int section = GetSectionIndexFromChunkXYZ(x, y, z);
    unsigned short sectionType = header->getType(section);

    if (sectionType == SECTION_TYPE_UNIFORM)
    {
        set_section0Bit(x, y, z, value, section, header, subChunkOffset, false);
    }
    else if (sectionType == SECTION_TYPE_RAW)
    {
        set_sectionRaw(x, y, z, value, section, header, subChunkOffset, false);
    }
    else
    {
        set_sectionPalettised(x, y, z, value, section, header, subChunkOffset, sectionType, false);
    }
}

void HierarchicalElementStorage::setExtraElement(unsigned int x, unsigned int y, unsigned int z, unsigned short value)
{
    unsigned int subChunk = y >> 4;

    if (mSubChunkInfo.getSize(subChunk) == 0)
    {
        if (value != 0)
        {
            set_subChunkEmpty(x, y, z, value, true);
        }

        return;
    }

    unsigned int subChunkOffset = mSubChunkInfo.getOffset(subChunk);
    SubChunkHeader* header = (SubChunkHeader*)(mData + subChunkOffset * SUBCHUNK_PAGE_BYTES);
    unsigned int section = GetSectionIndexFromChunkXYZ(x, y, z);
    unsigned short sectionType = header->getType(section);

    if (sectionType == SECTION_TYPE_UNIFORM)
    {
        set_section0Bit(x, y, z, value, section, header, subChunkOffset, true);
    }
    else if (sectionType == SECTION_TYPE_RAW)
    {
        set_sectionRaw(x, y, z, value, section, header, subChunkOffset, true);
    }
    else
    {
        set_sectionPalettised(x, y, z, value, section, header, subChunkOffset, sectionType, true);
    }
}

int HierarchicalElementStorage::setDataRegion(arrayWithLength<unsigned char> data, int x0, int y0, int z0, int x1,
                                              int y1, int z1, int offset,
                                              void (*changedCallback)(int, int, int, void*, int), void* callbackData,
                                              int callbackValue)
{
    unsigned short* elements = (unsigned short*)(data.data + offset);
    unsigned short* source = elements + (x1 - x0) * (y1 - y0) * (z1 - z0);

    for (int x = x0; x < x1; ++x)
    {
        for (int z = z0; z < z1; ++z)
        {
            for (int y = y0; y < y1; ++y)
            {
                setExtraElement(x, y, z, *source);
                ++source;
            }
        }
    }

    source = elements;

    if (changedCallback == NULL)
    {
        for (int x = x0; x < x1; ++x)
        {
            for (int z = z0; z < z1; ++z)
            {
                for (int y = y0; y < y1; ++y)
                {
                    set(x, y, z, *source);
                    ++source;
                }
            }
        }
    }
    else
    {
        for (int x = x0; x < x1; ++x)
        {
            for (int z = z0; z < z1; ++z)
            {
                for (int y = y0; y < y1; ++y)
                {
                    if (get(x, y, z, NULL) != *source)
                    {
                        set(x, y, z, *source);
                        (*changedCallback)(x, y, z, callbackData, callbackValue);
                    }

                    ++source;
                }
            }
        }
    }

    return (int)((unsigned char*)source - (unsigned char*)elements);
}

int HierarchicalElementStorage::getDataRegion(arrayWithLength<unsigned char> data, int x0, int y0, int z0, int x1,
                                              int y1, int z1, int offset)
{
    unsigned short* destination = (unsigned short*)(data.data + offset);
    unsigned short* elements = destination;

    for (int x = x0; x < x1; ++x)
    {
        for (int z = z0; z < z1; ++z)
        {
            for (int y = y0; y < y1; ++y)
            {
                *destination = get(x, y, z, NULL);
                ++destination;
            }
        }
    }

    for (int x = x0; x < x1; ++x)
    {
        for (int z = z0; z < z1; ++z)
        {
            for (int y = y0; y < y1; ++y)
            {
                *destination = getExtraElement(x, y, z);
                ++destination;
            }
        }
    }

    return (int)((unsigned char*)destination - (unsigned char*)elements);
}

bool HierarchicalElementStorage::sectionContainsElementData(int x, int y, int z, unsigned short value,
                                                            unsigned short mask)
{
    int subChunk = y / (int)SUBCHUNK_COUNT;

    if (mSubChunkInfo.getSize(subChunk) == 0)
    {
        return value == 0;
    }

    unsigned int section = GetSectionIndexFromChunkXYZ(x, y, z);
    SubChunkHeader* header = (SubChunkHeader*)(mData + mSubChunkInfo.getOffset(subChunk) * SUBCHUNK_PAGE_BYTES);
    unsigned int sectionType = header->getType(section);

    if (sectionType == SECTION_TYPE_UNIFORM)
    {
        return (header->getData(section) & mask) == value;
    }

    if (sectionType == SECTION_TYPE_RAW)
    {
        RawSectionReference sectionRef(header, section);

        for (unsigned int element = 0; element < ELEMENTS_PER_SECTION; ++element)
        {
            if ((sectionRef.getElement(element) & mask) == value)
            {
                return true;
            }
        }
    }
    else
    {
        PalettisedSectionReference sectionRef(header, section);
        int paletteSize = sectionRef.getPaletteSize();

        for (int entry = 0; entry < paletteSize; ++entry)
        {
            if ((sectionRef.getPaletteEntry(entry) & mask) == value)
            {
                return true;
            }
        }
    }

    return false;
}

int HierarchicalElementStorage::getAllocatedBytes()
{
    return mSubChunkCount * SUBCHUNK_PAGE_BYTES;
}

int HierarchicalElementStorage::getRawData(unsigned char* rawData)
{
    mSubChunkInfo.getRawData(rawData);

    unsigned int infoBytes = SubChunkInfo::getRawDataBytes();
    unsigned int subChunkBytes = mSubChunkCount * SUBCHUNK_PAGE_BYTES;

    memcpy(rawData + infoBytes, mData, subChunkBytes);

    return (int)(subChunkBytes + infoBytes);
}

void HierarchicalElementStorage::setFromRawData(unsigned char* rawData, unsigned int rawDataBytes)
{
    unsigned int infoBytes = SubChunkInfo::getRawDataBytes();

    sTotalSubChunkBytes -= mSubChunkCount * SUBCHUNK_PAGE_BYTES;
    mSubChunkCount = (rawDataBytes - infoBytes) / SUBCHUNK_PAGE_BYTES;
    freeData(mData);
    mData = (unsigned char*)allocData(rawDataBytes - infoBytes);
    sTotalSubChunkBytes += mSubChunkCount * SUBCHUNK_PAGE_BYTES;

    mSubChunkInfo.setFromRawData(rawData);
    memcpy(mData, rawData + infoBytes, mSubChunkCount * SUBCHUNK_PAGE_BYTES);
}

int HierarchicalElementStorage::getRawDataMaxBytes()
{
    return (int)(SubChunkInfo::getRawDataBytes() + SUBCHUNK_DATA_MAX_BYTES);
}

void HierarchicalElementStorage::write(DataOutputStream* out)
{
    out->writeUnsignedShort((unsigned short)mSubChunkCount);
    mSubChunkInfo.write(out);

    if (mSubChunkCount != 0)
    {
        arrayWithLength<unsigned char> subChunkData;
        subChunkData.data = mData;
        subChunkData.length = mSubChunkCount * SUBCHUNK_PAGE_BYTES;
        out->write(subChunkData);
    }
}

void HierarchicalElementStorage::read(DataInputStream* in)
{
    sTotalSubChunkBytes -= mSubChunkCount * SUBCHUNK_PAGE_BYTES;
    mSubChunkCount = in->readUnsignedShort();
    freeData(mData);
    mData = (unsigned char*)allocData(mSubChunkCount * SUBCHUNK_PAGE_BYTES);
    sTotalSubChunkBytes += mSubChunkCount * SUBCHUNK_PAGE_BYTES;

    mSubChunkInfo.read(in);

    arrayWithLength<unsigned char> subChunkData;
    subChunkData.data = mData;
    subChunkData.length = mSubChunkCount * SUBCHUNK_PAGE_BYTES;
    in->readFully(subChunkData);
}
