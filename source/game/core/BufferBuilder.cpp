#include "game/core/BufferBuilder.hpp"

#include <float.h>

#include "game/client/render/Renderer.hpp"
#include "game/core/GlStateManager.hpp"
#include "game/core/IntBuffer.hpp"
#include "game/core/MemoryTracker.hpp"
#include "game/core/VertexFormat.hpp"
#include "game/core/Minecraft.hpp"
#include "game/world/block/MaterialShaders.hpp"

static bool convertQuadsToTriangles = false;
static bool tryVBO = false;

BufferBuilder::BufferBuilder(int bufferSize)
{
    mVertexCount = 0;
    mU = 0.0f;
    mV = 0.0f;
    mColor = 0;
    mHasColor = false;
    mHasTexture = false;
    mHasBrightness = false;
    mHasNormal = false;
    mRawBufferIndex = 0;
    mUseCompactVertices = false;
    mUseProjectedTexture = false;
    mPixelShaderType = C4JRender::ePixelShaderType_Default;
    mVertexType = C4JRender::eVertexType_Default;
    mHasBoneIndex = false;
    mAddedVertices = 0;
    mIsColorDisabled = false;
    mDrawMode = 0;
    mXOffset = 0.0f;
    mYOffset = 0.0f;
    mZOffset = 0.0f;
    mBaseXOffset = 0.0f;
    mBaseYOffset = 0.0f;
    mBaseZOffset = 0.0f;
    mNormal = 0;
    mBoneIndex = 0;
    mBucket = 0;
    mIsDrawing = false;
    mMipmapEnable = true;
    mUseVBO = false;
    mVBOIndex = 0;
    mVBOCount = 10;
    mBufferSize = bufferSize;

    mRawBuffer = new arrayWithLength<int>(bufferSize, true);
    mBuckets = new arrayWithLength<unsigned char>(bufferSize / 8, true);

    mUseVBO = tryVBO;
    if (mUseVBO)
    {
        mVertexBuffers = MemoryTracker::createIntBuffer(mVBOCount);
    }

    mUseCustomShaders = false;
}

void BufferBuilder::clear()
{
    mVertexCount = 0;
    mRawBufferIndex = 0;
    mAddedVertices = 0;
}

void BufferBuilder::begin(int drawMode)
{
    mIsDrawing = true;

    clear();

    mDrawMode = drawMode;
    mHasTexture = false;
    mHasColor = false;
    mHasBoneIndex = false;
    mHasBrightness = false;
    mHasNormal = false;
    mIsColorDisabled = false;
}

void BufferBuilder::begin()
{
    begin(C4JRender::ePrimitiveType_Quads);

    mBounds.mMinX = FLT_MAX;
    mBounds.mMinY = FLT_MAX;
    mBounds.mMinZ = FLT_MAX;
    mBounds.mMaxX = -FLT_MAX;
    mBounds.mMaxY = -FLT_MAX;
    mBounds.mMaxZ = -FLT_MAX;
}

void BufferBuilder::finaliseShaders()
{
    if (mDrawMode == C4JRender::ePrimitiveType_Quads && convertQuadsToTriangles)
    {
        if (mHasBoneIndex)
        {
            mVertexType = C4JRender::eVertexType_Entity;
        }
        else if (mUseCompactVertices)
        {
            mVertexType = C4JRender::eVertexType_Terrain;
        }
        else
        {
            mVertexType = C4JRender::eVertexType_Default;
        }

        if (mUseProjectedTexture)
        {
            mPixelShaderType = C4JRender::ePixelShaderType_Terrain;
        }
        else
        {
            mPixelShaderType = C4JRender::ePixelShaderType_Default;
        }
    }
    else if (mUseCompactVertices)
    {
        mPixelShaderType = C4JRender::ePixelShaderType_Default;
        mVertexType = C4JRender::eVertexType_Terrain;
    }
    else if (mHasBoneIndex)
    {
        mPixelShaderType = C4JRender::ePixelShaderType_Default;
        mVertexType = C4JRender::eVertexType_Entity;
    }
    else if (mUseProjectedTexture)
    {
        mVertexType = C4JRender::eVertexType_TexGen;
        mPixelShaderType = C4JRender::ePixelShaderType_Terrain;
    }
    else
    {
        mVertexType = C4JRender::eVertexType_Default;
        mPixelShaderType = C4JRender::ePixelShaderType_Default;
    }
}

