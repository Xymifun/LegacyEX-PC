#ifndef CAFE_GX2_GX2TYPES_H
#define CAFE_GX2_GX2TYPES_H

#include <nn/types.h>

typedef enum _GX2Boolean
{
    GX2_FALSE = 0,
    GX2_TRUE = 1,
    GX2_DISABLE = GX2_FALSE,
    GX2_ENABLE = GX2_TRUE
} GX2Boolean;

typedef enum _GX2InitAttrib
{
    GX2_INIT_ATTRIB_NULL = 0,
    GX2_INIT_ATTRIB_CB_BASE = 1,
    GX2_INIT_ATTRIB_CB_SIZE = 2,
    GX2_INIT_ATTRIB_ARGC = 7,
    GX2_INIT_ATTRIB_ARGV = 8,
    GX2_INIT_ATTRIB_PROFILE_MODE = 9,
    GX2_INIT_ATTRIB_TOSS_STAGE = 10
} GX2InitAttrib;

#define GX2_SHADER_ALIGNMENT 0x100
#define GX2_CONTEXT_STATE_ALIGNMENT 0x100
#define GX2_VERTEX_BUFFER_ALIGNMENT 0x40
#define GX2_INDEX_BUFFER_ALIGNMENT 0x20
#define GX2_UNIFORM_BLOCK_ALIGNMENT 0x100

#define GX2_UNIFORM_VAR_INVALID_OFFSET (-1)
#define GX2_UNIFORM_BLOCK_INVALID_LOCATION (-1)

typedef enum _GX2AAMode
{
    GX2_AA_MODE_1X = 0,
    GX2_AA_MODE_2X = 1,
    GX2_AA_MODE_4X = 2,
    GX2_AA_MODE_8X = 3
} GX2AAMode;

typedef enum _GX2SurfaceDim
{
    GX2_SURFACE_DIM_1D = 0,
    GX2_SURFACE_DIM_2D = 1,
    GX2_SURFACE_DIM_3D = 2,
    GX2_SURFACE_DIM_CUBE = 3
} GX2SurfaceDim;

typedef enum _GX2SurfaceFormat
{
    GX2_SURFACE_FORMAT_INVALID = 0x000,
    GX2_SURFACE_FORMAT_TC_R8_UNORM = 0x001,
    GX2_SURFACE_FORMAT_TCD_R16_UNORM = 0x005,
    GX2_SURFACE_FORMAT_TC_R8_G8_UNORM = 0x007,
    GX2_SURFACE_FORMAT_D_D24_S8_UNORM = 0x011,
    GX2_SURFACE_FORMAT_TCS_R8_G8_B8_A8_UNORM = 0x01a,
    GX2_SURFACE_FORMAT_TC_R16_G16_B16_A16_UINT = 0x11f,
    GX2_SURFACE_FORMAT_TC_R32_G32_B32_A32_UINT = 0x122,
    GX2_SURFACE_FORMAT_T_BC1_UNORM = 0x031,
    GX2_SURFACE_FORMAT_T_BC3_UNORM = 0x033,
    GX2_SURFACE_FORMAT_TCS_R8_G8_B8_A8_SRGB = 0x41a,
    GX2_SURFACE_FORMAT_T_BC1_SRGB = 0x431,
    GX2_SURFACE_FORMAT_T_BC3_SRGB = 0x433,
    GX2_SURFACE_FORMAT_TCD_R32_FLOAT = 0x80e,
    GX2_SURFACE_FORMAT_D_D24_S8_FLOAT = 0x811,
    GX2_SURFACE_FORMAT_TC_R16_G16_B16_A16_FLOAT = 0x820
} GX2SurfaceFormat;

typedef enum _GX2SurfaceUse
{
    GX2_SURFACE_USE_TEXTURE = 0x1,
    GX2_SURFACE_USE_COLOR_BUFFER = 0x2,
    GX2_SURFACE_USE_DEPTH_BUFFER = 0x4,
    GX2_SURFACE_USE_SCAN_BUFFER = 0x8,
    GX2_SURFACE_USE_FTV = 0x80000000,
    GX2_SURFACE_USE_COLOR_BUFFER_TEXTURE = GX2_SURFACE_USE_COLOR_BUFFER | GX2_SURFACE_USE_TEXTURE,
    GX2_SURFACE_USE_DEPTH_BUFFER_TEXTURE = GX2_SURFACE_USE_DEPTH_BUFFER | GX2_SURFACE_USE_TEXTURE,
    GX2_SURFACE_USE_COLOR_BUFFER_FTV = GX2_SURFACE_USE_FTV | GX2_SURFACE_USE_COLOR_BUFFER,
    GX2_SURFACE_USE_COLOR_BUFFER_TEXTURE_FTV = GX2_SURFACE_USE_FTV | GX2_SURFACE_USE_COLOR_BUFFER_TEXTURE
} GX2SurfaceUse;

