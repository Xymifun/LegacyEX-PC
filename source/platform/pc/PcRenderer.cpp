#include "game/client/render/Renderer.hpp"
#include "game/core/PIXNamedEvents.hpp"

#include "platform/pc/PcCursorImage.hpp"
#include "platform/pc/PcDisplay.hpp"
#include "platform/pc/PcInput.hpp"
#include "platform/pc/PcOs.hpp"
#include "platform/pc/PcGl.hpp"
#include "platform/pc/PcPaths.hpp"
#include "platform/pc/PcProfiler.hpp"
#include "platform/pc/PcRendererShaders.hpp"
#include "platform/pc/PcSettings.hpp"
#include "platform/pc/PcModViews.hpp"
#include "platform/wiiu/NintendoFileSystem_WiiU.hpp"
#include "platform/wiiu/WiiU_FileCache.hpp"
#include "platform/wiiu/WiiU_Runtime.hpp"

#include <cafe/mtx.h>
#include <coreinit/memdefaultheap.h>
#include <png.h>
#include <stdio.h>
#include <stdlib.h>
#include <vector>
#include <string.h>

#include <string>

struct D3DXIMAGE_INFO
{
    unsigned int Width;
    unsigned int Height;
    unsigned int Depth;
    unsigned int MipLevels;
    unsigned int Format;
    unsigned int ResourceType;
    unsigned int ImageFileFormat;
};

Renderer* gRendererInstance = 0;

struct Renderer::CommandBuffer
{
    struct Command
    {
        bool mIsMatrix;
        float mMatrix[16];
        unsigned int mOffset;
        unsigned int mCount;
    };

    CommandBuffer()
        : mId(-1), mVertexType(0), mPixelShaderType(0), mPrimitiveType(0), mVertexCount(0), mBuffer(0),
          mVertexArray(0), mCountedBytes(0), mSingleMatrix(false)
    {
    }

    std::vector<unsigned char> mVertices;
    std::vector<Command> mCommands;
    int mId;
    int mVertexType;
    int mPixelShaderType;
    int mPrimitiveType;
    int mVertexCount;
    float mMatrix[16];
    unsigned int mBuffer;
    unsigned int mVertexArray;
    int mCountedBytes;
    bool mSingleMatrix;
};

namespace
{
const int COMMAND_BUFFER_FREE = -1;
const int COMMAND_BUFFER_RESERVED = -2;
const int COMMAND_BUFFER_RECORDED = 1;
const unsigned int PNG_FORMAT_ARGB_BYTES = PNG_FORMAT_FLAG_COLOR | PNG_FORMAT_FLAG_ALPHA | PNG_FORMAT_FLAG_AFIRST;
const unsigned int PNG_FORMAT_RGBA_BYTES = PNG_FORMAT_FLAG_COLOR | PNG_FORMAT_FLAG_ALPHA;
const int kScreenWidth[2] = {1280, 854};
const int kScreenHeight[2] = {720, 480};
const int kMinimumTargetSize = 16;
const int VERTEX_REGISTERS = 256;
const int PIXEL_REGISTERS = 32;
const unsigned int STREAM_BUFFER_SIZE = 16 * 1024 * 1024;
const unsigned int QUAD_INDEX_COUNT = 6 * 262144;
const int VERTEX_TEXTURE_UNIT = 4;
const int MAX_LAYERED_PARAMS = 7;
const unsigned int TEXTURE_FLAG_CLAMP_U = 1;
const unsigned int TEXTURE_FLAG_CLAMP_V = 2;
const unsigned int TEXTURE_FLAG_MIN_LINEAR = 4;
const unsigned int TEXTURE_FLAG_MAG_LINEAR = 8;
const int DEPTH_FUNC_LESS_EQUAL = 3;
const int BLEND_SOURCE_ALPHA = 4;
const int BLEND_INVERSE_SOURCE_ALPHA = 5;
const int COMPARE_ALWAYS = 7;
const float DEFAULT_ALPHA_REFERENCE = 0.1f;

const int VERTEX_STRIDES[C4JRender::eVertexType_Count] = {32, 16, 32, 32, 8, 8, 8, 20, 32, 32};
const int VERTEX_REGISTER_WORLD_VIEW = 0;
const int VERTEX_REGISTER_WORLD_VIEW2 = 4;
const int VERTEX_REGISTER_PROJECTION = 8;
const int VERTEX_REGISTER_TEXTURE = 12;
const int VERTEX_REGISTER_TEXTURE_UV2 = 16;
const int VERTEX_REGISTER_FOG = 17;
const int VERTEX_REGISTER_LIGHTS = 18;
const int VERTEX_REGISTER_TEX_GEN = 23;
const int VERTEX_REGISTER_UV_SCALE = 33;
const int VERTEX_REGISTER_GRID = 34;
const int VERTEX_REGISTER_VERTEX_ALPHA = 40;
const int VERTEX_REGISTER_BONES = 41;
const int PIXEL_REGISTER_COLOURS = 0;
const int PIXEL_REGISTER_FORCE_LOD = 8;
const int PIXEL_REGISTER_LAYERED = 9;

const GLenum COMPARE_FUNCTIONS[8] = {GL_NEVER,   GL_LESS,     GL_EQUAL,  GL_LEQUAL,
                                     GL_GREATER, GL_NOTEQUAL, GL_GEQUAL, GL_ALWAYS};
const GLenum BLEND_FACTORS[21] = {GL_ZERO,
                                  GL_ONE,
                                  GL_SRC_COLOR,
                                  GL_ONE_MINUS_SRC_COLOR,
                                  GL_SRC_ALPHA,
                                  GL_ONE_MINUS_SRC_ALPHA,
                                  GL_DST_ALPHA,
                                  GL_ONE_MINUS_DST_ALPHA,
                                  GL_DST_COLOR,
                                  GL_ONE_MINUS_DST_COLOR,
                                  GL_SRC_ALPHA_SATURATE,
                                  GL_SRC_ALPHA,
                                  GL_ONE_MINUS_SRC_ALPHA,
                                  GL_CONSTANT_COLOR,
                                  GL_ONE_MINUS_CONSTANT_COLOR,
                                  GL_SRC1_COLOR,
                                  GL_ONE_MINUS_SRC1_COLOR,
                                  GL_SRC1_ALPHA,
                                  GL_ONE_MINUS_SRC1_ALPHA,
                                  GL_CONSTANT_ALPHA,
                                  GL_ONE_MINUS_CONSTANT_ALPHA};

struct RenderState
{
    bool blendEnable;
    int blendSource;
    int blendDestination;
    float blendFactor[4];
    bool depthTestEnable;
    bool depthMask;
    int depthFunc;
    bool faceCull;
    bool frontFaceClockwise;
    float lineWidth;
    bool colourWrite[4];
    bool alphaTestEnable;
    float alphaReference;
    int stencilFunc;
    unsigned char stencilReference;
    unsigned char stencilCompareMask;
    unsigned char stencilWriteMask;
    float polygonSlope;
    float polygonBias;
    bool fogEnable;
    int fogMode;
    float fogNear;
    float fogFar;
    float fogDensity;
    float vertexAlphaDistance;
    bool lightingEnable;
    bool lightEnable[2];
    float lightDirection[2][4];
    float lightColour[2][4];
    float lightAmbient[4];
    int forceLod;
    bool orderIndependentAlpha;
    float texGen[2][4][4];
    int viewport[4];
    int scissor[4];
    int currentViewport;
    float clearColour[4];
    float colour[4];
    float fogColour[4];
    float lerpColour[4];
    float colourAdd[4];
    float texColour[4][4];
    float layeredTextureSize[4];
    float layeredParams[MAX_LAYERED_PARAMS][4];
    float layeredColour[16][4];
};

struct ProgramInfo
{
    GLuint program;
    GLint alphaReference;
    GLfloat alphaValue;
    GLint layers;
    GLint forceLod;
};

void EnterTimedCriticalSection(OSFastMutex* cs, const char* name)
{
    if (TryEnterCriticalSection(cs))
        return;
    PIXBeginNamedEvent(0.0f, name);
    EnterCriticalSection(cs);
    PIXEndNamedEvent();
}

int totalCommandBufferBytes = 0;
RenderState s_state;
PcDisplayMode s_displayMode = ePcDisplay_Window;
bool s_glReady = false;
__thread bool s_isRenderThread = false;

enum PcLeftover
{
    ePcLeftover_Stage1,
    ePcLeftover_Stage2,
    ePcLeftover_Stage3,
    ePcLeftover_Fog,
    ePcLeftover_Lerp,
    ePcLeftover_TexColour,
    ePcLeftover_Count
};

const int LEFTOVER_FRAMES = 8;
const int LEFTOVER_REPORTS = 32;

struct PcLeftoverTrace
{
    uintptr_t frames[LEFTOVER_FRAMES];
    int count;
};

PcLeftoverTrace leftoverTraces[ePcLeftover_Count];
uintptr_t leftoverReported[LEFTOVER_REPORTS];
int leftoverReportedCount = 0;

__attribute__((noinline)) void RecordLeftover(PcLeftover kind)
{
    if (!s_isRenderThread)
        return;
    PcLeftoverTrace& trace = leftoverTraces[kind];
    trace.count = 0;
    const uintptr_t* frame = static_cast<const uintptr_t*>(__builtin_frame_address(0));
    while (frame != 0 && trace.count < LEFTOVER_FRAMES)
    {
        const uintptr_t returnAddress = frame[1];
        if (returnAddress == 0)
            break;
        trace.frames[trace.count++] = returnAddress;
        const uintptr_t* next = reinterpret_cast<const uintptr_t*>(frame[0]);
        if (next <= frame || reinterpret_cast<uintptr_t>(next) - reinterpret_cast<uintptr_t>(frame) > (1u << 20))
            break;
        frame = next;
    }
}
__thread Renderer::Context* s_context = 0;
int s_screen = 0;
bool s_drcFrameStarted = false;
unsigned short s_gamma = 0x2AAA;
float vertexRegisters[VERTEX_REGISTERS][4];
float pixelRegisters[PIXEL_REGISTERS][4];
int vertexDirtyFirst = 0;
int vertexDirtyLast = VERTEX_REGISTERS;
int pixelDirtyFirst = 0;
int pixelDirtyLast = PIXEL_REGISTERS;
GLuint registerRing = 0;
GLint registerAlignment = 256;
GLintptr registerRingOffset = 0;
const GLsizeiptr REGISTER_RING_SIZE = 4 * 1024 * 1024;
const int VERTEX_BASE_REGISTERS = 41;
const int BONE_REGISTER_COUNT = 215;
GLuint frameBuffers[2] = {0, 0};
GLuint s_colourTextures[2] = {0, 0};
GLuint s_depthBuffers[2] = {0, 0};
GLuint s_vertexArray = 0;
GLuint streamBuffer = 0;
unsigned int streamOffset = 0;
GLuint quadIndexBuffer = 0;
GLsync s_frameFence = 0;
const GLuint64 kFrameFenceTimeoutNanoseconds = 100000000ULL;
GLuint s_presentProgram = 0;
GLuint s_cursorProgram = 0;
GLuint s_cursorTexture = 0;
std::string glRendererName;
GLint s_presentGamma = -1;
GLuint s_presentOverProgram = 0;
GLint s_presentOverGamma = -1;
GLuint panelProgram = 0;
GLint panelMatrix = -1;
GLint panelOpaque = -1;
GLint panelSource = -1;
GLint panelCuts = -1;
GLint panelCutCount = -1;
GLuint boxProgram = 0;
GLint boxMatrix = -1;
GLint boxColour = -1;
GLuint s_underlayFramebuffer = 0;
bool s_underlayAlone = false;
int s_underlayWidth = 0;
int s_underlayHeight = 0;
ProgramInfo programs[ePcVertexShader_Count][ePcPixelShader_Count];
int s_presentWidth = 1280;
int s_presentHeight = 720;
int s_targetWidth[2] = {1280, 854};
int s_targetHeight[2] = {720, 480};
int s_fixedRenderHeight = 0;
bool s_letterbox = false;
int s_windowWidth = 1280;
int s_windowHeight = 720;
bool s_wideWorldPass = false;
bool s_wideUiPass = false;
bool s_coverUiPass = false;
bool startFullscreen = false;
std::string s_screenshotPath;

struct MemoryWriter
{
    unsigned char* start;
    unsigned char* current;
    unsigned char* end;
};

MemoryWriter memoryWriter;

void WriteToMemory(png_structp, png_bytep data, png_size_t len)
{
    size_t room = (size_t)(memoryWriter.end - memoryWriter.current);
    size_t n = len < room ? len : room;
    memcpy(memoryWriter.current, data, n);
    memoryWriter.current += n;
}

void FlushMemory(png_structp)
{
}

int DecodePng(const void* fileData, unsigned int fileSize, D3DXIMAGE_INFO* info, int** data)
{
    png_image img;
    memset(&img, 0, sizeof(img));
    img.version = PNG_IMAGE_VERSION;
    if (!png_image_begin_read_from_memory(&img, fileData, fileSize))
        return -1;
    img.format = PNG_FORMAT_ARGB_BYTES;
    *data = reinterpret_cast<int*>(new unsigned char[PNG_IMAGE_SIZE(img)]);
    if (!png_image_finish_read(&img, 0, *data, 0, 0))
    {
        delete[] reinterpret_cast<unsigned char*>(*data);
        *data = 0;
        return -1;
    }
    info->Width = img.width;
    info->Height = img.height;
    return 0;
}

void SetIdentity(float* mat)
{
    memset(mat, 0, sizeof(float) * 16);
    mat[0] = mat[5] = mat[10] = mat[15] = 1.0f;
}

GLuint CompileShader(GLenum type, const char* const* sources, int n)
{
    GLuint sh = PcGl::CreateShader(type);
    PcGl::ShaderSource(sh, n, sources, 0);
    PcGl::CompileShader(sh);
    GLint compiled = 0;
    PcGl::GetShaderiv(sh, GL_COMPILE_STATUS, &compiled);
    if (!compiled)
    {
        char log[4096];
        PcGl::GetShaderInfoLog(sh, sizeof(log), 0, log);
        printf("[pc] gl: shader compile failed:\n%s\n", log);
        fflush(stdout);
    }
    return sh;
}

GLuint LinkProgram(GLuint vertexShader, GLuint pixelShader)
{
    GLuint prog = PcGl::CreateProgram();
    PcGl::AttachShader(prog, vertexShader);
    PcGl::AttachShader(prog, pixelShader);
    PcGl::LinkProgram(prog);
    GLint linked = 0;
    PcGl::GetProgramiv(prog, GL_LINK_STATUS, &linked);
    if (!linked)
    {
        char log[4096];
        PcGl::GetProgramInfoLog(prog, sizeof(log), 0, log);
        printf("[pc] gl: program link failed:\n%s\n", log);
        fflush(stdout);
    }
    PcGl::DeleteShader(vertexShader);
    PcGl::DeleteShader(pixelShader);
    return prog;
}

void SetViewportRect(int x, int y, int width, int height)
{
    s_state.viewport[0] = x;
    s_state.viewport[1] = y;
    s_state.viewport[2] = width;
    s_state.viewport[3] = height;
    s_state.scissor[0] = x;
    s_state.scissor[1] = y;
    s_state.scissor[2] = width;
    s_state.scissor[3] = height;
}

enum GlCapability
{
    eGlCapability_Blend,
    eGlCapability_DepthTest,
    eGlCapability_CullFace,
    eGlCapability_StencilTest,
    eGlCapability_PolygonOffsetFill,
    eGlCapability_ScissorTest,
    eGlCapability_Count
};

const GLenum GL_CAPABILITIES[eGlCapability_Count] = {GL_BLEND,        GL_DEPTH_TEST,          GL_CULL_FACE,
                                                     GL_STENCIL_TEST, GL_POLYGON_OFFSET_FILL, GL_SCISSOR_TEST};
const int GL_TEXTURE_UNITS = VERTEX_TEXTURE_UNIT + 1;
const int GL_VERTEX_ATTRIBUTES = 5;
const int CHUNK_OFFSET_ATTRIBUTE = 5;

struct GlCache
{
    GLint enabled[eGlCapability_Count];
    GLint blendFunc[2];
    GLfloat blendColour[4];
    GLint depthFunc;
    GLint depthMask;
    GLint cullFace;
    GLint frontFace;
    GLint colourMask[4];
    GLint stencilFunc[3];
    GLint stencilOp[3];
    GLint stencilMask;
    GLfloat polygonOffset[2];
    GLint viewport[4];
    GLint scissor[4];
    GLfloat lineWidth;
    GLint activeTexture;
    GLint textures[GL_TEXTURE_UNITS];
    GLint program;
    GLint arrayBuffer;
    GLint elementBuffer;
    GLint uniformBuffer;
    GLint attributes;
    GLint vertexFormat[3];
    GLint vertexArray;
    GLfloat chunkOffset[4];
};

GlCache s_gl;

void InvalidateGlCache()
{
    memset(&s_gl, 0xFF, sizeof(s_gl));
}

bool GlChanged(void* cached, const void* values, size_t len)
{
    if (memcmp(cached, values, len) == 0)
        return false;
    memcpy(cached, values, len);
    return true;
}

void GlSetEnabled(GlCapability cap, bool on)
{
    GLint v = on ? 1 : 0;
    if (s_gl.enabled[cap] == v)
        return;
    s_gl.enabled[cap] = v;
    if (on)
        PcGl::Enable(GL_CAPABILITIES[cap]);
    else
        PcGl::Disable(GL_CAPABILITIES[cap]);
}

void GlDepthMask(bool on)
{
    GLint v = on ? GL_TRUE : GL_FALSE;
    if (s_gl.depthMask == v)
        return;
    s_gl.depthMask = v;
    PcGl::DepthMask((GLboolean)v);
}

void GlColourMask(bool r, bool g, bool b, bool a)
{
    GLint mask[4] = {r, g, b, a};
    if (GlChanged(s_gl.colourMask, mask, sizeof(mask)))
        PcGl::ColorMask(r, g, b, a);
}

void GlActiveTexture(int unit)
{
    if (s_gl.activeTexture == unit)
        return;
    s_gl.activeTexture = unit;
    PcGl::ActiveTexture(GL_TEXTURE0 + unit);
}

void GlBindTexture(int unit, GLuint tex)
{
    if (s_gl.textures[unit] == (GLint)tex)
        return;
    GlActiveTexture(unit);
    s_gl.textures[unit] = (GLint)tex;
    PcGl::BindTexture(GL_TEXTURE_2D, tex);
}

void GlUseProgram(GLuint prog)
{
    if (s_gl.program == (GLint)prog)
        return;
    s_gl.program = (GLint)prog;
    PcGl::UseProgram(prog);
}

void GlBindBuffer(GLenum tgt, GLuint buf)
{
    GLint* cached = tgt == GL_ARRAY_BUFFER           ? &s_gl.arrayBuffer
                    : tgt == GL_ELEMENT_ARRAY_BUFFER ? &s_gl.elementBuffer
                                                        : &s_gl.uniformBuffer;
    if (*cached == (GLint)buf)
        return;
    *cached = (GLint)buf;
    PcGl::BindBuffer(tgt, buf);
}

void GlBindVertexArray(GLuint vao)
{
    if (s_gl.vertexArray == (GLint)vao)
        return;
    s_gl.vertexArray = (GLint)vao;
    PcGl::BindVertexArray(vao);
}

void GlChunkOffset(const float* off)
{
    if (GlChanged(s_gl.chunkOffset, off, sizeof(s_gl.chunkOffset)))
        PcGl::VertexAttrib4fv(CHUNK_OFFSET_ATTRIBUTE, off);
}

void GlEnableAttributes(GLint attrs)
{
    if (s_gl.attributes == attrs)
        return;
    for (int attr = 0; attr < GL_VERTEX_ATTRIBUTES; ++attr)
    {
        GLint bit = 1 << attr;
        bool known = s_gl.attributes != -1;
        if (known && (s_gl.attributes & bit) == (attrs & bit))
            continue;
        if ((attrs & bit) != 0)
            PcGl::EnableVertexAttribArray(attr);
        else
            PcGl::DisableVertexAttribArray(attr);
    }
    s_gl.attributes = attrs;
}

int ScaleToTarget(int value, int target, int logical)
{
    return static_cast<int>((static_cast<long long>(value) * target + logical / 2) / logical);
}

void FitArea(int width, int height, int aspectWidth, int aspectHeight, int* area)
{
    int fitWidth = width;
    int fitHeight = static_cast<int>(static_cast<long long>(width) * aspectHeight / aspectWidth);
    if (fitHeight > height)
    {
        fitHeight = height;
        fitWidth = static_cast<int>(static_cast<long long>(height) * aspectWidth / aspectHeight);
    }
    area[0] = (width - fitWidth) / 2;
    area[1] = (height - fitHeight) / 2;
    area[2] = fitWidth;
    area[3] = fitHeight;
}

void TargetArea(int* area)
{
    if (s_screen == 0 && s_wideWorldPass)
    {
        area[0] = 0;
        area[1] = 0;
        area[2] = s_targetWidth[0];
        area[3] = s_targetHeight[0];
        return;
    }
    if (s_screen == 0 && s_coverUiPass)
    {
        const long long width = s_targetWidth[0];
        const long long height = s_targetHeight[0];
        const bool wider = width * kScreenHeight[0] >= height * kScreenWidth[0];
        area[2] = static_cast<int>(wider ? width : (height * kScreenWidth[0] + kScreenHeight[0] - 1) / kScreenHeight[0]);
        area[3] = static_cast<int>(wider ? (width * kScreenHeight[0] + kScreenWidth[0] - 1) / kScreenWidth[0] : height);
        area[0] = static_cast<int>((width - area[2]) / 2);
        area[1] = static_cast<int>((height - area[3]) / 2);
        return;
    }
    FitArea(s_targetWidth[s_screen], s_targetHeight[s_screen], kScreenWidth[s_screen], kScreenHeight[s_screen], area);
    if (s_screen == 0 && s_wideUiPass)
    {
        area[0] = 0;
        area[2] = s_targetWidth[0];
    }
}

void ToTargetRect(const int* rect, GLint* target)
{
    int logicalWidth = kScreenWidth[s_screen];
    int logicalHeight = kScreenHeight[s_screen];
    int area[4];
    TargetArea(area);
    int left = area[0] + ScaleToTarget(rect[0], area[2], logicalWidth);
    int right = area[0] + ScaleToTarget(rect[0] + rect[2], area[2], logicalWidth);
    int bottom = area[1] + ScaleToTarget(logicalHeight - rect[1] - rect[3], area[3], logicalHeight);
    int top = area[1] + ScaleToTarget(logicalHeight - rect[1], area[3], logicalHeight);
    target[0] = left;
    target[1] = bottom;
    target[2] = right - left;
    target[3] = top - bottom;
}

void ClearOutsideUiArea()
{
    const int width = s_targetWidth[0];
    const int height = s_targetHeight[0];
    int area[4];
    FitArea(width, height, kScreenWidth[0], kScreenHeight[0], area);
    if (area[2] == width && area[3] == height)
        return;
    const bool sideBars = area[2] < width;
    const int bars[2][4] = {{0, 0, sideBars ? area[0] : width, sideBars ? height : area[1]},
                            {sideBars ? area[0] + area[2] : 0, sideBars ? 0 : area[1] + area[3], width, height}};
    PcGl::Enable(GL_SCISSOR_TEST);
    PcGl::ColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE);
    PcGl::ClearColor(0.0f, 0.0f, 0.0f, 1.0f);
    for (int i = 0; i < 2; ++i)
    {
        if (bars[i][2] <= bars[i][0] || bars[i][3] <= bars[i][1])
            continue;
        PcGl::Scissor(bars[i][0], bars[i][1], bars[i][2] - bars[i][0], bars[i][3] - bars[i][1]);
        PcGl::Clear(GL_COLOR_BUFFER_BIT);
    }
}