void BufferBuilder::end()
{
    mIsDrawing = false;

    if (!mUseCustomShaders)
    {
        finaliseShaders();
    }

    if (mVertexCount > 0)
    {
        int* rawBuffer = mRawBuffer->data;

        if (!mHasColor)
        {
            for (int i = 0; i < mVertexCount; i++)
            {
                rawBuffer[i * 8 + 5] = -1;
            }
        }

        if (mDrawMode == C4JRender::ePrimitiveType_Quads && convertQuadsToTriangles)
        {
            gRenderer->DrawVertices(C4JRender::ePrimitiveType_Triangles, mVertexCount, rawBuffer, mVertexType,
                                    mPixelShaderType, NULL);
        }
        else if (mUseCompactVertices)
        {
            gRenderer->DrawVertices((C4JRender::ePrimitiveType)mDrawMode, mVertexCount, rawBuffer, mVertexType,
                                    mPixelShaderType, mBuckets->data);
        }
        else
        {
            gRenderer->DrawVertices((C4JRender::ePrimitiveType)mDrawMode, mVertexCount, rawBuffer, mVertexType,
                                    mPixelShaderType, NULL);
        }

        GlStateManager::disableClientState(0);

        if (mHasTexture)
        {
            GlStateManager::disableClientState(0);
        }

        if (mHasColor)
        {
            GlStateManager::disableClientState(0);
        }

        if (mHasNormal)
        {
            GlStateManager::disableClientState(0);
        }
    }

    clear();
}

void BufferBuilder::color(int red, int green, int blue, int alpha)
{
    if (mIsColorDisabled)
    {
        return;
    }

    if (red > 255)
    {
        red = 255;
    }
    if (green > 255)
    {
        green = 255;
    }
    if (blue > 255)
    {
        blue = 255;
    }
    if (alpha > 255)
    {
        alpha = 255;
    }

    if (red < 0)
    {
        red = 0;
    }
    if (green < 0)
    {
        green = 0;
    }
    if (blue < 0)
    {
        blue = 0;
    }
    if (alpha < 0)
    {
        alpha = 0;
    }

    mHasColor = true;
    mColor = (red << 24) | (green << 16) | (blue << 8) | alpha;
}

void BufferBuilder::color(int red, int green, int blue)
{
    color(red, green, blue, 255);
}

void BufferBuilder::color(float red, float green, float blue, float alpha)
{
    color((int)(red * 255.0f), (int)(green * 255.0f), (int)(blue * 255.0f), (int)(alpha * 255.0f));
}

void BufferBuilder::color(float red, float green, float blue)
{
    color((int)(red * 255.0f), (int)(green * 255.0f), (int)(blue * 255.0f));
}

void BufferBuilder::color(int rgb, int alpha)
{
    color((rgb >> 16) & 0xFF, (rgb >> 8) & 0xFF, rgb & 0xFF, alpha);
}

void BufferBuilder::color(int rgb)
{
    color((rgb >> 16) & 0xFF, (rgb >> 8) & 0xFF, rgb & 0xFF);
}

void BufferBuilder::noColor()
{
    mIsColorDisabled = true;
}

void BufferBuilder::tex(float u, float v)
{
    mU = u;
    mV = v;
    mHasTexture = true;
}

void BufferBuilder::tex2(int brightness)
{
    mBrightness = brightness;
    mHasBrightness = true;
}

void BufferBuilder::normal(float x, float y, float z)
{
    unsigned char normalX = (unsigned char)(x * 127.0f);
    unsigned char normalY = (unsigned char)(y * 127.0f);
    unsigned char normalZ = (unsigned char)(z * 127.0f);

    mHasNormal = true;
    mNormal = normalX | (normalY << 8) | (normalZ << 16);
}

void BufferBuilder::boneIndex(int index)
{
    mBoneIndex = index;
    mHasBoneIndex = true;
}

void BufferBuilder::bucket(int index)
{
    mBucket = index;
}

void BufferBuilder::offset(float x, float y, float z)
{
    mXOffset = x;
    mYOffset = y;
    mZOffset = z;
    mBaseXOffset = x;
    mBaseYOffset = y;
    mBaseZOffset = z;
}