typedef enum _GX2TileMode
{
    GX2_TILE_MODE_DEFAULT = 0,
    GX2_TILE_MODE_LINEAR_ALIGNED = 1,
    GX2_TILE_MODE_1D_TILED_THIN1 = 2,
    GX2_TILE_MODE_1D_TILED_THICK = 3,
    GX2_TILE_MODE_2D_TILED_THIN1 = 4,
    GX2_TILE_MODE_2D_TILED_THIN2 = 5,
    GX2_TILE_MODE_2D_TILED_THIN4 = 6,
    GX2_TILE_MODE_2D_TILED_THICK = 7,
    GX2_TILE_MODE_2B_TILED_THIN1 = 8,
    GX2_TILE_MODE_2B_TILED_THIN2 = 9,
    GX2_TILE_MODE_2B_TILED_THIN4 = 10,
    GX2_TILE_MODE_2B_TILED_THICK = 11,
    GX2_TILE_MODE_3D_TILED_THIN1 = 12,
    GX2_TILE_MODE_3D_TILED_THICK = 13,
    GX2_TILE_MODE_3B_TILED_THIN1 = 14,
    GX2_TILE_MODE_3B_TILED_THICK = 15,
    GX2_TILE_MODE_LINEAR_SPECIAL = 16
} GX2TileMode;

typedef enum _GX2CompSel
{
    GX2_COMP_SEL_NONE = 0x04040405,
    GX2_COMP_SEL_X001 = 0x00040405,
    GX2_COMP_SEL_XY00 = 0x00010404,
    GX2_COMP_SEL_XY01 = 0x00010405,
    GX2_COMP_SEL_XYZ1 = 0x00010205,
    GX2_COMP_SEL_XYZW = 0x00010203,
    GX2_COMP_SEL_WZYX = 0x03020100
} GX2CompSel;

typedef enum _GX2InvalidateType
{
    GX2_INVALIDATE_ATTRIB_BUFFER = 0x01,
    GX2_INVALIDATE_TEXTURE = 0x02,
    GX2_INVALIDATE_UNIFORM_BLOCK = 0x04,
    GX2_INVALIDATE_SHADER = 0x08,
    GX2_INVALIDATE_COLOR_BUFFER = 0x10,
    GX2_INVALIDATE_DEPTH_BUFFER = 0x20,
    GX2_INVALIDATE_CPU = 0x40,
    GX2_INVALIDATE_CPU_ATTRIB_BUFFER = GX2_INVALIDATE_CPU | GX2_INVALIDATE_ATTRIB_BUFFER,
    GX2_INVALIDATE_CPU_TEXTURE = GX2_INVALIDATE_CPU | GX2_INVALIDATE_TEXTURE,
    GX2_INVALIDATE_CPU_UNIFORM_BLOCK = GX2_INVALIDATE_CPU | GX2_INVALIDATE_UNIFORM_BLOCK,
    GX2_INVALIDATE_CPU_SHADER = GX2_INVALIDATE_CPU | GX2_INVALIDATE_SHADER
} GX2InvalidateType;

typedef enum _GX2CompareFunction
{
    GX2_COMPARE_NEVER = 0,
    GX2_COMPARE_LESS = 1,
    GX2_COMPARE_EQUAL = 2,
    GX2_COMPARE_LEQUAL = 3,
    GX2_COMPARE_GREATER = 4,
    GX2_COMPARE_NOTEQUAL = 5,
    GX2_COMPARE_GEQUAL = 6,
    GX2_COMPARE_ALWAYS = 7
} GX2CompareFunction;