void ApplyViewportAndScissor()
{
    GLint viewport[4];
    ToTargetRect(s_state.viewport, viewport);
    if (GlChanged(s_gl.viewport, viewport, sizeof(viewport)))
        PcGl::Viewport(viewport[0], viewport[1], viewport[2], viewport[3]);
    GlSetEnabled(eGlCapability_ScissorTest, true);
    GLint scissor[4];
    ToTargetRect(s_state.scissor, scissor);
    if (s_coverUiPass && s_screen == 0)
    {
        const GLint left = scissor[0] < 0 ? 0 : scissor[0];
        const GLint bottom = scissor[1] < 0 ? 0 : scissor[1];
        GLint right = scissor[0] + scissor[2];
        GLint top = scissor[1] + scissor[3];
        right = right > s_targetWidth[0] ? s_targetWidth[0] : right;
        top = top > s_targetHeight[0] ? s_targetHeight[0] : top;
        scissor[0] = left;
        scissor[1] = bottom;
        scissor[2] = right > left ? right - left : 0;
        scissor[3] = top > bottom ? top - bottom : 0;
    }
    if (GlChanged(s_gl.scissor, scissor, sizeof(scissor)))
        PcGl::Scissor(scissor[0], scissor[1], scissor[2], scissor[3]);
}
}

void PcSetDisplayMode(PcDisplayMode mode)
{
    s_displayMode = mode;
}

void PcSetRenderHeight(int height)
{
    s_fixedRenderHeight = height;
}

void PcSetWindowSize(int width, int height)
{
    if (width >= kMinimumTargetSize && height >= kMinimumTargetSize)
    {
        s_windowWidth = width;
        s_windowHeight = height;
    }
}

void PcSetLetterbox(bool letterbox)
{
    s_letterbox = letterbox;
}

void PcSetWideWorldPass(bool wide)
{
    s_wideWorldPass = wide;
}

void PcSetWideUiPass(bool wide)
{
    s_wideUiPass = wide;
}

void PcSetCoverUiPass(bool cover)
{
    s_coverUiPass = cover;
}

float PcGetUiWidthScale()
{
    int area[4];
    FitArea(s_targetWidth[0], s_targetHeight[0], kScreenWidth[0], kScreenHeight[0], area);
    if (area[2] <= 0)
        return 1.0f;
    return static_cast<float>(s_targetWidth[0]) / static_cast<float>(area[2]);
}

void PcClearOutsideUiArea()
{
    if (!s_glReady || !s_isRenderThread || s_screen != 0)
        return;
    ClearOutsideUiArea();
    InvalidateGlCache();
    GlColourMask(s_state.colourWrite[0], s_state.colourWrite[1], s_state.colourWrite[2], s_state.colourWrite[3]);
    ApplyViewportAndScissor();
}

void PcResyncRendererState()
{
    if (!s_glReady || !s_isRenderThread)
        return;
    InvalidateGlCache();
    GlColourMask(s_state.colourWrite[0], s_state.colourWrite[1], s_state.colourWrite[2], s_state.colourWrite[3]);
    ApplyViewportAndScissor();
}

void PcNeutralGlState()
{
    if (!s_glReady || !s_isRenderThread)
        return;
    PcGl::BindFramebuffer(GL_FRAMEBUFFER, 0);
    PcGl::Disable(GL_SCISSOR_TEST);
    PcGl::Disable(GL_DEPTH_TEST);
    PcGl::Disable(GL_STENCIL_TEST);
    PcGl::Disable(GL_BLEND);
    PcGl::Disable(GL_CULL_FACE);
    PcGl::ColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE);
    PcGl::DepthMask(GL_TRUE);
    InvalidateGlCache();
}

namespace
{
bool externalTarget = false;
int mainTargetWidth = 0;
int mainTargetHeight = 0;
}

void PcBindMainTargetAfterCallback()
{
    if (!s_glReady || !s_isRenderThread)
        return;
    PcGl::BindFramebuffer(GL_FRAMEBUFFER, frameBuffers[s_screen]);
    PcResyncRendererState();
}

int screenBeforeExternal = 0;

bool PcBeginExternalTarget(unsigned int fbo, int w, int h)
{
    if (!s_glReady || !s_isRenderThread || externalTarget || w <= 0 || h <= 0)
        return false;
    externalTarget = true;
    screenBeforeExternal = s_screen;
    s_screen = 0;
    mainTargetWidth = s_targetWidth[0];
    mainTargetHeight = s_targetHeight[0];
    s_targetWidth[0] = w;
    s_targetHeight[0] = h;
    PcGl::BindFramebuffer(GL_FRAMEBUFFER, fbo);
    PcResyncRendererState();
    return true;
}

void PcEndExternalTarget()
{
    if (!externalTarget)
        return;
    externalTarget = false;
    s_targetWidth[0] = mainTargetWidth;
    s_targetHeight[0] = mainTargetHeight;
    s_screen = screenBeforeExternal;
    PcGl::BindFramebuffer(GL_FRAMEBUFFER, frameBuffers[s_screen]);
    PcResyncRendererState();
}

void PcCopyToMainTarget(unsigned int fbo, int w, int h)
{
    if (!s_glReady || !s_isRenderThread || externalTarget || w <= 0 || h <= 0)
        return;
    const int tgtW = s_targetWidth[0];
    const int tgtH = s_targetHeight[0];
    int srcW = w;
    int srcH = h;
    if ((long long)w * tgtH > (long long)h * tgtW)
        srcW = (int)((long long)h * tgtW / tgtH);
    else
        srcH = (int)((long long)w * tgtH / tgtW);
    const int srcX = (w - srcW) / 2;
    const int srcY = (h - srcH) / 2;
    PcGl::Disable(GL_SCISSOR_TEST);
    PcGl::BindFramebuffer(GL_READ_FRAMEBUFFER, fbo);
    PcGl::BindFramebuffer(GL_DRAW_FRAMEBUFFER, frameBuffers[0]);
    PcGl::BlitFramebuffer(srcX, srcY, srcX + srcW, srcY + srcH, 0, 0, tgtW,
                          tgtH, GL_COLOR_BUFFER_BIT, GL_LINEAR);
    PcGl::BindFramebuffer(GL_FRAMEBUFFER, frameBuffers[0]);
    PcResyncRendererState();
}

void PcScaleTarget(unsigned int src, int srcW, int srcH, unsigned int dst,
                   int dstW, int dstH)
{
    if (!s_glReady || !s_isRenderThread || externalTarget)
        return;
    PcGl::Disable(GL_SCISSOR_TEST);
    PcGl::BindFramebuffer(GL_READ_FRAMEBUFFER, src);
    PcGl::BindFramebuffer(GL_DRAW_FRAMEBUFFER, dst);
    PcGl::BlitFramebuffer(0, 0, srcW, srcH, 0, 0, dstW, dstH,
                          GL_COLOR_BUFFER_BIT, GL_LINEAR);
    PcGl::BindFramebuffer(GL_FRAMEBUFFER, frameBuffers[0]);
    PcResyncRendererState();
}

void PcClearDepth(unsigned int fbo)
{
    if (!s_glReady || !s_isRenderThread)
        return;
    PcGl::BindFramebuffer(GL_FRAMEBUFFER, fbo);
    PcGl::Disable(GL_SCISSOR_TEST);
    PcGl::DepthMask(GL_TRUE);
    PcGl::ClearDepth(1.0);
    PcGl::Clear(GL_DEPTH_BUFFER_BIT);
    PcGl::BindFramebuffer(GL_FRAMEBUFFER, frameBuffers[s_screen]);
    PcResyncRendererState();
}

void PcDrawBoxes(unsigned int fbo, int w, int h, const float* mats, const float* cols,
                 int n)
{
    if (!s_glReady || !s_isRenderThread || boxProgram == 0 || n <= 0)
        return;
    PcGl::BindFramebuffer(GL_FRAMEBUFFER, fbo);
    PcGl::Viewport(0, 0, w, h);
    PcGl::Disable(GL_SCISSOR_TEST);
    PcGl::Disable(GL_STENCIL_TEST);
    PcGl::Disable(GL_CULL_FACE);
    PcGl::Enable(GL_DEPTH_TEST);
    PcGl::DepthFunc(GL_LESS);
    PcGl::DepthMask(GL_TRUE);
    PcGl::Enable(GL_BLEND);
    PcGl::BlendFunc(GL_ONE, GL_ONE_MINUS_SRC_ALPHA);
    PcGl::ColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE);
    PcGl::UseProgram(boxProgram);
    PcGl::BindVertexArray(s_vertexArray);
    for (int box = 0; box < n; ++box)
    {
        PcGl::UniformMatrix4fv(boxMatrix, 1, GL_FALSE, mats + box * 16);
        PcGl::Uniform4f(boxColour, cols[box * 4], cols[box * 4 + 1], cols[box * 4 + 2], cols[box * 4 + 3]);
        PcGl::DrawArrays(GL_TRIANGLES, 0, 36);
    }
    PcGl::BindFramebuffer(GL_FRAMEBUFFER, frameBuffers[s_screen]);
    PcResyncRendererState();
}

void PcDrawPanel(unsigned int fbo, int w, int h, unsigned int tex, const float* mat,
                 bool opaque, const float* src, const float* cuts, int cutCount)
{
    if (!s_glReady || !s_isRenderThread || panelProgram == 0)
        return;
    PcGl::BindFramebuffer(GL_FRAMEBUFFER, fbo);
    PcGl::Viewport(0, 0, w, h);
    PcGl::Disable(GL_SCISSOR_TEST);
    PcGl::Enable(GL_DEPTH_TEST);
    PcGl::DepthFunc(GL_LESS);
    PcGl::DepthMask(GL_TRUE);
    PcGl::Disable(GL_STENCIL_TEST);
    PcGl::Disable(GL_CULL_FACE);
    PcGl::Enable(GL_BLEND);
    PcGl::BlendFunc(GL_ONE, GL_ONE_MINUS_SRC_ALPHA);
    PcGl::ColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE);
    PcGl::UseProgram(panelProgram);
    PcGl::UniformMatrix4fv(panelMatrix, 1, GL_FALSE, mat);
    PcGl::Uniform1f(panelOpaque, opaque ? 1.0f : 0.0f);
    if (src != 0)
        PcGl::Uniform4f(panelSource, src[0], 1.0f - src[3], src[2] - src[0], src[3] - src[1]);
    else
        PcGl::Uniform4f(panelSource, 0.0f, 0.0f, 1.0f, 1.0f);
    float flipped[4 * 4];
    if (cutCount > 4)
        cutCount = 4;
    for (int cut = 0; cut < cutCount; ++cut)
    {
        flipped[cut * 4] = cuts[cut * 4];
        flipped[cut * 4 + 1] = 1.0f - cuts[cut * 4 + 3];
        flipped[cut * 4 + 2] = cuts[cut * 4 + 2];
        flipped[cut * 4 + 3] = 1.0f - cuts[cut * 4 + 1];
    }
    if (cutCount > 0)
        PcGl::Uniform4fv(panelCuts, cutCount, flipped);
    PcGl::Uniform1i(panelCutCount, cutCount > 0 ? cutCount : 0);
    PcGl::BindVertexArray(s_vertexArray);
    PcGl::ActiveTexture(GL_TEXTURE0);
    PcGl::BindTexture(GL_TEXTURE_2D, tex);
    PcGl::DrawArrays(GL_TRIANGLE_STRIP, 0, 4);
    PcGl::BindFramebuffer(GL_FRAMEBUFFER, frameBuffers[s_screen]);
    PcResyncRendererState();
}