void BufferBuilder::addOffset(float x, float y, float z)
{
    mXOffset = mXOffset + x;
    mYOffset = mYOffset + y;
    mZOffset = mZOffset + z;
}

void BufferBuilder::vertexNoBounds(float x, float y, float z)
{
    mAddedVertices++;

    float texU = mU;
    if (!mMipmapEnable)
    {
        texU = texU + 1.0f;
    }

    int* rawBuffer = mRawBuffer->data;

    if (mUseCompactVertices)
    {
        unsigned short packedColor = (unsigned short)(((mColor >> 17) & 0x7C00) | ((mColor >> 14) & 0x03E0)
                                                      | ((mColor >> 11) & 0x001F));
        if ((mColor & 0xFF) == 0xFF)
        {
            packedColor = (unsigned short)(packedColor | 0x8000);
        }

        short* compactVertex = (short*)(rawBuffer + mRawBufferIndex);
        compactVertex[0] = (short)(int)((x + mXOffset) * 1024.0f);
        compactVertex[1] = (short)(int)((y + mYOffset) * 1024.0f);
        compactVertex[2] = (short)(int)((z + mZOffset) * 1024.0f);
        compactVertex[3] = (short)packedColor;
        compactVertex[4] = (short)(int)(texU * 8192.0f);
        compactVertex[5] = (short)(int)(mV * 8192.0f);
        compactVertex[6] = (short)((mBrightness << 8) | ((unsigned int)mBrightness >> 16));
        compactVertex[7] = (short)(int)(-mBaseXOffset);

        mVertexCount++;
        mRawBufferIndex += 4;

        if ((mVertexCount & 3) == 0)
        {
            if (mRawBufferIndex >= mBufferSize - 16 || mRawBufferIndex / 4 > 65531)
            {
                end();
                mIsDrawing = true;

                if ((mVertexCount & 3) != 0)
                {
                    return;
                }
            }

            mBuckets->data[mVertexCount / 4 - 1] = (unsigned char)mBucket;
            mBucket = 0;
        }

        return;
    }

    if (mDrawMode == C4JRender::ePrimitiveType_Quads && convertQuadsToTriangles && (mAddedVertices & 3) == 0)
    {
        for (int i = 0; i < 2; i++)
        {
            int copyIndex = mRawBufferIndex - 8 * (3 - i);

            if (mHasTexture)
            {
                rawBuffer[mRawBufferIndex + 3] = rawBuffer[copyIndex + 3];
                rawBuffer[mRawBufferIndex + 4] = rawBuffer[copyIndex + 4];
            }

            if (mHasColor)
            {
                rawBuffer[mRawBufferIndex + 5] = rawBuffer[copyIndex + 5];
            }

            rawBuffer[mRawBufferIndex + 0] = rawBuffer[copyIndex + 0];
            rawBuffer[mRawBufferIndex + 1] = rawBuffer[copyIndex + 1];
            rawBuffer[mRawBufferIndex + 2] = rawBuffer[copyIndex + 2];

            mVertexCount++;
            mRawBufferIndex += 8;
        }
    }

    float* positions = (float*)rawBuffer;

    if (mHasTexture)
    {
        positions[mRawBufferIndex + 3] = texU;
        positions[mRawBufferIndex + 4] = mV;
    }

    if (mHasColor)
    {
        rawBuffer[mRawBufferIndex + 5] = mColor;
    }

    if (mHasNormal)
    {
        rawBuffer[mRawBufferIndex + 6] = mNormal;
    }

    if (mHasBoneIndex)
    {
        rawBuffer[mRawBufferIndex + 6] = (mBoneIndex << 24) | (rawBuffer[mRawBufferIndex + 6] & 0x00FFFFFF);
    }

    if (mHasBrightness)
    {
        rawBuffer[mRawBufferIndex + 7] = (mBrightness << 16) | (int)((unsigned int)mBrightness >> 16);
    }
    else
    {
        rawBuffer[mRawBufferIndex + 7] = (int)0xFE00FE00;
    }

    positions[mRawBufferIndex + 0] = x + mXOffset;
    positions[mRawBufferIndex + 1] = y + mYOffset;
    positions[mRawBufferIndex + 2] = z + mZOffset;

    mVertexCount++;
    mRawBufferIndex += 8;

    if ((mVertexCount & 3) == 0 && mRawBufferIndex >= mBufferSize - 32)
    {
        end();
        mIsDrawing = true;
    }
}