typedef enum _GX2BlendFunction
{
    GX2_BLEND_ZERO = 0,
    GX2_BLEND_ONE = 1,
    GX2_BLEND_SRC_COLOR = 2,
    GX2_BLEND_ONE_MINUS_SRC_COLOR = 3,
    GX2_BLEND_SRC_ALPHA = 4,
    GX2_BLEND_ONE_MINUS_SRC_ALPHA = 5,
    GX2_BLEND_DST_ALPHA = 6,
    GX2_BLEND_ONE_MINUS_DST_ALPHA = 7,
    GX2_BLEND_DST_COLOR = 8,
    GX2_BLEND_ONE_MINUS_DST_COLOR = 9
} GX2BlendFunction;

typedef enum _GX2BlendCombine
{
    GX2_BLEND_COMBINE_ADD = 0,
    GX2_BLEND_COMBINE_SRC_MINUS_DST = 1,
    GX2_BLEND_COMBINE_MIN = 2,
    GX2_BLEND_COMBINE_MAX = 3,
    GX2_BLEND_COMBINE_DST_MINUS_SRC = 4
} GX2BlendCombine;

typedef enum _GX2LogicOp
{
    GX2_LOGIC_OP_CLEAR = 0x00,
    GX2_LOGIC_OP_COPY = 0xcc,
    GX2_LOGIC_OP_NONE = GX2_LOGIC_OP_COPY,
    GX2_LOGIC_OP_SET = 0xff
} GX2LogicOp;

typedef enum _GX2ChannelMask
{
    GX2_CHANNEL_MASK_NONE = 0x0,
    GX2_CHANNEL_MASK_R = 0x1,
    GX2_CHANNEL_MASK_G = 0x2,
    GX2_CHANNEL_MASK_B = 0x4,
    GX2_CHANNEL_MASK_A = 0x8,
    GX2_CHANNEL_MASK_RGB = 0x7,
    GX2_CHANNEL_MASK_RGBA = 0xf
} GX2ChannelMask;

typedef enum _GX2FrontFaceMode
{
    GX2_FRONT_FACE_CCW = 0,
    GX2_FRONT_FACE_CW = 1
} GX2FrontFaceMode;

typedef enum _GX2RenderTarget
{
    GX2_RENDER_TARGET_0 = 0,
    GX2_RENDER_TARGET_1 = 1,
    GX2_RENDER_TARGET_2 = 2,
    GX2_RENDER_TARGET_3 = 3,
    GX2_RENDER_TARGET_4 = 4,
    GX2_RENDER_TARGET_5 = 5,
    GX2_RENDER_TARGET_6 = 6,
    GX2_RENDER_TARGET_7 = 7
} GX2RenderTarget;

typedef enum _GX2PrimitiveType
{
    GX2_PRIMITIVE_POINTS = 1,
    GX2_PRIMITIVE_LINES = 2,
    GX2_PRIMITIVE_LINE_STRIP = 3,
    GX2_PRIMITIVE_TRIANGLES = 4,
    GX2_PRIMITIVE_TRIANGLE_FAN = 5,
    GX2_PRIMITIVE_TRIANGLE_STRIP = 6,
    GX2_PRIMITIVE_QUADS = 0x13,
    GX2_PRIMITIVE_QUAD_STRIP = 0x14
} GX2PrimitiveType;

typedef enum _GX2IndexFormat
{
    GX2_INDEX_FORMAT_U16_LE = 0,
    GX2_INDEX_FORMAT_U32_LE = 1,
    GX2_INDEX_FORMAT_U16 = 4,
    GX2_INDEX_FORMAT_U32 = 9
} GX2IndexFormat;

typedef enum _GX2ClearMode
{
    GX2_CLEAR_NONE = 0,
    GX2_CLEAR_DEPTH = 1,
    GX2_CLEAR_STENCIL = 2,
    GX2_CLEAR_BOTH = 3
} GX2ClearMode;

typedef enum _GX2ShaderMode
{
    GX2_SHADER_MODE_UNIFORM_REGISTER = 0,
    GX2_SHADER_MODE_UNIFORM_BLOCK = 1,
    GX2_SHADER_MODE_GEOMETRY_SHADER = 2,
    GX2_SHADER_MODE_COMPUTE_SHADER = 3
} GX2ShaderMode;

typedef enum _GX2TexClampMode
{
    GX2_TEX_CLAMP_WRAP = 0,
    GX2_TEX_CLAMP_MIRROR = 1,
    GX2_TEX_CLAMP_CLAMP = 2,
    GX2_TEX_CLAMP_MIRROR_ONCE = 3,
    GX2_TEX_CLAMP_CLAMP_BORDER = 6
} GX2TexClampMode;

