#ifndef GAME_CLIENT_RENDER_BLOCKRENDERER_HPP
#define GAME_CLIENT_RENDER_BLOCKRENDERER_HPP

#include <boost/shared_ptr.hpp>
#include <boost/unordered_map.hpp>

#include "game/core/Direction.hpp"

class AABB;
class Block;
class BlockColors;
class BlockEntity;
class BlockPos;
class BlockState;
class Level;
class LevelSource;
class Material;
class Minecraft;
class TextureAtlasSprite;
class Vec3;

extern bool gFancyGraphics;

class BlockRenderer
{
public:
    struct BoxDesc
    {
        unsigned char hiddenFaces;
        float x0;
        float y0;
        float z0;
        float x1;
        float y1;
        float z1;
    };

    struct BoxUV
    {
        float down[4];
        float up[4];
        float north[4];
        float south[4];
        float east[4];
        float west[4];
    };

    BlockRenderer(BlockColors* blockColors);
    BlockRenderer(LevelSource* levelSource, int x, int y, int z, unsigned int* cache);
    BlockRenderer(LevelSource* levelSource);
    ~BlockRenderer();

    static bool canRender(int renderShape);

    TextureAtlasSprite* getTextureOrMissing(TextureAtlasSprite* texture);
    TextureAtlasSprite* getTexture(const BlockState* state, const Direction* direction);
    TextureAtlasSprite* getTexture(Block* block, const Direction* direction);
    TextureAtlasSprite* getTexture(const BlockState* state, LevelSource* levelSource, const BlockPos& pos,
                                   const Direction* direction);
    void setShape(const AABB* shape);
    void setShape(float x0, float y0, float z0, float x1, float y1, float z1);
    bool hasFixedTexture();
    void renderFaceDown(const BlockState* state, double x, double y, double z, TextureAtlasSprite* texture, float u0,
                        float v0, float u1, float v1);
    void renderFaceDown(const BlockState* state, double x, double y, double z, TextureAtlasSprite* texture);
    void renderFaceUp(const BlockState* state, double x, double y, double z, TextureAtlasSprite* texture, float u0,
                      float v0, float u1, float v1);
    void renderFaceUp(const BlockState* state, double x, double y, double z, TextureAtlasSprite* texture);
    void renderNorth(const BlockState* state, double x, double y, double z, TextureAtlasSprite* texture, float u0,
                     float v0, float u1, float v1);
    void renderNorth(const BlockState* state, double x, double y, double z, TextureAtlasSprite* texture);
    void renderSouth(const BlockState* state, double x, double y, double z, TextureAtlasSprite* texture, float u0,
                     float v0, float u1, float v1);
    void renderSouth(const BlockState* state, double x, double y, double z, TextureAtlasSprite* texture);
    void renderWest(const BlockState* state, double x, double y, double z, TextureAtlasSprite* texture, float u0,
                    float v0, float u1, float v1);
    void renderWest(const BlockState* state, double x, double y, double z, TextureAtlasSprite* texture);
    void renderEast(const BlockState* state, double x, double y, double z, TextureAtlasSprite* texture, float u0,
                    float v0, float u1, float v1);
    void renderEast(const BlockState* state, double x, double y, double z, TextureAtlasSprite* texture);
    void tesselateCrossTexture(TextureAtlasSprite* texture, TextureAtlasSprite* overlay, float x, float y, float z,
                               float scale);
    void tesselateCrossTexture(const BlockState* state, float x, float y, float z, float scale);
    void tesselateStemTexture(const BlockState* state, float height, float x, float y, float z);
    void tesselateRowTexture(const BlockState* state, float x, float y, float z);
    void tesselateTorch(const BlockState* state, float x, float y, float z, float xTilt, float zTilt);
    void fjRenderCubes(const BlockState* state, const BoxDesc* boxes, unsigned int boxCount, const BoxUV* uvs);
    int getLightColor(Block* block, LevelSource* levelSource, const BlockPos& pos);
    bool isSolidBlockingAt(LevelSource* levelSource, const BlockPos& pos);
    float getShadeBrightness(LevelSource* levelSource, const BlockPos& pos);
    int blend(int c0, int c1, int c2, int c3);
    int blend(int c0, int c1, int c2, int c3, double w0, double w1, double w2, double w3);
    void AOCalculate(Block* block, LevelSource* levelSource, const BlockPos& pos, Direction::EDirection direction,
                     bool shade, int color, bool ownBrightness, float red, float green, float blue, bool tint);
    bool tesselateBlockInWorldWithAmbienceOcclusionTexLighting(const BlockState* state, const BlockPos& pos, float red,
                                                               float green, float blue, int lightColor, bool tint);
    bool hasLightOverride();
    bool tesselateBlockInWorld(const BlockState* state, const BlockPos& pos, float red, float green, float blue);
    bool tesselateBlockInWorld(const BlockState* state, const BlockPos& pos);
    float tesselateAnvilPiece(const BlockState* state, const BlockPos& pos, int part, float bottom, float width,
                              float height, float length, bool rotate, bool inventory);
    bool tesselateAnvilInWorld(const BlockState* state, const BlockPos& pos, bool inventory);
    void setFixedTexture(TextureAtlasSprite* texture);
    void clearFixedTexture();
    bool tesselateHopperInWorld(const BlockState* state, const BlockPos& pos, bool inventory);
    void renderAllFaces(const BlockState* state);
    void tesselateEndRodCenter(const BlockState* state, float x, float y, float z);
    void tesselateEndRodBase(const BlockState* state, float x, float y, float z);
    void tesselateEndRod(const BlockState* state, float x, float y, float z);
    void renderBlock(const BlockState* state, float brightness, float alpha, bool useDisplayList, bool centre);
    void prepare(LevelSource* levelSource);
    void setLightOverride(int lightColor);
    void clearLightOverride();
    void setFixedShape(float x0, float y0, float z0, float x1, float y1, float z1);
    void clearFixedShape();
    bool tesselateBlockInWorld(const BlockState* state, const BlockPos& pos, int lightColor);
    bool tesselateTrapDoorInWorld(const BlockState* state, const BlockPos& pos);
    bool tesselateSlimeBlockInWorld(const BlockState* state, const BlockPos& pos);
    bool tesselateTreeInWorld(const BlockState* state, const BlockPos& pos);
    bool tesselateQuartzInWorld(const BlockState* state, const BlockPos& pos);
    bool tesselateHorizontalDirectionalInWorld(const BlockState* state, const BlockPos& pos);
    float getWaterHeight(const BlockPos& pos, Material* material);
    bool tesselateWaterInWorld(const BlockState* state, const BlockPos& pos);
    bool tesselateCactusInWorld(const BlockState* state, const BlockPos& pos, float red, float green, float blue);
    bool tesselateCactusInWorld(const BlockState* state, const BlockPos& pos);
    bool tesselateCrossInWorld(const BlockState* state, const BlockPos& pos);
    void tesselateSunflowerCrossTexture(const BlockState* state, float x, float y, float z);
    void tesselateSunflowerDirTexture(const BlockState* state, float x, float y, float z);
    bool tesselateDoublePlantInWorld(const BlockState* state, const BlockPos& pos);
    bool tesselateKelpInWorld(const BlockState* state, const BlockPos& pos);
    bool tessellateCoralFanInWorld(const BlockState* state, const BlockPos& pos);
    bool tessellateCoralFanHangInWorld(const BlockState* state, const BlockPos& pos);
    void tesselateStemDirTexture(const BlockState* state, const Direction* direction, float height, float x, float y,
                                 float z);
    bool tesselateStemInWorld(const BlockState* state, const BlockPos& pos);
    bool tesselateLilypadInWorld(const BlockState* state, const BlockPos& pos);
    bool tesselateRowInWorld(const BlockState* state, const BlockPos& pos);
    bool tesselateTorchInWorld(const BlockState* state, const BlockPos& pos);
    bool tesselateFireInWorld(const BlockState* state, const BlockPos& pos);
    bool tesselateDustInWorld(const BlockState* state, const BlockPos& pos);
    bool tesselateLadderInWorld(const BlockState* state, const BlockPos& pos);
    bool tesselateDoorInWorld(const BlockState* state, const BlockPos& pos);
    bool tesselateRailInWorld(const BlockState* state, const BlockPos& pos);
    bool tesselateStairsInWorld(const BlockState* state, const BlockPos& pos);
    bool tesselateEggInWorld(const BlockState* state, const BlockPos& pos);
    bool tesselateFenceInWorld(const BlockState* state, const BlockPos& pos);
    bool tesselateWallInWorld(const BlockState* state, const BlockPos& pos);
    bool tesselateLeverInWorld(const BlockState* state, const BlockPos& pos);
    bool tesselateTripwireSourceInWorld(const BlockState* state, const BlockPos& pos);
    bool tesselateTripwireInWorld(const BlockState* state, const BlockPos& pos);
    bool tesselateBedInWorld(const BlockState* state, const BlockPos& pos);
    void tesselateDiodeInWorld(const BlockState* state, const BlockPos& pos, const Direction* direction);
    bool tesselateDiodeInWorld(const BlockState* state, const BlockPos& pos);
    bool tesselateRepeaterInWorld(const BlockState* state, const BlockPos& pos);
    bool tesselateComparatorInWorld(const BlockState* state, const BlockPos& pos);
    bool tesselatePistonBaseInWorld(const BlockState* state, const BlockPos& pos, bool forceExtended,
                                    const BlockState* actualState);
    void renderPistonArmUpDown(float x0, float x1, float y0, float y1, float z0, float z1, float shade, float length);
    void renderPistonArmNorthSouth(float x0, float x1, float y0, float y1, float z0, float z1, float shade,
                                   float length);
    void renderPistonArmEastWest(float x0, float x1, float y0, float y1, float z0, float z1, float shade, float length);
    bool tesselatePistonExtensionInWorld(const BlockState* state, const BlockPos& pos, bool fullArm,
                                         const BlockState* actualState);
    bool tesselateThinFenceInWorld(const BlockState* state, const BlockPos& pos);
    bool tesselateVineInWorld(const BlockState* state, const BlockPos& pos);
    bool tesselateFenceGateInWorld(const BlockState* state, const BlockPos& pos);
    bool tesselateCauldronInWorld(const BlockState* state, const BlockPos& pos);
    bool tesselateFlowerPotInWorld(const BlockState* state, const BlockPos& pos);
    bool tesselateAnvilInWorld(const BlockState* state, const BlockPos& pos);
    bool tesselateBrewingStandInWorld(const BlockState* state, const BlockPos& pos);
    bool tesselateAirPortalFrameInWorld(const BlockState* state, const BlockPos& pos);
    bool tesselateCocoaInWorld(const BlockState* state, const BlockPos& pos);
    bool tesselateBeaconInWorld(const BlockState* state, const BlockPos& pos);
    bool tesselateHopperInWorld(const BlockState* state, const BlockPos& pos);
    bool tesselateEndRodInWorld(const BlockState* state, const BlockPos& pos);
    bool tesselateChorusFlowerInWorld(const BlockState* state, const BlockPos& pos);
    bool tesselateChorusPlantInWorld(const BlockState* state, const BlockPos& pos);
    bool tesselateObserverInWorld(const BlockState* state, const BlockPos& pos);
    void tesselateTurtleEgg(const BlockState* state, const BlockPos& pos, float x, float y, float z);
    bool tesselateTurtleEggInWorld(const BlockState* state, const BlockPos& pos);
    bool tessellateSeagrassInWorld(const BlockState* state, const BlockPos& pos);
    bool tessellateIndividualSeaPickleInWorld(const BlockState* state, const BlockPos& pos, double height,
                                              const Vec3* offset);
    bool tessellateSeaPickleInWorld(const BlockState* state, const BlockPos& pos);
    bool tesselateCommandBlockInWorld(const BlockState* state, const BlockPos& pos);
    bool tesselateInWorld(const BlockState* state, const BlockPos& pos, const BlockState* actualState,
                          boost::shared_ptr<BlockEntity> blockEntity);
    void tesselateInWorldFixedTexture(const BlockState* state, const BlockPos& pos, TextureAtlasSprite* texture);
    bool tesselateInWorldNoCulling(const BlockState* state, const BlockPos& pos, const BlockState* actualState,
                                   boost::shared_ptr<BlockEntity> blockEntity);
    void tesselatePistonBaseForceExtended(const BlockState* state, const BlockPos& pos, const BlockState* actualState);
    void tesselatePistonArmNoCulling(const BlockState* state, const BlockPos& pos, bool shortHead,
                                     const BlockState* actualState);
    void renderBlock(const BlockState* state, Level* level, const BlockPos& pos);
    void renderCube(const BlockState* state, float scale);

private:
    friend class FallingBlockRenderer;