void BufferBuilder::vertex(float x, float y, float z)
{
    mBounds.addVert(x + mXOffset, y + mYOffset, z + mZOffset);
    vertexNoBounds(x, y, z);
}

void BufferBuilder::vertexDouble(double x, double y, double z)
{
    vertex((float)x, (float)y, (float)z);
}

void BufferBuilder::vertexUV(float x, float y, float z, float u, float v)
{
    tex(u, v);
    vertex(x, y, z);
}

void BufferBuilder::vertexUVSkinned(float x, float y, float z, float u, float v, int index)
{
    tex(u, v);
    boneIndex(index);
    vertex(x, y, z);
}

void BufferBuilder::blockRainQuad(float x0, float y0, float z0, float u0, float v0, float x1, float y1, float z1,
                                  float u1, float v1, float x2, float y2, float z2, float u2, float v2, float x3,
                                  float y3, float z3, float u3, float v3, float red0, float green0, float blue0,
                                  float alpha0, float red1, float green1, float blue1, float alpha1, int brightness)
{
    mHasTexture = true;
    mHasBrightness = true;
    mHasColor = true;

    int* vertexData = mRawBuffer->data + mRawBufferIndex;
    float* positions = (float*)vertexData;

    mAddedVertices += 4;
    mRawBufferIndex += 32;
    mVertexCount += 4;

    int color0 = ((int)(red0 * 255.0f) << 24) | ((int)(green0 * 255.0f) << 16) | ((int)(blue0 * 255.0f) << 8)
                 | (int)(alpha0 * 255.0f);

    mBounds.addVert(x0 + mXOffset, y0 + mYOffset, z0 + mZOffset);
    positions[0] = x0 + mXOffset;
    positions[1] = y0 + mYOffset;
    positions[2] = z0 + mZOffset;
    positions[3] = u0;
    positions[4] = v0;
    vertexData[5] = color0;
    vertexData[7] = brightness;

    mBounds.addVert(x1 + mXOffset, y1 + mYOffset, z1 + mZOffset);
    positions[8] = x1 + mXOffset;
    positions[9] = y1 + mYOffset;
    positions[10] = z1 + mZOffset;
    positions[11] = u1;
    positions[12] = v1;
    vertexData[13] = color0;
    vertexData[15] = brightness;

    int color1 = ((int)(red1 * 255.0f) << 24) | ((int)(green1 * 255.0f) << 16) | ((int)(blue1 * 255.0f) << 8)
                 | (int)(alpha1 * 255.0f);

    mBounds.addVert(x2 + mXOffset, y2 + mYOffset, z2 + mZOffset);
    positions[16] = x2 + mXOffset;
    positions[17] = y2 + mYOffset;
    positions[18] = z2 + mZOffset;
    positions[19] = u2;
    positions[20] = v2;
    vertexData[21] = color1;
    vertexData[23] = brightness;

    mBounds.addVert(x3 + mXOffset, y3 + mYOffset, z3 + mZOffset);
    positions[24] = x3 + mXOffset;
    positions[25] = y3 + mYOffset;
    positions[26] = z3 + mZOffset;
    positions[27] = u3;
    positions[28] = v3;
    vertexData[29] = color1;
    vertexData[31] = brightness;

    if ((mVertexCount & 3) == 0 && mRawBufferIndex >= mBufferSize - 32)
    {
        end();
        mIsDrawing = true;
    }
}

void BufferBuilder::useCompactVertices(bool enabled)
{
    mUseCompactVertices = enabled;
}

void BufferBuilder::useProjectedTexture(bool enabled)
{
    mUseProjectedTexture = enabled;
}

bool BufferBuilder::setMipmapEnable(bool enabled)
{
    bool previous = mMipmapEnable;
    mMipmapEnable = enabled;
    return previous;
}

void BufferBuilder::setVertexAndPixelShader(C4JRender::eVertexType vertexType,
                                            C4JRender::ePixelShaderType pixelShaderType)
{
    mVertexType = vertexType;
    mPixelShaderType = pixelShaderType;
    mUseCustomShaders = true;
}

