#ifndef GAME_CORE_BUFFERBUILDER_HPP
#define GAME_CORE_BUFFERBUILDER_HPP

#include <stdint.h>

#include "game/client/render/C4JRender.hpp"
#include "game/world/biome/arrayWithLength.hpp"
#include "game/world/block/MaterialTable.hpp"

class IntBuffer;
class VertexFormat;

class BufferBuilder
{
public:
    class Bounds
    {
    public:
        void addVert(float x, float y, float z);
        void addBounds(Bounds& bounds);

        float mMinX;
        float mMinY;
        float mMinZ;
        float mMaxX;
        float mMaxY;
        float mMaxZ;
    };

    BufferBuilder(int bufferSize);

    void begin();
    void begin(int drawMode);
    void end();
    void clear();

    void offset(float x, float y, float z);
    void addOffset(float x, float y, float z);

    void color(int red, int green, int blue, int alpha);
    void color(int red, int green, int blue);
    void color(float red, float green, float blue, float alpha);
    void color(float red, float green, float blue);
    void color(unsigned char red, unsigned char green, unsigned char blue);
    void color(int rgb, int alpha);
    void color(int rgb);
    void noColor();

    void tex(float u, float v);
    void tex2(int brightness);
    void faceTex2(int brightness0, int brightness1, int brightness2, int brightness3);
    void normal(float x, float y, float z);
    void boneIndex(int index);
    void bucket(int index);

    void vertex(float x, float y, float z);
    void vertexNoBounds(float x, float y, float z);
    void vertexDouble(double x, double y, double z);
    void vertexUV(float x, float y, float z, float u, float v);
    void vertexUVSkinned(float x, float y, float z, float u, float v, int index);

    void blockRainQuad(float x0, float y0, float z0, float u0, float v0, float x1, float y1, float z1, float u1,
                       float v1, float x2, float y2, float z2, float u2, float v2, float x3, float y3, float z3,
                       float u3, float v3, float red0, float green0, float blue0, float alpha0, float red1,
                       float green1, float blue1, float alpha1, int brightness);
    void blockParticleQuad(float x0, float y0, float z0, float u0, float v0, float x1, float y1, float z1, float u1,
                           float v1, float x2, float y2, float z2, float u2, float v2, float x3, float y3, float z3,
                           float u3, float v3, float red, float green, float blue, float alpha);

    void putBulkData(arrayWithLength<int> data);
    void putColor(int red, int green, int blue, int alpha);

    void useCompactVertices(bool enabled);
    bool getCompactVertices();
    void useProjectedTexture(bool enabled);
    bool setMipmapEnable(bool enabled);

    void setVertexAndPixelShader(C4JRender::eVertexType vertexType, C4JRender::ePixelShaderType pixelShaderType);
    void setVertexAndPixelShader(eMinecraftEntityMaterial material);
    void useDefaultShaders();
    void setVertexFormat(const VertexFormat* vertexFormat);

    uint32_t hasMaxVertices();

    void postNormal(float x, float y, float z);
    void postProcessFacePosition(double x, double y, double z);
    void faceTint(float red, float green, float blue, int face);
    void fixupVertexColor(int color, int index);
    void fixupVertexColor(float red, float green, float blue, int index);
    void fixupQuadColor(int color);
    void fixupQuadColor(float red, float green, float blue);

private:
    void finaliseShaders();
    void packCompactQuad();
    int getStartingColorIndex(int face);

    arrayWithLength<int>* mRawBuffer;
    arrayWithLength<unsigned char>* mBuckets;
    int mVertexCount;
    float mU;
    float mV;
    int mBrightness;
    int mColor;
    bool mHasColor;
    bool mHasTexture;
    bool mHasBrightness;
    bool mHasNormal;
    int mRawBufferIndex;
    bool mUseCompactVertices;
    bool mUseProjectedTexture;
    C4JRender::ePixelShaderType mPixelShaderType;
    C4JRender::eVertexType mVertexType;
    bool mHasBoneIndex;
    bool mUseCustomShaders;
    int mAddedVertices;
    bool mIsColorDisabled;
    int mDrawMode;
    float mXOffset;
    float mYOffset;
    float mZOffset;
    float mBaseXOffset;
    float mBaseYOffset;
    float mBaseZOffset;
    int mNormal;
    int mBoneIndex;
    int mBucket;

public:
    bool mIsDrawing;

private:
    bool mMipmapEnable;
    bool mUseVBO;
    IntBuffer* mVertexBuffers;
    int mVBOIndex;
    int mVBOCount;
    int mBufferSize;
    int mPackedQuad[29];

public:
    Bounds mBounds;
};

#endif