void PcClearMainTarget(float r, float g, float b, float a)
{
    if (!s_glReady || !s_isRenderThread || externalTarget)
        return;
    PcGl::BindFramebuffer(GL_FRAMEBUFFER, frameBuffers[0]);
    PcGl::Disable(GL_SCISSOR_TEST);
    PcGl::ColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE);
    PcGl::ClearColor(r, g, b, a);
    PcGl::Clear(GL_COLOR_BUFFER_BIT);
    PcResyncRendererState();
}

void PcCopyMainTargetTo(unsigned int fbo)
{
    if (!s_glReady || !s_isRenderThread || externalTarget)
        return;
    PcScaleTarget(frameBuffers[0], s_targetWidth[0], s_targetHeight[0], fbo, s_targetWidth[0],
                  s_targetHeight[0]);
}

void PcSetWindowUnderlay(unsigned int fbo, int w, int h, bool alone)
{
    s_underlayAlone = alone;
    s_underlayFramebuffer = fbo;
    s_underlayWidth = w;
    s_underlayHeight = h;
}

void PcMakeFrustumMatrix(float left, float right, float bottom, float top, float nearDist, float farDist,
                         float* mat)
{
    float frustum[4][4];
    C_MTXFrustum(frustum, top, bottom, left, right, nearDist, farDist);
    ASM_MTX44Transpose(frustum, frustum);
    memcpy(mat, frustum, sizeof(frustum));
}

float PcGetWorldAspect()
{
    if (s_targetHeight[0] <= 0)
        return static_cast<float>(kScreenWidth[0]) / static_cast<float>(kScreenHeight[0]);
    return static_cast<float>(s_targetWidth[0]) / static_cast<float>(s_targetHeight[0]);
}

void PcSetStartFullscreen(bool fullscreen)
{
    startFullscreen = fullscreen;
}

void PcGetRenderSize(int* width, int* height)
{
    *width = s_targetWidth[0];
    *height = s_targetHeight[0];
}

const char* PcGetGlRendererName()
{
    return glRendererName.c_str();
}

PcDisplayMode PcGetDisplayMode()
{
    return s_displayMode;
}

void PcRequestScreenshot(const char* path)
{
    s_screenshotPath = path;
}

bool PcDisplayPollEvent(PcOsWindowEvent* event)
{
    return s_glReady && PcOsWindowPollEvent(event);
}

Renderer* gRenderer = new Renderer;

Renderer::Context::Context()
    : mMatrixMode(0), mRecordingBuffer(0), mRecordingId(-1), mRecordingVertexType(0), mRecordingPixelShaderType(0),
      mRecordingPrimitiveType(0), mDeferredMode(false), mNumBoneMatrices(0)
{
    for (int mode = 0; mode < kMatrixModes; ++mode)
    {
        ASM_MTX44Identity(mMatrixStack[mode][0]);
        mMatrixStackDepth[mode] = 0;
        mMatrixDirty[mode] = true;
    }
    for (int stage = 0; stage < kTextureStages; ++stage)
    {
        mBoundTexture[stage] = 0;
        mPixelTexture[stage] = 0;
        mTextureIsMask[stage] = false;
    }
    mScissorRect[0] = 0;
    mScissorRect[1] = 0;
    mScissorRect[2] = kScreenWidth[0];
    mScissorRect[3] = kScreenHeight[0];
    memset(mBoneMatrix, 0, sizeof(mBoneMatrix));
}

Renderer::Renderer()
    : mCommandBufferState(kMaxCommandBufferIds, COMMAND_BUFFER_FREE), mCommandBuffers(kMaxCommandBufferIds, 0),
      mNextCommandBufferId(0), mAllowStaticCommandBufferIds(true), mFrameCount(0), mVerticesDrawnThisFrame(0),
      mVerticesDrawnLastFrame(0), mReservedMEM1(0), mDefaultTexture(0), mDefaultTextureLocked(false), mTexturesInUse(0), mTexturesLogged(0), mVertexTexture(0)
{
    gRendererInstance = this;
    InitializeCriticalSection(&mCommandBufferMutex);
    InitializeCriticalSection(&mTextureMutex);
    for (int tex = 0; tex < kMaxTextures; ++tex)
    {
        mTextures[tex].mUsed = false;
        mTextures[tex].mName = 0;
        mTextures[tex].mWidth = 0;
        mTextures[tex].mHeight = 0;
        mTextures[tex].mLevels = 1;
        mTextures[tex].mFlags = 0;
        mTextures[tex].mStorageDirty = false;
        mTextures[tex].mParametersDirty = false;
    }
    memset(&s_state, 0, sizeof(s_state));
    for (int ch = 0; ch < 4; ++ch)
    {
        s_state.colour[ch] = 1.0f;
        for (int stage = 0; stage < 4; ++stage)
            s_state.texColour[stage][ch] = 1.0f;
        s_state.colourWrite[ch] = true;
        s_state.blendFactor[ch] = 1.0f;
    }
    s_state.depthMask = true;
    s_state.depthTestEnable = true;
    s_state.depthFunc = COMPARE_ALWAYS;
    s_state.faceCull = true;
    s_state.stencilFunc = COMPARE_ALWAYS;
    s_state.forceLod = -1;
    s_state.alphaReference = 1.0f;
    SetViewportRect(0, 0, kScreenWidth[0], kScreenHeight[0]);
    memset(vertexRegisters, 0, sizeof(vertexRegisters));
    memset(pixelRegisters, 0, sizeof(pixelRegisters));
    memset(programs, 0, sizeof(programs));
}

Renderer::Context& Renderer::getContext()
{
    if (s_context == 0)
        s_context = new Context;
    return *s_context;
}

bool Renderer::OnRenderThread()
{
    return s_glReady && s_isRenderThread;
}

bool Renderer::HasWindow()
{
    return s_glReady;
}

void Renderer::PumpWindow()
{
}

void Renderer::SetPresentSize(int width, int height)
{
    s_presentWidth = width;
    s_presentHeight = height;
}

void Renderer::RequestScreenshot(const char* path)
{
    s_screenshotPath = path;
}

void Renderer::Tick()
{
}

void Renderer::UpdateGamma(unsigned short gamma)
{
    s_gamma = gamma;
}

void Renderer::CreateRenderTargets()
{
    for (int screen = 0; screen < 2; ++screen)
    {
        PcGl::GenTextures(1, &s_colourTextures[screen]);
        PcGl::BindTexture(GL_TEXTURE_2D, s_colourTextures[screen]);
        PcGl::TexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, s_targetWidth[screen], s_targetHeight[screen], 0, GL_RGBA,
                         GL_UNSIGNED_BYTE, 0);
        PcGl::TexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        PcGl::TexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        PcGl::TexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        PcGl::TexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
        PcGl::GenRenderbuffers(1, &s_depthBuffers[screen]);
        PcGl::BindRenderbuffer(GL_RENDERBUFFER, s_depthBuffers[screen]);
        PcGl::RenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8, s_targetWidth[screen], s_targetHeight[screen]);
        PcGl::GenFramebuffers(1, &frameBuffers[screen]);
        PcGl::BindFramebuffer(GL_FRAMEBUFFER, frameBuffers[screen]);
        PcGl::FramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, s_colourTextures[screen], 0);
        PcGl::FramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER,
                                      s_depthBuffers[screen]);
        if (PcGl::CheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
            printf("[pc] gl: render target %d is incomplete\n", screen);
    }
}

void Renderer::ResizeTvTarget(int width, int height)
{
    if (width == s_targetWidth[0] && height == s_targetHeight[0])
        return;
    s_targetWidth[0] = width;
    s_targetHeight[0] = height;
    PcGl::BindTexture(GL_TEXTURE_2D, s_colourTextures[0]);
    PcGl::TexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, 0);
    PcGl::BindRenderbuffer(GL_RENDERBUFFER, s_depthBuffers[0]);
    PcGl::RenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8, width, height);
}

void Renderer::BindRenderTarget(int screen)
{
    s_screen = screen;
    if (s_glReady)
        PcGl::BindFramebuffer(GL_FRAMEBUFFER, frameBuffers[screen]);
}

void Renderer::Initialise()
{
    getContext();
    s_isRenderThread = true;
    if (s_displayMode != ePcDisplay_Headless)
    {
        if (!PcOsWindowCreate(s_windowWidth, s_windowHeight, "Minecraft", s_displayMode == ePcDisplay_Window))
            printf("[pc] gl: could not create a window with an OpenGL 3.3 context; running headless\n");
        else if (!PcGl::Load())
            printf("[pc] gl: OpenGL functions are missing; running headless\n");
        else
            s_glReady = true;
        if (s_glReady && startFullscreen && s_displayMode == ePcDisplay_Window)
            PcOsWindowSetFullscreen(true);
        fflush(stdout);
    }

    if (s_glReady)
    {
        glRendererName = reinterpret_cast<const char*>(PcGl::GetString(GL_RENDERER));
        printf("[pc] gl: %s / %s\n", reinterpret_cast<const char*>(PcGl::GetString(GL_RENDERER)),
               reinterpret_cast<const char*>(PcGl::GetString(GL_VERSION)));
        fflush(stdout);
        PcOsWindowSetSwapInterval(s_displayMode == ePcDisplay_Window ? 1 : 0);
        CreateRenderTargets();
        PcGl::GenVertexArrays(1, &s_vertexArray);
        PcGl::BindVertexArray(s_vertexArray);
        PcGl::GenBuffers(1, &streamBuffer);
        PcGl::BindBuffer(GL_ARRAY_BUFFER, streamBuffer);
        PcGl::BufferData(GL_ARRAY_BUFFER, STREAM_BUFFER_SIZE, 0, GL_STREAM_DRAW);

        std::vector<unsigned int> indices(QUAD_INDEX_COUNT);
        for (unsigned int quad = 0; quad < QUAD_INDEX_COUNT / 6; ++quad)
        {
            unsigned int base = quad * 4;
            indices[quad * 6 + 0] = base;
            indices[quad * 6 + 1] = base + 1;
            indices[quad * 6 + 2] = base + 2;
            indices[quad * 6 + 3] = base;
            indices[quad * 6 + 4] = base + 2;
            indices[quad * 6 + 5] = base + 3;
        }
        PcGl::GenBuffers(1, &quadIndexBuffer);
        PcGl::BindBuffer(GL_ELEMENT_ARRAY_BUFFER, quadIndexBuffer);
        PcGl::BufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(unsigned int), &indices[0], GL_STATIC_DRAW);

        PcGl::GetIntegerv(GL_UNIFORM_BUFFER_OFFSET_ALIGNMENT, &registerAlignment);
        if (registerAlignment < 16)
            registerAlignment = 16;
        PcGl::GenBuffers(1, &registerRing);
        PcGl::BindBuffer(GL_UNIFORM_BUFFER, registerRing);
        PcGl::BufferData(GL_UNIFORM_BUFFER, REGISTER_RING_SIZE, 0, GL_STREAM_DRAW);
        registerRingOffset = REGISTER_RING_SIZE;
        vertexDirtyFirst = 0;
        vertexDirtyLast = VERTEX_REGISTERS;
        pixelDirtyFirst = 0;
        pixelDirtyLast = PIXEL_REGISTERS;

        const char* presentVertex[] = {PcShaderPresentVertex()};
        const char* presentPixel[] = {PcShaderPresentPixel()};
        s_presentProgram = LinkProgram(CompileShader(GL_VERTEX_SHADER, presentVertex, 1),
                                       CompileShader(GL_FRAGMENT_SHADER, presentPixel, 1));
        PcGl::UseProgram(s_presentProgram);
        PcGl::Uniform1i(PcGl::GetUniformLocation(s_presentProgram, "uScreen"), 0);
        s_presentGamma = PcGl::GetUniformLocation(s_presentProgram, "uGamma");
        const char* presentOverPixel[] = {PcShaderPresentOverPixel()};
        s_presentOverProgram = LinkProgram(CompileShader(GL_VERTEX_SHADER, presentVertex, 1),
                                           CompileShader(GL_FRAGMENT_SHADER, presentOverPixel, 1));
        PcGl::UseProgram(s_presentOverProgram);
        PcGl::Uniform1i(PcGl::GetUniformLocation(s_presentOverProgram, "uScreen"), 0);
        s_presentOverGamma = PcGl::GetUniformLocation(s_presentOverProgram, "uGamma");
        const char* panelVertex[] = {PcShaderPanelVertex()};
        const char* panelPixel[] = {PcShaderPanelPixel()};
        panelProgram = LinkProgram(CompileShader(GL_VERTEX_SHADER, panelVertex, 1),
                                     CompileShader(GL_FRAGMENT_SHADER, panelPixel, 1));
        PcGl::UseProgram(panelProgram);
        PcGl::Uniform1i(PcGl::GetUniformLocation(panelProgram, "uImage"), 0);
        panelMatrix = PcGl::GetUniformLocation(panelProgram, "uMatrix");
        panelOpaque = PcGl::GetUniformLocation(panelProgram, "uOpaque");
        panelSource = PcGl::GetUniformLocation(panelProgram, "uSource");
        panelCuts = PcGl::GetUniformLocation(panelProgram, "uCuts");
        panelCutCount = PcGl::GetUniformLocation(panelProgram, "uCutCount");
        const char* boxVertex[] = {PcShaderBoxVertex()};
        const char* boxPixel[] = {PcShaderBoxPixel()};
        boxProgram = LinkProgram(CompileShader(GL_VERTEX_SHADER, boxVertex, 1),
                                   CompileShader(GL_FRAGMENT_SHADER, boxPixel, 1));
        boxMatrix = PcGl::GetUniformLocation(boxProgram, "uMatrix");
        boxColour = PcGl::GetUniformLocation(boxProgram, "uColour");
        const char* cursorPixel[] = {PcShaderCursorPixel()};
        s_cursorProgram = LinkProgram(CompileShader(GL_VERTEX_SHADER, presentVertex, 1),
                                      CompileShader(GL_FRAGMENT_SHADER, cursorPixel, 1));
        PcGl::UseProgram(s_cursorProgram);
        PcGl::Uniform1i(PcGl::GetUniformLocation(s_cursorProgram, "uImage"), 0);
        PcGl::GenTextures(1, &s_cursorTexture);
        PcGl::BindTexture(GL_TEXTURE_2D, s_cursorTexture);
        std::vector<unsigned int> cursor(kPcCursorImage, kPcCursorImage + kPcCursorImageSize * kPcCursorImageSize);
        FILE* cursorFile = fopen((PcContentRoot() + "/PC/Cursor.rgba").c_str(), "rb");
        if (cursorFile != 0)
        {
            std::vector<unsigned int> loaded(cursor.size());
            if (fread(&loaded[0], 4, loaded.size(), cursorFile) == loaded.size())
            {
                cursor = loaded;
                printf("[pc] cursor: using PC/Cursor.rgba\n");
            }
            fclose(cursorFile);
        }
        PcGl::TexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, kPcCursorImageSize, kPcCursorImageSize, 0, GL_RGBA,
                         GL_UNSIGNED_BYTE, &cursor[0]);
        PcGl::TexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
        PcGl::TexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
        PcGl::TexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        PcGl::TexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
        InvalidateGlCache();
        BindRenderTarget(0);
    }

    mDefaultTexture = TextureCreate();
    TextureBind(mDefaultTexture);
    std::vector<unsigned char> white(16 * 16 * 4, 0xFF);
    TextureData(16, 16, &white[0], 0, C4JRender::eTextureFormat_RGBA8);
    mDefaultTextureLocked = true;
    mVertexTexture = mDefaultTexture;
}

void Renderer::InitialiseContext()
{
    getContext();
}

void Renderer::MatrixMode(int mode)
{
    getContext().mMatrixMode = mode;
}

void Renderer::MatrixSetIdentity()
{
    Context& ctx = getContext();
    ASM_MTX44Identity(ctx.mMatrixStack[ctx.mMatrixMode][ctx.mMatrixStackDepth[ctx.mMatrixMode]]);
    ctx.mMatrixDirty[ctx.mMatrixMode] = true;
}