void BufferBuilder::setVertexAndPixelShader(eMinecraftEntityMaterial material)
{
    mUseCustomShaders = true;

    MaterialShaders& shaders = Minecraft::GetInstance()->getMaterialTable()->getMaterialShadersFor(material);

    mVertexType = shaders.mVertexType;
    mPixelShaderType = shaders.mPixelShaderType;
}

void BufferBuilder::useDefaultShaders()
{
    mUseCustomShaders = false;
    mPixelShaderType = C4JRender::ePixelShaderType_Default;
    mVertexType = C4JRender::eVertexType_Default;
}

void BufferBuilder::setVertexFormat(const VertexFormat* vertexFormat)
{
}

uint32_t BufferBuilder::hasMaxVertices()
{
    return false;
}

void BufferBuilder::faceTex2(int brightness0, int brightness1, int brightness2, int brightness3)
{
    mPackedQuad[6] = brightness0;
    mPackedQuad[13] = brightness1;
    mPackedQuad[20] = brightness2;
    mPackedQuad[27] = brightness3;
    mHasBrightness = true;
}

void BufferBuilder::postProcessFacePosition(double x, double y, double z)
{
    vertex((float)x, (float)y, (float)z);
}

int BufferBuilder::getStartingColorIndex(int face)
{
    if (face < 0)
    {
        return 0;
    }
    return face * 4;
}

void BufferBuilder::faceTint(float red, float green, float blue, int face)
{
    fixupVertexColor(red, green, blue, getStartingColorIndex(face));
}

void BufferBuilder::putColor(int red, int green, int blue, int alpha)
{
    color(red, green, blue, alpha);
    if (mRawBufferIndex >= 8)
    {
        mRawBuffer->data[mRawBufferIndex - 3] = mColor;
    }
}

void BufferBuilder::fixupVertexColor(int colorValue, int index)
{
}

void BufferBuilder::fixupVertexColor(float red, float green, float blue, int index)
{
    int colorValue = ((int)(red * 255.0f) << 24) | ((int)(green * 255.0f) << 16) | ((int)(blue * 255.0f) << 8) | 255;
    fixupVertexColor(colorValue, index);
}

void BufferBuilder::putBulkData(arrayWithLength<int> data)
{
    if (data.data == NULL || data.length <= 0)
    {
        return;
    }
    int count = data.length;
    if (count > mBufferSize - mRawBufferIndex)
    {
        count = mBufferSize - mRawBufferIndex;
    }
    for (int i = 0; i < count; ++i)
    {
        mRawBuffer->data[mRawBufferIndex + i] = data.data[i];
    }
    mRawBufferIndex += count;
}

void BufferBuilder::postNormal(float x, float y, float z)
{
    normal(x, y, z);
}

void BufferBuilder::fixupQuadColor(int colorValue)
{
}

void BufferBuilder::fixupQuadColor(float red, float green, float blue)
{
    int colorValue = ((int)(red * 255.0f) << 24) | ((int)(green * 255.0f) << 16) | ((int)(blue * 255.0f) << 8) | 255;
    fixupQuadColor(colorValue);
}

void BufferBuilder::Bounds::addVert(float x, float y, float z)
{
    if (x < mMinX)
    {
        mMinX = x;
    }
    if (y < mMinY)
    {
        mMinY = y;
    }
    if (z < mMinZ)
    {
        mMinZ = z;
    }
    if (x > mMaxX)
    {
        mMaxX = x;
    }
    if (y > mMaxY)
    {
        mMaxY = y;
    }
    if (z > mMaxZ)
    {
        mMaxZ = z;
    }
}

void BufferBuilder::Bounds::addBounds(Bounds& bounds)
{
    if (bounds.mMinX < mMinX)
    {
        mMinX = bounds.mMinX;
    }
    if (bounds.mMinY < mMinY)
    {
        mMinY = bounds.mMinY;
    }
    if (bounds.mMinZ < mMinZ)
    {
        mMinZ = bounds.mMinZ;
    }
    if (mMaxX < bounds.mMaxX)
    {
        mMaxX = bounds.mMaxX;
    }
    if (mMaxY < bounds.mMaxY)
    {
        mMaxY = bounds.mMaxY;
    }
    if (mMaxZ < bounds.mMaxZ)
    {
        mMaxZ = bounds.mMaxZ;
    }
}