typedef enum _GX2TexXYFilterType
{
    GX2_TEX_XY_FILTER_POINT = 0,
    GX2_TEX_XY_FILTER_BILINEAR = 1
} GX2TexXYFilterType;

typedef enum _GX2AttribFormat
{
    GX2_ATTRIB_FORMAT_8_8_8_8_UNORM = 0x00a,
    GX2_ATTRIB_FORMAT_10_10_10_2_UNORM = 0x00b,
    GX2_ATTRIB_FORMAT_8_8_8_8_SNORM = 0x20a,
    GX2_ATTRIB_FORMAT_10_10_10_2_SNORM = 0x20b,
    GX2_ATTRIB_FORMAT_16_16_SINT = 0x307,
    GX2_ATTRIB_FORMAT_16_16_16_16_SINT = 0x30e,
    GX2_ATTRIB_FORMAT_32_32_FLOAT = 0x80d,
    GX2_ATTRIB_FORMAT_32_32_32_FLOAT = 0x811,
    GX2_ATTRIB_FORMAT_32_32_32_32_FLOAT = 0x813
} GX2AttribFormat;

typedef enum _GX2AttribIndexType
{
    GX2_ATTRIB_INDEX_VERTEX_ID = 0,
    GX2_ATTRIB_INDEX_INSTANCE_ID = 1
} GX2AttribIndexType;

typedef enum _GX2EndianSwapMode
{
    GX2_ENDIANSWAP_NONE = 0,
    GX2_ENDIANSWAP_8IN16 = 1,
    GX2_ENDIANSWAP_8IN32 = 2,
    GX2_ENDIANSWAP_DEFAULT = 3
} GX2EndianSwapMode;

typedef enum _GX2FetchShaderType
{
    GX2_FETCH_SHADER_TESSELATION_NONE = 0
} GX2FetchShaderType;

typedef enum _GX2TessellationMode
{
    GX2_TESSELLATION_MODE_DISCRETE = 0
} GX2TessellationMode;

typedef enum _GX2VarType
{
    GX2_VAR_TYPE_VOID = 0,
    GX2_VAR_TYPE_BOOL = 1,
    GX2_VAR_TYPE_INT = 2,
    GX2_VAR_TYPE_UINT = 3,
    GX2_VAR_TYPE_FLOAT = 4,
    GX2_VAR_TYPE_DOUBLE = 5,
    GX2_VAR_TYPE_DVEC2 = 6,
    GX2_VAR_TYPE_DVEC3 = 7,
    GX2_VAR_TYPE_DVEC4 = 8,
    GX2_VAR_TYPE_VEC2 = 9,
    GX2_VAR_TYPE_VEC3 = 10,
    GX2_VAR_TYPE_VEC4 = 11,
    GX2_VAR_TYPE_BVEC2 = 12,
    GX2_VAR_TYPE_BVEC3 = 13,
    GX2_VAR_TYPE_BVEC4 = 14,
    GX2_VAR_TYPE_IVEC2 = 15,
    GX2_VAR_TYPE_IVEC3 = 16,
    GX2_VAR_TYPE_IVEC4 = 17,
    GX2_VAR_TYPE_UVEC2 = 18,
    GX2_VAR_TYPE_UVEC3 = 19,
    GX2_VAR_TYPE_UVEC4 = 20,
    GX2_VAR_TYPE_MAT2 = 21,
    GX2_VAR_TYPE_MAT2X3 = 22,
    GX2_VAR_TYPE_MAT2X4 = 23,
    GX2_VAR_TYPE_MAT3X2 = 24,
    GX2_VAR_TYPE_MAT3 = 25,
    GX2_VAR_TYPE_MAT3X4 = 26,
    GX2_VAR_TYPE_MAT4X2 = 27,
    GX2_VAR_TYPE_MAT4X3 = 28,
    GX2_VAR_TYPE_MAT4 = 29,
    GX2_VAR_TYPE_DMAT2 = 30,
    GX2_VAR_TYPE_DMAT2X3 = 31,
    GX2_VAR_TYPE_DMAT2X4 = 32,
    GX2_VAR_TYPE_DMAT3X2 = 33,
    GX2_VAR_TYPE_DMAT3 = 34,
    GX2_VAR_TYPE_DMAT3X4 = 35,
    GX2_VAR_TYPE_DMAT4X2 = 36,
    GX2_VAR_TYPE_DMAT4X3 = 37,
    GX2_VAR_TYPE_DMAT4 = 38
} GX2VarType;

