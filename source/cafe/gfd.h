#ifndef CAFE_GFD_H
#define CAFE_GFD_H

#include <cafe/gx2.h>

#ifdef __cplusplus
extern "C"
{
#endif

#define GFD_SIGNATURE 0x47667832
#define GFD_BLOCK_SIGNATURE 0x424C4B7B
#define GFD_BLOCK_RELOCATION_SIGNATURE 0x7D424C4B

#define GFD_HEADER_MAJOR_VERSION 7
#define GFD_HEADER_MINOR_VERSION 1
#define GFD_BLOCK_HEADER_MAJOR_VERSION 1

#define GFD_TAG_MASK 0x000FFFFF

typedef enum _GFDBlockType
{
    GFD_BLOCK_TYPE_END = 1,
    GFD_BLOCK_TYPE_PAD = 2,
    GFD_BLOCK_TYPE_GX2_VSH_HEADER = 3,
    GFD_BLOCK_TYPE_GX2_VSH_PROGRAM = 5,
    GFD_BLOCK_TYPE_GX2_PSH_HEADER = 6,
    GFD_BLOCK_TYPE_GX2_PSH_PROGRAM = 7,
    GFD_BLOCK_TYPE_GX2_GSH_HEADER = 8,
    GFD_BLOCK_TYPE_GX2_GSH_PROGRAM = 9,
    GFD_BLOCK_TYPE_GX2_GSH_COPY_PROGRAM = 10,
    GFD_BLOCK_TYPE_GX2_TEX_HEADER = 11,
    GFD_BLOCK_TYPE_GX2_TEX_IMAGE_DATA = 12,
    GFD_BLOCK_TYPE_GX2_TEX_MIP_DATA = 13,
    GFD_BLOCK_TYPE_GX2_CSH_HEADER = 14,
    GFD_BLOCK_TYPE_GX2_CSH_PROGRAM = 15
} GFDBlockType;

typedef enum _GFDGPUVersion
{
    GFD_GPU_VERSION_0 = 0,
    GFD_GPU_VERSION_1 = 1,
    GFD_GPU_VERSION_GPU7 = 2
} GFDGPUVersion;

typedef enum _GFDAlignMode
{
    GFD_ALIGN_MODE_DISABLE = 0,
    GFD_ALIGN_MODE_ENABLE = 1
} GFDAlignMode;

typedef struct _GFDHeader
{
    u32 magic;
    u32 size;
    u32 majorVersion;
    u32 minorVersion;
    GFDGPUVersion gpuVersion;
    GFDAlignMode alignMode;
    u32 reserved1;
    u32 reserved2;
} GFDHeader;

typedef struct _GFDBlockHeader
{
    u32 magic;
    u32 size;
    u32 majorVersion;
    u32 minorVersion;
    GFDBlockType type;
    u32 dataSize;
    u32 id;
    u32 typeIdx;
} GFDBlockHeader;

typedef struct _GFDBlockRelocationHeader
{
    u32 magic;
    u32 size;
    u32 majorVersion;
    u32 dataSize;
    u32 dataOffset;
    u32 textSize;
    u32 textOffset;
    u32 basePatchAddress;
    u32 patchTableSize;
    u32 patchTableOffset;
} GFDBlockRelocationHeader;

u32 GFDGetVertexShaderCount(const void* data);
u32 GFDGetVertexShaderHeaderSize(u32 index, const void* data);
u32 GFDGetVertexShaderProgramSize(u32 index, const void* data);
GX2Boolean GFDGetVertexShader(GX2VertexShader* header, void* program, u32 index, const void* data);
u32 GFDGetPixelShaderCount(const void* data);
u32 GFDGetPixelShaderHeaderSize(u32 index, const void* data);
u32 GFDGetPixelShaderProgramSize(u32 index, const void* data);
GX2Boolean GFDGetPixelShader(GX2PixelShader* header, void* program, u32 index, const void* data);

u32 GFDCleanTag(u32 tag);

#ifdef __cplusplus
}
#endif

#endif