void Renderer::MultWithStack(float (*mat)[4])
{
    Context& ctx = getContext();
    float(*top)[4] = ctx.mMatrixStack[ctx.mMatrixMode][ctx.mMatrixStackDepth[ctx.mMatrixMode]];
    ASM_MTX44Concat(mat, top, top);
    ctx.mMatrixDirty[ctx.mMatrixMode] = true;
}

void Renderer::MatrixTranslate(float x, float y, float z)
{
    float mat[4][4];
    ASM_MTX44Trans(mat, x, y, z);
    ASM_MTX44Transpose(mat, mat);
    MultWithStack(mat);
}

void Renderer::MatrixRotate(float angle, float x, float y, float z)
{
    Vec axis;
    axis.x = x;
    axis.y = y;
    axis.z = z;
    float mat[4][4];
    ASM_MTX44RotAxisRad(mat, &axis, angle);
    ASM_MTX44Transpose(mat, mat);
    MultWithStack(mat);
}

void Renderer::MatrixScale(float x, float y, float z)
{
    float mat[4][4];
    ASM_MTX44Scale(mat, x, y, z);
    MultWithStack(mat);
}

void Renderer::MatrixPerspective(float fov, float aspect, float nearDist, float farDist)
{
    float mat[4][4];
    C_MTXPerspective(mat, fov, aspect, nearDist, farDist);
    ASM_MTX44Transpose(mat, mat);
    MultWithStack(mat);
}

void Renderer::MatrixOrthogonal(float left, float right, float bottom, float top, float nearDist, float farDist)
{
    float mat[4][4];
    C_MTXOrtho(mat, top, bottom, left, right, nearDist, farDist);
    ASM_MTX44Transpose(mat, mat);
    MultWithStack(mat);
}

void Renderer::MatrixPop()
{
    Context& ctx = getContext();
    if (ctx.mMatrixStackDepth[ctx.mMatrixMode] > 0)
        --ctx.mMatrixStackDepth[ctx.mMatrixMode];
    ctx.mMatrixDirty[ctx.mMatrixMode] = true;
}

void Renderer::MatrixPush()
{
    Context& ctx = getContext();
    int& depth = ctx.mMatrixStackDepth[ctx.mMatrixMode];
    if (depth + 1 >= kMatrixStackDepth)
        return;
    memcpy(ctx.mMatrixStack[ctx.mMatrixMode][depth + 1], ctx.mMatrixStack[ctx.mMatrixMode][depth],
           sizeof(ctx.mMatrixStack[0][0]));
    ++depth;
}

void Renderer::MatrixMult(float* mat)
{
    float local[4][4];
    memcpy(local, mat, sizeof(local));
    MultWithStack(local);
}

float* Renderer::MatrixGet(int mode)
{
    Context& ctx = getContext();
    return &ctx.mMatrixStack[mode][ctx.mMatrixStackDepth[mode]][0][0];
}

void Renderer::Set_matrixDirty()
{
    Context& ctx = getContext();
    for (int mode = 0; mode < kMatrixModes; ++mode)
    {
        ASM_MTX44Identity(ctx.mMatrixStack[mode][0]);
        ctx.mMatrixDirty[mode] = true;
    }
}

GX2ColorBuffer* Renderer::GetColorBuffer()
{
    return 0;
}

GX2DepthBuffer* Renderer::GetDepthStencilBuffer()
{
    return 0;
}

GX2ColorBuffer* Renderer::GetColorBufferDRC()
{
    return 0;
}

GX2DepthBuffer* Renderer::GetDepthStencilBufferDRC()
{
    return 0;
}

void Renderer::StartFrame()
{
    TextureBind(-1);
    StateSetColour(1.0f, 1.0f, 1.0f, 1.0f);
    StateSetDepthFunc(DEPTH_FUNC_LESS_EQUAL);
    StateSetDepthMask(true);
    StateSetStencil(COMPARE_ALWAYS, 0, 0, 0);
    StateSetBlendEnable(true);
    StateSetBlendFunc(BLEND_SOURCE_ALPHA, BLEND_INVERSE_SOURCE_ALPHA);
    StateSetBlendFactor(0xFFFFFFFF);
    StateSetAlphaTestEnable(false);
    StateSetAlphaFunc(4, DEFAULT_ALPHA_REFERENCE);
    StateSetFaceCull(true);
    StateSetLineWidth(1.0f);
    StateSetWriteEnable(true, true, true, true);
    StateSetDepthTestEnable(false);
    StateSetAlphaTestEnable(true);
    BindRenderTarget(0);
    SetViewportRect(0, 0, kScreenWidth[0], kScreenHeight[0]);
    s_state.currentViewport = C4JRender::eViewportType_FullScreen;
}

void Renderer::TEMP_PatchStateFromIggy()
{
}

void Renderer::ResetRenderState()
{
    PcCheckLeftovers("the menus");
    StartFrame();
    StateSetAlphaTestEnable(false);
}

void Renderer::StartDRCFrame(bool)
{
    StartFrame();
    BindRenderTarget(1);
    SetViewportRect(0, 0, kScreenWidth[1], kScreenHeight[1]);
    s_drcFrameStarted = true;
}

void Renderer::DoScreenGrabOnNextPresent()
{
}

void Renderer::WriteScreenshot()
{
    int w = s_targetWidth[0];
    int h = s_targetHeight[0];
    std::vector<unsigned char> pixels(w * h * 4);
    PcGl::BindFramebuffer(GL_FRAMEBUFFER, frameBuffers[0]);
    PcGl::PixelStorei(GL_PACK_ALIGNMENT, 1);
    PcGl::ReadPixels(0, 0, w, h, GL_RGBA, GL_UNSIGNED_BYTE, &pixels[0]);
    png_image img;
    memset(&img, 0, sizeof(img));
    img.version = PNG_IMAGE_VERSION;
    img.width = w;
    img.height = h;
    img.format = PNG_FORMAT_RGBA_BYTES;
    int stride = -w * 4;
    png_image_write_to_file(&img, s_screenshotPath.c_str(), 0, &pixels[0], stride, 0);
    printf("[pc] gl: wrote %s\n", s_screenshotPath.c_str());
    fflush(stdout);
    s_screenshotPath.clear();
}

void Renderer::ReleasePendingObjects()
{
    EnterTimedCriticalSection(&mCommandBufferMutex, "Wait command buffer lock");
    std::vector<unsigned int> buffers;
    buffers.swap(mPendingBufferDeletes);
    std::vector<unsigned int> vertexArrays;
    vertexArrays.swap(mPendingVertexArrayDeletes);
    LeaveCriticalSection(&mCommandBufferMutex);
    if (!vertexArrays.empty())
        PcGl::DeleteVertexArrays((GLsizei)vertexArrays.size(), &vertexArrays[0]);
    if (!buffers.empty())
        PcGl::DeleteBuffers((GLsizei)buffers.size(), &buffers[0]);

    EnterTimedCriticalSection(&mTextureMutex, "Wait texture lock");
    std::vector<unsigned int> textures;
    textures.swap(mPendingTextureDeletes);
    LeaveCriticalSection(&mTextureMutex);
    if (!textures.empty())
        PcGl::DeleteTextures((GLsizei)textures.size(), &textures[0]);
}

void Renderer::DrawMenuCursor(int windowWidth, int windowHeight, int frameWidth)
{
    int x = 0;
    int y = 0;
    if (!PcInputMenuCursor(&x, &y))
        return;
    int scale = (frameWidth + kScreenWidth[0] / 2) / kScreenWidth[0];
    if (scale < 1)
        scale = 1;
    const int size = kPcCursorImageSize * scale;
    const int left = x - kPcCursorHotspotX * scale;
    const int top = y - kPcCursorHotspotY * scale;
    PcGl::Viewport(left, windowHeight - top - size, size, size);
    PcGl::Enable(GL_BLEND);
    PcGl::BlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    PcGl::UseProgram(s_cursorProgram);
    PcGl::BindTexture(GL_TEXTURE_2D, s_cursorTexture);
    PcGl::DrawArrays(GL_TRIANGLE_STRIP, 0, 4);
    PcGl::Disable(GL_BLEND);
    (void)windowWidth;
}

void Renderer::Present()
{
    ++mFrameCount;
    mVerticesDrawnLastFrame = mVerticesDrawnThisFrame;
    mVerticesDrawnThisFrame = 0;
    s_drcFrameStarted = false;
    if (!OnRenderThread())
        return;

    if (!s_screenshotPath.empty())
        WriteScreenshot();
    PcModViewsBeforePresent();

    int windowWidth = 0;
    int windowHeight = 0;
    PcOsWindowGetClientSize(&windowWidth, &windowHeight);
    if (windowWidth <= 0 || windowHeight <= 0)
    {
        windowWidth = s_presentWidth;
        windowHeight = s_presentHeight;
    }
    int frame[4];
    FitArea(windowWidth, windowHeight, s_targetWidth[0], s_targetHeight[0], frame);
    int uiFrame[4];
    FitArea(windowWidth, windowHeight, kScreenWidth[0], kScreenHeight[0], uiFrame);

    PcGl::BindFramebuffer(GL_FRAMEBUFFER, 0);
    PcGl::Disable(GL_SCISSOR_TEST);
    PcGl::ColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE);
    PcGl::ClearColor(0.0f, 0.0f, 0.0f, 1.0f);
    PcGl::Clear(GL_COLOR_BUFFER_BIT);
    PcGl::Viewport(frame[0], frame[1], frame[2], frame[3]);
    PcGl::Disable(GL_DEPTH_TEST);
    PcGl::Disable(GL_BLEND);
    PcGl::Disable(GL_CULL_FACE);
    PcGl::Disable(GL_STENCIL_TEST);
    const float gamma = 1.0f / (static_cast<float>(s_gamma) * 3.0517578125e-05f * 1.5f + 0.5f);
    bool viewOnly = false;
    if (s_underlayFramebuffer != 0)
    {
        int sourceWidth = s_underlayWidth;
        int sourceHeight = s_underlayHeight;
        if (static_cast<long long>(sourceWidth) * frame[3] > static_cast<long long>(sourceHeight) * frame[2])
            sourceWidth = static_cast<int>(static_cast<long long>(sourceHeight) * frame[2] / frame[3]);
        else
            sourceHeight = static_cast<int>(static_cast<long long>(sourceWidth) * frame[3] / frame[2]);
        const int sourceX = (s_underlayWidth - sourceWidth) / 2;
        const int sourceY = (s_underlayHeight - sourceHeight) / 2;
        PcGl::BindFramebuffer(GL_READ_FRAMEBUFFER, s_underlayFramebuffer);
        PcGl::BindFramebuffer(GL_DRAW_FRAMEBUFFER, 0);
        PcGl::BlitFramebuffer(sourceX, sourceY, sourceX + sourceWidth, sourceY + sourceHeight, frame[0], frame[1],
                              frame[0] + frame[2], frame[1] + frame[3], GL_COLOR_BUFFER_BIT, GL_LINEAR);
        PcGl::BindFramebuffer(GL_FRAMEBUFFER, 0);
        PcGl::Enable(GL_BLEND);
        PcGl::BlendFunc(GL_ONE, GL_ONE_MINUS_SRC_ALPHA);
        PcGl::UseProgram(s_presentOverProgram);
        PcGl::Uniform1f(s_presentOverGamma, gamma);
        s_underlayFramebuffer = 0;
        viewOnly = s_underlayAlone;
    }
    else
    {
        PcGl::UseProgram(s_presentProgram);
        PcGl::Uniform1f(s_presentGamma, gamma);
    }
    if (!viewOnly)
    {
        PcGl::ActiveTexture(GL_TEXTURE0);
        PcGl::BindTexture(GL_TEXTURE_2D, s_colourTextures[0]);
        PcGl::DrawArrays(GL_TRIANGLE_STRIP, 0, 4);
    }
    PcGl::Disable(GL_BLEND);
    if (!viewOnly)
        DrawMenuCursor(windowWidth, windowHeight, uiFrame[2]);
    if (PcProfilerSyncGpu())
    {
        PIXBeginNamedEvent(0.0f, "GPU finish");
        PcGl::Finish();
        PIXEndNamedEvent();
    }
    PIXBeginNamedEvent(0.0f, "Swap buffers");
    PcOsWindowSwapBuffers();
    PIXEndNamedEvent();
    PIXBeginNamedEvent(0.0f, "Wait for previous frame");
    if (s_frameFence != 0)
    {
        PcGl::ClientWaitSync(s_frameFence, GL_SYNC_FLUSH_COMMANDS_BIT, kFrameFenceTimeoutNanoseconds);
        PcGl::DeleteSync(s_frameFence);
    }
    s_frameFence = PcGl::FenceSync(GL_SYNC_GPU_COMMANDS_COMPLETE, 0);
    PIXEndNamedEvent();

    if (s_fixedRenderHeight > 0)
    {
        ResizeTvTarget(s_fixedRenderHeight * kScreenWidth[0] / kScreenHeight[0], s_fixedRenderHeight);
    }
    else
    {
        int targetWidth = kScreenWidth[0];
        int targetHeight = kScreenHeight[0];
        if (s_displayMode == ePcDisplay_Window)
        {
            targetWidth = s_letterbox ? uiFrame[2] : windowWidth;
            targetHeight = s_letterbox ? uiFrame[3] : windowHeight;
        }
        const int scale = PcSettingsRenderScalePercent();
        targetWidth = targetWidth * scale / 100;
        targetHeight = targetHeight * scale / 100;
        if (targetWidth >= kMinimumTargetSize && targetHeight >= kMinimumTargetSize)
            ResizeTvTarget(targetWidth, targetHeight);
    }
    ReleasePendingObjects();
    BindRenderTarget(0);
    ClearOutsideUiArea();
    InvalidateGlCache();
}

void Renderer::Clear(int flags)
{
    if (!OnRenderThread())
        return;
    ApplyViewportAndScissor();
    GLbitfield mask = 0;
    if ((flags & 1) != 0)
    {
        GlDepthMask(true);
        PcGl::ClearDepth(1.0);
        mask |= GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT;
        if (s_gl.stencilMask != 0xFF)
        {
            s_gl.stencilMask = 0xFF;
            PcGl::StencilMask(0xFF);
        }
    }
    if ((flags & 2) != 0)
    {
        GlColourMask(true, true, true, true);
        PcGl::ClearColor(s_state.clearColour[0], s_state.clearColour[1], s_state.clearColour[2],
                         s_state.clearColour[3]);
        mask |= GL_COLOR_BUFFER_BIT;
    }
    if (mask != 0)
        PcGl::Clear(mask);
    GlDepthMask(s_state.depthMask);
    GlColourMask(s_state.colourWrite[0], s_state.colourWrite[1], s_state.colourWrite[2], s_state.colourWrite[3]);
}

void Renderer::SetClearColour(const float* colour)
{
    memcpy(s_state.clearColour, colour, sizeof(s_state.clearColour));
}

bool Renderer::IsWidescreen()
{
    return true;
}

bool Renderer::IsHiDef()
{
    return true;
}

static void EncodeThumbnail(ImageFileBuffer* buf, int len, std::vector<unsigned char>& px)
{
    const int capacity = len * len * 24 / 5 + 0x100;
    void* png = malloc(capacity);
    if (png == 0)
        return;
    int written = 0;
    gRenderer->SaveTextureDataToMemory(png, capacity, &written, len, len, reinterpret_cast<int*>(&px[0]));
    if (written <= 0)
    {
        free(png);
        return;
    }
    buf->mData = png;
    buf->mSize = written;
}

void Renderer::CaptureThumbnail(ImageFileBuffer* buf)
{
    const int kSize = 64;
    buf->mFormat = 0;
    buf->mData = 0;
    buf->mSize = 0;
    std::vector<unsigned char> thumbnail(kSize * kSize * 4);
    if (!s_glReady)
    {
        for (size_t i = 0; i < thumbnail.size(); i += 4)
        {
            thumbnail[i] = 0x55;
            thumbnail[i + 1] = 0x8a;
            thumbnail[i + 2] = 0x3b;
            thumbnail[i + 3] = 255;
        }
        EncodeThumbnail(buf, kSize, thumbnail);
        return;
    }
    GLint rect[4];
    ToTargetRect(s_state.viewport, rect);
    const int side = rect[2] < rect[3] ? rect[2] : rect[3];
    if (side < kSize)
        return;
    const int left = rect[0] + (rect[2] - side) / 2;
    const int bottom = rect[1] + (rect[3] - side) / 2;
    std::vector<unsigned char> pixels((size_t)side * side * 4);
    PcGl::PixelStorei(GL_PACK_ALIGNMENT, 1);
    PcGl::ReadPixels(left, bottom, side, side, GL_RGBA, GL_UNSIGNED_BYTE, &pixels[0]);

    for (int ty = 0; ty < kSize; ++ty)
    {
        const int y0 = (kSize - 1 - ty) * side / kSize;
        const int y1 = (kSize - ty) * side / kSize;
        for (int tx = 0; tx < kSize; ++tx)
        {
            const int x0 = tx * side / kSize;
            const int x1 = (tx + 1) * side / kSize;
            unsigned int sum[3] = {0, 0, 0};
            for (int y = y0; y < y1; ++y)
            {
                const unsigned char* row = &pixels[((size_t)y * side + x0) * 4];
                for (int x = x0; x < x1; ++x, row += 4)
                {
                    sum[0] += row[0];
                    sum[1] += row[1];
                    sum[2] += row[2];
                }
            }
            const unsigned int n = (unsigned int)((y1 - y0) * (x1 - x0));
            unsigned char* out = &thumbnail[(ty * kSize + tx) * 4];
            for (int ch = 0; ch < 3; ++ch)
                out[ch] = (unsigned char)(n != 0 ? sum[ch] / n : 0);
            out[3] = 255;
        }
    }

    EncodeThumbnail(buf, kSize, thumbnail);
}