typedef enum _GX2SamplerType
{
    GX2_SAMPLER_TYPE_1D = 0,
    GX2_SAMPLER_TYPE_2D = 1
} GX2SamplerType;

typedef enum _GX2TVRenderMode
{
    GX2_TV_RENDER_NONE = 0,
    GX2_TV_RENDER_480_STANDARD = 1,
    GX2_TV_RENDER_480_WIDE = 2,
    GX2_TV_RENDER_720 = 3,
    GX2_TV_RENDER_1080 = 5
} GX2TVRenderMode;

typedef enum _GX2TVScanMode
{
    GX2_TV_SCAN_MODE_NONE = 0,
    GX2_TV_SCAN_MODE_576I = 1,
    GX2_TV_SCAN_MODE_480I = 2,
    GX2_TV_SCAN_MODE_480P = 3,
    GX2_TV_SCAN_MODE_720P = 4,
    GX2_TV_SCAN_MODE_1080I = 6,
    GX2_TV_SCAN_MODE_1080P = 7
} GX2TVScanMode;

typedef enum _GX2DRCMode
{
    GX2_DRC_NONE = 0,
    GX2_DRC_SINGLE = 1,
    GX2_DRC_DOUBLE = 2
} GX2DRCMode;

typedef enum _GX2BufferingMode
{
    GX2_BUFFERING_SINGLE = 1,
    GX2_BUFFERING_DOUBLE = 2,
    GX2_BUFFERING_TRIPLE = 3
} GX2BufferingMode;

typedef enum _GX2ScanTarget
{
    GX2_SCAN_TARGET_TV = 0x1,
    GX2_SCAN_TARGET_TV_LEFT = 0x1,
    GX2_SCAN_TARGET_TV_RIGHT = 0x2,
    GX2_SCAN_TARGET_DRC_FIRST = 0x4,
    GX2_SCAN_TARGET_DRC_SECOND = 0x8
} GX2ScanTarget;

typedef enum _GX2PolygonMode
{
    GX2_POLYGON_MODE_POINT = 0,
    GX2_POLYGON_MODE_LINE = 1,
    GX2_POLYGON_MODE_TRIANGLE = 2
} GX2PolygonMode;

typedef enum _GX2StencilFunction
{
    GX2_STENCIL_KEEP = 0,
    GX2_STENCIL_ZERO = 1,
    GX2_STENCIL_REPLACE = 2,
    GX2_STENCIL_INCR = 3,
    GX2_STENCIL_DECR = 4,
    GX2_STENCIL_INVERT = 5,
    GX2_STENCIL_INCR_WRAP = 6,
    GX2_STENCIL_DECR_WRAP = 7
} GX2StencilFunction;

typedef enum _GX2TexZFilterType
{
    GX2_TEX_Z_FILTER_NONE = 0,
    GX2_TEX_Z_FILTER_POINT = 1,
    GX2_TEX_Z_FILTER_LINEAR = 2
} GX2TexZFilterType;

typedef enum _GX2TexMipFilterType
{
    GX2_TEX_MIP_FILTER_NONE = 0,
    GX2_TEX_MIP_FILTER_POINT = 1,
    GX2_TEX_MIP_FILTER_LINEAR = 2
} GX2TexMipFilterType;

typedef enum _GX2TexAnisoRatio
{
    GX2_TEX_ANISO_1_TO_1 = 0,
    GX2_TEX_ANISO_2_TO_1 = 1,
    GX2_TEX_ANISO_4_TO_1 = 2,
    GX2_TEX_ANISO_8_TO_1 = 3,
    GX2_TEX_ANISO_16_TO_1 = 4
} GX2TexAnisoRatio;

typedef struct _GX2Surface
{
    GX2SurfaceDim dim;
    u32 width;
    u32 height;
    u32 depth;
    u32 numMips;
    GX2SurfaceFormat format;
    GX2AAMode aa;
    GX2SurfaceUse use;
    u32 imageSize;
    void* imagePtr;
    u32 mipSize;
    void* mipPtr;
    GX2TileMode tileMode;
    u32 swizzle;
    u32 alignment;
    u32 pitch;
    u32 mipOffset[13];
} GX2Surface;