    void _init();

    BlockColors* mBlockColors;
    LevelSource* mLevelSource;
    TextureAtlasSprite* mFixedTexture;
    int mLightOverride;
    bool mFlipTexture;
    bool mRenderAllFaces;
    bool mUseInventoryTint;

public:
    bool mUseColor;

private:
    float mMinX;
    float mMaxX;
    float mMinY;
    float mMaxY;
    float mMinZ;
    float mMaxZ;
    bool mFixedShape;
    bool mShapeOutside;
    Minecraft* mMinecraft;
    boost::unordered_map<int, int> mBrightnessCache;
    int mOriginX;
    int mOriginY;
    int mOriginZ;
    int mCacheMinX;
    int mCacheMinY;
    int mCacheMinZ;
    unsigned int* mCache;
    int mFaceRotation0;
    int mFaceRotation1;
    int mFaceRotation2;
    int mFaceRotation3;
    int mFaceRotation4;
    int mFaceRotation5;
    bool mRenderFromInside;
    int mFaceTransform0;
    int mFaceTransform1;
    int mFaceTransform2;
    int mFaceTransform3;
    int mFaceTransform4;
    int mFaceTransform5;
    bool mEnableAO;
    float mAoLightValueScratch[20];
    int mAoBrightnessScratch[20];
    int mAoBrightnessTopLeft;
    int mAoBrightnessBottomLeft;
    int mAoBrightnessBottomRight;
    int mAoBrightnessTopRight;
    float mColorRedTopLeft;
    float mColorRedBottomLeft;
    float mColorRedBottomRight;
    float mColorRedTopRight;
    float mColorGreenTopLeft;
    float mColorGreenBottomLeft;
    float mColorGreenBottomRight;
    float mColorGreenTopRight;
    float mColorBlueTopLeft;
    float mColorBlueBottomLeft;
    float mColorBlueBottomRight;
    float mColorBlueTopRight;
};

#endif