void Renderer::CaptureScreen(ImageFileBuffer* buf, _XSOCIAL_PREVIEWIMAGE*)
{
    buf->mData = 0;
    buf->mSize = 0;
}

void Renderer::BeginConditionalSurvey(int)
{
}

void Renderer::EndConditionalSurvey()
{
}

void Renderer::BeginConditionalRendering(int)
{
}

void Renderer::EndConditionalRendering()
{
}

void Renderer::BeginOrderIndependentAlpha()
{
}

void Renderer::EndOrderIndependentAlpha()
{
}

void Renderer::ReleaseForeground()
{
}

void Renderer::AcquiredForeground()
{
}

void Renderer::WiiUWaitDrawDone()
{
}

int Renderer::GetFramesSinceBoot()
{
    return mFrameCount;
}

void Renderer::SetVertexRegisters(int first, int n, const float* values)
{
    if (memcmp(vertexRegisters[first], values, n * 4 * sizeof(float)) == 0)
        return;
    memcpy(vertexRegisters[first], values, n * 4 * sizeof(float));
    if (first < vertexDirtyFirst)
        vertexDirtyFirst = first;
    if (first + n > vertexDirtyLast)
        vertexDirtyLast = first + n;
}

void Renderer::SetPixelRegisters(int first, int n, const float* values)
{
    if (memcmp(pixelRegisters[first], values, n * 4 * sizeof(float)) == 0)
        return;
    memcpy(pixelRegisters[first], values, n * 4 * sizeof(float));
    if (first < pixelDirtyFirst)
        pixelDirtyFirst = first;
    if (first + n > pixelDirtyLast)
        pixelDirtyLast = first + n;
}

namespace
{
GLintptr AllocateRegisters(GLsizeiptr len, bool* wrapped)
{
    GLintptr off = (registerRingOffset + registerAlignment - 1) / registerAlignment * registerAlignment;
    if (off + len > REGISTER_RING_SIZE)
    {
        PcGl::BufferData(GL_UNIFORM_BUFFER, REGISTER_RING_SIZE, 0, GL_STREAM_DRAW);
        off = 0;
        *wrapped = true;
    }
    registerRingOffset = off + len;
    return off;
}

void UploadRegisterBlock(GLuint binding, const void* data, GLsizeiptr len, bool* wrapped)
{
    GLintptr off = AllocateRegisters(len, wrapped);
    PcGl::BufferSubData(GL_UNIFORM_BUFFER, off, len, data);
    PcGl::BindBufferRange(GL_UNIFORM_BUFFER, binding, registerRing, off, len);
}
}

void Renderer::UploadRegisters()
{
    bool vertex = vertexDirtyLast > vertexDirtyFirst && vertexDirtyFirst < VERTEX_BASE_REGISTERS;
    bool bones = vertexDirtyLast > VERTEX_BASE_REGISTERS;
    bool pixel = pixelDirtyLast > pixelDirtyFirst;
    if (vertex || bones || pixel)
    {
        PIXBeginNamedEvent(0.0f, "GL register upload");
        GlBindBuffer(GL_UNIFORM_BUFFER, registerRing);
        bool wrapped = false;
        for (int pass = 0; pass < 2; ++pass)
        {
            if (vertex)
                UploadRegisterBlock(0, vertexRegisters[0], VERTEX_BASE_REGISTERS * 16, &wrapped);
            if (pixel)
                UploadRegisterBlock(1, pixelRegisters[0], PIXEL_REGISTERS * 16, &wrapped);
            if (bones)
                UploadRegisterBlock(2, vertexRegisters[VERTEX_BASE_REGISTERS], BONE_REGISTER_COUNT * 16, &wrapped);
            if (!wrapped || pass == 1)
                break;
            vertex = true;
            pixel = true;
            bones = true;
            wrapped = false;
        }
        PIXEndNamedEvent();
    }
    vertexDirtyFirst = VERTEX_REGISTERS;
    vertexDirtyLast = 0;
    pixelDirtyFirst = PIXEL_REGISTERS;
    pixelDirtyLast = 0;
}

unsigned int Renderer::GetProgram(int vertexShader, int pixelShader)
{
    ProgramInfo& info = programs[vertexShader][pixelShader];
    if (info.program != 0)
        return info.program;

    const char* vertexSources[] = {PcShaderCommonHeader(),
                                   vertexShader == ePcVertexShader_EntityLighting ? "#define ENTITY_LIGHTING 1\n" : "",
                                   PcShaderVertexCommon(),
                                   PcShaderVertexSource(static_cast<PcVertexShader>(vertexShader))};
    const char* pixelSources[] = {PcShaderCommonHeader(), PcShaderPixelCommon(),
                                  PcShaderPixelSource(static_cast<PcPixelShader>(pixelShader))};
    info.program = LinkProgram(CompileShader(GL_VERTEX_SHADER, vertexSources, 4),
                               CompileShader(GL_FRAGMENT_SHADER, pixelSources, 3));
    GLuint prog = info.program;
    PcGl::UniformBlockBinding(prog, PcGl::GetUniformBlockIndex(prog, "VertexRegisters"), 0);
    PcGl::UniformBlockBinding(prog, PcGl::GetUniformBlockIndex(prog, "PixelRegisters"), 1);
    GLuint boneBlock = PcGl::GetUniformBlockIndex(prog, "BoneRegisters");
    if (boneBlock != GL_INVALID_INDEX)
        PcGl::UniformBlockBinding(prog, boneBlock, 2);
    GlUseProgram(prog);
    PcGl::Uniform1i(PcGl::GetUniformLocation(prog, "uTexture0"), 0);
    PcGl::Uniform1i(PcGl::GetUniformLocation(prog, "uTexture1"), 1);
    PcGl::Uniform1i(PcGl::GetUniformLocation(prog, "uTexture2"), 2);
    PcGl::Uniform1i(PcGl::GetUniformLocation(prog, "uTexture3"), 3);
    PcGl::Uniform1i(PcGl::GetUniformLocation(prog, "uVertexTexture"), VERTEX_TEXTURE_UNIT);
    GLint lighting = PcGl::GetUniformLocation(prog, "uLighting");
    if (lighting >= 0)
        PcGl::Uniform1i(lighting, vertexShader == ePcVertexShader_EntityLighting ? 1 : 0);
    info.alphaReference = PcGl::GetUniformLocation(prog, "uAlphaReference");
    info.alphaValue = -1.0f;
    info.layers = PcGl::GetUniformLocation(prog, "uLayers");
    info.forceLod = PcGl::GetUniformLocation(prog, "uForceLod");
    if (info.layers >= 0)
    {
        int layers = 1;
        if (pixelShader == ePcPixelShader_DefaultTwoLayers)
            layers = 2;
        else if (pixelShader == ePcPixelShader_DefaultThreeLayers)
            layers = 3;
        else if (pixelShader == ePcPixelShader_DefaultFourLayers)
            layers = 4;
        PcGl::Uniform1i(info.layers, layers);
    }
    if (info.forceLod >= 0)
        PcGl::Uniform1i(info.forceLod, pixelShader == ePcPixelShader_TerrainForceLod ? 1 : 0);
    return prog;
}

int Renderer::FinalisePixelShaderType(int pixelShaderType)
{
    Context& ctx = getContext();
    if (pixelShaderType != C4JRender::ePixelShaderType_Default)
        return pixelShaderType;
    if (ctx.mPixelTexture[1] == mDefaultTexture)
        return C4JRender::ePixelShaderType_Default;
    if (ctx.mPixelTexture[2] == mDefaultTexture)
        return ctx.mTextureIsMask[1] ? C4JRender::ePixelShaderType_LayeredColourMask
                                         : C4JRender::ePixelShaderType_DefaultTwoLayers;
    if (ctx.mPixelTexture[3] == mDefaultTexture)
        return C4JRender::ePixelShaderType_DefaultThreeLayers;
    return C4JRender::ePixelShaderType_DefaultFourLayers;
}

void Renderer::PrepareTexture(int unit, int tex)
{
    Texture& e = mTextures[tex];
    if (e.mName == 0)
        PcGl::GenTextures(1, &e.mName);
    GlBindTexture(unit, e.mName);
    if (e.mStorageDirty || e.mParametersDirty)
        GlActiveTexture(unit);
    if (e.mStorageDirty)
    {
        EnterTimedCriticalSection(&mTextureMutex, "Wait texture lock");
        PcGl::PixelStorei(GL_UNPACK_ALIGNMENT, 1);
        PcGl::TexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAX_LEVEL, e.mLevels > 0 ? e.mLevels - 1 : 0);
        for (unsigned int lvl = 0; lvl < e.mPendingLevels.size(); ++lvl)
        {
            std::vector<unsigned char>& px = e.mPendingLevels[lvl];
            if (px.empty())
                continue;
            int w = e.mWidth >> lvl;
            int h = e.mHeight >> lvl;
            if (w < 1)
                w = 1;
            if (h < 1)
                h = 1;
            PcGl::TexImage2D(GL_TEXTURE_2D, lvl, GL_RGBA8, w, h, 0, GL_RGBA, GL_UNSIGNED_BYTE, &px[0]);
            std::vector<unsigned char>().swap(px);
        }
        e.mStorageDirty = false;
        e.mParametersDirty = true;
        LeaveCriticalSection(&mTextureMutex);
    }
    if (e.mParametersDirty)
    {
        bool mipmapped = e.mLevels > 1;
        bool minLinear = (e.mFlags & TEXTURE_FLAG_MIN_LINEAR) != 0;
        GLint minFilter = mipmapped ? (minLinear ? GL_LINEAR_MIPMAP_LINEAR : GL_NEAREST_MIPMAP_LINEAR)
                                    : (minLinear ? GL_LINEAR : GL_NEAREST);
        PcGl::TexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, minFilter);
        PcGl::TexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER,
                            (e.mFlags & TEXTURE_FLAG_MAG_LINEAR) != 0 ? GL_LINEAR : GL_NEAREST);
        PcGl::TexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S,
                            (e.mFlags & TEXTURE_FLAG_CLAMP_U) != 0 ? GL_CLAMP_TO_EDGE : GL_REPEAT);
        PcGl::TexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T,
                            (e.mFlags & TEXTURE_FLAG_CLAMP_V) != 0 ? GL_CLAMP_TO_EDGE : GL_REPEAT);
        e.mParametersDirty = false;
    }
}

void Renderer::BindTextures()
{
    Context& ctx = getContext();
    for (int stage = 0; stage < kTextureStages; ++stage)
    {
        int tex = ctx.mPixelTexture[stage];
        if (tex < 0 || tex >= kMaxTextures || !mTextures[tex].mUsed)
            tex = mDefaultTexture;
        PrepareTexture(stage, tex);
    }
    int vertexTexture = mVertexTexture;
    if (vertexTexture < 0 || vertexTexture >= kMaxTextures || !mTextures[vertexTexture].mUsed)
        vertexTexture = mDefaultTexture;
    PrepareTexture(VERTEX_TEXTURE_UNIT, vertexTexture);
}

void Renderer::ApplyRenderState()
{
    ApplyViewportAndScissor();
    GlSetEnabled(eGlCapability_Blend, s_state.blendEnable);
    if (s_state.blendEnable)
    {
        GLint blendFunc[2] = {(GLint)BLEND_FACTORS[s_state.blendSource % 21],
                              (GLint)BLEND_FACTORS[s_state.blendDestination % 21]};
        if (GlChanged(s_gl.blendFunc, blendFunc, sizeof(blendFunc)))
            PcGl::BlendFunc(blendFunc[0], blendFunc[1]);
        if (GlChanged(s_gl.blendColour, s_state.blendFactor, sizeof(s_gl.blendColour)))
            PcGl::BlendColor(s_state.blendFactor[0], s_state.blendFactor[1], s_state.blendFactor[2],
                             s_state.blendFactor[3]);
    }
    GlSetEnabled(eGlCapability_DepthTest, s_state.depthTestEnable);
    if (s_state.depthTestEnable)
    {
        GLint depthFunc = (GLint)COMPARE_FUNCTIONS[s_state.depthFunc & 7];
        if (GlChanged(&s_gl.depthFunc, &depthFunc, sizeof(depthFunc)))
            PcGl::DepthFunc(depthFunc);
    }
    GlDepthMask(s_state.depthMask);
    GlSetEnabled(eGlCapability_CullFace, s_state.faceCull);
    if (s_state.faceCull)
    {
        GLint cullFace = GL_BACK;
        if (GlChanged(&s_gl.cullFace, &cullFace, sizeof(cullFace)))
            PcGl::CullFace(GL_BACK);
    }
    GLfloat lineWidth = s_state.lineWidth * (float)s_targetHeight[s_screen] / kScreenHeight[s_screen];
    if (lineWidth < 1.0f)
        lineWidth = 1.0f;
    if (GlChanged(&s_gl.lineWidth, &lineWidth, sizeof(lineWidth)))
        PcGl::LineWidth(lineWidth);
    GLint frontFace = s_state.frontFaceClockwise ? GL_CW : GL_CCW;
    if (GlChanged(&s_gl.frontFace, &frontFace, sizeof(frontFace)))
        PcGl::FrontFace(frontFace);
    GlColourMask(s_state.colourWrite[0], s_state.colourWrite[1], s_state.colourWrite[2], s_state.colourWrite[3]);
    bool stencil = s_state.stencilFunc != COMPARE_ALWAYS || s_state.stencilWriteMask != 0;
    GlSetEnabled(eGlCapability_StencilTest, stencil);
    if (stencil)
    {
        GLint stencilFunc[3] = {(GLint)COMPARE_FUNCTIONS[s_state.stencilFunc & 7],
                                s_state.stencilReference, s_state.stencilCompareMask};
        if (GlChanged(s_gl.stencilFunc, stencilFunc, sizeof(stencilFunc)))
            PcGl::StencilFunc(stencilFunc[0], stencilFunc[1], stencilFunc[2]);
        GLint stencilOp[3] = {GL_KEEP, GL_KEEP, GL_REPLACE};
        if (GlChanged(s_gl.stencilOp, stencilOp, sizeof(stencilOp)))
            PcGl::StencilOp(GL_KEEP, GL_KEEP, GL_REPLACE);
        GLint stencilMask = s_state.stencilWriteMask;
        if (GlChanged(&s_gl.stencilMask, &stencilMask, sizeof(stencilMask)))
            PcGl::StencilMask(stencilMask);
    }
    bool polygonOffset = s_state.polygonSlope != 0.0f || s_state.polygonBias != 0.0f;
    GlSetEnabled(eGlCapability_PolygonOffsetFill, polygonOffset);
    if (polygonOffset)
    {
        GLfloat off[2] = {s_state.polygonSlope, s_state.polygonBias};
        if (GlChanged(s_gl.polygonOffset, off, sizeof(off)))
            PcGl::PolygonOffset(off[0], off[1]);
    }
}

void Renderer::UpdateFogRegisters()
{
    float fog[4] = {0.0f, 0.0f, 0.0f, 0.0f};
    if (s_state.fogEnable)
    {
        if (s_state.fogMode == 1)
        {
            fog[0] = s_state.fogFar;
            fog[1] = s_state.fogNear;
            fog[2] = 1.0f;
        }
        else
        {
            fog[0] = s_state.fogDensity;
            fog[2] = s_state.fogMode == 2 ? 2.0f : 3.0f;
        }
        fog[3] = s_state.vertexAlphaDistance;
    }
    SetVertexRegisters(VERTEX_REGISTER_FOG, 1, fog);
}

void Renderer::UpdateLightingRegisters()
{
    if (!s_state.lightingEnable)
        return;
    float lights[5][4];
    memset(lights, 0, sizeof(lights));
    for (int light = 0; light < 2; ++light)
    {
        if (s_state.lightEnable[light])
        {
            memcpy(lights[light], s_state.lightDirection[light], sizeof(lights[light]));
            memcpy(lights[2 + light], s_state.lightColour[light], sizeof(lights[light]));
        }
    }
    memcpy(lights[4], s_state.lightAmbient, sizeof(lights[4]));
    SetVertexRegisters(VERTEX_REGISTER_LIGHTS, 5, &lights[0][0]);
}