typedef struct _GX2Texture
{
    GX2Surface surface;
    u32 viewFirstMip;
    u32 viewNumMips;
    u32 viewFirstSlice;
    u32 viewNumSlices;
    u32 compSel;
    u32 _regs[5];
} GX2Texture;

typedef struct _GX2ColorBuffer
{
    GX2Surface surface;
    u32 viewMip;
    u32 viewFirstSlice;
    u32 viewNumSlices;
    void* auxPtr;
    u32 auxSize;
    u32 _regs[5];
} GX2ColorBuffer;

typedef struct _GX2DepthBuffer
{
    GX2Surface surface;
    u32 viewMip;
    u32 viewFirstSlice;
    u32 viewNumSlices;
    void* hiZPtr;
    u32 hiZSize;
    f32 clearDepth;
    u32 clearStencil;
    u32 _regs[7];
} GX2DepthBuffer;

typedef struct _GX2Sampler
{
    u32 samplerReg[3];
} GX2Sampler;

typedef struct _GX2AttribStream
{
    u32 location;
    u32 buffer;
    u32 offset;
    GX2AttribFormat format;
    GX2AttribIndexType indexType;
    u32 aluDivisor;
    u32 destSel;
    GX2EndianSwapMode endianSwap;
} GX2AttribStream;

typedef struct _GX2FetchShader
{
    GX2FetchShaderType shaderType;
    u32 _regs[1];
    u32 shaderSize;
    void* shaderPtr;
    u32 _numAttribs;
    u32 _numDivisors;
    u32 _divisors[2];
} GX2FetchShader;

typedef struct _GX2UniformBlock
{
    const char* name;
    u32 location;
    u32 size;
} GX2UniformBlock;

typedef struct _GX2UniformVar
{
    const char* name;
    GX2VarType type;
    u32 arrayCount;
    u32 offset;
    s32 blockIndex;
} GX2UniformVar;

typedef struct _GX2UniformInitialValue
{
    f32 value[4];
    u32 offset;
} GX2UniformInitialValue;

typedef struct _GX2LoopVar
{
    u32 _offset;
    u32 _value;
} GX2LoopVar;

typedef struct _GX2SamplerVar
{
    const char* name;
    GX2SamplerType type;
    u32 location;
} GX2SamplerVar;

typedef struct _GX2AttribVar
{
    const char* name;
    GX2VarType type;
    u32 arrayCount;
    u32 location;
} GX2AttribVar;

typedef struct _GX2RBuffer
{
    u32 flags;
    u32 elementSize;
    u32 elementCount;
    void* buffer;
} GX2RBuffer;

typedef struct _GX2VertexShader
{
    u32 _regs[52];
    u32 shaderSize;
    void* shaderPtr;
    GX2ShaderMode shaderMode;
    u32 numUniformBlocks;
    GX2UniformBlock* uniformBlocks;
    u32 numUniforms;
    GX2UniformVar* uniformVars;
    u32 numInitialValues;
    GX2UniformInitialValue* initialValues;
    u32 _numLoops;
    GX2LoopVar* _loopVars;
    u32 numSamplers;
    GX2SamplerVar* samplerVars;
    u32 numAttribs;
    GX2AttribVar* attribVars;
    u32 ringItemsize;
    GX2Boolean hasStreamOut;
    u32 streamOutVertexStride[4];
    GX2RBuffer shaderProgram;
} GX2VertexShader;

typedef struct _GX2PixelShader
{
    u32 _regs[41];
    u32 shaderSize;
    void* shaderPtr;
    GX2ShaderMode shaderMode;
    u32 numUniformBlocks;
    GX2UniformBlock* uniformBlocks;
    u32 numUniforms;
    GX2UniformVar* uniformVars;
    u32 numInitialValues;
    GX2UniformInitialValue* initialValues;
    u32 _numLoops;
    GX2LoopVar* _loopVars;
    u32 numSamplers;
    GX2SamplerVar* samplerVars;
    GX2RBuffer shaderProgram;
} GX2PixelShader;

typedef struct _GX2ContextState
{
    u32 data[0xa100 / 4];
} GX2ContextState;

#endif
