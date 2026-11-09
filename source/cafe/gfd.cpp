#include <cafe/gfd.h>

#include <coreinit/cosreport.h>

#include <string.h>

extern "C"
{
u32 GX2TempGetGPUVersion(void);

GX2Boolean _GFDGetHeaderVersions(u32* majorVersion, u32* minorVersion, u32* gpuVersion, const void* data)
{
    const GFDHeader* header = static_cast<const GFDHeader*>(data);

    *majorVersion = 0;
    *minorVersion = 0;
    *gpuVersion = 0;

    if (header->magic != GFD_SIGNATURE)
        return GX2_FALSE;

    *majorVersion = header->majorVersion;
    *minorVersion = header->minorVersion;
    *gpuVersion = header->gpuVersion;
    return GX2_TRUE;
}

GX2Boolean _GFDCheckHeaderVersions(const void* data)
{
    u32 majorVersion;
    u32 minorVersion;
    u32 gpuVersion;

    if (data == NULL)
        return GX2_FALSE;

    if (!_GFDGetHeaderVersions(&majorVersion, &minorVersion, &gpuVersion, data))
        return GX2_FALSE;

    if (majorVersion != GFD_HEADER_MAJOR_VERSION)
        return GX2_FALSE;

    if (minorVersion > GFD_HEADER_MINOR_VERSION)
    {
        COSWarn(COS_REPORT_MODULE_UNKNOWN_1, "Warning: File minor version is greater than that of code.");
        return GX2_FALSE;
    }

    if (GX2TempGetGPUVersion() != gpuVersion)
        return GX2_FALSE;

    return GX2_TRUE;
}

GX2Boolean _GFDCheckBlockHeaderMagicVersions(const GFDBlockHeader* blockHeader)
{
    if (blockHeader->magic != GFD_BLOCK_SIGNATURE)
    {
        COSWarn(COS_REPORT_MODULE_UNKNOWN_1, "Warning: Unknown block format. \n");
        return GX2_FALSE;
    }

    if (blockHeader->majorVersion != GFD_BLOCK_HEADER_MAJOR_VERSION)
    {
        COSWarn(COS_REPORT_MODULE_UNKNOWN_1, "Warning: Unsupported block version %d. \n", blockHeader->majorVersion);
        return GX2_FALSE;
    }

    return GX2_TRUE;
}

u32 _GFDGetBlockCount(GFDBlockType blockType, const void* data)
{
    const u8* blockData;
    const GFDBlockHeader* blockHeader;
    u32 count = 0;

    if (data == NULL || !_GFDCheckHeaderVersions(data))
        return 0;

    blockData = static_cast<const u8*>(data) + static_cast<const GFDHeader*>(data)->size;
    blockHeader = reinterpret_cast<const GFDBlockHeader*>(blockData);
    while (_GFDCheckBlockHeaderMagicVersions(blockHeader))
    {
        blockHeader = reinterpret_cast<const GFDBlockHeader*>(blockData);
        if (blockType == blockHeader->type)
            count++;

        blockData = blockData + blockHeader->size + blockHeader->dataSize;
        if (blockHeader->type == GFD_BLOCK_TYPE_END)
            break;
    }

    return count;
}

u32 GFDGetVertexShaderCount(const void* data)
{
    return _GFDGetBlockCount(GFD_BLOCK_TYPE_GX2_VSH_HEADER, data);
}

u32 GFDGetPixelShaderCount(const void* data)
{
    return _GFDGetBlockCount(GFD_BLOCK_TYPE_GX2_PSH_HEADER, data);
}

u32 _GFDGetBlockDataSize(GFDBlockType blockType, u32 index, const void* data)
{
    const u8* blockData;
    const GFDBlockHeader* blockHeader;
    u32 count = 0;

    if (data == NULL || !_GFDCheckHeaderVersions(data))
        return 0;

    blockData = static_cast<const u8*>(data) + static_cast<const GFDHeader*>(data)->size;
    blockHeader = reinterpret_cast<const GFDBlockHeader*>(blockData);
    while (_GFDCheckBlockHeaderMagicVersions(blockHeader))
    {
        blockHeader = reinterpret_cast<const GFDBlockHeader*>(blockData);
        if (blockType == blockHeader->type)
        {
            if (index == count)
                return blockHeader->dataSize;
            count++;
        }

        blockData = blockData + blockHeader->size + blockHeader->dataSize;
        if (blockHeader->type == GFD_BLOCK_TYPE_END)
            break;
    }

    return 0;
}

u32 GFDGetVertexShaderHeaderSize(u32 index, const void* data)
{
    return _GFDGetBlockDataSize(GFD_BLOCK_TYPE_GX2_VSH_HEADER, index, data);
}

u32 GFDGetPixelShaderHeaderSize(u32 index, const void* data)
{
    return _GFDGetBlockDataSize(GFD_BLOCK_TYPE_GX2_PSH_HEADER, index, data);
}

u32 GFDGetVertexShaderProgramSize(u32 index, const void* data)
{
    return _GFDGetBlockDataSize(GFD_BLOCK_TYPE_GX2_VSH_PROGRAM, index, data);
}

u32 GFDGetPixelShaderProgramSize(u32 index, const void* data)
{
    return _GFDGetBlockDataSize(GFD_BLOCK_TYPE_GX2_PSH_PROGRAM, index, data);
}

u32 GFDCleanTag(u32 tag)
{
    return tag & GFD_TAG_MASK;
}

GX2Boolean _GFDRelocateBlockEx(GFDBlockRelocationHeader* relocationHeader, u32 fromOffset, u32 toOffset, u8* data)
{
    u32 patchTableOffset = GFDCleanTag(relocationHeader->patchTableOffset);
    u32 patchTableSize = relocationHeader->patchTableSize;

    if (data == NULL)
        return GX2_FALSE;

    const u32* patchTable = reinterpret_cast<const u32*>(data + patchTableOffset);
    for (u32 i = 0; i < patchTableSize; i++)
    {
        if (patchTable[i] != 0)
        {
            u32* patch = reinterpret_cast<u32*>(data + GFDCleanTag(patchTable[i]));
            *patch = GFDCleanTag(*patch) - fromOffset + toOffset;
        }
    }

    relocationHeader->basePatchAddress = toOffset;
    return GX2_TRUE;
}

GX2Boolean _GFDRelocateBlock(u32 blockDataSize, u8* blockData)
{
    if (blockData == NULL)
        return GX2_FALSE;

    GFDBlockRelocationHeader* relocationHeader = reinterpret_cast<GFDBlockRelocationHeader*>(
        blockData + blockDataSize - sizeof(GFDBlockRelocationHeader));
    if (relocationHeader->basePatchAddress != 0)
        return GX2_TRUE;

    return _GFDRelocateBlockEx(relocationHeader, 0, reinterpret_cast<u32>(blockData), blockData);
}

GX2Boolean GFDGetVertexShader(GX2VertexShader* header, void* program, u32 index, const void* data)
{
    const u8* blockData;
    const GFDBlockHeader* blockHeader;
    GX2Boolean valid;
    u32 headerCount = 0;
    u32 programCount = 0;

    if (header == NULL || program == NULL || data == NULL || !_GFDCheckHeaderVersions(data))
        return GX2_FALSE;

    if ((reinterpret_cast<u32>(program) & (GX2_SHADER_ALIGNMENT - 1)) != 0)
    {
        COSWarn(COS_REPORT_MODULE_UNKNOWN_1,
                "Warning: Shader program buffer not aligned correctly. It needs %d "
                "byte align buffer.\n",
                GX2_SHADER_ALIGNMENT);
        return GX2_FALSE;
    }

    blockData = static_cast<const u8*>(data) + static_cast<const GFDHeader*>(data)->size;
    blockHeader = reinterpret_cast<const GFDBlockHeader*>(blockData);
    while ((valid = _GFDCheckBlockHeaderMagicVersions(blockHeader)) != GX2_FALSE)
    {
        blockHeader = reinterpret_cast<const GFDBlockHeader*>(blockData);
        blockData += blockHeader->size;

        switch (blockHeader->type)
        {
        case GFD_BLOCK_TYPE_GX2_VSH_HEADER:
            if (index == headerCount)
            {
                memcpy(header, blockData, blockHeader->dataSize);
                if (!_GFDRelocateBlock(blockHeader->dataSize, reinterpret_cast<u8*>(header)))
                    return GX2_FALSE;
            }
            headerCount++;
            break;
        case GFD_BLOCK_TYPE_GX2_VSH_PROGRAM:
            if (index == programCount)
            {
                header->shaderPtr = program;
                memcpy(program, blockData, blockHeader->dataSize);
            }
            programCount++;
            break;
        default:
            break;
        }

        blockData += blockHeader->dataSize;
        if ((headerCount > index && programCount > index) || blockHeader->type == GFD_BLOCK_TYPE_END)
            break;
    }

    return (valid && headerCount >= index && programCount >= index) ? GX2_TRUE : GX2_FALSE;
}

GX2Boolean GFDGetPixelShader(GX2PixelShader* header, void* program, u32 index, const void* data)
{
    const u8* blockData;
    const GFDBlockHeader* blockHeader;
    GX2Boolean valid;
    u32 headerCount = 0;
    u32 programCount = 0;

    if (header == NULL || program == NULL || data == NULL || !_GFDCheckHeaderVersions(data))
        return GX2_FALSE;

    if ((reinterpret_cast<u32>(program) & (GX2_SHADER_ALIGNMENT - 1)) != 0)
    {
        COSWarn(COS_REPORT_MODULE_UNKNOWN_1,
                "Warning: Shader program buffer not aligned correctly. It needs %d "
                "byte align buffer.\n",
                GX2_SHADER_ALIGNMENT);
        return GX2_FALSE;
    }

    blockData = static_cast<const u8*>(data) + static_cast<const GFDHeader*>(data)->size;
    blockHeader = reinterpret_cast<const GFDBlockHeader*>(blockData);
    while ((valid = _GFDCheckBlockHeaderMagicVersions(blockHeader)) != GX2_FALSE)
    {
        blockHeader = reinterpret_cast<const GFDBlockHeader*>(blockData);
        blockData += blockHeader->size;

        switch (blockHeader->type)
        {
        case GFD_BLOCK_TYPE_GX2_PSH_HEADER:
            if (index == headerCount)
            {
                memcpy(header, blockData, blockHeader->dataSize);
                if (!_GFDRelocateBlock(blockHeader->dataSize, reinterpret_cast<u8*>(header)))
                    return GX2_FALSE;
            }
            headerCount++;
            break;
        case GFD_BLOCK_TYPE_GX2_PSH_PROGRAM:
            if (index == programCount)
            {
                header->shaderPtr = program;
                memcpy(program, blockData, blockHeader->dataSize);
            }
            programCount++;
            break;
        default:
            break;
        }

        blockData += blockHeader->dataSize;
        if ((headerCount > index && programCount > index) || blockHeader->type == GFD_BLOCK_TYPE_END)
            break;
    }

    return (valid && headerCount >= index && programCount >= index) ? GX2_TRUE : GX2_FALSE;
}
}