void Renderer::ApplyUniforms(int vertexShader, int pixelShader, unsigned int prog)
{
    Context& ctx = getContext();
    SetVertexRegisters(VERTEX_REGISTER_WORLD_VIEW, 4, MatrixGet(0));
    SetVertexRegisters(VERTEX_REGISTER_PROJECTION, 4, MatrixGet(1));
    SetVertexRegisters(VERTEX_REGISTER_TEXTURE, 4, MatrixGet(2));
    UpdateFogRegisters();
    if (vertexShader != ePcVertexShader_Terrain)
        UpdateLightingRegisters();
    SetVertexRegisters(VERTEX_REGISTER_TEX_GEN, 4, &s_state.texGen[0][0][0]);
    SetVertexRegisters(VERTEX_REGISTER_TEX_GEN + 4, 4, &s_state.texGen[1][0][0]);
    if (ctx.mNumBoneMatrices > 0)
    {
        SetVertexRegisters(VERTEX_REGISTER_BONES, ctx.mNumBoneMatrices * 4, &ctx.mBoneMatrix[0][0][0]);
        ctx.mNumBoneMatrices = 0;
    }
    float cols[8][4];
    memcpy(cols[0], s_state.colour, sizeof(cols[0]));
    memcpy(cols[1], s_state.fogColour, sizeof(cols[1]));
    memcpy(cols[2], s_state.lerpColour, sizeof(cols[2]));
    memcpy(cols[3], s_state.colourAdd, sizeof(cols[3]));
    memcpy(cols[4], s_state.texColour, sizeof(s_state.texColour));
    SetPixelRegisters(PIXEL_REGISTER_COLOURS, 8, &cols[0][0]);
    if (pixelShader == ePcPixelShader_TerrainForceLod)
    {
        float forceLod[4] = {(float)s_state.forceLod, 0.0f, 0.0f, 0.0f};
        SetPixelRegisters(PIXEL_REGISTER_FORCE_LOD, 1, forceLod);
    }
    if (pixelShader == ePcPixelShader_LayeredColourMask)
    {
        SetPixelRegisters(PIXEL_REGISTER_LAYERED, 1, s_state.layeredTextureSize);
        SetPixelRegisters(PIXEL_REGISTER_LAYERED + 1, MAX_LAYERED_PARAMS, &s_state.layeredParams[0][0]);
    }
    UploadRegisters();
    GlUseProgram(prog);
    ProgramInfo& info = programs[vertexShader][pixelShader];
    GLfloat alphaValue = s_state.alphaTestEnable ? s_state.alphaReference : 0.0f;
    if (info.alphaReference >= 0 && info.alphaValue != alphaValue)
    {
        info.alphaValue = alphaValue;
        PcGl::Uniform1f(info.alphaReference, alphaValue);
    }
}

void Renderer::SetupVertexFormat(int vertexShader, unsigned int off)
{
    GlBindVertexArray(s_vertexArray);
    if (vertexShader == ePcVertexShader_Terrain)
        GlChunkOffset(vertexRegisters[VERTEX_REGISTER_LIGHTS]);
    GLint format[3] = {vertexShader, s_gl.arrayBuffer, (GLint)off};
    if (!GlChanged(s_gl.vertexFormat, format, sizeof(format)))
        return;
    const GLsizei stride = VERTEX_STRIDES[vertexShader];
    const char* base = reinterpret_cast<const char*>(static_cast<uintptr_t>(off));
    switch (vertexShader)
    {
    case ePcVertexShader_Terrain:
        GlEnableAttributes(0x3);
        PcGl::VertexAttribIPointer(0, 4, GL_SHORT, stride, base);
        PcGl::VertexAttribIPointer(1, 4, GL_SHORT, stride, base + 8);
        break;
    case ePcVertexShader_Ui:
    case ePcVertexShader_ScalingGridUi:
    case ePcVertexShader_TiledUi:
        GlEnableAttributes(0x1);
        PcGl::VertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, stride, base);
        break;
    case ePcVertexShader_ColouredUi:
        GlEnableAttributes(0x7);
        PcGl::VertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, stride, base);
        PcGl::VertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, stride, base + 8);
        PcGl::VertexAttribPointer(2, 4, GL_UNSIGNED_BYTE, GL_TRUE, stride, base + 16);
        break;
    default:
        GlEnableAttributes(0x1F);
        PcGl::VertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, stride, base);
        PcGl::VertexAttribPointer(1, 4, GL_UNSIGNED_BYTE, GL_TRUE, stride, base + 20);
        PcGl::VertexAttribPointer(2, 4, GL_BYTE, GL_TRUE, stride, base + 24);
        PcGl::VertexAttribPointer(3, 2, GL_FLOAT, GL_FALSE, stride, base + 12);
        PcGl::VertexAttribIPointer(4, 1, GL_INT, stride, base + 28);
        break;
    }
}

void Renderer::IssueDraw(int primitiveType, int firstVertex, int numVertices)
{
    PIXBeginNamedEvent(0.0f, "GL draw");
    switch (primitiveType)
    {
    case C4JRender::ePrimitiveType_Triangles:
        PcGl::DrawArrays(GL_TRIANGLES, firstVertex, numVertices);
        break;
    case C4JRender::ePrimitiveType_TriangleStrip:
        PcGl::DrawArrays(GL_TRIANGLE_STRIP, firstVertex, numVertices);
        break;
    case C4JRender::ePrimitiveType_TriangleFan:
        PcGl::DrawArrays(GL_TRIANGLE_FAN, firstVertex, numVertices);
        break;
    case C4JRender::ePrimitiveType_Quads:
        if (s_gl.vertexArray == (GLint)s_vertexArray)
            GlBindBuffer(GL_ELEMENT_ARRAY_BUFFER, quadIndexBuffer);
        PcGl::DrawElementsBaseVertex(GL_TRIANGLES, numVertices / 4 * 6, GL_UNSIGNED_INT, 0, firstVertex);
        break;
    case C4JRender::ePrimitiveType_Lines:
        PcGl::DrawArrays(GL_LINES, firstVertex, numVertices);
        break;
    case C4JRender::ePrimitiveType_LineStrip:
        PcGl::DrawArrays(GL_LINE_STRIP, firstVertex, numVertices);
        break;
    default:
        break;
    }
    PIXEndNamedEvent();
}

void Renderer::DrawVertices(C4JRender::ePrimitiveType primitiveType, int numVertices, void* vertices,
                            C4JRender::eVertexType vertexType, C4JRender::ePixelShaderType pixelShaderType,
                            unsigned char* layers)
{
    Context& ctx = getContext();
    if (ctx.mRecordingBuffer != 0)
    {
        int recordedType = vertexType;
        if (vertexType == C4JRender::eVertexType_Default && s_state.lightingEnable)
            recordedType = C4JRender::eVertexType_DefaultLighting;
        CommandBuffer* buf = ctx.mRecordingBuffer;
        buf->mVertexType = recordedType;
        buf->mPrimitiveType = primitiveType;
        buf->mPixelShaderType = pixelShaderType;
        buf->mVertexCount += numVertices;
        if (ctx.mMatrixDirty[3])
        {
            CommandBuffer::Command mat;
            mat.mIsMatrix = true;
            memcpy(mat.mMatrix, MatrixGet(3), sizeof(mat.mMatrix));
            mat.mOffset = 0;
            mat.mCount = 0;
            buf->mCommands.push_back(mat);
            ctx.mMatrixDirty[3] = false;
        }
        unsigned int stride = VERTEX_STRIDES[recordedType];
        CommandBuffer::Command draw;
        draw.mIsMatrix = false;
        draw.mOffset = (unsigned int)buf->mVertices.size();
        draw.mCount = numVertices;
        if (s_displayMode != ePcDisplay_Headless && vertices != 0)
        {
            const unsigned char* src = static_cast<const unsigned char*>(vertices);
            if (layers == 0)
                buf->mVertices.insert(buf->mVertices.end(), src, src + numVertices * stride);
            else
            {
                for (unsigned char layer = 0; layer < 7; ++layer)
                {
                    for (int quad = 0; quad < numVertices / 4; ++quad)
                    {
                        if (layers[quad] == layer)
                            buf->mVertices.insert(buf->mVertices.end(), src + quad * stride * 4,
                                                     src + (quad + 1) * stride * 4);
                    }
                }
            }
        }
        buf->mCommands.push_back(draw);
        return;
    }

    mVerticesDrawnThisFrame += numVertices;
    if (!OnRenderThread() || numVertices <= 0 || vertices == 0)
        return;

    int vertexShader = vertexType;
    if (vertexType == C4JRender::eVertexType_Default && s_state.lightingEnable)
        vertexShader = C4JRender::eVertexType_DefaultLighting;
    int pixelShader = FinalisePixelShaderType(pixelShaderType);
    unsigned int stride = VERTEX_STRIDES[vertexShader];
    unsigned int len = numVertices * stride;
    if (len > STREAM_BUFFER_SIZE)
        return;
    PIXBeginNamedEvent(0.0f, "GL stream upload");
    GlBindBuffer(GL_ARRAY_BUFFER, streamBuffer);
    unsigned int off = (streamOffset + stride - 1) / stride * stride;
    if (off + len > STREAM_BUFFER_SIZE)
    {
        PcGl::BufferData(GL_ARRAY_BUFFER, STREAM_BUFFER_SIZE, 0, GL_STREAM_DRAW);
        off = 0;
    }
    PcGl::BufferSubData(GL_ARRAY_BUFFER, off, len, vertices);
    streamOffset = off + len;
    PIXEndNamedEvent();

    float identity[16];
    SetIdentity(identity);
    SetVertexRegisters(VERTEX_REGISTER_WORLD_VIEW2, 4, identity);
    unsigned int prog = GetProgram(vertexShader, pixelShader);
    ApplyUniforms(vertexShader, pixelShader, prog);
    BindTextures();
    ApplyRenderState();
    SetupVertexFormat(vertexShader, 0);
    IssueDraw(primitiveType, off / stride, numVertices);
}

void Renderer::RenderCommandBuffer(CommandBuffer* buf, int vertexShader, bool bApplyMatrix)
{
    if (buf->mCountedBytes == 0)
        return;
    const unsigned int stride = VERTEX_STRIDES[vertexShader];
    if (vertexShader == ePcVertexShader_Terrain && buf->mSingleMatrix)
    {
        if (buf->mVertexArray == 0)
        {
            PIXBeginNamedEvent(0.0f, "GL command buffer upload");
            unsigned int offsetPosition = ((unsigned int)buf->mVertices.size() + 15) & ~15u;
            PcGl::GenBuffers(1, &buf->mBuffer);
            GlBindBuffer(GL_ARRAY_BUFFER, buf->mBuffer);
            PcGl::BufferData(GL_ARRAY_BUFFER, offsetPosition + 4 * sizeof(float), 0, GL_STATIC_DRAW);
            PcGl::BufferSubData(GL_ARRAY_BUFFER, 0, buf->mVertices.size(), &buf->mVertices[0]);
            PcGl::BufferSubData(GL_ARRAY_BUFFER, offsetPosition, 4 * sizeof(float), &buf->mCommands[0].mMatrix[12]);
            PcGl::GenVertexArrays(1, &buf->mVertexArray);
            GlBindVertexArray(buf->mVertexArray);
            PcGl::EnableVertexAttribArray(0);
            PcGl::VertexAttribIPointer(0, 4, GL_SHORT, stride, 0);
            PcGl::EnableVertexAttribArray(1);
            PcGl::VertexAttribIPointer(1, 4, GL_SHORT, stride, reinterpret_cast<const void*>(8));
            PcGl::EnableVertexAttribArray(CHUNK_OFFSET_ATTRIBUTE);
            PcGl::VertexAttribPointer(CHUNK_OFFSET_ATTRIBUTE, 4, GL_FLOAT, GL_FALSE, 0,
                                      reinterpret_cast<const void*>(static_cast<uintptr_t>(offsetPosition)));
            PcGl::VertexAttribDivisor(CHUNK_OFFSET_ATTRIBUTE, 1);
            PcGl::BindBuffer(GL_ELEMENT_ARRAY_BUFFER, quadIndexBuffer);
            std::vector<unsigned char>().swap(buf->mVertices);
            PIXEndNamedEvent();
        }
        GlBindVertexArray(buf->mVertexArray);
        for (unsigned int i = 1; i < buf->mCommands.size(); ++i)
        {
            const CommandBuffer::Command& cmd = buf->mCommands[i];
            if (cmd.mCount == 0)
                continue;
            IssueDraw(buf->mPrimitiveType, cmd.mOffset / stride, cmd.mCount);
            mVerticesDrawnThisFrame += cmd.mCount;
        }
        return;
    }
    if (buf->mBuffer == 0)
    {
        PIXBeginNamedEvent(0.0f, "GL command buffer upload");
        PcGl::GenBuffers(1, &buf->mBuffer);
        GlBindBuffer(GL_ARRAY_BUFFER, buf->mBuffer);
        PcGl::BufferData(GL_ARRAY_BUFFER, buf->mVertices.size(), &buf->mVertices[0], GL_STATIC_DRAW);
        std::vector<unsigned char>().swap(buf->mVertices);
        PIXEndNamedEvent();
    }
    else
        GlBindBuffer(GL_ARRAY_BUFFER, buf->mBuffer);
    SetupVertexFormat(vertexShader, 0);

    bool matrixPushed = false;
    for (unsigned int i = 0; i < buf->mCommands.size(); ++i)
    {
        const CommandBuffer::Command& cmd = buf->mCommands[i];
        if (cmd.mIsMatrix)
        {
            if (vertexShader == ePcVertexShader_Terrain)
            {
                SetVertexRegisters(VERTEX_REGISTER_LIGHTS, 1, &cmd.mMatrix[12]);
                GlChunkOffset(&cmd.mMatrix[12]);
            }
            else
            {
                SetVertexRegisters(VERTEX_REGISTER_WORLD_VIEW2, 4, cmd.mMatrix);
                matrixPushed = true;
            }
            UploadRegisters();
            continue;
        }
        if (cmd.mCount == 0)
            continue;
        IssueDraw(buf->mPrimitiveType, cmd.mOffset / stride, cmd.mCount);
        mVerticesDrawnThisFrame += cmd.mCount;
    }
    if (matrixPushed)
    {
        float identity[16];
        SetIdentity(identity);
        SetVertexRegisters(VERTEX_REGISTER_WORLD_VIEW2, 4, identity);
    }
    (void)bApplyMatrix;
}

void Renderer::CBuffLockStaticCreations()
{
    mAllowStaticCommandBufferIds = false;
}

int Renderer::CBuffCreate(int n)
{
    EnterTimedCriticalSection(&mCommandBufferMutex, "Wait command buffer lock");
    int first = mNextCommandBufferId;
    for (;;)
    {
        if (first + n > kMaxCommandBufferIds)
        {
            LeaveCriticalSection(&mCommandBufferMutex);
            return -1;
        }
        bool free = true;
        for (int i = 0; i < n; ++i)
        {
            if (mCommandBufferState[first + i] != COMMAND_BUFFER_FREE)
            {
                free = false;
                break;
            }
        }
        if (free)
            break;
        ++first;
    }
    for (int i = 0; i < n; ++i)
        mCommandBufferState[first + i] = COMMAND_BUFFER_RESERVED;
    if (mAllowStaticCommandBufferIds)
        mNextCommandBufferId = first + n;
    LeaveCriticalSection(&mCommandBufferMutex);
    return first;
}

void Renderer::DeleteCommandBuffer(CommandBuffer* buf)
{
    if (buf == 0)
        return;
    if (buf->mBuffer != 0)
        mPendingBufferDeletes.push_back(buf->mBuffer);
    if (buf->mVertexArray != 0)
        mPendingVertexArrayDeletes.push_back(buf->mVertexArray);
    __atomic_sub_fetch(&totalCommandBufferBytes, buf->mCountedBytes, __ATOMIC_RELAXED);
    delete buf;
}

void Renderer::CBuffDelete(int firstId, int n)
{
    EnterTimedCriticalSection(&mCommandBufferMutex, "Wait command buffer lock");
    for (int i = 0; i < n; ++i)
    {
        DeleteCommandBuffer(mCommandBuffers[firstId + i]);
        mCommandBuffers[firstId + i] = 0;
        mCommandBufferState[firstId + i] = COMMAND_BUFFER_FREE;
    }
    LeaveCriticalSection(&mCommandBufferMutex);
}

void Renderer::CBuffStart(int id, bool)
{
    Context& ctx = getContext();
    ctx.mMatrixMode = 3;
    ctx.mMatrixStackDepth[3] = 0;
    ASM_MTX44Identity(ctx.mMatrixStack[3][0]);
    ctx.mMatrixDirty[3] = true;
    ctx.mRecordingId = id;
    ctx.mRecordingBuffer = new CommandBuffer;
}

void Renderer::CBuffClear(int id)
{
    EnterTimedCriticalSection(&mCommandBufferMutex, "Wait command buffer lock");
    if (mCommandBufferState[id] == COMMAND_BUFFER_RECORDED)
    {
        DeleteCommandBuffer(mCommandBuffers[id]);
        mCommandBuffers[id] = 0;
        mCommandBufferState[id] = COMMAND_BUFFER_RESERVED;
    }
    LeaveCriticalSection(&mCommandBufferMutex);
}

int Renderer::CBuffSize(int id)
{
    if (id == -1)
    {
        int total = __atomic_load_n(&totalCommandBufferBytes, __ATOMIC_RELAXED);
        return total < 0 ? 0 : total;
    }
    EnterTimedCriticalSection(&mCommandBufferMutex, "Wait command buffer lock");
    int len = 0;
    if (id >= 0 && id < kMaxCommandBufferIds && mCommandBuffers[id] != 0)
        len = mCommandBuffers[id]->mCountedBytes;
    LeaveCriticalSection(&mCommandBufferMutex);
    return len;
}

void Renderer::CBuffEnd()
{
    Context& ctx = getContext();
    CommandBuffer* buf = ctx.mRecordingBuffer;
    ctx.mRecordingBuffer = 0;
    ctx.mMatrixMode = 0;
    if (buf == 0)
        return;
    memcpy(buf->mMatrix, ctx.mMatrixStack[3][0], sizeof(buf->mMatrix));
    int id = ctx.mRecordingId;
    ctx.mRecordingId = -1;
    buf->mId = id;
    std::vector<unsigned char>(buf->mVertices).swap(buf->mVertices);
    buf->mCountedBytes = (int)buf->mVertices.size();
    buf->mSingleMatrix = !buf->mCommands.empty() && buf->mCommands[0].mIsMatrix;
    for (unsigned int i = 1; i < buf->mCommands.size(); ++i)
    {
        if (buf->mCommands[i].mIsMatrix)
            buf->mSingleMatrix = false;
    }
    __atomic_add_fetch(&totalCommandBufferBytes, buf->mCountedBytes, __ATOMIC_RELAXED);
    if (ctx.mDeferredMode)
    {
        ctx.mDeferredBuffers.push_back(buf);
        return;
    }
    EnterTimedCriticalSection(&mCommandBufferMutex, "Wait command buffer lock");
    DeleteCommandBuffer(mCommandBuffers[id]);
    mCommandBuffers[id] = buf;
    mCommandBufferState[id] = COMMAND_BUFFER_RECORDED;
    LeaveCriticalSection(&mCommandBufferMutex);
}

bool Renderer::CBuffCall(int id, bool bApplyMatrix, unsigned char)
{
    if (id < 0 || id >= kMaxCommandBufferIds)
        return false;
    EnterTimedCriticalSection(&mCommandBufferMutex, "Wait command buffer lock");
    CommandBuffer* buf = mCommandBuffers[id];
    if (buf == 0)
    {
        LeaveCriticalSection(&mCommandBufferMutex);
        return false;
    }
    Context& ctx = getContext();
    if (!OnRenderThread())
    {
        mVerticesDrawnThisFrame += buf->mVertexCount;
        LeaveCriticalSection(&mCommandBufferMutex);
        return true;
    }

    int vertexShader = buf->mVertexType;
    if (vertexShader == C4JRender::eVertexType_Default && s_state.lightingEnable)
        vertexShader = C4JRender::eVertexType_DefaultLighting;
    else if (vertexShader == C4JRender::eVertexType_DefaultLighting && !s_state.lightingEnable)
        vertexShader = C4JRender::eVertexType_Default;
    else if (vertexShader == C4JRender::eVertexType_Entity && s_state.lightingEnable)
        vertexShader = C4JRender::eVertexType_EntityLighting;
    else if (vertexShader == C4JRender::eVertexType_EntityLighting && !s_state.lightingEnable)
        vertexShader = C4JRender::eVertexType_Entity;
    int pixelShader = buf->mPixelShaderType;
    if (s_state.forceLod >= 0)
        pixelShader = C4JRender::ePixelShaderType_TerrainForceLod;
    pixelShader = FinalisePixelShaderType(pixelShader);

    float identity[16];
    SetIdentity(identity);
    SetVertexRegisters(VERTEX_REGISTER_WORLD_VIEW2, 4, identity);
    unsigned int prog = GetProgram(vertexShader, pixelShader);
    ApplyUniforms(vertexShader, pixelShader, prog);
    BindTextures();
    ApplyRenderState();
    RenderCommandBuffer(buf, vertexShader, bApplyMatrix);
    if (bApplyMatrix)
    {
        float mat[4][4];
        memcpy(mat, buf->mMatrix, sizeof(mat));
        MultWithStack(mat);
        ASM_MTX44Identity(ctx.mMatrixStack[3][0]);
        ctx.mMatrixDirty[3] = true;
    }
    LeaveCriticalSection(&mCommandBufferMutex);
    return true;
}

void Renderer::CBuffCallRun(const int* ids, int n)
{
    const int kIdsPerLock = 128;
    bool prepared = false;
    int preparedVertexShader = -1;
    int preparedPixelShader = -1;
    for (int first = 0; first < n; first += kIdsPerLock)
    {
        const int last = first + kIdsPerLock < n ? first + kIdsPerLock : n;
        EnterTimedCriticalSection(&mCommandBufferMutex, "Wait command buffer lock");
        for (int i = first; i < last; ++i)
        {
            const int id = ids[i];
            if (id < 0 || id >= kMaxCommandBufferIds)
                continue;
            CommandBuffer* buf = mCommandBuffers[id];
            if (buf == 0)
                continue;
            if (!OnRenderThread())
            {
                mVerticesDrawnThisFrame += buf->mVertexCount;
                continue;
            }
            int vertexShader = buf->mVertexType;
            if (vertexShader == C4JRender::eVertexType_Default && s_state.lightingEnable)
                vertexShader = C4JRender::eVertexType_DefaultLighting;
            else if (vertexShader == C4JRender::eVertexType_DefaultLighting && !s_state.lightingEnable)
                vertexShader = C4JRender::eVertexType_Default;
            else if (vertexShader == C4JRender::eVertexType_Entity && s_state.lightingEnable)
                vertexShader = C4JRender::eVertexType_EntityLighting;
            else if (vertexShader == C4JRender::eVertexType_EntityLighting && !s_state.lightingEnable)
                vertexShader = C4JRender::eVertexType_Entity;
            int pixelShader = buf->mPixelShaderType;
            if (s_state.forceLod >= 0)
                pixelShader = C4JRender::ePixelShaderType_TerrainForceLod;
            pixelShader = FinalisePixelShaderType(pixelShader);
            const bool chunkSection = vertexShader == ePcVertexShader_Terrain && buf->mSingleMatrix;
            if (!prepared || !chunkSection || vertexShader != preparedVertexShader
                || pixelShader != preparedPixelShader)
            {
                float identity[16];
                SetIdentity(identity);
                SetVertexRegisters(VERTEX_REGISTER_WORLD_VIEW2, 4, identity);
                unsigned int prog = GetProgram(vertexShader, pixelShader);
                ApplyUniforms(vertexShader, pixelShader, prog);
                BindTextures();
                ApplyRenderState();
                preparedVertexShader = vertexShader;
                preparedPixelShader = pixelShader;
            }
            RenderCommandBuffer(buf, vertexShader, false);
            prepared = chunkSection;
        }
        LeaveCriticalSection(&mCommandBufferMutex);
    }
}

void Renderer::CBuffTick()
{
}

void Renderer::CBuffDeferredModeStart()
{
    getContext().mDeferredMode = true;
}

void Renderer::CBuffDeferredModeEnd()
{
    Context& ctx = getContext();
    if (!ctx.mDeferredMode)
        return;
    ctx.mDeferredMode = false;
    EnterTimedCriticalSection(&mCommandBufferMutex, "Wait command buffer lock");
    for (unsigned int i = 0; i < ctx.mDeferredBuffers.size(); ++i)
    {
        CommandBuffer* buf = ctx.mDeferredBuffers[i];
        int id = buf->mId;
        DeleteCommandBuffer(mCommandBuffers[id]);
        mCommandBuffers[id] = buf;
        mCommandBufferState[id] = COMMAND_BUFFER_RECORDED;
    }
    LeaveCriticalSection(&mCommandBufferMutex);
    ctx.mDeferredBuffers.clear();
}

int Renderer::GetMaxTextures()
{
    return kMaxTextures;
}

int Renderer::TextureCreate()
{
    EnterTimedCriticalSection(&mTextureMutex, "Wait texture lock");
    for (int tex = 0; tex < kMaxTextures; ++tex)
    {
        if (!mTextures[tex].mUsed)
        {
            Texture& e = mTextures[tex];
            e.mUsed = true;
            e.mWidth = 0;
            e.mHeight = 0;
            e.mLevels = 1;
            e.mFlags = 0;
            e.mStorageDirty = false;
            e.mParametersDirty = true;
            e.mPendingLevels.clear();
            ++mTexturesInUse;
            if (mTexturesInUse >= mTexturesLogged + 256)
            {
                mTexturesLogged = mTexturesInUse - mTexturesInUse % 256;
                printf("[pc] gl: %d of %d textures in use\n", mTexturesInUse, kMaxTextures);
                fflush(stdout);
            }
            LeaveCriticalSection(&mTextureMutex);
            return tex;
        }
    }
    LeaveCriticalSection(&mTextureMutex);
    static bool s_reportedFull = false;
    if (!s_reportedFull)
    {
        s_reportedFull = true;
        printf("[pc] gl: all %d textures are in use; a new texture was refused\n", kMaxTextures);
        fflush(stdout);
    }
    return -1;
}

void Renderer::TextureFree(int tex)
{
    if (tex < 0 || tex >= kMaxTextures)
        return;
    EnterTimedCriticalSection(&mTextureMutex, "Wait texture lock");
    Texture& e = mTextures[tex];
    if (tex == mDefaultTexture && mDefaultTextureLocked)
    {
        LeaveCriticalSection(&mTextureMutex);
        return;
    }
    if (e.mUsed)
        --mTexturesInUse;
    if (e.mName != 0)
        mPendingTextureDeletes.push_back(e.mName);
    e.mName = 0;
    e.mUsed = false;
    e.mPendingLevels.clear();
    LeaveCriticalSection(&mTextureMutex);
}

void Renderer::TextureBind(int tex)
{
    TextureBind(0, tex);
}

void Renderer::TextureBind(int stage, int tex)
{
    if (tex == -1)
        tex = mDefaultTexture;
    Context& ctx = getContext();
    ctx.mBoundTexture[stage] = tex;
    ctx.mPixelTexture[stage] = tex;
    ctx.mTextureIsMask[stage] = false;
    if (stage >= 1 && stage <= 3 && tex != mDefaultTexture)
        RecordLeftover(static_cast<PcLeftover>(ePcLeftover_Stage1 + stage - 1));
}

void Renderer::TextureBindMask(int stage)
{
    getContext().mTextureIsMask[stage] = true;
}

void Renderer::TextureBindVertex(int tex)
{
    if (tex == -1)
        tex = mDefaultTexture;
    Context& ctx = getContext();
    ctx.mBoundTexture[0] = tex;
    ctx.mTextureIsMask[0] = false;
    mVertexTexture = tex;
}

void Renderer::TextureSetTextureLevels(int levels)
{
    int tex = getContext().mBoundTexture[0];
    if (tex >= 0 && tex < kMaxTextures && !(tex == mDefaultTexture && mDefaultTextureLocked))
        mTextures[tex].mLevels = levels;
}

int Renderer::TextureGetTextureLevels()
{
    int tex = getContext().mBoundTexture[0];
    return tex >= 0 && tex < kMaxTextures ? mTextures[tex].mLevels : 1;
}

void Renderer::TextureData(int w, int h, void* data, int lvl, C4JRender::eTextureFormat)
{
    int tex = getContext().mBoundTexture[0];
    if (tex < 0 || tex >= kMaxTextures || (tex == mDefaultTexture && mDefaultTextureLocked))
        return;
    EnterTimedCriticalSection(&mTextureMutex, "Wait texture lock");
    Texture& e = mTextures[tex];
    if (lvl == 0)
    {
        e.mWidth = w;
        e.mHeight = h;
    }
    if (s_displayMode != ePcDisplay_Headless)
    {
        if ((int)e.mPendingLevels.size() <= lvl)
            e.mPendingLevels.resize(lvl + 1);
        size_t len = (size_t)w * h * 4;
        if (data != 0)
            e.mPendingLevels[lvl].assign(static_cast<unsigned char*>(data),
                                               static_cast<unsigned char*>(data) + len);
        else
            e.mPendingLevels[lvl].assign(len, 0);
        e.mStorageDirty = true;
    }
    LeaveCriticalSection(&mTextureMutex);
}

void Renderer::TextureDataUpdate(int x, int y, int w, int h, void* data, int lvl)
{
    int tex = getContext().mBoundTexture[0];
    if (tex < 0 || tex >= kMaxTextures || x != 0 || y != 0 || data == 0)
        return;
    Texture& e = mTextures[tex];
    if (w != (e.mWidth >> lvl) || h != (e.mHeight >> lvl))
        return;
    TextureData(w, h, data, lvl, C4JRender::eTextureFormat_RGBA8);
}

void Renderer::TextureSetParam(int param, int v)
{
    if (param == 0)
        return;
    int tex = getContext().mBoundTexture[0];
    if (tex < 0 || tex >= kMaxTextures || (tex == mDefaultTexture && mDefaultTextureLocked))
        return;
    Texture& e = mTextures[tex];
    unsigned int flags = e.mFlags;
    if (param == 1)
        flags = (flags & ~TEXTURE_FLAG_MIN_LINEAR) | (v == 1 ? TEXTURE_FLAG_MIN_LINEAR : 0);
    else if (param == 2)
        flags = (flags & ~TEXTURE_FLAG_MAG_LINEAR) | (v == 1 ? TEXTURE_FLAG_MAG_LINEAR : 0);
    else if (param == 3)
        flags = (flags & ~TEXTURE_FLAG_CLAMP_U) | (v == 0 ? TEXTURE_FLAG_CLAMP_U : 0);
    else if (param == 4)
        flags = (flags & ~TEXTURE_FLAG_CLAMP_V) | (v == 0 ? TEXTURE_FLAG_CLAMP_V : 0);
    e.mFlags = flags;
    e.mParametersDirty = true;
}

void Renderer::TextureDynamicUpdateStart()
{
}

void Renderer::TextureDynamicUpdateEnd()
{
}

int Renderer::LoadTextureData(const char* fileName, D3DXIMAGE_INFO* info, int** data)
{
    int file = CreateFileA(fileName, GENERIC_READ, 0, 0, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, 0);
    if (file == INVALID_HANDLE_VALUE)
        return -1;
    unsigned int fileSize = GetFileSize(file, 0);
    unsigned char* fileData = static_cast<unsigned char*>(malloc(fileSize));
    unsigned int bytesRead = 0;
    ReadFile(file, fileData, fileSize, &bytesRead, 0);
    CloseHandle(file);
    int res = bytesRead == fileSize ? DecodePng(fileData, fileSize, info, data) : -1;
    free(fileData);
    return res;
}

int Renderer::LoadTextureData(unsigned char* fileData, unsigned int fileSize, D3DXIMAGE_INFO* info, int** data)
{
    return DecodePng(fileData, fileSize, info, data);
}

int Renderer::SaveTextureData(const char* fileName, D3DXIMAGE_INFO* info, int* data)
{
    png_image img;
    memset(&img, 0, sizeof(img));
    img.version = PNG_IMAGE_VERSION;
    img.width = info->Width;
    img.height = info->Height;
    img.format = PNG_FORMAT_RGBA_BYTES;
    png_image_write_to_file(&img, PcResolveWritePath(fileName).c_str(), 0, data, 0, 0);
    return 0;
}

int Renderer::SaveTextureDataToMemory(void* dst, int destinationSize, int* bytesWritten, int w, int h,
                                      int* data)
{
    png_image img;
    memset(&img, 0, sizeof(img));
    img.version = PNG_IMAGE_VERSION;
    img.width = w;
    img.height = h;
    img.format = PNG_FORMAT_RGBA_BYTES;
    memoryWriter.start = static_cast<unsigned char*>(dst);
    memoryWriter.current = memoryWriter.start;
    memoryWriter.end = memoryWriter.start + destinationSize;
    png_image_write_to_stdio(&img, 0, 0, data, 0, 0, reinterpret_cast<void*>(WriteToMemory),
                             reinterpret_cast<void*>(FlushMemory));
    *bytesWritten = (int)(memoryWriter.current - memoryWriter.start);
    return 0;
}

void Renderer::TextureGetStats()
{
}

GX2Texture* Renderer::TextureGetTexture(int)
{
    return 0;
}

void Renderer::StateSetColour(float r, float g, float b, float a)
{
    if (getContext().mRecordingBuffer != 0)
        return;
    s_state.colour[0] = r;
    s_state.colour[1] = g;
    s_state.colour[2] = b;
    s_state.colour[3] = a;
}

void Renderer::StateSetColourAdd(float r, float g, float b, float a)
{
    if (getContext().mRecordingBuffer != 0)
        return;
    s_state.colourAdd[0] = r;
    s_state.colourAdd[1] = g;
    s_state.colourAdd[2] = b;
    s_state.colourAdd[3] = a;
}

void Renderer::StateSetTexColour(int stage, float r, float g, float b, float a)
{
    if (getContext().mRecordingBuffer != 0 || stage < 0 || stage >= 4)
        return;
    s_state.texColour[stage][0] = r;
    s_state.texColour[stage][1] = g;
    s_state.texColour[stage][2] = b;
    s_state.texColour[stage][3] = a;
    if (r != 1.0f || g != 1.0f || b != 1.0f || a != 1.0f)
        RecordLeftover(ePcLeftover_TexColour);
}

void Renderer::StateSetDepthMask(bool bDepthMask)
{
    if (getContext().mRecordingBuffer == 0)
        s_state.depthMask = bDepthMask;
}

void Renderer::StateSetBlendEnable(bool bBlend)
{
    if (getContext().mRecordingBuffer == 0)
        s_state.blendEnable = bBlend;
}

void Renderer::StateSetBlendFunc(int src, int dst)
{
    if (getContext().mRecordingBuffer != 0)
        return;
    s_state.blendSource = src;
    s_state.blendDestination = dst;
}

void Renderer::StateSetBlendFactor(unsigned int factor)
{
    if (getContext().mRecordingBuffer != 0)
        return;
    s_state.blendFactor[0] = (float)(factor & 0xFF) / 255.0f;
    s_state.blendFactor[1] = (float)((factor >> 8) & 0xFF) / 255.0f;
    s_state.blendFactor[2] = (float)((factor >> 16) & 0xFF) / 255.0f;
    s_state.blendFactor[3] = (float)(factor >> 24) / 255.0f;
}

void Renderer::StateSetAlphaFunc(int, float ref)
{
    s_state.alphaReference = ref;
}

void Renderer::StateSetDepthFunc(int depthFunc)
{
    if (getContext().mRecordingBuffer == 0)
        s_state.depthFunc = depthFunc;
}

void Renderer::StateSetFaceCull(bool bFaceCull)
{
    if (getContext().mRecordingBuffer == 0)
        s_state.faceCull = bFaceCull;
}

void Renderer::StateSetFaceCullCW(bool bClockwise)
{
    if (getContext().mRecordingBuffer == 0)
        s_state.frontFaceClockwise = !bClockwise;
}

void Renderer::StateSetLineWidth(float w)
{
    if (getContext().mRecordingBuffer == 0)
        s_state.lineWidth = w;
}

void Renderer::StateSetWriteEnable(bool bRed, bool bGreen, bool bBlue, bool bAlpha)
{
    s_state.colourWrite[0] = bRed;
    s_state.colourWrite[1] = bGreen;
    s_state.colourWrite[2] = bBlue;
    s_state.colourWrite[3] = bAlpha;
}

void Renderer::StateSetDepthTestEnable(bool bDepthTest)
{
    if (getContext().mRecordingBuffer == 0)
        s_state.depthTestEnable = bDepthTest;
}

void Renderer::StateSetAlphaTestEnable(bool bAlphaTest)
{
    if (getContext().mRecordingBuffer == 0)
        s_state.alphaTestEnable = bAlphaTest;
}

void Renderer::StateSetDepthSlopeAndBias(float slope, float bias)
{
    if (getContext().mRecordingBuffer != 0)
        return;
    s_state.polygonSlope = slope;
    s_state.polygonBias = bias;
}

void Renderer::StateSetFogEnable(bool bFog)
{
    s_state.fogEnable = bFog;
    if (bFog)
        RecordLeftover(ePcLeftover_Fog);
}

void Renderer::StateSetFogMode(int mode)
{
    s_state.fogMode = mode;
}

void Renderer::StateSetFogNearDistance(float nearDist)
{
    s_state.fogNear = nearDist;
}

void Renderer::StateSetFogFarDistance(float farDist)
{
    s_state.fogFar = farDist;
}

void Renderer::StateSetFogDensity(float dens)
{
    s_state.fogDensity = dens;
}

void Renderer::StateSetFogColour(float r, float g, float b)
{
    s_state.fogColour[0] = r;
    s_state.fogColour[1] = g;
    s_state.fogColour[2] = b;
    s_state.fogColour[3] = 1.0f;
}

void Renderer::StateSetLightingEnable(bool bLighting)
{
    if (getContext().mRecordingBuffer == 0)
        s_state.lightingEnable = bLighting;
}

void Renderer::StateSetVertexTextureUV(float u, float v)
{
    float uniform[4] = {u - 1.0f, v - 1.0f, 0.0f, 0.0f};
    SetVertexRegisters(VERTEX_REGISTER_TEXTURE_UV2, 1, uniform);
}

void Renderer::StateSetTextureScaleUI(float scaleU, float scaleV, float offsetU, float offsetV)
{
    float uniform[4] = {scaleU, scaleV, offsetU, offsetV};
    SetVertexRegisters(VERTEX_REGISTER_UV_SCALE, 1, uniform);
}

void Renderer::StateSetScalingGridUI(float left, float right, float top, float bottom, float innerLeft,
                                     float innerRight, float innerTop, float innerBottom, float scaleU, float scaleV)
{
    float grid[3][4] = {{left, right, top, bottom}, {innerLeft, innerRight, innerTop, innerBottom},
                        {scaleU, scaleV, 0.0f, 0.0f}};
    SetVertexRegisters(VERTEX_REGISTER_GRID, 3, &grid[0][0]);
}

void Renderer::StateSetLightColour(int light, float r, float g, float b)
{
    if (light >= 2 || getContext().mRecordingBuffer != 0)
        return;
    s_state.lightColour[light][0] = r;
    s_state.lightColour[light][1] = g;
    s_state.lightColour[light][2] = b;
    s_state.lightColour[light][3] = 1.0f;
}

void Renderer::StateSetLightAmbientColour(float r, float g, float b)
{
    if (getContext().mRecordingBuffer != 0)
        return;
    s_state.lightAmbient[0] = r;
    s_state.lightAmbient[1] = g;
    s_state.lightAmbient[2] = b;
    s_state.lightAmbient[3] = 1.0f;
}

void Renderer::StateSetLightDirection(int light, float x, float y, float z)
{
    if (light >= 2 || getContext().mRecordingBuffer != 0)
        return;
    Context& ctx = getContext();
    Vec dir;
    dir.x = x;
    dir.y = y;
    dir.z = z;
    float transposed[4][4];
    ASM_MTX44Transpose(ctx.mMatrixStack[0][ctx.mMatrixStackDepth[0]], transposed);
    ASM_MTX44MultVecSR(transposed, &dir, &dir);
    ASM_VECNormalize(&dir, &dir);
    s_state.lightDirection[light][0] = dir.x;
    s_state.lightDirection[light][1] = dir.y;
    s_state.lightDirection[light][2] = dir.z;
    s_state.lightDirection[light][3] = 0.0f;
}

void Renderer::StateSetLightEnable(int light, bool bEnable)
{
    if (light >= 2 || getContext().mRecordingBuffer != 0)
        return;
    s_state.lightEnable[light] = bEnable;
}

void Renderer::StateSetViewport(C4JRender::eViewportType viewport)
{
    if (getContext().mRecordingBuffer != 0)
        return;
    s_state.currentViewport = viewport;
    int screenWidth = kScreenWidth[s_screen];
    int screenHeight = kScreenHeight[s_screen];
    int halfWidth = screenWidth / 2;
    int halfHeight = screenHeight / 2;
    switch (viewport)
    {
    case C4JRender::eViewportType_TopHalf:
        SetViewportRect(0, 0, screenWidth, halfHeight);
        break;
    case C4JRender::eViewportType_BottomHalf:
        SetViewportRect(0, halfHeight, screenWidth, halfHeight);
        break;
    case C4JRender::eViewportType_LeftHalf:
        SetViewportRect(0, 0, halfWidth, screenHeight);
        break;
    case C4JRender::eViewportType_RightHalf:
        SetViewportRect(halfWidth, 0, halfWidth, screenHeight);
        break;
    case C4JRender::eViewportType_TopLeftQuarter:
        SetViewportRect(0, 0, halfWidth, halfHeight);
        break;
    case C4JRender::eViewportType_TopRightQuarter:
        SetViewportRect(halfWidth, 0, halfWidth, halfHeight);
        break;
    case C4JRender::eViewportType_BottomLeftQuarter:
        SetViewportRect(0, halfHeight, halfWidth, halfHeight);
        break;
    case C4JRender::eViewportType_BottomRightQuarter:
        SetViewportRect(halfWidth, halfHeight, halfWidth, halfHeight);
        break;
    default:
        SetViewportRect(0, 0, screenWidth, screenHeight);
        break;
    }
}

void Renderer::StateGetScissorRect(unsigned int& left, unsigned int& top, unsigned int& right, unsigned int& bottom)
{
    Context& context = getContext();
    left = context.mScissorRect[0];
    top = context.mScissorRect[1];
    right = context.mScissorRect[2];
    bottom = context.mScissorRect[3];
}

void Renderer::StateSetScissorRect(unsigned int left, unsigned int top, unsigned int right, unsigned int bottom)
{
    Context& context = getContext();
    context.mScissorRect[0] = left;
    context.mScissorRect[1] = top;
    context.mScissorRect[2] = right;
    context.mScissorRect[3] = bottom;
    s_state.scissor[0] = static_cast<int>(left);
    s_state.scissor[1] = static_cast<int>(top);
    s_state.scissor[2] = static_cast<int>(right - left);
    s_state.scissor[3] = static_cast<int>(bottom - top);
}

void Renderer::StateSetEnableViewportClipPlanes(bool)
{
}

void Renderer::StateSetTexGenCol(int coord, float x, float y, float z, float w, bool bEyeLinear)
{
    if (getContext().mRecordingBuffer != 0)
        return;
    float plane[4] = {x, y, z, w};
    if (bEyeLinear)
    {
        float modelView[4][4];
        memcpy(modelView, MatrixGet(0), sizeof(modelView));
        ASM_MTX44Transpose(modelView, modelView);
        float inverse[4][4];
        C_MTX44Inverse(modelView, inverse);
        float transformed[4];
        for (int row = 0; row < 4; ++row)
            transformed[row] = inverse[0][row] * plane[0] + inverse[1][row] * plane[1] + inverse[2][row] * plane[2]
                               + inverse[3][row] * plane[3];
        memcpy(plane, transformed, sizeof(plane));
    }
    int mat = bEyeLinear ? 0 : 1;
    int other = bEyeLinear ? 1 : 0;
    for (int row = 0; row < 4; ++row)
    {
        s_state.texGen[mat][row][coord] = plane[row];
        s_state.texGen[other][row][coord] = 0.0f;
    }
}

void Renderer::StateSetStencil(int stencilFunc, unsigned char ref, unsigned char compareMask,
                               unsigned char writeMask)
{
    s_state.stencilFunc = stencilFunc;
    if (getContext().mRecordingBuffer != 0)
        return;
    s_state.stencilReference = ref;
    s_state.stencilCompareMask = compareMask;
    s_state.stencilWriteMask = writeMask;
}

void Renderer::StateSetForceLOD(int lod)
{
    if (getContext().mRecordingBuffer == 0)
        s_state.forceLod = lod;
}

void Renderer::StateSetLerpColour(float r, float g, float b, float a)
{
    if (getContext().mRecordingBuffer != 0)
        return;
    s_state.lerpColour[0] = r;
    s_state.lerpColour[1] = g;
    s_state.lerpColour[2] = b;
    s_state.lerpColour[3] = a;
    if (a != 0.0f)
        RecordLeftover(ePcLeftover_Lerp);
}

void Renderer::SetLayeredColour(int layer, float r, float g, float b)
{
    s_state.layeredColour[layer][0] = r;
    s_state.layeredColour[layer][1] = g;
    s_state.layeredColour[layer][2] = b;
    s_state.layeredColour[layer][3] = 1.0f;
}

void Renderer::SetLayeredParam(int slot, int layer, float off, float scale)
{
    if (slot < 0 || slot >= MAX_LAYERED_PARAMS)
        return;
    s_state.layeredParams[slot][0] = s_state.layeredColour[layer][0];
    s_state.layeredParams[slot][1] = s_state.layeredColour[layer][1];
    s_state.layeredParams[slot][2] = s_state.layeredColour[layer][2];
    s_state.layeredParams[slot][3] = off + scale;
}

void Renderer::SetLayeredTextureSize(int w, int h)
{
    s_state.layeredTextureSize[0] = (float)w;
    s_state.layeredTextureSize[1] = (float)h;
}

void Renderer::StateSetBoneMatrixData(int bone)
{
    Context& ctx = getContext();
    memcpy(&ctx.mBoneMatrix[bone], MatrixGet(0), sizeof(ctx.mBoneMatrix[bone]));
    if (bone + 1 > ctx.mNumBoneMatrices)
        ctx.mNumBoneMatrices = bone + 1;
}

void Renderer::StateSetBoneMatrixNull(int bone)
{
    Context& ctx = getContext();
    memset(&ctx.mBoneMatrix[bone], 0, sizeof(ctx.mBoneMatrix[bone]));
    if (bone + 1 > ctx.mNumBoneMatrices)
        ctx.mNumBoneMatrices = bone + 1;
}

void Renderer::StateSetVertexAlphaDistance(float dist)
{
    s_state.vertexAlphaDistance = dist;
}

void Renderer::StateSetVertexAlpha(float a)
{
    float uniform[4] = {a, 0.0f, 0.0f, 0.0f};
    SetVertexRegisters(VERTEX_REGISTER_VERTEX_ALPHA, 1, uniform);
}

int Renderer::GetNumCommandBufferVertsDrawnLastFrame()
{
    return mVerticesDrawnLastFrame;
}

int Renderer::GetNumVertsInCommandBuffer(int id)
{
    if (id < 0 || id >= kMaxCommandBufferIds)
        return 0;
    EnterTimedCriticalSection(&mCommandBufferMutex, "Wait command buffer lock");
    int n = mCommandBuffers[id] != 0 ? mCommandBuffers[id]->mVertexCount : 0;
    LeaveCriticalSection(&mCommandBufferMutex);
    return n;
}

void Renderer::BeginEvent(const wchar_t*)
{
}

void Renderer::EndEvent()
{
}

void* Renderer::MemoryAllocMEM1(unsigned int size, unsigned int alignment, bool bReserved)
{
    if (!bReserved)
        return MEMAllocFromDefaultHeapEx(size, alignment < 4 ? 4 : alignment);
    if (mReservedMEM1 == 0)
        mReservedMEM1 = MEMAllocFromDefaultHeapEx(size, alignment < 4 ? 4 : alignment);
    return mReservedMEM1;
}

void Renderer::MemoryFreeMEM1(void* memory)
{
    if (memory != mReservedMEM1)
        MEMFreeToDefaultHeap(memory);
}

void* Renderer::MemoryAllocMEM2(unsigned int size, unsigned int alignment)
{
    return MEMAllocFromDefaultHeapEx(size, alignment < 4 ? 4 : alignment);
}

void Renderer::MemoryFreeMEM2(void* memory)
{
    MEMFreeToDefaultHeap(memory);
}

void* Renderer::MemoryAllocBucket(unsigned int size, unsigned int alignment)
{
    return MEMAllocFromDefaultHeapEx(size, alignment < 4 ? 4 : alignment);
}

void Renderer::MemoryFreeBucket(void* memory)
{
    MEMFreeToDefaultHeap(memory);
}

void Renderer::PcCheckLeftovers(const char* where)
{
    static const char* const kNames[ePcLeftover_Count] = {"texture stage 1", "texture stage 2", "texture stage 3",
                                                          "fog", "lerp colour", "layer colour"};
    Context& ctx = getContext();
    bool present[ePcLeftover_Count];
    for (int stage = 1; stage <= 3; ++stage)
        present[ePcLeftover_Stage1 + stage - 1] = ctx.mPixelTexture[stage] != mDefaultTexture;
    present[ePcLeftover_Fog] = s_state.fogEnable;
    present[ePcLeftover_Lerp] = s_state.lerpColour[3] != 0.0f;
    present[ePcLeftover_TexColour] = false;
    for (int stage = 0; stage < 4; ++stage)
        for (int ch = 0; ch < 4; ++ch)
            present[ePcLeftover_TexColour] = present[ePcLeftover_TexColour] || s_state.texColour[stage][ch] != 1.0f;

    for (int kind = 0; kind < ePcLeftover_Count; ++kind)
    {
        if (!present[kind])
            continue;
        const PcLeftoverTrace& trace = leftoverTraces[kind];
        const uintptr_t key = (trace.count > 0 ? trace.frames[trace.count > 2 ? 2 : trace.count - 1] : 0) ^ kind;
        bool reported = false;
        for (int i = 0; i < leftoverReportedCount; ++i)
            reported = reported || leftoverReported[i] == key;
        if (!reported && leftoverReportedCount < LEFTOVER_REPORTS)
        {
            leftoverReported[leftoverReportedCount++] = key;
            printf("[pc] render: %s left set before %s; program loaded at 0x%08lx; set from:", kNames[kind], where,
                   (unsigned long)PcOsProgramBase());
            for (int i = 0; i < trace.count; ++i)
                printf(" 0x%08lx", (unsigned long)trace.frames[i]);
            printf("\n");
            fflush(stdout);
        }
    }

    for (int stage = 1; stage <= 3; ++stage)
        if (present[ePcLeftover_Stage1 + stage - 1])
            TextureBind(stage, -1);
    if (present[ePcLeftover_Fog])
        s_state.fogEnable = false;
    if (present[ePcLeftover_Lerp])
        memset(s_state.lerpColour, 0, sizeof(s_state.lerpColour));
    if (present[ePcLeftover_TexColour])
        for (int stage = 0; stage < 4; ++stage)
            for (int ch = 0; ch < 4; ++ch)
                s_state.texColour[stage][ch] = 1.0f;
}
