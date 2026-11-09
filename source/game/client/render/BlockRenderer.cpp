#include "game/client/render/BlockRenderer.hpp"
#include "game/world/block/BlockId.hpp"
#include "game/core/EntityType.hpp"

#include <cmath>
#include <cstdlib>
#include <cstring>

#include "game/client/render/TextureAtlasSprite.hpp"
#include "game/client/render/Textures.hpp"
#include "game/client/render/Tesselator.hpp"
#include "game/core/BufferBuilder.hpp"
#include "game/core/ColourTable.hpp"
#include "game/core/GlStateManager.hpp"
#include "game/core/CoralFan.hpp"
#include "game/core/CoralFanHang.hpp"
#include "game/core/Minecraft.hpp"
#include "game/core/SeaPickle.hpp"
#include "game/core/Mth.hpp"
#include "game/core/XMem.hpp"
#include "game/world/generation/MultiPlayerLevel.hpp"
#include "game/util/AABB.hpp"
#include "game/util/Vec3.hpp"
#include "game/world/Level.hpp"
#include "game/world/LevelSource.hpp"
#include "game/world/block/AnvilBlock.hpp"
#include "game/world/block/BaseRailBlock.hpp"
#include "game/world/block/BedBlock.hpp"
#include "game/world/block/CauldronBlockEntity.hpp"
#include "game/world/block/CauldronBlock.hpp"
#include "game/world/block/CocoaBlock.hpp"
#include "game/world/block/MutableBlockPos.hpp"
#include "game/world/block/QuartzBlock.hpp"
#include "game/world/block/TurtleEggBlock.hpp"
#include "game/world/block/EndRodBlock.hpp"
#include "game/world/block/TripWireHookBlock.hpp"
#include "game/world/block/TripWireBlock.hpp"
#include "game/world/block/TallGrassBlock.hpp"
#include "game/world/block/PlanksBlock.hpp"
#include "game/world/block/SaplingBlock.hpp"
#include "game/world/block/FlowerPotBlockEntity.hpp"
#include "game/world/block/FlowerPotBlock.hpp"
#include "game/world/block/RepeaterBlock.hpp"
#include "game/world/block/ComparatorBlock.hpp"
#include "game/world/block/Block.hpp"
#include "game/world/block/BlockColors.hpp"
#include "game/world/block/BlockEntity.hpp"
#include "game/world/block/BlockPos.hpp"
#include "game/world/block/BlockState.hpp"
#include "game/world/block/TypedBoxed.hpp"
#include "game/world/biome/Biome.hpp"
#include "game/item/ItemInstance.hpp"
#include "game/client/render/EntityBlockRenderer.hpp"
#include "game/client/render/GameRenderer.hpp"
#include "game/world/block/Blocks.hpp"
#include "game/world/block/FireBlock.hpp"
#include "game/world/block/VineBlock.hpp"
#include "game/world/block/BrewingStandBlock.hpp"
#include "game/world/block/ChorusFlowerBlock.hpp"
#include "game/world/block/ChorusPlantBlock.hpp"
#include "game/world/block/CommandBlock.hpp"
#include "game/world/block/DirectionalBlock.hpp"
#include "game/world/block/DirectionProperty.hpp"
#include "game/world/block/DoorBlock.hpp"
#include "game/world/block/EndPortalFrameBlock.hpp"
#include "game/world/block/FenceBlock.hpp"
#include "game/world/block/FenceGateBlock.hpp"
#include "game/world/block/GrassBlock.hpp"
#include "game/world/block/HopperBlock.hpp"
#include "game/world/block/DoublePlantBlock.hpp"
#include "game/world/block/HorizontalDirectionalBlock.hpp"
#include "game/world/block/LadderBlock.hpp"
#include "game/world/block/ButtonBlock.hpp"
#include "game/world/block/LeverBlock.hpp"
#include "game/core/Options.hpp"
#include "game/world/block/LiquidBlock.hpp"
#include "game/world/block/Material.hpp"
#include "game/world/block/PistonBaseBlock.hpp"
#include "game/world/block/PistonHeadBlock.hpp"
#include "game/world/block/RotatedPillarBlock.hpp"
#include "game/world/block/RedStoneWireBlock.hpp"
#include "game/world/block/StemBlock.hpp"
#include "game/world/block/ThinFenceBlock.hpp"
#include "game/world/block/TrapDoorBlock.hpp"
#include "game/world/block/TorchBlock.hpp"
#include "game/world/block/StainedGlassPaneBlock.hpp"
#include "game/world/block/WallBlock.hpp"
#include "game/util/Random.hpp"

BlockRenderer::BlockRenderer(BlockColors* blockColors) : mBrightnessCache()
{
    mLevelSource = NULL;
    mBlockColors = blockColors;
    _init();
}

BlockRenderer::BlockRenderer(LevelSource* levelSource, int x, int y, int z, unsigned int* cache) : mBrightnessCache()
{
    mBlockColors = Minecraft::GetInstance()->getBlockColors();
    mLevelSource = levelSource;
    _init();
    mOriginX = x;
    mOriginY = y;
    mOriginZ = z;
    mCacheMinX = x - 2;
    mCacheMinY = y - 2;
    mCacheMinZ = z - 2;
    mCache = cache;
    XMemSet(cache, 0, 0x20000);
}

BlockRenderer::BlockRenderer(LevelSource* levelSource) : mBrightnessCache()
{
    mBlockColors = Minecraft::GetInstance()->getBlockColors();
    mLevelSource = levelSource;
    _init();
}

BlockRenderer::~BlockRenderer()
{
}

void BlockRenderer::_init()
{
    mFixedTexture = NULL;
    mLightOverride = -1;
    mFlipTexture = false;
    mRenderAllFaces = false;
    mUseColor = true;
    mMinX = 0.0f;
    mMaxX = 0.0f;
    mMinY = 0.0f;
    mMaxY = 0.0f;
    mMinZ = 0.0f;
    mMaxZ = 0.0f;
    mFixedShape = false;
    mShapeOutside = false;
    for (int i = 0; i < 6; ++i)
    {
        (&mFaceRotation0)[i] = 0;
        (&mFaceTransform0)[i] = 0;
    }
    mRenderFromInside = false;
    mEnableAO = false;
    mMinecraft = Minecraft::GetInstance();
    mUseInventoryTint = false;
    mOriginX = 0;
    mOriginY = 0;
    mOriginZ = 0;
    mCache = NULL;
}

bool BlockRenderer::canRender(int renderShape)
{
    switch (renderShape)
    {
    case BLOCK_RENDER_MODEL:
    case BLOCK_RENDER_WATER:
    case BLOCK_RENDER_STAIRS:
    case BLOCK_RENDER_FENCE:
    case BLOCK_RENDER_CACTUS:
    case BLOCK_RENDER_PISTON_BASE:
    case BLOCK_RENDER_FENCE_GATE:
    case BLOCK_RENDER_ENTITY:
    case BLOCK_RENDER_AIR_PORTAL_FRAME:
    case BLOCK_RENDER_EGG:
    case BLOCK_RENDER_TREE:
    case BLOCK_RENDER_WALL:
    case BLOCK_RENDER_BEACON:
    case BLOCK_RENDER_ANVIL:
    case BLOCK_RENDER_HOPPER:
    case BLOCK_RENDER_QUARTZ:
    case BLOCK_RENDER_SLIME_BLOCK:
    case BLOCK_RENDER_TRAPDOOR:
    case BLOCK_RENDER_END_ROD:
    case BLOCK_RENDER_CHORUS_FLOWER:
    case BLOCK_RENDER_CHORUS_PLANT:
    case BLOCK_RENDER_HORIZONTAL_DIRECTIONAL:
    case BLOCK_RENDER_OBSERVER:
    case BLOCK_RENDER_TURTLE_EGG:
    case BLOCK_RENDER_CONDUIT:
    case BLOCK_RENDER_COMMAND_BLOCK:
        return true;
    default:
        return false;
    }
}

TextureAtlasSprite* BlockRenderer::getTextureOrMissing(TextureAtlasSprite* texture)
{
    if (texture != NULL)
        return texture;
    return mMinecraft->mTextures->getMissingIcon(0);
}

TextureAtlasSprite* BlockRenderer::getTexture(const BlockState* state, const Direction* direction)
{
    return getTextureOrMissing(state->getBlock()->getTexture(direction, state));
}

TextureAtlasSprite* BlockRenderer::getTexture(Block* block, const Direction* direction)
{
    return getTextureOrMissing(block->getTexture(direction));
}

TextureAtlasSprite* BlockRenderer::getTexture(const BlockState* state, LevelSource* levelSource, const BlockPos& pos,
                                              const Direction* direction)
{
    return getTextureOrMissing(state->getBlock()->getTexture(levelSource, pos, direction, state));
}

void BlockRenderer::setShape(const AABB* shape)
{
    if (mFixedShape)
        return;
    float x0 = (float)shape->mMinX;
    float y0 = (float)shape->mMinY;
    float z0 = (float)shape->mMinZ;
    float x1 = (float)shape->mMaxX;
    float y1 = (float)shape->mMaxY;
    float z1 = (float)shape->mMaxZ;
    mMinX = x0;
    mMaxX = x1;
    mMinY = y0;
    mMaxY = y1;
    mMinZ = z0;
    mMaxZ = z1;
    mShapeOutside = x0 > 0.0f || x1 < 1.0f || y0 > 0.0f || y1 < 1.0f || z0 > 0.0f || z1 < 1.0f;
}

void BlockRenderer::setShape(float x0, float y0, float z0, float x1, float y1, float z1)
{
    if (mFixedShape)
        return;
    mMinY = y0;
    mMaxX = x1;
    mMaxZ = z1;
    mMinZ = z0;
    mMinX = x0;
    mMaxY = y1;
    bool outside = true;
    if (x0 <= 0.0f && x1 >= 1.0f && y0 <= 0.0f && y1 >= 1.0f && z0 <= 0.0f && z1 >= 1.0f)
        outside = false;
    mShapeOutside = outside;
}

bool BlockRenderer::hasFixedTexture()
{
    return mFixedTexture != NULL;
}

void BlockRenderer::setFixedTexture(TextureAtlasSprite* texture)
{
    mFixedTexture = texture;
}

void BlockRenderer::clearFixedTexture()
{
    mFixedTexture = NULL;
}

bool BlockRenderer::hasLightOverride()
{
    return mLightOverride != -1;
}

void BlockRenderer::prepare(LevelSource* levelSource)
{
    mLevelSource = levelSource;
}

void BlockRenderer::setLightOverride(int lightColor)
{
    mLightOverride = lightColor;
}

void BlockRenderer::clearLightOverride()
{
    mLightOverride = -1;
}

void BlockRenderer::setFixedShape(float x0, float y0, float z0, float x1, float y1, float z1)
{
    mMinX = x0;
    mMinY = y0;
    mMinZ = z0;
    mMaxX = x1;
    mMaxY = y1;
    mMaxZ = z1;
    mFixedShape = true;
    bool outside = true;
    if (x0 <= 0.0f && x1 >= 1.0f && y0 <= 0.0f && y1 >= 1.0f && z0 <= 0.0f && z1 >= 1.0f)
        outside = false;
    mShapeOutside = outside;
}

void BlockRenderer::clearFixedShape()
{
    mFixedShape = false;
}

int BlockRenderer::blend(int c0, int c1, int c2, int c3)
{
    if (c0 == 0)
        c0 = c3;
    if (c1 == 0)
        c1 = c3;
    if (c2 == 0)
        c2 = c3;
    return ((c0 + c1 + c2 + c3) >> 2) & 0xff00ff;
}

int BlockRenderer::blend(int c0, int c1, int c2, int c3, double w0, double w1, double w2, double w3)
{
    int red = (int)(((c0 >> 16 & 255) * w0) + ((c1 >> 16 & 255) * w1) + ((c2 >> 16 & 255) * w2)
                    + ((c3 >> 16 & 255) * w3))
              & 255;
    int blue = (int)(((c0 & 255) * w0) + ((c1 & 255) * w1) + ((c2 & 255) * w2) + ((c3 & 255) * w3)) & 255;
    return red << 16 | blue;
}

void BlockRenderer::AOCalculate(Block* block, LevelSource* levelSource, const BlockPos& pos,
                                Direction::EDirection direction, bool atBoundary, int lightColor, bool useColor,
                                float red, float green, float blue, bool partialBounds)
{
    MutableBlockPos neighbours[8];
    BlockPos facePos(0, 0, 0);
    const float faceShades[6] = {0.5f, 1.0f, 0.8f, 0.8f, 0.6f, 0.6f};
    const int neighbourOffsets[12][8][3] = {
        {{0, 0, 1}, {-1, 0, 1}, {-1, 0, 0}, {-1, 0, -1}, {0, 0, -1}, {1, 0, -1}, {1, 0, 0}, {1, 0, 1}},
        {{0, 0, 1}, {1, 0, 1}, {1, 0, 0}, {1, 0, -1}, {0, 0, -1}, {-1, 0, -1}, {-1, 0, 0}, {-1, 0, 1}},
        {{-1, 0, 0}, {-1, 1, 0}, {0, 1, 0}, {1, 1, 0}, {1, 0, 0}, {1, -1, 0}, {0, -1, 0}, {-1, -1, 0}},
        {{0, 1, 0}, {-1, 1, 0}, {-1, 0, 0}, {-1, -1, 0}, {0, -1, 0}, {1, -1, 0}, {1, 0, 0}, {1, 1, 0}},
        {{0, 0, 1}, {0, 1, 1}, {0, 1, 0}, {0, 1, -1}, {0, 0, -1}, {0, -1, -1}, {0, -1, 0}, {0, -1, 1}},
        {{0, 0, 1}, {0, -1, 1}, {0, -1, 0}, {0, -1, -1}, {0, 0, -1}, {0, 1, -1}, {0, 1, 0}, {0, 1, 1}},
        {{0, -1, 1}, {-1, -1, 1}, {-1, -1, 0}, {-1, -1, -1}, {0, -1, -1}, {1, -1, -1}, {1, -1, 0}, {1, -1, 1}},
        {{0, 1, 1}, {1, 1, 1}, {1, 1, 0}, {1, 1, -1}, {0, 1, -1}, {-1, 1, -1}, {-1, 1, 0}, {-1, 1, 1}},
        {{-1, 0, -1}, {-1, 1, -1}, {0, 1, -1}, {1, 1, -1}, {1, 0, -1}, {1, -1, -1}, {0, -1, -1}, {-1, -1, -1}},
        {{0, 1, 1}, {-1, 1, 1}, {-1, 0, 1}, {-1, -1, 1}, {0, -1, 1}, {1, -1, 1}, {1, 0, 1}, {1, 1, 1}},
        {{-1, 0, 1}, {-1, 1, 1}, {-1, 1, 0}, {-1, 1, -1}, {-1, 0, -1}, {-1, -1, -1}, {-1, -1, 0}, {-1, -1, 1}},
        {{1, 0, 1}, {1, -1, 1}, {1, -1, 0}, {1, -1, -1}, {1, 0, -1}, {1, 1, -1}, {1, 1, 0}, {1, 1, 1}}};
    float ringShade[8];
    int ringLight[8];
    bool ringSolid[8];
    float weights[4][4][4];

    const int (*offsets)[3] = neighbourOffsets[atBoundary * 6 + direction];

    for (int i = 0; i < 8; i += 2)
        neighbours[i].set(pos.x + offsets[i][0], pos.y + offsets[i][1], pos.z + offsets[i][2]);

    switch (direction)
    {
    case Direction::eDirection_Down:
        facePos.x = pos.x;
        facePos.y = pos.y - 1;
        facePos.z = pos.z;
        break;
    case Direction::eDirection_Up:
        facePos.x = pos.x;
        facePos.y = pos.y + 1;
        facePos.z = pos.z;
        break;
    case Direction::eDirection_North:
        facePos.x = pos.x;
        facePos.y = pos.y;
        facePos.z = pos.z - 1;
        break;
    case Direction::eDirection_South:
        facePos.x = pos.x;
        facePos.y = pos.y;
        facePos.z = pos.z + 1;
        break;
    case Direction::eDirection_West:
        facePos.x = pos.x - 1;
        facePos.y = pos.y;
        facePos.z = pos.z;
        break;
    case Direction::eDirection_East:
        facePos.x = pos.x + 1;
        facePos.y = pos.y;
        facePos.z = pos.z;
        break;
    }

    if (partialBounds)
    {
        switch (direction)
        {
        case Direction::eDirection_North:
            weights[0][0][0] = 1.0f - mMinX;
            weights[0][0][1] = mMaxY;
            weights[0][0][2] = 1.0f;
            weights[1][1][0] = mMaxX;
            weights[1][1][1] = mMaxY;
            weights[1][1][2] = 1.0f;
            weights[2][2][0] = mMaxX;
            weights[2][2][1] = 1.0f - mMinY;
            weights[2][2][2] = 1.0f;
            weights[3][3][0] = 1.0f - mMinX;
            weights[3][3][1] = 1.0f - mMinY;
            weights[3][3][2] = 1.0f;
            break;
        case Direction::eDirection_South:
            weights[0][0][0] = 1.0f - mMinX;
            weights[0][0][1] = mMaxY;
            weights[0][0][2] = 1.0f;
            weights[1][1][0] = 1.0f - mMinX;
            weights[1][1][1] = 1.0f - mMinY;
            weights[1][1][2] = 1.0f;
            weights[2][2][0] = mMaxX;
            weights[2][2][1] = 1.0f - mMinY;
            weights[2][2][2] = 1.0f;
            weights[3][3][0] = mMaxX;
            weights[3][3][1] = mMaxY;
            weights[3][3][2] = 1.0f;
            break;
        case Direction::eDirection_West:
            weights[0][0][0] = 1.0f;
            weights[0][0][1] = mMaxY;
            weights[0][0][2] = mMaxZ;
            weights[1][1][0] = 1.0f;
            weights[1][1][1] = mMaxY;
            weights[1][1][2] = 1.0f - mMinZ;
            weights[2][2][0] = 1.0f;
            weights[2][2][1] = 1.0f - mMinY;
            weights[2][2][2] = 1.0f - mMinZ;
            weights[3][3][0] = 1.0f;
            weights[3][3][1] = 1.0f - mMinY;
            weights[3][3][2] = mMaxZ;
            break;
        case Direction::eDirection_East:
            weights[0][0][0] = 1.0f;
            weights[0][0][1] = 1.0f - mMinY;
            weights[0][0][2] = mMaxZ;
            weights[1][1][0] = 1.0f;
            weights[1][1][1] = 1.0f - mMinY;
            weights[1][1][2] = 1.0f - mMinZ;
            weights[2][2][0] = 1.0f;
            weights[2][2][1] = mMaxY;
            weights[2][2][2] = 1.0f - mMinZ;
            weights[3][3][0] = 1.0f;
            weights[3][3][1] = mMaxY;
            weights[3][3][2] = mMaxZ;
            break;
        }

        for (int vertex = 0; vertex < 4; ++vertex)
        {
            float first = weights[vertex][vertex][0];
            float second = weights[vertex][vertex][1];
            float third = weights[vertex][vertex][2];
            for (int corner = 0; corner < 4; ++corner)
            {
                if (vertex != corner)
                {
                    const int* vertexOffset = neighbourOffsets[direction][vertex * 2 + 1];
                    const int* cornerOffset = neighbourOffsets[direction][corner * 2 + 1];
                    weights[vertex][corner][0] = vertexOffset[0] == cornerOffset[0] ? first : 1.0f - first;
                    weights[vertex][corner][1] = vertexOffset[1] == cornerOffset[1] ? second : 1.0f - second;
                    weights[vertex][corner][2] = vertexOffset[2] == cornerOffset[2] ? third : 1.0f - third;
                }
                weights[vertex][corner][3] = weights[vertex][corner][0] * weights[vertex][corner][1]
                                             * weights[vertex][corner][2];
            }
        }
    }

    for (int i = 0; i < 8; i += 2)
    {
        ringLight[i] = getLightColor(block, levelSource, neighbours[i]);
        ringShade[i] = getShadeBrightness(levelSource, neighbours[i]);
        ringSolid[i] = isSolidBlockingAt(levelSource, neighbours[i]);
    }

    for (int i = 1; i < 8; i += 4)
    {
        if (ringSolid[i - 1] && ringSolid[(i + 1) % 8])
        {
            ringLight[i] = ringLight[i - 1];
            ringShade[i] = ringShade[i - 1];
        }
        else
        {
            neighbours[i].set(pos.x + offsets[i][0], pos.y + offsets[i][1], pos.z + offsets[i][2]);
            ringLight[i] = getLightColor(block, levelSource, neighbours[i]);
            ringShade[i] = getShadeBrightness(levelSource, neighbours[i]);
        }
        if (ringSolid[i + 1] && ringSolid[(i + 3) % 8])
        {
            ringLight[i + 2] = ringLight[i + 1];
            ringShade[i + 2] = ringShade[i + 1];
        }
        else
        {
            neighbours[i + 2].set(pos.x + offsets[i + 2][0], pos.y + offsets[i + 2][1], pos.z + offsets[i + 2][2]);
            ringLight[i + 2] = getLightColor(block, levelSource, neighbours[i + 2]);
            ringShade[i + 2] = getShadeBrightness(levelSource, neighbours[i + 2]);
        }
    }

    if (atBoundary || !levelSource->getBlockState(facePos)->isSolidRender())
        lightColor = getLightColor(block, levelSource, facePos);
    float faceShade = getShadeBrightness(levelSource, facePos);

    float vertexShade[4];
    vertexShade[0] = (ringShade[0] + ringShade[1] + ringShade[2] + faceShade) * 0.25f;
    vertexShade[1] = (ringShade[2] + ringShade[3] + ringShade[4] + faceShade) * 0.25f;
    vertexShade[2] = (ringShade[4] + ringShade[5] + ringShade[6] + faceShade) * 0.25f;
    vertexShade[3] = (ringShade[6] + ringShade[7] + ringShade[0] + faceShade) * 0.25f;

    int vertexLight[4];
    float outputShade[4];
    if (!partialBounds)
    {
        mAoBrightnessTopLeft = blend(ringLight[0], ringLight[1], ringLight[2], lightColor);
        mAoBrightnessBottomLeft = blend(ringLight[2], ringLight[3], ringLight[4], lightColor);
        mAoBrightnessBottomRight = blend(ringLight[4], ringLight[5], ringLight[6], lightColor);
        mAoBrightnessTopRight = blend(ringLight[6], ringLight[7], ringLight[0], lightColor);
        for (int i = 0; i < 4; ++i)
            outputShade[i] = vertexShade[i];
    }
    else
    {
        vertexLight[0] = blend(ringLight[0], ringLight[1], ringLight[2], lightColor);
        vertexLight[1] = blend(ringLight[2], ringLight[3], ringLight[4], lightColor);
        vertexLight[2] = blend(ringLight[4], ringLight[5], ringLight[6], lightColor);
        vertexLight[3] = blend(ringLight[6], ringLight[7], ringLight[0], lightColor);
        for (int i = 0; i < 4; ++i)
        {
            outputShade[i] = vertexShade[0] * weights[i][0][3] + vertexShade[1] * weights[i][1][3]
                             + vertexShade[2] * weights[i][2][3] + vertexShade[3] * weights[i][3][3];
        }
        mAoBrightnessTopLeft = blend(vertexLight[0], vertexLight[1], vertexLight[2], vertexLight[3], weights[0][0][3],
                                     weights[0][1][3], weights[0][2][3], weights[0][3][3]);
        mAoBrightnessBottomLeft = blend(vertexLight[0], vertexLight[1], vertexLight[2], vertexLight[3],
                                        weights[1][0][3], weights[1][1][3], weights[1][2][3], weights[1][3][3]);
        mAoBrightnessBottomRight = blend(vertexLight[0], vertexLight[1], vertexLight[2], vertexLight[3],
                                         weights[2][0][3], weights[2][1][3], weights[2][2][3], weights[2][3][3]);
        mAoBrightnessTopRight = blend(vertexLight[0], vertexLight[1], vertexLight[2], vertexLight[3], weights[3][0][3],
                                      weights[3][1][3], weights[3][2][3], weights[3][3][3]);
    }

    float shade = faceShades[direction];
    if (!useColor)
    {
        mColorRedTopLeft = outputShade[0] * shade;
        mColorGreenTopLeft = outputShade[0] * shade;
        mColorBlueTopLeft = outputShade[0] * shade;
        mColorRedBottomLeft = outputShade[1] * shade;
        mColorGreenBottomLeft = outputShade[1] * shade;
        mColorBlueBottomLeft = outputShade[1] * shade;
        mColorRedBottomRight = outputShade[2] * shade;
        mColorGreenBottomRight = outputShade[2] * shade;
        mColorBlueBottomRight = outputShade[2] * shade;
        mColorRedTopRight = outputShade[3] * shade;
        mColorGreenTopRight = outputShade[3] * shade;
        mColorBlueTopRight = outputShade[3] * shade;
    }
    else
    {
        mColorRedTopLeft = outputShade[0] * red * shade;
        mColorRedBottomLeft = outputShade[1] * red * shade;
        mColorRedBottomRight = outputShade[2] * red * shade;
        mColorRedTopRight = outputShade[3] * red * shade;
        mColorGreenTopLeft = outputShade[0] * green * shade;
        mColorGreenBottomLeft = outputShade[1] * green * shade;
        mColorGreenBottomRight = outputShade[2] * green * shade;
        mColorGreenTopRight = outputShade[3] * green * shade;
        mColorBlueTopLeft = outputShade[0] * blue * shade;
        mColorBlueBottomLeft = outputShade[1] * blue * shade;
        mColorBlueBottomRight = outputShade[2] * blue * shade;
        mColorBlueTopRight = outputShade[3] * blue * shade;
    }
}

void BlockRenderer::renderFaceDown(const BlockState* state, double x, double y, double z, TextureAtlasSprite* texture)
{
    if (hasFixedTexture())
        texture = mFixedTexture;
    float u0 = (float)texture->getU(mMinX * 16.0f);
    float u1 = (float)texture->getU(mMaxX * 16.0f);
    float v0 = (float)texture->getV(mMinZ * 16.0f);
    float v1 = (float)texture->getV(mMaxZ * 16.0f);
    texture->adjustUVs(u0, u1, v0, v1);
    renderFaceDown(state, x, y, z, texture, u0, v0, u1, v1);
}

void BlockRenderer::renderFaceDown(const BlockState*, double x, double y, double z, TextureAtlasSprite* texture,
                                   float u0, float v0, float u1, float v1)
{
    BufferBuilder* builder = Tesselator::getInstance()->getBuilder();
    if (mMinX < 0.0f || 1.0f < mMaxX)
    {
        u0 = (float)texture->getU0(true);
        u1 = (float)texture->getU1(true);
    }
    if (mMinZ < 0.0f || 1.0f < mMaxZ)
    {
        v0 = (float)texture->getV0(true);
        v1 = (float)texture->getV1(true);
    }
    if (mFaceRotation5 == 2)
    {
        u0 = (float)texture->getU(mMinZ * 16.0f);
        v0 = (float)texture->getV(16.0f - mMaxX * 16.0f);
        u1 = (float)texture->getU(mMaxZ * 16.0f);
        v1 = (float)texture->getV(16.0f - mMinX * 16.0f);
        texture->adjustUVs(u0, u1, v0, v1);
    }
    else if (mFaceRotation5 == 1)
    {
        u0 = (float)texture->getU(16.0f - mMaxZ * 16.0f);
        v0 = (float)texture->getV(mMinX * 16.0f);
        u1 = (float)texture->getU(16.0f - mMinZ * 16.0f);
        v1 = (float)texture->getV(mMaxX * 16.0f);
        texture->adjustUVs(u0, u1, v0, v1);
    }
    else if (mFaceRotation5 == 3)
    {
        u0 = (float)texture->getU(16.0f - mMinX * 16.0f);
        u1 = (float)texture->getU(16.0f - mMaxX * 16.0f);
        v0 = (float)texture->getV(16.0f - mMinZ * 16.0f);
        v1 = (float)texture->getV(16.0f - mMaxZ * 16.0f);
        texture->adjustUVs(u0, u1, v0, v1);
    }
    if (mFaceTransform5 == 3)
    {
        float swapped = u0;
        u0 = u1;
        u1 = swapped;
        swapped = v0;
        v0 = v1;
        v1 = swapped;
    }
    else if (mFaceTransform5 == 2)
    {
        float swapped = u0;
        u0 = u1;
        u1 = swapped;
    }
    else if (mFaceTransform5 == 1)
    {
        float swapped = v0;
        v0 = v1;
        v1 = swapped;
    }
    float faceU[4];
    float faceV[4];
    if (mFaceRotation5 == 2 || mFaceRotation5 == 5)
    {
        faceU[0] = u1;
        faceV[0] = v1;
        faceU[1] = u0;
        faceV[1] = v1;
        faceU[2] = u0;
        faceV[2] = v0;
        faceU[3] = u1;
        faceV[3] = v0;
    }
    else if (mFaceRotation5 == 1 || mFaceRotation5 == 4)
    {
        faceU[0] = u0;
        faceV[0] = v0;
        faceU[1] = u1;
        faceV[1] = v0;
        faceU[2] = u1;
        faceV[2] = v1;
        faceU[3] = u0;
        faceV[3] = v1;
    }
    else
    {
        faceU[0] = u0;
        faceV[0] = v1;
        faceU[1] = u0;
        faceV[1] = v0;
        faceU[2] = u1;
        faceV[2] = v0;
        faceU[3] = u1;
        faceV[3] = v1;
    }
    float x0 = (float)(x + mMinX);
    float x1 = (float)(x + mMaxX);
    float yy = (float)(y + mMinY);
    float z0 = (float)(z + mMinZ);
    float z1 = (float)(z + mMaxZ);
    builder->bucket(6);
    if (!mEnableAO)
    {
        builder->vertexUV(x0, yy, z1, faceU[0], faceV[0]);
        builder->vertexUV(x0, yy, z0, faceU[1], faceV[1]);
        builder->vertexUV(x1, yy, z0, faceU[2], faceV[2]);
        builder->vertexUV(x1, yy, z1, faceU[3], faceV[3]);
        return;
    }
    builder->color(mColorRedTopLeft, mColorGreenTopLeft, mColorBlueTopLeft);
    builder->tex2(mAoBrightnessTopLeft);
    builder->vertexUV(x0, yy, z1, faceU[0], faceV[0]);
    builder->color(mColorRedBottomLeft, mColorGreenBottomLeft, mColorBlueBottomLeft);
    builder->tex2(mAoBrightnessBottomLeft);
    builder->vertexUV(x0, yy, z0, faceU[1], faceV[1]);
    builder->color(mColorRedBottomRight, mColorGreenBottomRight, mColorBlueBottomRight);
    builder->tex2(mAoBrightnessBottomRight);
    builder->vertexUV(x1, yy, z0, faceU[2], faceV[2]);
    builder->color(mColorRedTopRight, mColorGreenTopRight, mColorBlueTopRight);
    builder->tex2(mAoBrightnessTopRight);
    builder->vertexUV(x1, yy, z1, faceU[3], faceV[3]);
}

void BlockRenderer::renderFaceUp(const BlockState* state, double x, double y, double z, TextureAtlasSprite* texture)
{
    if (hasFixedTexture())
        texture = mFixedTexture;
    float u0 = (float)texture->getU(mMinX * 16.0f);
    float u1 = (float)texture->getU(mMaxX * 16.0f);
    float v0 = (float)texture->getV(mMinZ * 16.0f);
    float v1 = (float)texture->getV(mMaxZ * 16.0f);
    texture->adjustUVs(u0, u1, v0, v1);
    renderFaceUp(state, x, y, z, texture, u0, v0, u1, v1);
}

void BlockRenderer::renderFaceUp(const BlockState*, double x, double y, double z, TextureAtlasSprite* texture, float u0,
                                 float v0, float u1, float v1)
{
    BufferBuilder* builder = Tesselator::getInstance()->getBuilder();
    if (mMinX < 0.0f || 1.0f < mMaxX)
    {
        u0 = (float)texture->getU0(true);
        u1 = (float)texture->getU1(true);
    }
    if (mMinZ < 0.0f || 1.0f < mMaxZ)
    {
        v0 = (float)texture->getV0(true);
        v1 = (float)texture->getV1(true);
    }
    if (mFaceRotation4 == 1)
    {
        u0 = (float)texture->getU(mMinZ * 16.0f);
        v0 = (float)texture->getV(16.0f - mMaxX * 16.0f);
        u1 = (float)texture->getU(mMaxZ * 16.0f);
        v1 = (float)texture->getV(16.0f - mMinX * 16.0f);
        texture->adjustUVs(u0, u1, v0, v1);
    }
    else if (mFaceRotation4 == 2)
    {
        u0 = (float)texture->getU(16.0f - mMaxZ * 16.0f);
        v0 = (float)texture->getV(mMinX * 16.0f);
        u1 = (float)texture->getU(16.0f - mMinZ * 16.0f);
        v1 = (float)texture->getV(mMaxX * 16.0f);
        texture->adjustUVs(u0, u1, v0, v1);
    }
    else if (mFaceRotation4 == 3)
    {
        u0 = (float)texture->getU(16.0f - mMinX * 16.0f);
        u1 = (float)texture->getU(16.0f - mMaxX * 16.0f);
        v0 = (float)texture->getV(16.0f - mMinZ * 16.0f);
        v1 = (float)texture->getV(16.0f - mMaxZ * 16.0f);
        texture->adjustUVs(u0, u1, v0, v1);
    }
    if (mFaceTransform4 == 3)
    {
        float swapped = u0;
        u0 = u1;
        u1 = swapped;
        swapped = v0;
        v0 = v1;
        v1 = swapped;
    }
    else if (mFaceTransform4 == 2)
    {
        float swapped = u0;
        u0 = u1;
        u1 = swapped;
    }
    else if (mFaceTransform4 == 1)
    {
        float swapped = v0;
        v0 = v1;
        v1 = swapped;
    }
    float faceU[4];
    float faceV[4];
    if (mFaceRotation4 == 1 || mFaceRotation4 == 4)
    {
        faceU[0] = u1;
        faceV[0] = v0;
        faceU[1] = u0;
        faceV[1] = v0;
        faceU[2] = u0;
        faceV[2] = v1;
        faceU[3] = u1;
        faceV[3] = v1;
    }
    else if (mFaceRotation4 == 2 || mFaceRotation4 == 5)
    {
        faceU[0] = u0;
        faceV[0] = v1;
        faceU[1] = u1;
        faceV[1] = v1;
        faceU[2] = u1;
        faceV[2] = v0;
        faceU[3] = u0;
        faceV[3] = v0;
    }
    else
    {
        faceU[0] = u1;
        faceV[0] = v1;
        faceU[1] = u1;
        faceV[1] = v0;
        faceU[2] = u0;
        faceV[2] = v0;
        faceU[3] = u0;
        faceV[3] = v1;
    }
    float x0 = (float)(x + mMinX);
    float x1 = (float)(x + mMaxX);
    float yy = (float)(y + mMaxY);
    float z0 = (float)(z + mMinZ);
    float z1 = (float)(z + mMaxZ);
    builder->bucket(1);
    if (!mEnableAO)
    {
        builder->vertexUV(x1, yy, z1, faceU[0], faceV[0]);
        builder->vertexUV(x1, yy, z0, faceU[1], faceV[1]);
        builder->vertexUV(x0, yy, z0, faceU[2], faceV[2]);
        builder->vertexUV(x0, yy, z1, faceU[3], faceV[3]);
        return;
    }
    builder->color(mColorRedTopLeft, mColorGreenTopLeft, mColorBlueTopLeft);
    builder->tex2(mAoBrightnessTopLeft);
    builder->vertexUV(x1, yy, z1, faceU[0], faceV[0]);
    builder->color(mColorRedBottomLeft, mColorGreenBottomLeft, mColorBlueBottomLeft);
    builder->tex2(mAoBrightnessBottomLeft);
    builder->vertexUV(x1, yy, z0, faceU[1], faceV[1]);
    builder->color(mColorRedBottomRight, mColorGreenBottomRight, mColorBlueBottomRight);
    builder->tex2(mAoBrightnessBottomRight);
    builder->vertexUV(x0, yy, z0, faceU[2], faceV[2]);
    builder->color(mColorRedTopRight, mColorGreenTopRight, mColorBlueTopRight);
    builder->tex2(mAoBrightnessTopRight);
    builder->vertexUV(x0, yy, z1, faceU[3], faceV[3]);
}

void BlockRenderer::renderNorth(const BlockState* state, double x, double y, double z, TextureAtlasSprite* texture)
{
    if (hasFixedTexture())
        texture = mFixedTexture;
    float u0 = (float)texture->getU(mMinX * 16.0f);
    float u1 = (float)texture->getU(mMaxX * 16.0f);
    float v0 = (float)texture->getV(16.0f - mMaxY * 16.0f);
    float v1 = (float)texture->getV(16.0f - mMinY * 16.0f);
    texture->adjustUVs(u0, u1, v0, v1);
    renderNorth(state, x, y, z, texture, u0, v0, u1, v1);
}

void BlockRenderer::renderNorth(const BlockState*, double x, double y, double z, TextureAtlasSprite* texture, float u0,
                                float v0, float u1, float v1)
{
    BufferBuilder* builder = Tesselator::getInstance()->getBuilder();
    if (mFlipTexture)
    {
        float swapped = u0;
        u0 = u1;
        u1 = swapped;
    }
    if (mMinX < 0.0f || 1.0f < mMaxX)
    {
        u0 = (float)texture->getU0(true);
        u1 = (float)texture->getU1(true);
    }
    if (mMinY < 0.0f || 1.0f < mMaxY)
    {
        v0 = (float)texture->getV0(true);
        v1 = (float)texture->getV1(true);
    }
    if (mFaceRotation0 == 2)
    {
        u0 = (float)texture->getU(mMinY * 16.0f);
        v0 = (float)texture->getV(16.0f - mMinX * 16.0f);
        u1 = (float)texture->getU(mMaxY * 16.0f);
        v1 = (float)texture->getV(16.0f - mMaxX * 16.0f);
        texture->adjustUVs(u0, u1, v0, v1);
    }
    else if (mFaceRotation0 == 1)
    {
        u0 = (float)texture->getU(16.0f - mMaxY * 16.0f);
        v0 = (float)texture->getV(mMinX * 16.0f);
        u1 = (float)texture->getU(16.0f - mMinY * 16.0f);
        v1 = (float)texture->getV(mMaxX * 16.0f);
        texture->adjustUVs(u0, u1, v0, v1);
    }
    else if (mFaceRotation0 == 3)
    {
        u0 = (float)texture->getU(16.0f - mMinX * 16.0f);
        u1 = (float)texture->getU(16.0f - mMaxX * 16.0f);
        v0 = (float)texture->getV(mMaxY * 16.0f);
        v1 = (float)texture->getV(mMinY * 16.0f);
        texture->adjustUVs(u0, u1, v0, v1);
    }
    if (mFaceTransform0 == 3)
    {
        float swapped = u0;
        u0 = u1;
        u1 = swapped;
        swapped = v0;
        v0 = v1;
        v1 = swapped;
    }
    else if (mFaceTransform0 == 2)
    {
        float swapped = u0;
        u0 = u1;
        u1 = swapped;
    }
    else if (mFaceTransform0 == 1)
    {
        float swapped = v0;
        v0 = v1;
        v1 = swapped;
    }
    float faceU[4];
    float faceV[4];
    if (mFaceRotation0 == 2 || mFaceRotation0 == 5)
    {
        faceU[0] = u0;
        faceV[0] = v0;
        faceU[1] = u0;
        faceV[1] = v1;
        faceU[2] = u1;
        faceV[2] = v1;
        faceU[3] = u1;
        faceV[3] = v0;
    }
    else if (mFaceRotation0 == 1 || mFaceRotation0 == 4)
    {
        faceU[0] = u1;
        faceV[0] = v1;
        faceU[1] = u1;
        faceV[1] = v0;
        faceU[2] = u0;
        faceV[2] = v0;
        faceU[3] = u0;
        faceV[3] = v1;
    }
    else
    {
        faceU[0] = u1;
        faceV[0] = v0;
        faceU[1] = u0;
        faceV[1] = v0;
        faceU[2] = u0;
        faceV[2] = v1;
        faceU[3] = u1;
        faceV[3] = v1;
    }
    float x0 = (float)(x + mMinX);
    float x1 = (float)(x + mMaxX);
    float y0 = (float)(y + mMinY);
    float y1 = (float)(y + mMaxY);
    float zz = (float)(z + mMinZ);
    builder->bucket(2);
    if (!mEnableAO)
    {
        builder->vertexUV(x0, y1, zz, faceU[0], faceV[0]);
        builder->vertexUV(x1, y1, zz, faceU[1], faceV[1]);
        builder->vertexUV(x1, y0, zz, faceU[2], faceV[2]);
        builder->vertexUV(x0, y0, zz, faceU[3], faceV[3]);
        return;
    }
    builder->color(mColorRedTopLeft, mColorGreenTopLeft, mColorBlueTopLeft);
    builder->tex2(mAoBrightnessTopLeft);
    builder->vertexUV(x0, y1, zz, faceU[0], faceV[0]);
    builder->color(mColorRedBottomLeft, mColorGreenBottomLeft, mColorBlueBottomLeft);
    builder->tex2(mAoBrightnessBottomLeft);
    builder->vertexUV(x1, y1, zz, faceU[1], faceV[1]);
    builder->color(mColorRedBottomRight, mColorGreenBottomRight, mColorBlueBottomRight);
    builder->tex2(mAoBrightnessBottomRight);
    builder->vertexUV(x1, y0, zz, faceU[2], faceV[2]);
    builder->color(mColorRedTopRight, mColorGreenTopRight, mColorBlueTopRight);
    builder->tex2(mAoBrightnessTopRight);
    builder->vertexUV(x0, y0, zz, faceU[3], faceV[3]);
}

void BlockRenderer::renderSouth(const BlockState* state, double x, double y, double z, TextureAtlasSprite* texture)
{
    if (hasFixedTexture())
        texture = mFixedTexture;
    float u0 = (float)texture->getU(mMinX * 16.0f);
    float u1 = (float)texture->getU(mMaxX * 16.0f);
    float v0 = (float)texture->getV(16.0f - mMaxY * 16.0f);
    float v1 = (float)texture->getV(16.0f - mMinY * 16.0f);
    texture->adjustUVs(u0, u1, v0, v1);
    renderSouth(state, x, y, z, texture, u0, v0, u1, v1);
}

void BlockRenderer::renderSouth(const BlockState*, double x, double y, double z, TextureAtlasSprite* texture, float u0,
                                float v0, float u1, float v1)
{
    BufferBuilder* builder = Tesselator::getInstance()->getBuilder();
    if (mFlipTexture)
    {
        float swapped = u0;
        u0 = u1;
        u1 = swapped;
    }
    if (mMinX < 0.0f || mMaxX > 1.0f)
    {
        u0 = (float)texture->getU0(true);
        u1 = (float)texture->getU1(true);
    }
    if (mMinY < 0.0f || mMaxY > 1.0f)
    {
        v0 = (float)texture->getV0(true);
        v1 = (float)texture->getV1(true);
    }
    if (mFaceRotation1 == 1)
    {
        u0 = (float)texture->getU(mMinY * 16.0f);
        v1 = (float)texture->getV(16.0f - mMinX * 16.0f);
        u1 = (float)texture->getU(mMaxY * 16.0f);
        v0 = (float)texture->getV(16.0f - mMaxX * 16.0f);
        texture->adjustUVs(u0, u1, v0, v1);
    }
    else if (mFaceRotation1 == 2)
    {
        u0 = (float)texture->getU(16.0f - mMaxY * 16.0f);
        v0 = (float)texture->getV(mMinX * 16.0f);
        u1 = (float)texture->getU(16.0f - mMinY * 16.0f);
        v1 = (float)texture->getV(mMaxX * 16.0f);
        texture->adjustUVs(u0, u1, v0, v1);
    }
    else if (mFaceRotation1 == 3)
    {
        u0 = (float)texture->getU(16.0f - mMinX * 16.0f);
        u1 = (float)texture->getU(16.0f - mMaxX * 16.0f);
        v0 = (float)texture->getV(mMaxY * 16.0f);
        v1 = (float)texture->getV(mMinY * 16.0f);
        texture->adjustUVs(u0, u1, v0, v1);
    }
    if (mFaceTransform1 == 3)
    {
        float swapped = u0;
        u0 = u1;
        u1 = swapped;
        swapped = v0;
        v0 = v1;
        v1 = swapped;
    }
    else if (mFaceTransform1 == 2)
    {
        float swapped = u0;
        u0 = u1;
        u1 = swapped;
    }
    else if (mFaceTransform1 == 1)
    {
        float swapped = v0;
        v0 = v1;
        v1 = swapped;
    }
    float faceU[4];
    float faceV[4];
    if (mFaceRotation1 == 1 || mFaceRotation1 == 4)
    {
        faceU[0] = u0;
        faceV[0] = v1;
        faceU[1] = u1;
        faceV[1] = v1;
        faceU[2] = u1;
        faceV[2] = v0;
        faceU[3] = u0;
        faceV[3] = v0;
    }
    else if (mFaceRotation1 == 2 || mFaceRotation1 == 5)
    {
        faceU[0] = u1;
        faceV[0] = v0;
        faceU[1] = u0;
        faceV[1] = v0;
        faceU[2] = u0;
        faceV[2] = v1;
        faceU[3] = u1;
        faceV[3] = v1;
    }
    else
    {
        faceU[0] = u0;
        faceV[0] = v0;
        faceU[1] = u0;
        faceV[1] = v1;
        faceU[2] = u1;
        faceV[2] = v1;
        faceU[3] = u1;
        faceV[3] = v0;
    }
    float x0 = (float)(x + mMinX);
    float x1 = (float)(x + mMaxX);
    float y0 = (float)(y + mMinY);
    float y1 = (float)(y + mMaxY);
    float zz = (float)(z + mMaxZ);
    builder->bucket(4);
    if (!mEnableAO)
    {
        builder->vertexUV(x0, y1, zz, faceU[0], faceV[0]);
        builder->vertexUV(x0, y0, zz, faceU[1], faceV[1]);
        builder->vertexUV(x1, y0, zz, faceU[2], faceV[2]);
        builder->vertexUV(x1, y1, zz, faceU[3], faceV[3]);
        return;
    }
    builder->color(mColorRedTopLeft, mColorGreenTopLeft, mColorBlueTopLeft);
    builder->tex2(mAoBrightnessTopLeft);
    builder->vertexUV(x0, y1, zz, faceU[0], faceV[0]);
    builder->color(mColorRedBottomLeft, mColorGreenBottomLeft, mColorBlueBottomLeft);
    builder->tex2(mAoBrightnessBottomLeft);
    builder->vertexUV(x0, y0, zz, faceU[1], faceV[1]);
    builder->color(mColorRedBottomRight, mColorGreenBottomRight, mColorBlueBottomRight);
    builder->tex2(mAoBrightnessBottomRight);
    builder->vertexUV(x1, y0, zz, faceU[2], faceV[2]);
    builder->color(mColorRedTopRight, mColorGreenTopRight, mColorBlueTopRight);
    builder->tex2(mAoBrightnessTopRight);
    builder->vertexUV(x1, y1, zz, faceU[3], faceV[3]);
}

void BlockRenderer::renderWest(const BlockState* state, double x, double y, double z, TextureAtlasSprite* texture)
{
    if (hasFixedTexture())
        texture = mFixedTexture;
    float u0 = (float)texture->getU(mMinZ * 16.0f);
    float u1 = (float)texture->getU(mMaxZ * 16.0f);
    float v0 = (float)texture->getV(16.0f - mMaxY * 16.0f);
    float v1 = (float)texture->getV(16.0f - mMinY * 16.0f);
    texture->adjustUVs(u0, u1, v0, v1);
    renderWest(state, x, y, z, texture, u0, v0, u1, v1);
}

void BlockRenderer::renderWest(const BlockState*, double x, double y, double z, TextureAtlasSprite* texture, float u0,
                               float v0, float u1, float v1)
{
    BufferBuilder* builder = Tesselator::getInstance()->getBuilder();
    if (mFlipTexture)
    {
        float swapped = u0;
        u0 = u1;
        u1 = swapped;
    }
    if (mMinZ < 0.0f || mMaxZ > 1.0f)
    {
        u0 = (float)texture->getU0(true);
        u1 = (float)texture->getU1(true);
    }
    if (mMinY < 0.0f || mMaxY > 1.0f)
    {
        v0 = (float)texture->getV0(true);
        v1 = (float)texture->getV1(true);
    }
    if (mFaceRotation3 == 1)
    {
        u0 = (float)texture->getU(mMinY * 16.0f);
        v0 = (float)texture->getV(16.0f - mMaxZ * 16.0f);
        u1 = (float)texture->getU(mMaxY * 16.0f);
        v1 = (float)texture->getV(16.0f - mMinZ * 16.0f);
        texture->adjustUVs(u0, u1, v0, v1);
    }
    else if (mFaceRotation3 == 2)
    {
        u0 = (float)texture->getU(16.0f - mMaxY * 16.0f);
        v0 = (float)texture->getV(mMinZ * 16.0f);
        u1 = (float)texture->getU(16.0f - mMinY * 16.0f);
        v1 = (float)texture->getV(mMaxZ * 16.0f);
        texture->adjustUVs(u0, u1, v0, v1);
    }
    else if (mFaceRotation3 == 3)
    {
        u0 = (float)texture->getU(16.0f - mMinZ * 16.0f);
        u1 = (float)texture->getU(16.0f - mMaxZ * 16.0f);
        v0 = (float)texture->getV(mMaxY * 16.0f);
        v1 = (float)texture->getV(mMinY * 16.0f);
        texture->adjustUVs(u0, u1, v0, v1);
    }
    if (mFaceTransform3 == 3)
    {
        float swapped = u0;
        u0 = u1;
        u1 = swapped;
        swapped = v0;
        v0 = v1;
        v1 = swapped;
    }
    else if (mFaceTransform3 == 2)
    {
        float swapped = u0;
        u0 = u1;
        u1 = swapped;
    }
    else if (mFaceTransform3 == 1)
    {
        float swapped = v0;
        v0 = v1;
        v1 = swapped;
    }
    float faceU[4];
    float faceV[4];
    if (mFaceRotation3 == 1 || mFaceRotation3 == 4)
    {
        faceU[0] = u0;
        faceV[0] = v0;
        faceU[1] = u0;
        faceV[1] = v1;
        faceU[2] = u1;
        faceV[2] = v1;
        faceU[3] = u1;
        faceV[3] = v0;
    }
    else if (mFaceRotation3 == 2 || mFaceRotation3 == 5)
    {
        faceU[0] = u1;
        faceV[0] = v1;
        faceU[1] = u1;
        faceV[1] = v0;
        faceU[2] = u0;
        faceV[2] = v0;
        faceU[3] = u0;
        faceV[3] = v1;
    }
    else
    {
        faceU[0] = u1;
        faceV[0] = v0;
        faceU[1] = u0;
        faceV[1] = v0;
        faceU[2] = u0;
        faceV[2] = v1;
        faceU[3] = u1;
        faceV[3] = v1;
    }
    float xx = (float)(x + mMinX);
    float y0 = (float)(y + mMinY);
    float y1 = (float)(y + mMaxY);
    float z0 = (float)(z + mMinZ);
    float z1 = (float)(z + mMaxZ);
    builder->bucket(5);
    if (!mEnableAO)
    {
        builder->vertexUV(xx, y1, z1, faceU[0], faceV[0]);
        builder->vertexUV(xx, y1, z0, faceU[1], faceV[1]);
        builder->vertexUV(xx, y0, z0, faceU[2], faceV[2]);
        builder->vertexUV(xx, y0, z1, faceU[3], faceV[3]);
        return;
    }
    builder->color(mColorRedTopLeft, mColorGreenTopLeft, mColorBlueTopLeft);
    builder->tex2(mAoBrightnessTopLeft);
    builder->vertexUV(xx, y1, z1, faceU[0], faceV[0]);
    builder->color(mColorRedBottomLeft, mColorGreenBottomLeft, mColorBlueBottomLeft);
    builder->tex2(mAoBrightnessBottomLeft);
    builder->vertexUV(xx, y1, z0, faceU[1], faceV[1]);
    builder->color(mColorRedBottomRight, mColorGreenBottomRight, mColorBlueBottomRight);
    builder->tex2(mAoBrightnessBottomRight);
    builder->vertexUV(xx, y0, z0, faceU[2], faceV[2]);
    builder->color(mColorRedTopRight, mColorGreenTopRight, mColorBlueTopRight);
    builder->tex2(mAoBrightnessTopRight);
    builder->vertexUV(xx, y0, z1, faceU[3], faceV[3]);
}

void BlockRenderer::renderEast(const BlockState* state, double x, double y, double z, TextureAtlasSprite* texture)
{
    if (hasFixedTexture())
        texture = mFixedTexture;
    float u0 = (float)texture->getU(mMinZ * 16.0f);
    float u1 = (float)texture->getU(mMaxZ * 16.0f);
    float v0 = (float)texture->getV(16.0f - mMaxY * 16.0f);
    float v1 = (float)texture->getV(16.0f - mMinY * 16.0f);
    texture->adjustUVs(u0, u1, v0, v1);
    renderEast(state, x, y, z, texture, u0, v0, u1, v1);
}

void BlockRenderer::renderEast(const BlockState*, double x, double y, double z, TextureAtlasSprite* texture, float u0,
                               float v0, float u1, float v1)
{
    BufferBuilder* builder = Tesselator::getInstance()->getBuilder();
    if (mFlipTexture)
    {
        float swapped = u0;
        u0 = u1;
        u1 = swapped;
    }
    if (mMinZ < 0.0f || 1.0f < mMaxZ)
    {
        u0 = (float)texture->getU0(true);
        u1 = (float)texture->getU1(true);
    }
    if (mMinY < 0.0f || 1.0f < mMaxY)
    {
        v0 = (float)texture->getV0(true);
        v1 = (float)texture->getV1(true);
    }
    if (mFaceRotation2 == 2)
    {
        u0 = (float)texture->getU(mMinY * 16.0f);
        v0 = (float)texture->getV(16.0f - mMinZ * 16.0f);
        u1 = (float)texture->getU(mMaxY * 16.0f);
        v1 = (float)texture->getV(16.0f - mMaxZ * 16.0f);
        texture->adjustUVs(u0, u1, v0, v1);
    }
    else if (mFaceRotation2 == 1)
    {
        u0 = (float)texture->getU(16.0f - mMaxY * 16.0f);
        v0 = (float)texture->getV(mMaxZ * 16.0f);
        u1 = (float)texture->getU(16.0f - mMinY * 16.0f);
        v1 = (float)texture->getV(mMinZ * 16.0f);
        texture->adjustUVs(u0, u1, v0, v1);
    }
    else if (mFaceRotation2 == 3)
    {
        u0 = (float)texture->getU(16.0f - mMinZ * 16.0f);
        u1 = (float)texture->getU(16.0f - mMaxZ * 16.0f);
        v0 = (float)texture->getV(mMaxY * 16.0f);
        v1 = (float)texture->getV(mMinY * 16.0f);
        texture->adjustUVs(u0, u1, v0, v1);
    }
    if (mFaceTransform2 == 3)
    {
        float swapped = u0;
        u0 = u1;
        u1 = swapped;
        swapped = v0;
        v0 = v1;
        v1 = swapped;
    }
    else if (mFaceTransform2 == 2)
    {
        float swapped = u0;
        u0 = u1;
        u1 = swapped;
    }
    else if (mFaceTransform2 == 1)
    {
        float swapped = v0;
        v0 = v1;
        v1 = swapped;
    }
    float faceU[4];
    float faceV[4];
    if (mFaceRotation2 == 2 || mFaceRotation2 == 5)
    {
        faceU[0] = u1;
        faceV[0] = v1;
        faceU[1] = u1;
        faceV[1] = v0;
        faceU[2] = u0;
        faceV[2] = v0;
        faceU[3] = u0;
        faceV[3] = v1;
    }
    else if (mFaceRotation2 == 1 || mFaceRotation2 == 4)
    {
        faceU[0] = u0;
        faceV[0] = v0;
        faceU[1] = u0;
        faceV[1] = v1;
        faceU[2] = u1;
        faceV[2] = v1;
        faceU[3] = u1;
        faceV[3] = v0;
    }
    else
    {
        faceU[0] = u0;
        faceV[0] = v1;
        faceU[1] = u1;
        faceV[1] = v1;
        faceU[2] = u1;
        faceV[2] = v0;
        faceU[3] = u0;
        faceV[3] = v0;
    }
    float xx = (float)(x + mMaxX);
    float y0 = (float)(y + mMinY);
    float y1 = (float)(y + mMaxY);
    float z0 = (float)(z + mMinZ);
    float z1 = (float)(z + mMaxZ);
    builder->bucket(3);
    if (!mEnableAO)
    {
        builder->vertexUV(xx, y0, z1, faceU[0], faceV[0]);
        builder->vertexUV(xx, y0, z0, faceU[1], faceV[1]);
        builder->vertexUV(xx, y1, z0, faceU[2], faceV[2]);
        builder->vertexUV(xx, y1, z1, faceU[3], faceV[3]);
        return;
    }
    builder->color(mColorRedTopLeft, mColorGreenTopLeft, mColorBlueTopLeft);
    builder->tex2(mAoBrightnessTopLeft);
    builder->vertexUV(xx, y0, z1, faceU[0], faceV[0]);
    builder->color(mColorRedBottomLeft, mColorGreenBottomLeft, mColorBlueBottomLeft);
    builder->tex2(mAoBrightnessBottomLeft);
    builder->vertexUV(xx, y0, z0, faceU[1], faceV[1]);
    builder->color(mColorRedBottomRight, mColorGreenBottomRight, mColorBlueBottomRight);
    builder->tex2(mAoBrightnessBottomRight);
    builder->vertexUV(xx, y1, z0, faceU[2], faceV[2]);
    builder->color(mColorRedTopRight, mColorGreenTopRight, mColorBlueTopRight);
    builder->tex2(mAoBrightnessTopRight);
    builder->vertexUV(xx, y1, z1, faceU[3], faceV[3]);
}

void BlockRenderer::tesselateCrossTexture(TextureAtlasSprite* texture, TextureAtlasSprite* overlay, float x, float y,
                                          float z, float scale)
{
    BufferBuilder* builder = Tesselator::getInstance()->getBuilder();
    float u0 = (float)texture->getU0(true);
    float v0 = (float)texture->getV0(true);
    float u1 = (float)texture->getU1(true);
    float v1 = (float)texture->getV1(true);
    float overlayU0 = (float)overlay->getU0(true);
    float overlayV0 = (float)overlay->getV0(true);
    float overlayU1 = (float)overlay->getU1(true);
    float overlayV1 = (float)overlay->getV1(true);
    float y1 = y + scale;
    float radius = 0.45 * scale;
    float x0 = x + 0.5 - radius;
    float x1 = x + 0.5 + radius;
    float z0 = z + 0.5 - radius;
    float z1 = z + 0.5 + radius;
    builder->vertexUV(x0, y1, z0, u0, v0);
    builder->vertexUV(x0, y, z0, u0, v1);
    builder->vertexUV(x1, y, z1, u1, v1);
    builder->vertexUV(x1, y1, z1, u1, v0);
    builder->vertexUV(x1, y1, z1, u0, v0);
    builder->vertexUV(x1, y, z1, u0, v1);
    builder->vertexUV(x0, y, z0, u1, v1);
    builder->vertexUV(x0, y1, z0, u1, v0);
    builder->vertexUV(x0, y1, z1, overlayU0, overlayV0);
    builder->vertexUV(x0, y, z1, overlayU0, overlayV1);
    builder->vertexUV(x1, y, z0, overlayU1, overlayV1);
    builder->vertexUV(x1, y1, z0, overlayU1, overlayV0);
    builder->vertexUV(x1, y1, z0, overlayU0, overlayV0);
    builder->vertexUV(x1, y, z0, overlayU0, overlayV1);
    builder->vertexUV(x0, y, z1, overlayU1, overlayV1);
    builder->vertexUV(x0, y1, z1, overlayU1, overlayV0);
}

void BlockRenderer::tesselateCrossTexture(const BlockState* state, float x, float y, float z, float scale)
{
    TextureAtlasSprite* texture = getTexture(state, Direction::UP);
    if (hasFixedTexture())
        texture = mFixedTexture;
    tesselateCrossTexture(texture, texture, x, y, z, scale);
}

void BlockRenderer::tesselateStemTexture(const BlockState* state, float height, float x, float y, float z)
{
    BufferBuilder* builder = Tesselator::getInstance()->getBuilder();
    TextureAtlasSprite* texture = getTexture(state, Direction::UP);
    if (hasFixedTexture())
        texture = mFixedTexture;
    float u0 = (float)texture->getU0(false);
    float v0 = (float)texture->getV0(false);
    float u1 = (float)texture->getU1(false);
    float v1 = (float)texture->getV(height * 16.0f);
    texture->adjustUVs(u0, u1, v0, v1);
    float y1 = y + height;
    float x0 = x + 0.050000012f;
    float x1 = x + 0.95f;
    float z0 = z + 0.050000012f;
    float z1 = z + 0.95f;
    builder->vertexUV(x0, y1, z0, u0, v0);
    builder->vertexUV(x0, y, z0, u0, v1);
    builder->vertexUV(x1, y, z1, u1, v1);
    builder->vertexUV(x1, y1, z1, u1, v0);
    builder->vertexUV(x1, y1, z1, u0, v0);
    builder->vertexUV(x1, y, z1, u0, v1);
    builder->vertexUV(x0, y, z0, u1, v1);
    builder->vertexUV(x0, y1, z0, u1, v0);
    builder->vertexUV(x0, y1, z1, u0, v0);
    builder->vertexUV(x0, y, z1, u0, v1);
    builder->vertexUV(x1, y, z0, u1, v1);
    builder->vertexUV(x1, y1, z0, u1, v0);
    builder->vertexUV(x1, y1, z0, u0, v0);
    builder->vertexUV(x1, y, z0, u0, v1);
    builder->vertexUV(x0, y, z1, u1, v1);
    builder->vertexUV(x0, y1, z1, u1, v0);
}

void BlockRenderer::fjRenderCubes(const BlockState* state, const BoxDesc* boxes, unsigned int boxCount,
                                  const BoxUV* uvs)
{
    BufferBuilder* builder = Tesselator::getInstance()->getBuilder();
    GlStateManager::pushMatrix();
    GlStateManager::translatef(-0.5f, -0.5f, -0.5f);
    for (unsigned int i = 0; i < boxCount; ++i)
    {
        BoxDesc box = boxes[i];
        setShape(box.x0, box.y0, box.z0, box.x1, box.y1, box.z1);
        const BoxUV* uv = uvs == NULL ? NULL : &uvs[i];
        if ((box.hiddenFaces & 1) == 0)
        {
            builder->begin();
            builder->normal(0.0f, -1.0f, 0.0f);
            TextureAtlasSprite* texture = getTexture(state, Direction::DOWN);
            if (uv == NULL)
                renderFaceDown(state, 0.0, 0.0, 0.0, texture);
            else
                renderFaceDown(state, 0.0, 0.0, 0.0, texture, uv->down[0], uv->down[2], uv->down[1], uv->down[3]);
            builder->end();
        }
        if ((box.hiddenFaces & 2) == 0)
        {
            builder->begin();
            builder->normal(0.0f, 1.0f, 0.0f);
            TextureAtlasSprite* texture = getTexture(state, Direction::UP);
            if (uv == NULL)
                renderFaceUp(state, 0.0, 0.0, 0.0, texture);
            else
                renderFaceUp(state, 0.0, 0.0, 0.0, texture, uv->up[0], uv->up[2], uv->up[1], uv->up[3]);
            builder->end();
        }
        if ((box.hiddenFaces & 4) == 0)
        {
            builder->begin();
            builder->normal(0.0f, 0.0f, -1.0f);
            TextureAtlasSprite* texture = getTexture(state, Direction::NORTH);
            if (uv == NULL)
                renderNorth(state, 0.0, 0.0, 0.0, texture);
            else
                renderNorth(state, 0.0, 0.0, 0.0, texture, uv->north[0], uv->north[2], uv->north[1], uv->north[3]);
            builder->end();
        }
        if ((box.hiddenFaces & 8) == 0)
        {
            builder->begin();
            builder->normal(0.0f, 0.0f, 1.0f);
            TextureAtlasSprite* texture = getTexture(state, Direction::SOUTH);
            if (uv == NULL)
                renderSouth(state, 0.0, 0.0, 0.0, texture);
            else
                renderSouth(state, 0.0, 0.0, 0.0, texture, uv->south[0], uv->south[2], uv->south[1], uv->south[3]);
            builder->end();
        }
        if ((box.hiddenFaces & 0x20) == 0)
        {
            builder->begin();
            builder->normal(-1.0f, 0.0f, 0.0f);
            TextureAtlasSprite* texture = getTexture(state, Direction::WEST);
            if (uv == NULL)
                renderWest(state, 0.0, 0.0, 0.0, texture);
            else
                renderWest(state, 0.0, 0.0, 0.0, texture, uv->west[0], uv->west[2], uv->west[1], uv->west[3]);
            builder->end();
        }
        if ((box.hiddenFaces & 0x10) == 0)
        {
            builder->begin();
            builder->normal(1.0f, 0.0f, 0.0f);
            TextureAtlasSprite* texture = getTexture(state, Direction::EAST);
            if (uv == NULL)
                renderEast(state, 0.0, 0.0, 0.0, texture);
            else
                renderEast(state, 0.0, 0.0, 0.0, texture, uv->east[0], uv->east[2], uv->east[1], uv->east[3]);
            builder->end();
        }
    }
    GlStateManager::popMatrix();
    setShape(0.0f, 0.0f, 0.0f, 1.0f, 1.0f, 1.0f);
}

int BlockRenderer::getLightColor(Block* block, LevelSource* levelSource, const BlockPos& pos)
{
    if (mCache != NULL)
    {
        unsigned int index = (pos.x - mCacheMinX) * 0x400 + (pos.y - mCacheMinY) * 0x20 + (pos.z - mCacheMinZ);
        if ((index & 0xffff8000) == 0 && block != Blocks::water && block != Blocks::flowing_water
            && block != Blocks::flowing_lava && block != Blocks::lava && block != Blocks::magma
            && block != Blocks::blue_ice)
        {
            unsigned int cached = mCache[index];
            if ((cached & 0x80000000) != 0)
                return cached & 0xf000f0;
            int lightColor = levelSource->getBlockState(pos)->getLightColor(levelSource, pos);
            mCache[index] |= (unsigned int)(lightColor & 0xf000f0) | 0x80000000;
            return lightColor;
        }
    }
    return levelSource->getBlockState(pos)->getLightColor(levelSource, pos);
}

bool BlockRenderer::isSolidBlockingAt(LevelSource* levelSource, const BlockPos& pos)
{
    if (mCache != NULL)
    {
        unsigned int index = (pos.x - mCacheMinX) * 0x400 + (pos.y - mCacheMinY) * 0x20 + (pos.z - mCacheMinZ);
        if ((index & 0xffff8000) == 0)
        {
            unsigned int cached = mCache[index];
            if ((cached & 0x40000000) != 0)
                return (cached & 1) != 0;
            bool solid = levelSource->getBlockState(pos)->isSolidBlockingCube()
                         && !levelSource->getBlockState(pos)->isTransparent();
            mCache[index] |= solid ? 0x40000001 : 0x40000000;
            return solid;
        }
    }
    return levelSource->getBlockState(pos)->isSolidBlockingCube()
           && !levelSource->getBlockState(pos)->isTransparent();
}

float BlockRenderer::getShadeBrightness(LevelSource* levelSource, const BlockPos& pos)
{
    return isSolidBlockingAt(levelSource, pos) ? 0.2f : 1.0f;
}

bool BlockRenderer::tesselateBlockInWorld(const BlockState* state, const BlockPos& pos, float red, float green,
                                          float blue)
{
    bool rendered = false;
    mEnableAO = false;
    BufferBuilder* builder = Tesselator::getInstance()->getBuilder();
    float downShade = 0.5f;
    float upShade = 1.0f;
    float northShade = 0.8f;
    float westShade = 0.6f;
    float upRed = upShade * red;
    float upGreen = upShade * green;
    float upBlue = upShade * blue;
    float downRed = downShade;
    float downGreen = downShade;
    float downBlue = downShade;
    float northRed = northShade;
    float northGreen = northShade;
    float northBlue = northShade;
    float westRed = westShade;
    float westGreen = westShade;
    float westBlue = westShade;
    Block* block = state->getBlock();
    if (block != Blocks::grass)
    {
        downRed = downShade * red;
        downGreen = downShade * green;
        downBlue = downShade * blue;
        northRed = northShade * red;
        northGreen = northShade * green;
        northBlue = northShade * blue;
        westRed = westShade * red;
        westGreen = westShade * green;
        westBlue = westShade * blue;
    }
    int blockLight = hasLightOverride() ? mLightOverride : getLightColor(block, mLevelSource, pos);

    if (mRenderAllFaces || block->shouldRenderFace(state, mLevelSource, pos, Direction::DOWN))
    {
        int light = mMinY > 0.0f ? blockLight : getLightColor(block, mLevelSource, pos.offset(0, -1, 0));
        if (hasLightOverride())
            light = mLightOverride;
        builder->tex2(light);
        builder->color(downRed, downGreen, downBlue);
        renderFaceDown(state, pos.x, pos.y, pos.z, getTexture(state, mLevelSource, pos, Direction::DOWN));
        rendered = true;
    }
    if (mRenderAllFaces || block->shouldRenderFace(state, mLevelSource, pos, Direction::UP))
    {
        int light = mMaxY < 1.0f ? blockLight : getLightColor(block, mLevelSource, pos.offset(0, 1, 0));
        if (hasLightOverride())
            light = mLightOverride;
        builder->tex2(light);
        builder->color(upRed, upGreen, upBlue);
        renderFaceUp(state, pos.x, pos.y, pos.z, getTexture(state, mLevelSource, pos, Direction::UP));
        rendered = true;
    }
    if (mRenderAllFaces || block->shouldRenderFace(state, mLevelSource, pos, Direction::NORTH))
    {
        int light = mMinZ > 0.0f ? blockLight : getLightColor(block, mLevelSource, pos.offset(0, 0, -1));
        if (hasLightOverride())
            light = mLightOverride;
        builder->tex2(light);
        builder->color(northRed, northGreen, northBlue);
        TextureAtlasSprite* texture = getTexture(state, mLevelSource, pos, Direction::NORTH);
        renderNorth(state, pos.x, pos.y, pos.z, texture);
        if (gFancyGraphics && texture->getFlags() == 1 && !hasFixedTexture())
        {
            builder->color(northRed * red, northGreen * green, northBlue * blue);
            renderNorth(state, pos.x, pos.y, pos.z, GrassBlock::getSideTextureOverlay());
        }
        rendered = true;
    }
    if (mRenderAllFaces || block->shouldRenderFace(state, mLevelSource, pos, Direction::SOUTH))
    {
        int light = mMaxZ < 1.0f ? blockLight : getLightColor(block, mLevelSource, pos.offset(0, 0, 1));
        if (hasLightOverride())
            light = mLightOverride;
        builder->tex2(light);
        builder->color(northRed, northGreen, northBlue);
        TextureAtlasSprite* texture = getTexture(state, mLevelSource, pos, Direction::SOUTH);
        renderSouth(state, pos.x, pos.y, pos.z, texture);
        if (gFancyGraphics && texture->getFlags() == 1 && !hasFixedTexture())
        {
            builder->color(northRed * red, northGreen * green, northBlue * blue);
            renderSouth(state, pos.x, pos.y, pos.z, GrassBlock::getSideTextureOverlay());
        }
        rendered = true;
    }
    if (mRenderAllFaces || block->shouldRenderFace(state, mLevelSource, pos, Direction::WEST))
    {
        int light = mMinX > 0.0f ? blockLight : getLightColor(block, mLevelSource, pos.offset(-1, 0, 0));
        if (hasLightOverride())
            light = mLightOverride;
        builder->tex2(light);
        builder->color(westRed, westGreen, westBlue);
        TextureAtlasSprite* texture = getTexture(state, mLevelSource, pos, Direction::WEST);
        renderWest(state, pos.x, pos.y, pos.z, texture);
        if (gFancyGraphics && texture->getFlags() == 1 && !hasFixedTexture())
        {
            builder->color(westRed * red, westGreen * green, westBlue * blue);
            renderWest(state, pos.x, pos.y, pos.z, GrassBlock::getSideTextureOverlay());
        }
        rendered = true;
    }
    if (mRenderAllFaces || block->shouldRenderFace(state, mLevelSource, pos, Direction::EAST))
    {
        int light = mMaxX < 1.0f ? blockLight : getLightColor(block, mLevelSource, pos.offset(1, 0, 0));
        if (hasLightOverride())
            light = mLightOverride;
        builder->tex2(light);
        builder->color(westRed, westGreen, westBlue);
        TextureAtlasSprite* texture = getTexture(state, mLevelSource, pos, Direction::EAST);
        renderEast(state, pos.x, pos.y, pos.z, texture);
        if (gFancyGraphics && texture->getFlags() == 1 && !hasFixedTexture())
        {
            builder->color(westRed * red, westGreen * green, westBlue * blue);
            renderEast(state, pos.x, pos.y, pos.z, GrassBlock::getSideTextureOverlay());
        }
        rendered = true;
    }
    return rendered;
}

bool BlockRenderer::tesselateBlockInWorldWithAmbienceOcclusionTexLighting(const BlockState* state, const BlockPos& pos,
                                                                          float red, float green, float blue,
                                                                          int faceFlags, bool partialBounds)
{
    Block* block = state->getBlock();
    if (faceFlags == 0)
    {
        if (mRenderAllFaces)
            faceFlags = 0x3f;
        else
        {
            if (block->shouldRenderFace(state, mLevelSource, pos, Direction::DOWN))
                faceFlags |= 1;
            if (block->shouldRenderFace(state, mLevelSource, pos, Direction::UP))
                faceFlags |= 2;
            if (block->shouldRenderFace(state, mLevelSource, pos, Direction::NORTH))
                faceFlags |= 4;
            if (block->shouldRenderFace(state, mLevelSource, pos, Direction::SOUTH))
                faceFlags |= 8;
            if (block->shouldRenderFace(state, mLevelSource, pos, Direction::WEST))
                faceFlags |= 0x10;
            if (block->shouldRenderFace(state, mLevelSource, pos, Direction::EAST))
                faceFlags |= 0x20;
        }
    }
    if (faceFlags == 0 || (faceFlags == 1 && pos.y == 0))
        return false;

    TextureAtlasSprite* leafTexture = NULL;
    if (block == Blocks::leaves || block == Blocks::leaves2)
    {
        leafTexture = getTexture(state, mLevelSource, pos, NULL);
    }

    mEnableAO = true;
    int lightColor = getLightColor(block, mLevelSource, pos);
    BufferBuilder* builder = Tesselator::getInstance()->getBuilder();
    builder->tex2(0xf000f);

    bool useColor = true;
    if (leafTexture == NULL ? getTexture(block, Direction::UP)->getFlags() == 2 : hasFixedTexture())
    {
        useColor = false;
    }

    if ((faceFlags & 1) != 0)
    {
        AOCalculate(block, mLevelSource, pos, Direction::eDirection_Down, mMinY <= 0.0f, lightColor, useColor, red,
                    green, blue, false);
        renderFaceDown(state, pos.x, pos.y, pos.z,
                       (leafTexture != NULL ? leafTexture : getTexture(state, mLevelSource, pos, Direction::DOWN)));
    }
    if ((faceFlags & 2) != 0)
    {
        AOCalculate(block, mLevelSource, pos, Direction::eDirection_Up, mMaxY >= 1.0f, lightColor, true, red, green,
                    blue, false);
        renderFaceUp(state, pos.x, pos.y, pos.z,
                     (leafTexture != NULL ? leafTexture : getTexture(state, mLevelSource, pos, Direction::UP)));
    }
    if ((faceFlags & 4) != 0)
    {
        AOCalculate(block, mLevelSource, pos, Direction::eDirection_North, mMinZ <= 0.0f, lightColor, useColor, red,
                    green, blue, partialBounds);
        TextureAtlasSprite* texture = (leafTexture != NULL ? leafTexture
                                                           : getTexture(state, mLevelSource, pos, Direction::NORTH));
        renderNorth(state, pos.x, pos.y, pos.z, texture);
        if (gFancyGraphics && texture->getFlags() == 1 && !hasFixedTexture())
        {
            mColorRedTopLeft *= red;
            mColorRedBottomLeft *= red;
            mColorRedBottomRight *= red;
            mColorRedTopRight *= red;
            mColorGreenTopLeft *= green;
            mColorGreenBottomLeft *= green;
            mColorGreenBottomRight *= green;
            mColorGreenTopRight *= green;
            mColorBlueTopLeft *= blue;
            mColorBlueBottomLeft *= blue;
            mColorBlueBottomRight *= blue;
            mColorBlueTopRight *= blue;
            bool mipmap = builder->setMipmapEnable(false);
            renderNorth(state, pos.x, pos.y, pos.z, GrassBlock::getSideTextureOverlay());
            builder->setMipmapEnable(mipmap);
        }
    }
    if ((faceFlags & 8) != 0)
    {
        AOCalculate(block, mLevelSource, pos, Direction::eDirection_South, mMaxZ >= 1.0f, lightColor, useColor, red,
                    green, blue, partialBounds);
        TextureAtlasSprite* texture = (leafTexture != NULL ? leafTexture
                                                           : getTexture(state, mLevelSource, pos, Direction::SOUTH));
        renderSouth(state, pos.x, pos.y, pos.z, texture);
        if (gFancyGraphics && texture->getFlags() == 1 && !hasFixedTexture())
        {
            mColorRedTopLeft *= red;
            mColorRedBottomLeft *= red;
            mColorRedBottomRight *= red;
            mColorRedTopRight *= red;
            mColorGreenTopLeft *= green;
            mColorGreenBottomLeft *= green;
            mColorGreenBottomRight *= green;
            mColorGreenTopRight *= green;
            mColorBlueTopLeft *= blue;
            mColorBlueBottomLeft *= blue;
            mColorBlueBottomRight *= blue;
            mColorBlueTopRight *= blue;
            bool mipmap = builder->setMipmapEnable(false);
            renderSouth(state, pos.x, pos.y, pos.z, GrassBlock::getSideTextureOverlay());
            builder->setMipmapEnable(mipmap);
        }
    }
    if ((faceFlags & 0x10) != 0)
    {
        AOCalculate(block, mLevelSource, pos, Direction::eDirection_West, mMinX <= 0.0f, lightColor, useColor, red,
                    green, blue, partialBounds);
        TextureAtlasSprite* texture = (leafTexture != NULL ? leafTexture
                                                           : getTexture(state, mLevelSource, pos, Direction::WEST));
        renderWest(state, pos.x, pos.y, pos.z, texture);
        if (gFancyGraphics && texture->getFlags() == 1 && !hasFixedTexture())
        {
            mColorRedTopLeft *= red;
            mColorRedBottomLeft *= red;
            mColorRedBottomRight *= red;
            mColorRedTopRight *= red;
            mColorGreenTopLeft *= green;
            mColorGreenBottomLeft *= green;
            mColorGreenBottomRight *= green;
            mColorGreenTopRight *= green;
            mColorBlueTopLeft *= blue;
            mColorBlueBottomLeft *= blue;
            mColorBlueBottomRight *= blue;
            mColorBlueTopRight *= blue;
            bool mipmap = builder->setMipmapEnable(false);
            renderWest(state, pos.x, pos.y, pos.z, GrassBlock::getSideTextureOverlay());
            builder->setMipmapEnable(mipmap);
        }
    }
    if ((faceFlags & 0x20) != 0)
    {
        AOCalculate(block, mLevelSource, pos, Direction::eDirection_East, mMaxX >= 1.0f, lightColor, useColor, red,
                    green, blue, partialBounds);
        TextureAtlasSprite* texture = (leafTexture != NULL ? leafTexture
                                                           : getTexture(state, mLevelSource, pos, Direction::EAST));
        renderEast(state, pos.x, pos.y, pos.z, texture);
        if (gFancyGraphics && texture->getFlags() == 1 && !hasFixedTexture())
        {
            mColorRedTopLeft *= red;
            mColorRedBottomLeft *= red;
            mColorRedBottomRight *= red;
            mColorRedTopRight *= red;
            mColorGreenTopLeft *= green;
            mColorGreenBottomLeft *= green;
            mColorGreenBottomRight *= green;
            mColorGreenTopRight *= green;
            mColorBlueTopLeft *= blue;
            mColorBlueBottomLeft *= blue;
            mColorBlueBottomRight *= blue;
            mColorBlueTopRight *= blue;
            bool mipmap = builder->setMipmapEnable(false);
            renderEast(state, pos.x, pos.y, pos.z, GrassBlock::getSideTextureOverlay());
            builder->setMipmapEnable(mipmap);
        }
    }

    mEnableAO = false;
    return true;
}

bool BlockRenderer::tesselateInWorld(const BlockState* state, const BlockPos& pos, const BlockState* actualState,
                                     boost::shared_ptr<BlockEntity> blockEntity)
{
    BufferBuilder* builder = Tesselator::getInstance()->getBuilder();
    Block* block = state->getBlock();
    state = block->fillVirtualBlockStateProperties(state, mLevelSource, pos);
    int renderShape = state->getRenderShape();

    if (renderShape != BLOCK_RENDER_MODEL)
        setShape(block->getShape(state, mLevelSource, pos));

    bool oldMipmap = builder->setMipmapEnable(block->mMipmap);
    bool rendered = false;

    switch (renderShape)
    {
    case BLOCK_RENDER_MODEL:
    {
        int faces = block->getFaceFlags(state, mLevelSource, pos, mRenderAllFaces);
        if (faces == 0)
        {
            rendered = false;
        }
        else
        {
            setShape(block->getShape(state, mLevelSource, pos));
            rendered = tesselateBlockInWorld(state, pos, faces);
        }
        break;
    }
    case BLOCK_RENDER_TRAPDOOR:
        rendered = tesselateTrapDoorInWorld(state, pos);
        break;
    case BLOCK_RENDER_SLIME_BLOCK:
        rendered = tesselateSlimeBlockInWorld(state, pos);
        break;
    case BLOCK_RENDER_TREE:
        rendered = tesselateTreeInWorld(state, pos);
        break;
    case BLOCK_RENDER_QUARTZ:
        rendered = tesselateQuartzInWorld(state, pos);
        break;
    case BLOCK_RENDER_HORIZONTAL_DIRECTIONAL:
        rendered = tesselateHorizontalDirectionalInWorld(state, pos);
        break;
    case BLOCK_RENDER_WATER:
        rendered = tesselateWaterInWorld(state, pos);
        break;
    case BLOCK_RENDER_CACTUS:
        rendered = tesselateCactusInWorld(state, pos);
        break;
    case BLOCK_RENDER_CROSS:
        rendered = tesselateCrossInWorld(state, pos);
        break;
    case BLOCK_RENDER_DOUBLE_PLANT:
        rendered = tesselateDoublePlantInWorld(state, pos);
        break;
    case BLOCK_RENDER_KELP:
        rendered = tesselateKelpInWorld(state, pos);
        break;
    case BLOCK_RENDER_CORAL_FAN:
        if (block == Blocks::coral_fan || block == Blocks::coral_fan_dead)
            rendered = tessellateCoralFanInWorld(state, pos);
        else
            rendered = tessellateCoralFanHangInWorld(state, pos);
        break;
    case BLOCK_RENDER_STEM:
        rendered = tesselateStemInWorld(state, pos);
        break;
    case BLOCK_RENDER_LILYPAD:
        rendered = tesselateLilypadInWorld(state, pos);
        break;
    case BLOCK_RENDER_ROW:
        rendered = tesselateRowInWorld(state, pos);
        break;
    case BLOCK_RENDER_TORCH:
        rendered = tesselateTorchInWorld(state, pos);
        break;
    case BLOCK_RENDER_FIRE:
        rendered = tesselateFireInWorld(state, pos);
        break;
    case BLOCK_RENDER_DUST:
        rendered = tesselateDustInWorld(state, pos);
        break;
    case BLOCK_RENDER_LADDER:
        rendered = tesselateLadderInWorld(state, pos);
        break;
    case BLOCK_RENDER_DOOR:
        rendered = tesselateDoorInWorld(state, pos);
        break;
    case BLOCK_RENDER_RAIL:
        rendered = tesselateRailInWorld(state, pos);
        break;
    case BLOCK_RENDER_STAIRS:
        rendered = tesselateStairsInWorld(state, pos);
        break;
    case BLOCK_RENDER_EGG:
        rendered = tesselateEggInWorld(state, pos);
        break;
    case BLOCK_RENDER_FENCE:
        rendered = tesselateFenceInWorld(state, pos);
        break;
    case BLOCK_RENDER_WALL:
        rendered = tesselateWallInWorld(state, pos);
        break;
    case BLOCK_RENDER_LEVER:
        rendered = tesselateLeverInWorld(state, pos);
        break;
    case BLOCK_RENDER_TRIPWIRE_SOURCE:
        rendered = tesselateTripwireSourceInWorld(state, pos);
        break;
    case BLOCK_RENDER_TRIPWIRE:
        rendered = tesselateTripwireInWorld(state, pos);
        break;
    case BLOCK_RENDER_BED:
        rendered = tesselateBedInWorld(state, pos);
        break;
    case BLOCK_RENDER_REPEATER:
        rendered = tesselateRepeaterInWorld(state, pos);
        break;
    case BLOCK_RENDER_DIODE:
        rendered = tesselateDiodeInWorld(state, pos);
        break;
    case BLOCK_RENDER_COMPARATOR:
        rendered = tesselateComparatorInWorld(state, pos);
        break;
    case BLOCK_RENDER_PISTON_BASE:
        rendered = tesselatePistonBaseInWorld(state, pos, false, actualState);
        break;
    case BLOCK_RENDER_PISTON_EXTENSION:
        rendered = tesselatePistonExtensionInWorld(state, pos, true, actualState);
        break;
    case BLOCK_RENDER_THIN_FENCE:
    case BLOCK_RENDER_GLASS_PANE:
        rendered = tesselateThinFenceInWorld(state, pos);
        break;
    case BLOCK_RENDER_VINE:
        rendered = tesselateVineInWorld(state, pos);
        break;
    case BLOCK_RENDER_FENCE_GATE:
        rendered = tesselateFenceGateInWorld(state, pos);
        break;
    case BLOCK_RENDER_CAULDRON:
        rendered = tesselateCauldronInWorld(state, pos);
        break;
    case BLOCK_RENDER_FLOWER_POT:
        rendered = tesselateFlowerPotInWorld(state, pos);
        break;
    case BLOCK_RENDER_ANVIL:
        rendered = tesselateAnvilInWorld(state, pos);
        break;
    case BLOCK_RENDER_BREWING_STAND:
        rendered = tesselateBrewingStandInWorld(state, pos);
        break;
    case BLOCK_RENDER_AIR_PORTAL_FRAME:
        rendered = tesselateAirPortalFrameInWorld(state, pos);
        break;
    case BLOCK_RENDER_COCOA:
        rendered = tesselateCocoaInWorld(state, pos);
        break;
    case BLOCK_RENDER_BEACON:
        rendered = tesselateBeaconInWorld(state, pos);
        break;
    case BLOCK_RENDER_HOPPER:
        rendered = tesselateHopperInWorld(state, pos);
        break;
    case BLOCK_RENDER_END_ROD:
        rendered = tesselateEndRodInWorld(state, pos);
        break;
    case BLOCK_RENDER_CHORUS_FLOWER:
        rendered = tesselateChorusFlowerInWorld(state, pos);
        break;
    case BLOCK_RENDER_CHORUS_PLANT:
        rendered = tesselateChorusPlantInWorld(state, pos);
        break;
    case BLOCK_RENDER_OBSERVER:
        rendered = tesselateObserverInWorld(state, pos);
        break;
    case BLOCK_RENDER_TURTLE_EGG:
        rendered = tesselateTurtleEggInWorld(state, pos);
        break;
    case BLOCK_RENDER_SEAGRASS:
        rendered = tessellateSeagrassInWorld(state, pos);
        break;
    case BLOCK_RENDER_SEA_PICKLE:
        rendered = tessellateSeaPickleInWorld(state, pos);
        break;
    case BLOCK_RENDER_COMMAND_BLOCK:
        rendered = tesselateCommandBlockInWorld(state, pos);
        break;
    }

    builder->setMipmapEnable(oldMipmap);
    return rendered;
}

void BlockRenderer::tesselateInWorldFixedTexture(const BlockState* state, const BlockPos& pos,
                                                 TextureAtlasSprite* texture)
{
    setFixedTexture(texture);
    tesselateInWorld(state, pos, NULL, boost::shared_ptr<BlockEntity>());
    clearFixedTexture();
}

bool BlockRenderer::tesselateInWorldNoCulling(const BlockState* state, const BlockPos& pos,
                                              const BlockState* actualState, boost::shared_ptr<BlockEntity> blockEntity)
{
    mRenderAllFaces = true;
    bool rendered = tesselateInWorld(state, pos, actualState, blockEntity);
    mRenderAllFaces = false;
    return rendered;
}

void BlockRenderer::tesselatePistonBaseForceExtended(const BlockState* state, const BlockPos& pos,
                                                     const BlockState* actualState)
{
    mRenderAllFaces = true;
    tesselatePistonBaseInWorld(state, pos, true, actualState);
    mRenderAllFaces = false;
}

void BlockRenderer::tesselatePistonArmNoCulling(const BlockState* state, const BlockPos& pos, bool shortHead,
                                                const BlockState* actualState)
{
    mRenderAllFaces = true;
    tesselatePistonExtensionInWorld(state, pos, shortHead, actualState);
    mRenderAllFaces = false;
}

void BlockRenderer::renderAllFaces(const BlockState* state)
{
    BufferBuilder* builder = Tesselator::getInstance()->getBuilder();

    builder->begin();
    builder->normal(0.0f, -1.0f, 0.0f);
    renderFaceDown(state, 0.0, 0.0, 0.0, getTexture(state, Direction::DOWN));
    builder->end();

    builder->begin();
    builder->normal(0.0f, 1.0f, 0.0f);
    renderFaceUp(state, 0.0, 0.0, 0.0, getTexture(state, Direction::UP));
    builder->end();

    builder->begin();
    builder->normal(0.0f, 0.0f, -1.0f);
    renderNorth(state, 0.0, 0.0, 0.0, getTexture(state, Direction::NORTH));
    builder->end();

    builder->begin();
    builder->normal(0.0f, 0.0f, 1.0f);
    renderSouth(state, 0.0, 0.0, 0.0, getTexture(state, Direction::SOUTH));
    builder->end();

    builder->begin();
    builder->normal(-1.0f, 0.0f, 0.0f);
    renderWest(state, 0.0, 0.0, 0.0, getTexture(state, Direction::WEST));
    builder->end();

    builder->begin();
    builder->normal(1.0f, 0.0f, 0.0f);
    renderEast(state, 0.0, 0.0, 0.0, getTexture(state, Direction::EAST));
    builder->end();
}

void BlockRenderer::tesselateEndRod(const BlockState* state, float x, float y, float z)
{
    tesselateEndRodCenter(state, x, y, z);
    tesselateEndRodBase(state, x, y, z);
}

bool BlockRenderer::tesselateSlimeBlockInWorld(const BlockState* state, const BlockPos& pos)
{
    if (tesselateBlockInWorld(state, pos, 0))
    {
        setFixedShape(0.1875f, 0.1875f, 0.1875f, 0.8125f, 0.8125f, 0.8125f);
        tesselateBlockInWorld(state, pos, 0x3f);
        clearFixedShape();
    }
    return true;
}

bool BlockRenderer::tesselateCrossInWorld(const BlockState* state, const BlockPos& pos)
{
    BufferBuilder* builder = Tesselator::getInstance()->getBuilder();
    builder->tex2(getLightColor(state->getBlock(), mLevelSource, pos));

    int color = mBlockColors->getColor(state, mLevelSource, &pos, 0);
    float red = (float)(color >> 16 & 255) / 255.0f;
    float green = (float)(color >> 8 & 255) / 255.0f;
    float blue = (float)(color & 255) / 255.0f;
    if (GameRenderer::anaglyph3d)
    {
        float redPart = red * 30.0f;
        float newRed = (blue * 11.0f + (green * 59.0f + redPart)) / 100.0f;
        float newGreen = (green * 70.0f + redPart) / 100.0f;
        float newBlue = (blue * 70.0f + redPart) / 100.0f;
        red = newRed;
        green = newGreen;
        blue = newBlue;
    }
    builder->color(red, green, blue);

    float x = (float)pos.x;
    float y = (float)pos.y;
    float z = (float)pos.z;
    if (!mUseInventoryTint)
    {
        Vec3* offset = state->getOffset(mLevelSource, pos);
        x += offset->x;
        y += offset->y;
        z += offset->z;
    }
    tesselateCrossTexture(state, x, y, z, 1.0f);
    return true;
}

bool BlockRenderer::tesselateRowInWorld(const BlockState* state, const BlockPos& pos)
{
    BufferBuilder* builder = Tesselator::getInstance()->getBuilder();
    builder->tex2(getLightColor(state->getBlock(), mLevelSource, pos));
    builder->color(1.0f, 1.0f, 1.0f);
    tesselateRowTexture(state, (float)pos.x, (float)pos.y - 0.0625f, (float)pos.z);
    return true;
}

bool BlockRenderer::tesselateCactusInWorld(const BlockState* state, const BlockPos& pos)
{
    int color = mBlockColors->getColor(state, mLevelSource, &pos, 0);
    float red = (float)(color >> 16 & 255) / 255.0f;
    float green = (float)(color >> 8 & 255) / 255.0f;
    float blue = (float)(color & 255) / 255.0f;
    if (GameRenderer::anaglyph3d)
    {
        float redPart = red * 30.0f;
        float newRed = (blue * 11.0f + (green * 59.0f + redPart)) / 100.0f;
        float newGreen = (green * 70.0f + redPart) / 100.0f;
        float newBlue = (blue * 70.0f + redPart) / 100.0f;
        red = newRed;
        green = newGreen;
        blue = newBlue;
    }
    return tesselateCactusInWorld(state, pos, red, green, blue);
}

bool BlockRenderer::tesselateTurtleEggInWorld(const BlockState* state, const BlockPos& pos)
{
    Block* block = state->getBlock();
    BufferBuilder* builder = Tesselator::getInstance()->getBuilder();
    builder->tex2(getLightColor(block, mLevelSource, pos));
    int color = mBlockColors->getColor(state, mLevelSource, &pos, 0);
    float red = (float)(color >> 16 & 255) / 255.0f;
    float green = (float)(color >> 8 & 255) / 255.0f;
    float blue = (float)(color & 255) / 255.0f;
    if (GameRenderer::anaglyph3d)
    {
        float anaglyphRed = (red * 30.0f + green * 59.0f + blue * 11.0f) / 100.0f;
        float anaglyphGreen = (red * 30.0f + green * 70.0f) / 100.0f;
        float anaglyphBlue = (red * 30.0f + blue * 70.0f) / 100.0f;
        red = anaglyphRed;
        green = anaglyphGreen;
        blue = anaglyphBlue;
    }
    builder->color(red, green, blue);
    tesselateTurtleEgg(state, pos, red, green, blue);
    return true;
}

bool BlockRenderer::tesselateEndRodInWorld(const BlockState* state, const BlockPos& pos)
{
    BufferBuilder* builder = Tesselator::getInstance()->getBuilder();
    builder->tex2(getLightColor(state->getBlock(), mLevelSource, pos));
    builder->color(1.0f, 1.0f, 1.0f);
    tesselateEndRod(state, (float)pos.x, (float)pos.y, (float)pos.z);
    return true;
}

void BlockRenderer::renderBlock(const BlockState* state, Level* level, const BlockPos& pos)
{
    Block* block = state->getBlock();
    BufferBuilder* builder = Tesselator::getInstance()->getBuilder();
    builder->begin();
    builder->tex2(getLightColor(block, level, pos));

    builder->color(0.5f, 0.5f, 0.5f);
    renderFaceDown(state, -0.5, -0.5, -0.5, getTexture(state, Direction::DOWN));
    builder->color(1.0f, 1.0f, 1.0f);
    renderFaceUp(state, -0.5, -0.5, -0.5, getTexture(state, Direction::UP));
    builder->color(0.8f, 0.8f, 0.8f);
    renderNorth(state, -0.5, -0.5, -0.5, getTexture(state, Direction::NORTH));
    builder->color(0.8f, 0.8f, 0.8f);
    renderSouth(state, -0.5, -0.5, -0.5, getTexture(state, Direction::SOUTH));
    builder->color(0.6f, 0.6f, 0.6f);
    renderWest(state, -0.5, -0.5, -0.5, getTexture(state, Direction::WEST));
    builder->color(0.6f, 0.6f, 0.6f);
    renderEast(state, -0.5, -0.5, -0.5, getTexture(state, Direction::EAST));
    builder->end();
}

void BlockRenderer::renderCube(const BlockState* state, float alpha)
{
    int renderShape = state->getRenderShape();
    BufferBuilder* builder = Tesselator::getInstance()->getBuilder();
    if (renderShape != BLOCK_RENDER_MODEL)
        return;

    GlStateManager::translatef(-0.5f, -0.5f, -0.5f);
    builder->begin();
    builder->color(1.0f, 1.0f, 1.0f, alpha);
    renderFaceDown(state, 0.0, 0.0, 0.0, getTexture(state, Direction::DOWN));
    builder->color(0.5f, 0.5f, 0.5f, alpha);
    renderFaceUp(state, 0.0, 0.0, 0.0, getTexture(state, Direction::UP));
    builder->color(0.8f, 0.8f, 0.8f, alpha);
    renderNorth(state, 0.0, 0.0, 0.0, getTexture(state, Direction::NORTH));
    renderSouth(state, 0.0, 0.0, 0.0, getTexture(state, Direction::SOUTH));
    builder->color(0.6f, 0.6f, 0.6f, alpha);
    renderWest(state, 0.0, 0.0, 0.0, getTexture(state, Direction::WEST));
    renderEast(state, 0.0, 0.0, 0.0, getTexture(state, Direction::EAST));
    builder->end();
    GlStateManager::translatef(0.5f, 0.5f, 0.5f);
}

void BlockRenderer::tesselateRowTexture(const BlockState* state, float x, float y, float z)
{
    BufferBuilder* builder = Tesselator::getInstance()->getBuilder();
    TextureAtlasSprite* texture = getTexture(state, Direction::UP);
    if (hasFixedTexture())
        texture = mFixedTexture;

    float u0 = (float)texture->getU0(true);
    float v0 = (float)texture->getV0(true);
    float u1 = (float)texture->getU1(true);
    float v1 = (float)texture->getV1(true);

    float y1 = y + 1.0f;
    float xNear = x + 0.25f;
    float xFar = x + 0.75f;
    float zNear = z + 0.25f;
    float zFar = z + 0.75f;
    float xEnd = x + 1.0f;
    float zEnd = z + 1.0f;

    builder->vertexUV(xNear, y1, z, u0, v0);
    builder->vertexUV(xNear, y, z, u0, v1);
    builder->vertexUV(xNear, y, zEnd, u1, v1);
    builder->vertexUV(xNear, y1, zEnd, u1, v0);
    builder->vertexUV(xNear, y1, zEnd, u0, v0);
    builder->vertexUV(xNear, y, zEnd, u0, v1);
    builder->vertexUV(xNear, y, z, u1, v1);
    builder->vertexUV(xNear, y1, z, u1, v0);

    builder->vertexUV(xFar, y1, zEnd, u0, v0);
    builder->vertexUV(xFar, y, zEnd, u0, v1);
    builder->vertexUV(xFar, y, z, u1, v1);
    builder->vertexUV(xFar, y1, z, u1, v0);
    builder->vertexUV(xFar, y1, z, u0, v0);
    builder->vertexUV(xFar, y, z, u0, v1);
    builder->vertexUV(xFar, y, zEnd, u1, v1);
    builder->vertexUV(xFar, y1, zEnd, u1, v0);

    builder->vertexUV(x, y1, zNear, u0, v0);
    builder->vertexUV(x, y, zNear, u0, v1);
    builder->vertexUV(xEnd, y, zNear, u1, v1);
    builder->vertexUV(xEnd, y1, zNear, u1, v0);
    builder->vertexUV(xEnd, y1, zNear, u0, v0);
    builder->vertexUV(xEnd, y, zNear, u0, v1);
    builder->vertexUV(x, y, zNear, u1, v1);
    builder->vertexUV(x, y1, zNear, u1, v0);

    builder->vertexUV(xEnd, y1, zFar, u0, v0);
    builder->vertexUV(xEnd, y, zFar, u0, v1);
    builder->vertexUV(x, y, zFar, u1, v1);
    builder->vertexUV(x, y1, zFar, u1, v0);
    builder->vertexUV(x, y1, zFar, u0, v0);
    builder->vertexUV(x, y, zFar, u0, v1);
    builder->vertexUV(xEnd, y, zFar, u1, v1);
    builder->vertexUV(xEnd, y1, zFar, u1, v0);
}

void BlockRenderer::tesselateTorch(const BlockState* state, float x, float y, float z, float xTilt, float zTilt)
{
    BufferBuilder* builder = Tesselator::getInstance()->getBuilder();
    TextureAtlasSprite* texture = getTexture(state, Direction::DOWN);
    if (hasFixedTexture())
        texture = mFixedTexture;

    float u0 = (float)texture->getU0(true);
    float v0 = (float)texture->getV0(true);
    float u1 = (float)texture->getU1(true);
    float v1 = (float)texture->getV1(true);

    float headU0 = (float)texture->getU(7.0);
    float headV0 = (float)texture->getV(6.0);
    float headU1 = (float)texture->getU(9.0);
    float headV1 = (float)texture->getV(8.0);
    texture->adjustUVs(headU0, headU1, headV0, headV1);

    float capU0 = (float)texture->getU(7.0);
    float capV0 = (float)texture->getV(13.0);
    float capU1 = (float)texture->getU(9.0);
    float capV1 = (float)texture->getV(15.0);
    texture->adjustUVs(capU0, capU1, capV0, capV1);

    float xCenter = x + 0.5f;
    float zCenter = z + 0.5f;
    float top = y + 0.625f;
    float zc = zTilt * 0.375f + zCenter;
    float xc = xTilt * 0.375f + xCenter;
    float zc0 = zc - 0.0625f;
    float xc0 = xc - 0.0625f;
    float xFar = xCenter - 0.5f;
    float xNear = xCenter + 0.5f;
    float zFar = zCenter - 0.5f;
    float zNear = zCenter + 0.5f;
    float xb0 = xCenter - 0.0625f;
    float xb1 = xCenter + 0.0625f;
    float zb0 = zCenter - 0.0625f;
    float zb1 = zCenter + 0.0625f;
    float zc1 = zc + 0.0625f;
    float xc1 = xc + 0.0625f;

    builder->vertexUV(xc0, top, zc0, headU0, headV0);
    builder->vertexUV(xc0, top, zc1, headU0, headV1);
    builder->vertexUV(xc1, top, zc1, headU1, headV1);
    builder->vertexUV(xc1, top, zc0, headU1, headV0);

    builder->vertexUV(xb1 + xTilt, y, zb0 + zTilt, capU1, capV0);
    builder->vertexUV(xb1 + xTilt, y, zb1 + zTilt, capU1, capV1);
    builder->vertexUV(xb0 + xTilt, y, zb1 + zTilt, capU0, capV1);
    builder->vertexUV(xb0 + xTilt, y, zb0 + zTilt, capU0, capV0);

    float yTop = y + 1.0f;

    builder->vertexUV(xb0, yTop, zFar, u0, v0);
    builder->vertexUV(xb0 + xTilt, y, zFar + zTilt, u0, v1);
    builder->vertexUV(xb0 + xTilt, y, zNear + zTilt, u1, v1);
    builder->vertexUV(xb0, yTop, zNear, u1, v0);
    builder->vertexUV(xb1, yTop, zNear, u0, v0);
    builder->vertexUV(xCenter + xTilt + 0.0625f, y, zNear + zTilt, u0, v1);
    builder->vertexUV(xCenter + xTilt + 0.0625f, y, zFar + zTilt, u1, v1);
    builder->vertexUV(xb1, yTop, zFar, u1, v0);

    builder->vertexUV(xFar, yTop, zb1, u0, v0);
    builder->vertexUV(xFar + xTilt, y, zb1 + zTilt, u0, v1);
    builder->vertexUV(xNear + xTilt, y, zb1 + zTilt, u1, v1);
    builder->vertexUV(xNear, yTop, zb1, u1, v0);
    builder->vertexUV(xNear, yTop, zb0, u0, v0);
    builder->vertexUV(xNear + xTilt, y, zb0 + zTilt, u0, v1);
    builder->vertexUV(xFar + xTilt, y, zb0 + zTilt, u1, v1);
    builder->vertexUV(xFar, yTop, zb0, u1, v0);
}

bool BlockRenderer::tesselateBlockInWorld(const BlockState* state, const BlockPos& pos)
{
    int color = mBlockColors->getColor(state, mLevelSource, &pos, 0);
    float red = (float)(color >> 16 & 255) / 255.0f;
    float green = (float)(color >> 8 & 255) / 255.0f;
    float blue = (float)(color & 255) / 255.0f;
    if (GameRenderer::anaglyph3d)
    {
        float anaglyphRed = (red * 30.0f + green * 59.0f + blue * 11.0f) / 100.0f;
        float anaglyphGreen = (red * 30.0f + green * 70.0f) / 100.0f;
        float anaglyphBlue = (red * 30.0f + blue * 70.0f) / 100.0f;
        red = anaglyphRed;
        green = anaglyphGreen;
        blue = anaglyphBlue;
    }
    Block* block = state->getBlock();
    if (block->defaultBlockState()->getLightEmission() == 0)
        return tesselateBlockInWorldWithAmbienceOcclusionTexLighting(state, pos, red, green, blue, 0, mShapeOutside);
    return tesselateBlockInWorld(state, pos, red, green, blue);
}

bool BlockRenderer::tesselateBlockInWorld(const BlockState* state, const BlockPos& pos, int faces)
{
    int color = mBlockColors->getColor(state, mLevelSource, &pos, 0);
    float red = (float)(color >> 16 & 255) / 255.0f;
    float green = (float)(color >> 8 & 255) / 255.0f;
    float blue = (float)(color & 255) / 255.0f;
    if (GameRenderer::anaglyph3d)
    {
        float anaglyphRed = (red * 30.0f + green * 59.0f + blue * 11.0f) / 100.0f;
        float anaglyphGreen = (red * 30.0f + green * 70.0f) / 100.0f;
        float anaglyphBlue = (red * 30.0f + blue * 70.0f) / 100.0f;
        red = anaglyphRed;
        green = anaglyphGreen;
        blue = anaglyphBlue;
    }
    Block* block = state->getBlock();
    if (block->defaultBlockState()->getLightEmission() == 0 && !hasLightOverride())
        return tesselateBlockInWorldWithAmbienceOcclusionTexLighting(state, pos, red, green, blue, faces,
                                                                     mShapeOutside);
    return tesselateBlockInWorld(state, pos, red, green, blue);
}

float BlockRenderer::tesselateAnvilPiece(const BlockState* state, const BlockPos& pos, int part, float bottom,
                                         float width, float height, float length, bool rotate, bool inventory)
{
    if (rotate)
    {
        float swap = width;
        width = length;
        length = swap;
    }

    static_cast<AnvilBlock*>(state->getBlock())->mTextureType = part;
    setShape(0.5f - width * 0.5f, bottom, 0.5f - length * 0.5f, width * 0.5f + 0.5f, bottom + height,
             length * 0.5f + 0.5f);

    if (!inventory)
    {
        tesselateBlockInWorld(state, pos);
    }
    else
    {
        BufferBuilder* builder = Tesselator::getInstance()->getBuilder();

        builder->begin();
        builder->normal(0.0f, -1.0f, 0.0f);
        renderFaceDown(state, 0.0, 0.0, 0.0, getTexture(state, Direction::DOWN));
        builder->end();

        builder->begin();
        builder->normal(0.0f, 1.0f, 0.0f);
        renderFaceUp(state, 0.0, 0.0, 0.0, getTexture(state, Direction::UP));
        builder->end();

        builder->begin();
        builder->normal(0.0f, 0.0f, -1.0f);
        renderNorth(state, 0.0, 0.0, 0.0, getTexture(state, Direction::NORTH));
        builder->end();

        builder->begin();
        builder->normal(0.0f, 0.0f, 1.0f);
        renderSouth(state, 0.0, 0.0, 0.0, getTexture(state, Direction::SOUTH));
        builder->end();

        builder->begin();
        builder->normal(-1.0f, 0.0f, 0.0f);
        renderWest(state, 0.0, 0.0, 0.0, getTexture(state, Direction::WEST));
        builder->end();

        builder->begin();
        builder->normal(1.0f, 0.0f, 0.0f);
        renderEast(state, 0.0, 0.0, 0.0, getTexture(state, Direction::EAST));
        builder->end();
    }
    return bottom + height;
}

bool BlockRenderer::tesselateAnvilInWorld(const BlockState* state, const BlockPos& pos, bool inventory)
{
    mRenderFromInside = true;
    int direction;
    if (!inventory)
        direction = state->getValue(AnvilBlock::FACING)->get3DDataValue();
    else
        direction = Direction::DOWN->get3DDataValue();

    bool rotate = false;
    switch (direction)
    {
    case 2:
        mFaceRotation2 = 1;
        mFaceRotation3 = 2;
        break;
    case 3:
        mFaceRotation2 = 2;
        mFaceRotation3 = 1;
        mFaceRotation4 = 3;
        mFaceRotation5 = 3;
        break;
    case 4:
        mFaceRotation5 = 1;
        mFaceRotation1 = 2;
        mFaceRotation0 = 1;
        rotate = true;
        mFaceRotation4 = 2;
        break;
    case 5:
        rotate = true;
        mFaceRotation0 = 2;
        mFaceRotation4 = 1;
        mFaceRotation5 = 2;
        mFaceRotation1 = 1;
        break;
    }

    float bottom = tesselateAnvilPiece(state, pos, 0, 0.0f, 0.75f, 0.25f, 0.75f, rotate, inventory);
    bottom = tesselateAnvilPiece(state, pos, 1, bottom, 0.5f, 0.0625f, 0.625f, rotate, inventory);
    bottom = tesselateAnvilPiece(state, pos, 2, bottom, 0.25f, 0.3125f, 0.5f, rotate, inventory);
    tesselateAnvilPiece(state, pos, 3, bottom, 0.625f, 0.375f, 1.0f, rotate, inventory);
    setShape(0.0f, 0.0f, 0.0f, 1.0f, 1.0f, 1.0f);
    mRenderFromInside = false;
    mFaceRotation4 = 0;
    mFaceRotation1 = 0;
    mFaceRotation3 = 0;
    mFaceRotation2 = 0;
    mFaceRotation5 = 0;
    mFaceRotation0 = 0;
    return true;
}

bool BlockRenderer::tesselateHopperInWorld(const BlockState* state, const BlockPos& pos, bool inventory)
{
    BufferBuilder* builder = Tesselator::getInstance()->getBuilder();
    const Direction* facing = state->getValue(HopperBlock::FACING);
    setShape(0.0f, 0.625f, 0.0f, 1.0f, 1.0f, 1.0f);

    TextureAtlasSprite* outside;
    TextureAtlasSprite* inside;
    if (!inventory)
    {
        tesselateBlockInWorld(state, pos);
        builder->tex2(state->getLightColor(mLevelSource, pos));
        int color = mBlockColors->getColor(state, mLevelSource, &pos, 0);
        float red = (float)(color >> 16 & 255) / 255.0f;
        float green = (float)(color >> 8 & 255) / 255.0f;
        float blue = (float)(color & 255) / 255.0f;
        if (GameRenderer::anaglyph3d)
        {
            float anaglyphRed = (red * 30.0f + green * 59.0f + blue * 11.0f) / 100.0f;
            float anaglyphGreen = (red * 30.0f + green * 70.0f) / 100.0f;
            float anaglyphBlue = (red * 30.0f + blue * 70.0f) / 100.0f;
            red = anaglyphRed;
            green = anaglyphGreen;
            blue = anaglyphBlue;
        }
        builder->color(red, green, blue);
        outside = HopperBlock::getTexture(HopperBlock::TEXTURE_OUTSIDE);
        inside = HopperBlock::getTexture(HopperBlock::TEXTURE_INSIDE);

        float wall = 0.125f;
        renderEast(state, (double)((float)pos.x - 1.0f + wall), (double)pos.y, (double)pos.z, outside);
        renderWest(state, (double)((float)pos.x + 1.0f - wall), (double)pos.y, (double)pos.z, outside);
        renderSouth(state, (double)pos.x, (double)pos.y, (double)((float)pos.z - 1.0f + wall), outside);
        renderNorth(state, (double)pos.x, (double)pos.y, (double)((float)pos.z + 1.0f - wall), outside);
        renderFaceUp(state, (double)pos.x, (double)((float)pos.y - 1.0f) + 0.625, (double)pos.z, inside);
    }
    else
    {
        builder->begin();
        builder->normal(0.0f, -1.0f, 0.0f);
        renderFaceDown(state, 0.0, 0.0, 0.0, getTexture(state, Direction::DOWN));
        builder->end();
        builder->begin();
        builder->normal(0.0f, 1.0f, 0.0f);
        renderFaceUp(state, 0.0, 0.0, 0.0, getTexture(state, Direction::UP));
        builder->end();
        builder->begin();
        builder->normal(0.0f, 0.0f, -1.0f);
        renderNorth(state, 0.0, 0.0, 0.0, getTexture(state, Direction::NORTH));
        builder->end();
        builder->begin();
        builder->normal(0.0f, 0.0f, 1.0f);
        renderSouth(state, 0.0, 0.0, 0.0, getTexture(state, Direction::SOUTH));
        builder->end();
        builder->begin();
        builder->normal(-1.0f, 0.0f, 0.0f);
        renderWest(state, 0.0, 0.0, 0.0, getTexture(state, Direction::WEST));
        builder->end();
        builder->begin();
        builder->normal(1.0f, 0.0f, 0.0f);
        renderEast(state, 0.0, 0.0, 0.0, getTexture(state, Direction::EAST));
        builder->end();
        outside = HopperBlock::getTexture(HopperBlock::TEXTURE_OUTSIDE);
        inside = HopperBlock::getTexture(HopperBlock::TEXTURE_INSIDE);

        builder->begin();
        builder->normal(1.0f, 0.0f, 0.0f);
        renderEast(state, -0.875, 0.0, 0.0, outside);
        builder->end();
        builder->begin();
        builder->normal(-1.0f, 0.0f, 0.0f);
        renderWest(state, 0.875, 0.0, 0.0, outside);
        builder->end();
        builder->begin();
        builder->normal(0.0f, 0.0f, 1.0f);
        renderSouth(state, 0.0, 0.0, -0.875, outside);
        builder->end();
        builder->begin();
        builder->normal(0.0f, 0.0f, -1.0f);
        renderNorth(state, 0.0, 0.0, 0.875, outside);
        builder->end();
        builder->begin();
        builder->normal(0.0f, 1.0f, 0.0f);
        renderFaceUp(state, 0.0, -0.375, 0.0, inside);
        builder->end();
    }

    setFixedTexture(outside);
    float middle = 0.25f;
    setShape(middle, middle, middle, 0.75f, 0.623f, 0.75f);

    if (!inventory)
    {
        tesselateBlockInWorld(state, pos);
        setFixedTexture(outside);
        if (facing == Direction::DOWN)
        {
            setShape(0.375f, 0.0f, 0.375f, 0.625f, middle, 0.625f);
            tesselateBlockInWorld(state, pos);
        }
        if (facing == Direction::NORTH)
        {
            setShape(0.375f, middle, 0.0f, 0.625f, 0.5f, middle);
            tesselateBlockInWorld(state, pos);
        }
        if (facing == Direction::SOUTH)
        {
            setShape(0.375f, middle, 0.75f, 0.625f, 0.5f, 1.0f);
            tesselateBlockInWorld(state, pos);
        }
        if (facing == Direction::WEST)
        {
            setShape(0.0f, middle, 0.375f, middle, 0.5f, 0.625f);
            tesselateBlockInWorld(state, pos);
        }
        if (facing == Direction::EAST)
        {
            setShape(0.75f, middle, 0.375f, 1.0f, 0.5f, 0.625f);
            tesselateBlockInWorld(state, pos);
        }
        clearFixedTexture();
    }
    else
    {
        builder->begin();
        builder->normal(1.0f, 0.0f, 0.0f);
        renderEast(state, 0.0, 0.0, 0.0, outside);
        builder->end();
        builder->begin();
        builder->normal(-1.0f, 0.0f, 0.0f);
        renderWest(state, 0.0, 0.0, 0.0, outside);
        builder->end();
        builder->begin();
        builder->normal(0.0f, 0.0f, 1.0f);
        renderSouth(state, 0.0, 0.0, 0.0, outside);
        builder->end();
        builder->begin();
        builder->normal(0.0f, 0.0f, -1.0f);
        renderNorth(state, 0.0, 0.0, 0.0, outside);
        builder->end();
        builder->begin();
        builder->normal(0.0f, 1.0f, 0.0f);
        renderFaceUp(state, 0.0, 0.0, 0.0, outside);
        builder->end();
        builder->begin();
        builder->normal(0.0f, -1.0f, 0.0f);
        renderFaceDown(state, 0.0, 0.0, 0.0, outside);
        builder->end();
        clearFixedTexture();
    }
    return true;
}

void BlockRenderer::tesselateEndRodCenter(const BlockState* state, float x, float y, float z)
{
    BufferBuilder* builder = Tesselator::getInstance()->getBuilder();
    setShape(static_cast<EndRodBlock*>(Blocks::end_rod)->getShapeRod(state));
    TextureAtlasSprite* texture = getTexture(state, Direction::DOWN);

    float sideU0 = (float)texture->getU(0.0);
    float sideV0 = (float)texture->getV(0.0);
    float sideU1 = (float)texture->getU(2.0);
    float sideV1 = (float)texture->getV(15.0);
    texture->adjustUVs(sideU0, sideU1, sideV0, sideV1);
    float topU0 = (float)texture->getU(2.0);
    float topV0 = (float)texture->getV(0.0);
    float topU1 = (float)texture->getU(4.0);
    float topV1 = (float)texture->getV(2.0);
    texture->adjustUVs(topU0, topU1, topV0, topV1);
    float bottomU0 = (float)texture->getU(4.0);
    float bottomV0 = (float)texture->getV(2.0);
    float bottomU1 = (float)texture->getU(2.0);
    float bottomV1 = (float)texture->getV(0.0);
    texture->adjustUVs(bottomU0, bottomU1, bottomV0, bottomV1);

    const Direction* facing = state->getValue(DirectionalBlock::FACING_PROPERTY);
    switch (facing->get3DDataValue())
    {
    case 2:
        mFaceRotation2 = 5;
        mFaceRotation3 = 5;
        break;
    case 3:
        mFaceRotation2 = 4;
        mFaceRotation3 = 4;
        break;
    case 4:
        mFaceRotation0 = 5;
        mFaceRotation1 = 5;
        mFaceRotation4 = 5;
        mFaceRotation5 = 4;
        break;
    case 5:
        mFaceRotation0 = 4;
        mFaceRotation1 = 4;
        mFaceRotation4 = 4;
        mFaceRotation5 = 5;
        break;
    }
    builder->color(1.0f, 1.0f, 1.0f, 1.0f);

    switch (facing->get3DDataValue())
    {
    case 0:
        renderFaceUp(state, x, y, z, texture, topU0, topV0, topU1, topV1);
        builder->color(0.5f, 0.5f, 0.5f, 1.0f);
        renderFaceDown(state, x, y, z, texture, bottomU0, bottomV0, bottomU1, bottomV1);
        builder->color(0.8f, 0.8f, 0.8f, 1.0f);
        renderNorth(state, x, y, z, texture, sideU1, sideV1, sideU0, sideV0);
        renderSouth(state, x, y, z, texture, sideU1, sideV1, sideU0, sideV0);
        builder->color(0.6f, 0.6f, 0.6f, 1.0f);
        renderEast(state, x, y, z, texture, sideU1, sideV1, sideU0, sideV0);
        renderWest(state, x, y, z, texture, sideU1, sideV1, sideU0, sideV0);
        break;
    case 2:
        renderFaceUp(state, x, y, z, texture, sideU0, sideV0, sideU1, sideV1);
        builder->color(0.5f, 0.5f, 0.5f, 1.0f);
        renderFaceDown(state, x, y, z, texture, sideU0, sideV0, sideU1, sideV1);
        builder->color(0.8f, 0.8f, 0.8f, 1.0f);
        renderNorth(state, x, y, z, texture, topU0, topV0, topU1, topV1);
        renderSouth(state, x, y, z, texture, bottomU0, bottomV0, bottomU1, bottomV1);
        builder->color(0.6f, 0.6f, 0.6f, 1.0f);
        renderEast(state, x, y, z, texture, sideU0, sideV0, sideU1, sideV1);
        renderWest(state, x, y, z, texture, sideU0, sideV0, sideU1, sideV1);
        break;
    case 3:
        renderFaceUp(state, x, y, z, texture, sideU1, sideV1, sideU0, sideV0);
        builder->color(0.5f, 0.5f, 0.5f, 1.0f);
        renderFaceDown(state, x, y, z, texture, sideU1, sideV1, sideU0, sideV0);
        builder->color(0.8f, 0.8f, 0.8f, 1.0f);
        renderNorth(state, x, y, z, texture, topU0, topV0, topU1, topV1);
        renderSouth(state, x, y, z, texture, bottomU1, bottomV1, bottomU0, bottomV0);
        builder->color(0.6f, 0.6f, 0.6f, 1.0f);
        renderEast(state, x, y, z, texture, sideU0, sideV0, sideU1, sideV1);
        renderWest(state, x, y, z, texture, sideU0, sideV0, sideU1, sideV1);
        break;
    case 4:
        renderFaceUp(state, x, y, z, texture, sideU0, sideV0, sideU1, sideV1);
        builder->color(0.5f, 0.5f, 0.5f, 1.0f);
        renderFaceDown(state, x, y, z, texture, sideU0, sideV0, sideU1, sideV1);
        builder->color(0.8f, 0.8f, 0.8f, 1.0f);
        renderNorth(state, x, y, z, texture, sideU0, sideV0, sideU1, sideV1);
        renderSouth(state, x, y, z, texture, sideU0, sideV0, sideU1, sideV1);
        builder->color(0.6f, 0.6f, 0.6f, 1.0f);
        renderEast(state, x, y, z, texture, topU0, topV0, topU1, topV1);
        renderWest(state, x, y, z, texture, bottomU1, bottomV1, bottomU0, bottomV0);
        break;
    case 5:
        renderFaceUp(state, x, y, z, texture, sideU0, sideV0, sideU1, sideV1);
        builder->color(0.5f, 0.5f, 0.5f, 1.0f);
        renderFaceDown(state, x, y, z, texture, sideU0, sideV0, sideU1, sideV1);
        builder->color(0.8f, 0.8f, 0.8f, 1.0f);
        renderNorth(state, x, y, z, texture, sideU0, sideV0, sideU1, sideV1);
        renderSouth(state, x, y, z, texture, sideU0, sideV0, sideU1, sideV1);
        builder->color(0.6f, 0.6f, 0.6f, 1.0f);
        renderEast(state, x, y, z, texture, topU0, topV0, topU1, topV1);
        renderWest(state, x, y, z, texture, bottomU0, bottomV0, bottomU1, bottomV1);
        break;
    default:
        renderFaceUp(state, x, y, z, texture, topU0, topV0, topU1, topV1);
        builder->color(0.5f, 0.5f, 0.5f, 1.0f);
        renderFaceDown(state, x, y, z, texture, bottomU0, bottomV0, bottomU1, bottomV1);
        builder->color(0.8f, 0.8f, 0.8f, 1.0f);
        renderNorth(state, x, y, z, texture, sideU0, sideV0, sideU1, sideV1);
        renderSouth(state, x, y, z, texture, sideU0, sideV0, sideU1, sideV1);
        builder->color(0.6f, 0.6f, 0.6f, 1.0f);
        renderEast(state, x, y, z, texture, sideU0, sideV0, sideU1, sideV1);
        renderWest(state, x, y, z, texture, sideU0, sideV0, sideU1, sideV1);
        break;
    }

    mFaceRotation0 = 0;
    mFaceRotation1 = 0;
    mFaceRotation2 = 0;
    mFaceRotation3 = 0;
    mFaceRotation4 = 0;
    mFaceRotation5 = 0;
    mFaceTransform0 = 0;
    mFaceTransform1 = 0;
    mFaceTransform2 = 0;
    mFaceTransform3 = 0;
    mFaceTransform4 = 0;
    mFaceTransform5 = 0;
}

void BlockRenderer::tesselateEndRodBase(const BlockState* state, float x, float y, float z)
{
    BufferBuilder* builder = Tesselator::getInstance()->getBuilder();
    setShape(static_cast<EndRodBlock*>(Blocks::end_rod)->getShapeBase(state));
    TextureAtlasSprite* texture = getTexture(state, Direction::DOWN);

    float sideU0 = (float)texture->getU(2.0);
    float sideV0 = (float)texture->getV(6.0);
    float sideU1 = (float)texture->getU(6.0);
    float sideV1 = (float)texture->getV(7.0);
    texture->adjustUVs(sideU0, sideU1, sideV0, sideV1);
    float topU0 = (float)texture->getU(2.0);
    float topV0 = (float)texture->getV(2.0);
    float topU1 = (float)texture->getU(6.0);
    float topV1 = (float)texture->getV(6.0);
    texture->adjustUVs(topU0, topU1, topV0, topV1);
    float bottomU0 = (float)texture->getU(6.0);
    float bottomV0 = (float)texture->getV(6.0);
    float bottomU1 = (float)texture->getU(2.0);
    float bottomV1 = (float)texture->getV(2.0);
    texture->adjustUVs(bottomU0, bottomU1, bottomV0, bottomV1);

    const Direction* facing = state->getValue(DirectionalBlock::FACING_PROPERTY);
    switch (facing->get3DDataValue())
    {
    case 2:
        mFaceRotation2 = 5;
        mFaceRotation3 = 5;
        break;
    case 3:
        mFaceRotation2 = 4;
        mFaceRotation3 = 4;
        break;
    case 4:
        mFaceRotation0 = 5;
        mFaceRotation1 = 5;
        mFaceRotation4 = 5;
        mFaceRotation5 = 4;
        break;
    case 5:
        mFaceRotation0 = 4;
        mFaceRotation1 = 4;
        mFaceRotation4 = 4;
        mFaceRotation5 = 5;
        break;
    }
    builder->color(1.0f, 1.0f, 1.0f, 1.0f);

    switch (facing->get3DDataValue())
    {
    case 0:
        renderFaceUp(state, x, y, z, texture, topU0, topV0, topU1, topV1);
        builder->color(0.5f, 0.5f, 0.5f, 1.0f);
        renderFaceDown(state, x, y, z, texture, bottomU0, bottomV0, bottomU1, bottomV1);
        builder->color(0.8f, 0.8f, 0.8f, 1.0f);
        renderNorth(state, x, y, z, texture, sideU0, sideV0, sideU1, sideV1);
        renderSouth(state, x, y, z, texture, sideU0, sideV0, sideU1, sideV1);
        builder->color(0.6f, 0.6f, 0.6f, 1.0f);
        renderEast(state, x, y, z, texture, sideU0, sideV0, sideU1, sideV1);
        renderWest(state, x, y, z, texture, sideU0, sideV0, sideU1, sideV1);
        break;
    case 2:
        renderFaceUp(state, x, y, z, texture, sideU0, sideV0, sideU1, sideV1);
        builder->color(0.5f, 0.5f, 0.5f, 1.0f);
        renderFaceDown(state, x, y, z, texture, sideU0, sideV0, sideU1, sideV1);
        builder->color(0.8f, 0.8f, 0.8f, 1.0f);
        renderNorth(state, x, y, z, texture, topU0, topV0, topU1, topV1);
        renderSouth(state, x, y, z, texture, bottomU0, bottomV0, bottomU1, bottomV1);
        builder->color(0.6f, 0.6f, 0.6f, 1.0f);
        renderEast(state, x, y, z, texture, sideU0, sideV0, sideU1, sideV1);
        renderWest(state, x, y, z, texture, sideU0, sideV0, sideU1, sideV1);
        break;
    case 3:
        renderFaceUp(state, x, y, z, texture, sideU1, sideV1, sideU0, sideV0);
        builder->color(0.5f, 0.5f, 0.5f, 1.0f);
        renderFaceDown(state, x, y, z, texture, sideU1, sideV1, sideU0, sideV0);
        builder->color(0.8f, 0.8f, 0.8f, 1.0f);
        renderNorth(state, x, y, z, texture, topU0, topV0, topU1, topV1);
        renderSouth(state, x, y, z, texture, bottomU1, bottomV1, bottomU0, bottomV0);
        builder->color(0.6f, 0.6f, 0.6f, 1.0f);
        renderEast(state, x, y, z, texture, sideU0, sideV0, sideU1, sideV1);
        renderWest(state, x, y, z, texture, sideU0, sideV0, sideU1, sideV1);
        break;
    case 4:
        renderFaceUp(state, x, y, z, texture, sideU0, sideV0, sideU1, sideV1);
        builder->color(0.5f, 0.5f, 0.5f, 1.0f);
        renderFaceDown(state, x, y, z, texture, sideU0, sideV0, sideU1, sideV1);
        builder->color(0.8f, 0.8f, 0.8f, 1.0f);
        renderNorth(state, x, y, z, texture, sideU0, sideV0, sideU1, sideV1);
        renderSouth(state, x, y, z, texture, sideU0, sideV0, sideU1, sideV1);
        builder->color(0.6f, 0.6f, 0.6f, 1.0f);
        renderEast(state, x, y, z, texture, topU0, topV0, topU1, topV1);
        renderWest(state, x, y, z, texture, bottomU1, bottomV1, bottomU0, bottomV0);
        break;
    case 5:
        renderFaceUp(state, x, y, z, texture, sideU0, sideV0, sideU1, sideV1);
        builder->color(0.5f, 0.5f, 0.5f, 1.0f);
        renderFaceDown(state, x, y, z, texture, sideU0, sideV0, sideU1, sideV1);
        builder->color(0.8f, 0.8f, 0.8f, 1.0f);
        renderNorth(state, x, y, z, texture, sideU0, sideV0, sideU1, sideV1);
        renderSouth(state, x, y, z, texture, sideU0, sideV0, sideU1, sideV1);
        builder->color(0.6f, 0.6f, 0.6f, 1.0f);
        renderEast(state, x, y, z, texture, topU0, topV0, topU1, topV1);
        renderWest(state, x, y, z, texture, bottomU0, bottomV0, bottomU1, bottomV1);
        break;
    default:
        renderFaceUp(state, x, y, z, texture, bottomU0, bottomV0, bottomU1, bottomV1);
        builder->color(0.5f, 0.5f, 0.5f, 1.0f);
        renderFaceDown(state, x, y, z, texture, topU0, topV0, topU1, topV1);
        builder->color(0.8f, 0.8f, 0.8f, 1.0f);
        renderNorth(state, x, y, z, texture, sideU0, sideV0, sideU1, sideV1);
        renderSouth(state, x, y, z, texture, sideU0, sideV0, sideU1, sideV1);
        builder->color(0.6f, 0.6f, 0.6f, 1.0f);
        renderEast(state, x, y, z, texture, sideU0, sideV0, sideU1, sideV1);
        renderWest(state, x, y, z, texture, sideU0, sideV0, sideU1, sideV1);
        break;
    }

    mFaceRotation0 = 0;
    mFaceRotation1 = 0;
    mFaceRotation2 = 0;
    mFaceRotation3 = 0;
    mFaceRotation4 = 0;
    mFaceRotation5 = 0;
    mFaceTransform0 = 0;
    mFaceTransform1 = 0;
    mFaceTransform2 = 0;
    mFaceTransform3 = 0;
    mFaceTransform4 = 0;
    mFaceTransform5 = 0;
}

void BlockRenderer::renderBlock(const BlockState* state, float brightness, float alpha, bool useDisplayList,
                                bool centre)
{
    static const BoxDesc STAIRS_BOXES[3] = {
        {0x10, 0.0f, 0.0f, 0.0f, 0.5f, 0.5f, 1.0f},
        {0x22, 0.5f, 0.0f, 0.0f, 1.0f, 0.5f, 1.0f},
        {0x01, 0.5f, 0.5f, 0.0f, 1.0f, 1.0f, 1.0f},
    };
    static const BoxDesc EGG_BOXES[8] = {
        {0x00, 0.375f, 0.9375f, 0.375f, 0.625f, 1.0f, 0.625f},
        {0x00, 0.3125f, 0.875f, 0.3125f, 0.6875f, 0.9375f, 0.6875f},
        {0x00, 0.3125f, 0.8125f, 0.3125f, 0.6875f, 0.875f, 0.6875f},
        {0x00, 0.1875f, 0.6875f, 0.1875f, 0.8125f, 0.8125f, 0.8125f},
        {0x00, 0.125f, 0.5f, 0.125f, 0.875f, 0.6875f, 0.875f},
        {0x00, 0.0625f, 0.1875f, 0.0625f, 0.9375f, 0.5f, 0.9375f},
        {0x00, 0.125f, 0.0625f, 0.125f, 0.875f, 0.1875f, 0.875f},
        {0x00, 0.1875f, 0.0f, 0.1875f, 0.8125f, 0.0625f, 0.8125f},
    };
    static const BoxDesc FENCE_BOXES[4] = {
        {0x00, 0.375f, 0.0f, 0.0f, 0.625f, 1.0f, 0.25f},
        {0x00, 0.375f, 0.0f, 0.75f, 0.625f, 1.0f, 1.0f},
        {0x00, 0.4375f, 0.8125f, -0.125f, 0.5625f, 0.9375f, 1.125f},
        {0x00, 0.4375f, 0.3125f, -0.125f, 0.5625f, 0.4375f, 1.125f},
    };
    static const BoxDesc FENCE_GATE_BOXES[10] = {
        {0x00, 0.0f, 0.3125f, 0.4375f, 0.125f, 1.0f, 0.5625f},
        {0x00, 0.875f, 0.3125f, 0.4375f, 1.0f, 1.0f, 0.5625f},
        {0x00, 0.375f, 0.375f, 0.4375f, 0.5f, 0.9375f, 0.5625f},
        {0x00, 0.5f, 0.375f, 0.4375f, 0.625f, 0.9375f, 0.5625f},
        {0x00, 0.125f, 0.375f, 0.4375f, 0.375f, 0.5625f, 0.5625f},
        {0x00, 0.125f, 0.75f, 0.4375f, 0.375f, 0.9375f, 0.5625f},
        {0x00, 0.625f, 0.375f, 0.4375f, 0.875f, 0.5625f, 0.5625f},
        {0x00, 0.625f, 0.75f, 0.4375f, 0.875f, 0.9375f, 0.5625f},
    };
    static const BoxDesc WALL_BOXES[2] = {
        {0x00, 0.25f, 0.0f, 0.25f, 0.75f, 1.0f, 0.75f},
        {0x00, 0.3125f, 0.0f, 0.0f, 0.6875f, 0.8125f, 1.0f},
    };
    static const BoxDesc TURTLE_EGG_BOX = {0x00, 0.3125f, 0.0f, 0.25f, 0.625f, 0.4375f, 0.5625f};

    BufferBuilder* builder = Tesselator::getInstance()->getBuilder();
    Block* block = state->getBlock();
    bool isGrass = block == Blocks::grass;

    if (block == Blocks::carved_pumpkin || block == Blocks::lit_pumpkin || block == Blocks::dispenser
        || block == Blocks::furnace || block == Blocks::dropper || block == Blocks::observer
        || block == Blocks::white_glazed_terracotta || block == Blocks::orange_glazed_terracotta
        || block == Blocks::magenta_glazed_terracotta || block == Blocks::light_blue_glazed_terracotta
        || block == Blocks::yellow_glazed_terracotta || block == Blocks::lime_glazed_terracotta
        || block == Blocks::pink_glazed_terracotta || block == Blocks::gray_glazed_terracotta
        || block == Blocks::silver_glazed_terracotta || block == Blocks::cyan_glazed_terracotta
        || block == Blocks::purple_glazed_terracotta || block == Blocks::blue_glazed_terracotta
        || block == Blocks::brown_glazed_terracotta || block == Blocks::green_glazed_terracotta
        || block == Blocks::red_glazed_terracotta || block == Blocks::black_glazed_terracotta)
    {
        state = block->getBlockState(Direction::NORTH->get3DDataValue());
    }

    if (mUseColor)
    {
        if (block == Blocks::water)
        {
            unsigned int waterColor = Biome::OCEAN->getWaterColor();
            alpha = (float)(waterColor >> 24) / 255.0f;
            GlStateManager::color4f((float)(waterColor >> 16 & 255) / 255.0f * brightness,
                                    (float)(waterColor >> 8 & 255) / 255.0f * brightness,
                                    (float)(waterColor & 255) / 255.0f * brightness, alpha);
        }
        else if (isGrass)
        {
            int grassColor = 0xffffff;
            GlStateManager::color4f((float)(grassColor >> 16 & 255) / 255.0f * brightness,
                                    (float)(grassColor >> 8 & 255) / 255.0f * brightness,
                                    (float)(grassColor & 255) / 255.0f * brightness, alpha);
        }
        else
        {
            int blockColor = block->getColor(state);
            GlStateManager::color4f((float)(blockColor >> 16 & 255) / 255.0f * brightness,
                                    (float)(blockColor >> 8 & 255) / 255.0f * brightness,
                                    (float)(blockColor & 255) / 255.0f * brightness, alpha);
        }
    }

    int shape = state->getRenderShape();
    bool oldMipmap = builder->setMipmapEnable(block->mMipmap);

    if (shape == BLOCK_RENDER_HORIZONTAL_DIRECTIONAL)
    {
        mFaceRotation4 = 3;
        mFaceRotation5 = 3;
        mFaceRotation0 = 2;
        mFaceRotation1 = 1;
        mFaceTransform5 = 1;
        mFaceTransform0 = 2;
        mFaceTransform2 = 0;
        mFaceTransform3 = 3;
    }
    else if (shape == BLOCK_RENDER_OBSERVER)
    {
        mFaceRotation4 = 3;
        mFaceRotation5 = 3;
        mFaceRotation0 = 0;
    }
    else if (shape == BLOCK_RENDER_COMMAND_BLOCK)
    {
        mFaceRotation4 = 3;
        mFaceRotation5 = 3;
        mFaceRotation0 = 0;
        mFaceRotation1 = 0;
    }
    else if (shape == BLOCK_RENDER_TRAPDOOR)
    {
        mFaceRotation0 = 0;
        mFaceTransform4 = 0;
        mFaceTransform5 = 0;
        mFaceRotation4 = 3;
        mFaceRotation5 = 3;
        mFaceRotation1 = 3;
        mFaceRotation2 = 3;
        mFaceRotation3 = 3;
        mFaceTransform0 = 1;
        mFaceTransform1 = 1;
        mFaceTransform2 = 1;
        mFaceTransform3 = 1;
    }
    else if (shape == BLOCK_RENDER_PISTON_BASE)
    {
        TypedBoxed<const Direction*> up(Direction::UP);
        state = state->setBoxedValue(PistonBaseBlock::FACING, &up);
    }

    if (shape == BLOCK_RENDER_MODEL || shape == BLOCK_RENDER_WATER || shape == BLOCK_RENDER_PISTON_BASE
        || shape == BLOCK_RENDER_AIR_PORTAL_FRAME || shape == BLOCK_RENDER_TREE || shape == BLOCK_RENDER_QUARTZ
        || shape == BLOCK_RENDER_SLIME_BLOCK || shape == BLOCK_RENDER_TRAPDOOR
        || shape == BLOCK_RENDER_HORIZONTAL_DIRECTIONAL || shape == BLOCK_RENDER_OBSERVER
        || shape == BLOCK_RENDER_COMMAND_BLOCK)
    {
        const AABB* blockShape;
        if (block == Blocks::stone_button || block == Blocks::wooden_button || block == Blocks::acacia_button
            || block == Blocks::birch_button || block == Blocks::dark_oak_button || block == Blocks::jungle_button
            || block == Blocks::spruce_button)
        {
            static const AABB* buttonShape = AABB::newPermanent(0.3125, 0.375, 0.375, 0.6875, 0.625, 0.625);
            blockShape = buttonShape;
            setShape(blockShape);
        }
        else
        {
            blockShape = block->getShape(state, mLevelSource, BlockPos());
            setShape(blockShape);
        }

        if (centre)
        {
            Vec3* center = blockShape->getCenter();
            GlStateManager::translatef(-(float)((center->x + 0.5) * 0.5), -(float)((center->y + 0.5) * 0.5),
                                       -(float)((center->z + 0.5) * 0.5));
        }

        builder->begin();
        builder->normal(0.0f, -1.0f, 0.0f);
        if (isGrass && mUseColor)
            builder->color(brightness, brightness, brightness, alpha);
        renderFaceDown(state, 0.0, 0.0, 0.0, getTexture(state, Direction::DOWN));
        if (isGrass && mUseColor)
        {
            int grassColor = block->getColor(state);
            builder->color((float)(grassColor >> 16 & 255) / 255.0f * brightness,
                           (float)(grassColor >> 8 & 255) / 255.0f * brightness,
                           (float)(grassColor & 255) / 255.0f * brightness, alpha);
        }
        builder->normal(0.0f, 1.0f, 0.0f);
        renderFaceUp(state, 0.0, 0.0, 0.0, getTexture(state, Direction::UP));
        if (isGrass && mUseColor)
            builder->color(brightness, brightness, brightness, alpha);
        builder->normal(0.0f, 0.0f, -1.0f);
        renderNorth(state, 0.0, 0.0, 0.0, getTexture(state, Direction::NORTH));
        if (isGrass && mUseColor)
        {
            int grassColor = block->getColor(state);
            builder->color((float)(grassColor >> 16 & 255) / 255.0f * brightness,
                           (float)(grassColor >> 8 & 255) / 255.0f * brightness,
                           (float)(grassColor & 255) / 255.0f * brightness, alpha);
            builder->normal(0.0f, 0.0f, -1.0f);
            renderNorth(state, 0.0, 0.0, 0.0, GrassBlock::getSideTextureOverlay());
            builder->color(brightness, brightness, brightness, alpha);
        }
        builder->normal(0.0f, 0.0f, 1.0f);
        renderSouth(state, 0.0, 0.0, 0.0, getTexture(state, Direction::SOUTH));
        if (isGrass && mUseColor)
        {
            int grassColor = block->getColor(state);
            builder->color((float)(grassColor >> 16 & 255) / 255.0f * brightness,
                           (float)(grassColor >> 8 & 255) / 255.0f * brightness,
                           (float)(grassColor & 255) / 255.0f * brightness, alpha);
            builder->normal(0.0f, 0.0f, 1.0f);
            renderSouth(state, 0.0, 0.0, 0.0, GrassBlock::getSideTextureOverlay());
            builder->color(brightness, brightness, brightness, alpha);
        }
        builder->normal(-1.0f, 0.0f, 0.0f);
        renderWest(state, 0.0, 0.0, 0.0, getTexture(state, Direction::WEST));
        if (isGrass && mUseColor)
        {
            int grassColor = block->getColor(state);
            builder->color((float)(grassColor >> 16 & 255) / 255.0f * brightness,
                           (float)(grassColor >> 8 & 255) / 255.0f * brightness,
                           (float)(grassColor & 255) / 255.0f * brightness, alpha);
            builder->normal(-1.0f, 0.0f, 0.0f);
            renderWest(state, 0.0, 0.0, 0.0, GrassBlock::getSideTextureOverlay());
            builder->color(brightness, brightness, brightness, alpha);
        }
        builder->normal(1.0f, 0.0f, 0.0f);
        renderEast(state, 0.0, 0.0, 0.0, getTexture(state, Direction::EAST));
        if (isGrass && mUseColor)
        {
            int grassColor = block->getColor(state);
            builder->color((float)(grassColor >> 16 & 255) / 255.0f * brightness,
                           (float)(grassColor >> 8 & 255) / 255.0f * brightness,
                           (float)(grassColor & 255) / 255.0f * brightness, alpha);
            builder->normal(1.0f, 0.0f, 0.0f);
            renderEast(state, 0.0, 0.0, 0.0, GrassBlock::getSideTextureOverlay());
            builder->color(brightness, brightness, brightness, alpha);
        }
        builder->end();

        if (centre)
            GlStateManager::translatef(0.5f, 0.5f, 0.5f);

        if (shape == BLOCK_RENDER_HORIZONTAL_DIRECTIONAL || shape == BLOCK_RENDER_OBSERVER
            || shape == BLOCK_RENDER_COMMAND_BLOCK)
        {
            mFaceRotation4 = 0;
            mFaceRotation5 = 0;
            mFaceRotation0 = 0;
            mFaceRotation1 = 0;
            mFaceTransform5 = 0;
            mFaceTransform0 = 0;
            mFaceTransform2 = 0;
            mFaceTransform3 = 0;
        }
    }
    else if (shape == BLOCK_RENDER_CROSS)
    {
        builder->begin();
        builder->normal(0.0f, -1.0f, 0.0f);
        tesselateCrossTexture(state, -0.5f, -0.5f, -0.5f, 1.0f);
        builder->end();
    }
    else if (shape == BLOCK_RENDER_STEM)
    {
        builder->begin();
        builder->normal(0.0f, -1.0f, 0.0f);
        tesselateStemTexture(state, mMaxY, -0.5f, -0.5f, -0.5f);
        builder->end();
    }
    else if (shape == BLOCK_RENDER_LILYPAD)
    {
        builder->begin();
        builder->normal(0.0f, -1.0f, 0.0f);
        builder->end();
    }
    else if (shape == BLOCK_RENDER_CACTUS)
    {
        setShape(0.0f, 0.0f, 0.0f, 1.0f, 1.0f, 1.0f);
        GlStateManager::translatef(-0.5f, -0.5f, -0.5f);
        builder->begin();
        builder->normal(0.0f, -1.0f, 0.0f);
        renderFaceDown(state, 0.0, 0.0, 0.0, getTexture(state, Direction::DOWN));
        builder->end();
        builder->begin();
        builder->normal(0.0f, 1.0f, 0.0f);
        renderFaceUp(state, 0.0, 0.0, 0.0, getTexture(state, Direction::UP));
        builder->end();
        builder->begin();
        builder->normal(0.0f, 0.0f, -1.0f);
        builder->addOffset(0.0f, 0.0f, 0.0625f);
        renderNorth(state, 0.0, 0.0, 0.0, getTexture(state, Direction::NORTH));
        builder->addOffset(0.0f, 0.0f, -0.0625f);
        builder->end();
        builder->begin();
        builder->normal(0.0f, 0.0f, 1.0f);
        builder->addOffset(0.0f, 0.0f, -0.0625f);
        renderSouth(state, 0.0, 0.0, 0.0, getTexture(state, Direction::SOUTH));
        builder->addOffset(0.0f, 0.0f, 0.0625f);
        builder->end();
        builder->begin();
        builder->normal(-1.0f, 0.0f, 0.0f);
        builder->addOffset(0.0625f, 0.0f, 0.0f);
        renderWest(state, 0.0, 0.0, 0.0, getTexture(state, Direction::WEST));
        builder->addOffset(-0.0625f, 0.0f, 0.0f);
        builder->end();
        builder->begin();
        builder->normal(1.0f, 0.0f, 0.0f);
        builder->addOffset(-0.0625f, 0.0f, 0.0f);
        renderEast(state, 0.0, 0.0, 0.0, getTexture(state, Direction::EAST));
        builder->addOffset(0.0625f, 0.0f, 0.0f);
        builder->end();
        GlStateManager::translatef(0.5f, 0.5f, 0.5f);
    }
    else if (shape == BLOCK_RENDER_ENTITY)
    {
        GlStateManager::rotatef(90.0f, 0.0f, 1.0f, 0.0f);
        GlStateManager::translatef(-0.5f, -0.5f, -0.5f);
        Block* chest = state->getBlock();
        EntityBlockRenderer::instance->renderByItem(
            boost::shared_ptr<ItemInstance>(new ItemInstance(chest, 1, chest->convertBlockStateToLegacyData(state))),
            alpha, mUseColor, useDisplayList);
        GlStateManager::enableRescaleNormal();
    }
    else if (shape == BLOCK_RENDER_ROW)
    {
        builder->begin();
        builder->normal(0.0f, -1.0f, 0.0f);
        tesselateRowTexture(state, -0.5f, -0.5f, -0.5f);
        builder->end();
    }
    else if (shape == BLOCK_RENDER_TORCH)
    {
        builder->begin();
        builder->normal(0.0f, -1.0f, 0.0f);
        tesselateTorch(block->defaultBlockState(), -0.5f, -0.5f, -0.5f, 0.0f, 0.0f);
        builder->end();
    }
    else if (shape == BLOCK_RENDER_STAIRS)
    {
        fjRenderCubes(state, STAIRS_BOXES, 3, NULL);
    }
    else if (shape == BLOCK_RENDER_EGG)
    {
        fjRenderCubes(state, EGG_BOXES, 8, NULL);
    }
    else if (shape == BLOCK_RENDER_FENCE)
    {
        fjRenderCubes(state, FENCE_BOXES, 4, NULL);
    }
    else if (shape == BLOCK_RENDER_FENCE_GATE)
    {
        fjRenderCubes(state, FENCE_GATE_BOXES, 10, NULL);
    }
    else if (shape == BLOCK_RENDER_WALL)
    {
        fjRenderCubes(state, WALL_BOXES, 2, NULL);
    }
    else if (shape == BLOCK_RENDER_ANVIL)
    {
        GlStateManager::translatef(-0.5f, -0.5f, -0.5f);
        tesselateAnvilInWorld(state, BlockPos(0, 0, 0), true);
        GlStateManager::translatef(0.5f, 0.5f, 0.5f);
    }
    else if (shape == BLOCK_RENDER_AIR_PORTAL_FRAME)
    {
        setShape(0.0f, 0.0f, 0.0f, 1.0f, 0.8125f, 1.0f);
        GlStateManager::translatef(-0.5f, -0.5f, -0.5f);
        builder->begin();
        builder->normal(0.0f, -1.0f, 0.0f);
        renderFaceDown(state, 0.0, 0.0, 0.0, getTexture(block->defaultBlockState(), Direction::DOWN));
        builder->end();
        builder->begin();
        builder->normal(0.0f, 1.0f, 0.0f);
        renderFaceUp(state, 0.0, 0.0, 0.0, getTexture(block->defaultBlockState(), Direction::UP));
        builder->end();
        builder->begin();
        builder->normal(0.0f, 0.0f, -1.0f);
        renderNorth(state, 0.0, 0.0, 0.0, getTexture(block->defaultBlockState(), Direction::NORTH));
        builder->end();
        builder->begin();
        builder->normal(0.0f, 0.0f, 1.0f);
        renderSouth(state, 0.0, 0.0, 0.0, getTexture(block->defaultBlockState(), Direction::SOUTH));
        builder->end();
        builder->begin();
        builder->normal(-1.0f, 0.0f, 0.0f);
        renderWest(state, 0.0, 0.0, 0.0, getTexture(block->defaultBlockState(), Direction::WEST));
        builder->end();
        builder->begin();
        builder->normal(1.0f, 0.0f, 0.0f);
        renderEast(state, 0.0, 0.0, 0.0, getTexture(block->defaultBlockState(), Direction::EAST));
        builder->end();
        GlStateManager::translatef(0.5f, 0.5f, 0.5f);
    }
    else if (shape == BLOCK_RENDER_BEACON)
    {
        for (int i = 0; i < 3; ++i)
        {
            TextureAtlasSprite* texture;
            if (i == 0)
            {
                setShape(0.125f, 0.0f, 0.125f, 0.875f, 0.1875f, 0.875f);
                texture = getTexture(Blocks::obsidian, Direction::UP);
            }
            else if (i == 1)
            {
                setShape(0.1875f, 0.1875f, 0.1875f, 0.8125f, 0.875f, 0.8125f);
                texture = getTexture(Blocks::beacon, Direction::UP);
            }
            else
            {
                setShape(0.0f, 0.0f, 0.0f, 1.0f, 1.0f, 1.0f);
                texture = getTexture(Blocks::glass, Direction::UP);
            }
            setFixedTexture(texture);
            GlStateManager::translatef(-0.5f, -0.5f, -0.5f);
            builder->begin();
            builder->normal(0.0f, -1.0f, 0.0f);
            renderFaceDown(state, 0.0, 0.0, 0.0, getTexture(block->defaultBlockState(), Direction::DOWN));
            builder->end();
            builder->begin();
            builder->normal(0.0f, 1.0f, 0.0f);
            renderFaceUp(state, 0.0, 0.0, 0.0, getTexture(block->defaultBlockState(), Direction::UP));
            builder->end();
            builder->begin();
            builder->normal(0.0f, 0.0f, -1.0f);
            renderNorth(state, 0.0, 0.0, 0.0, getTexture(block->defaultBlockState(), Direction::NORTH));
            builder->end();
            builder->begin();
            builder->normal(0.0f, 0.0f, 1.0f);
            renderSouth(state, 0.0, 0.0, 0.0, getTexture(block->defaultBlockState(), Direction::SOUTH));
            builder->end();
            builder->begin();
            builder->normal(-1.0f, 0.0f, 0.0f);
            renderWest(state, 0.0, 0.0, 0.0, getTexture(block->defaultBlockState(), Direction::WEST));
            builder->end();
            builder->begin();
            builder->normal(1.0f, 0.0f, 0.0f);
            renderEast(state, 0.0, 0.0, 0.0, getTexture(block->defaultBlockState(), Direction::EAST));
            builder->end();
            GlStateManager::translatef(0.5f, 0.5f, 0.5f);
        }
        setShape(0.0f, 0.0f, 0.0f, 1.0f, 1.0f, 1.0f);
        clearFixedTexture();
    }
    else if (shape == BLOCK_RENDER_HOPPER)
    {
        GlStateManager::translatef(-0.5f, -0.5f, -0.5f);
        tesselateHopperInWorld(block->defaultBlockState(), BlockPos(0, 0, 0), true);
        GlStateManager::translatef(0.5f, 0.5f, 0.5f);
    }
    else if (shape == BLOCK_RENDER_GLASS_PANE)
    {
        setShape(0.4375f, 0.0f, 0.0f, 0.5625f, 1.0f, 1.0f);
        GlStateManager::translatef(-0.5f, -0.5f, -0.5f);
        builder->begin();
        builder->normal(0.0f, -1.0f, 0.0f);
        renderFaceDown(state, 0.0, 0.0, 0.0, block->getTexture(Direction::DOWN, state));
        builder->end();
        builder->begin();
        builder->normal(0.0f, 1.0f, 0.0f);
        renderFaceUp(state, 0.0, 0.0, 0.0, block->getTexture(Direction::UP, state));
        builder->end();
        builder->begin();
        builder->normal(0.0f, 0.0f, -1.0f);
        renderNorth(state, 0.0, 0.0, 0.0, block->getTexture(Direction::NORTH, state));
        builder->end();
        builder->begin();
        builder->normal(0.0f, 0.0f, 1.0f);
        renderSouth(state, 0.0, 0.0, 0.0, block->getTexture(Direction::SOUTH, state));
        builder->end();
        builder->begin();
        builder->normal(-1.0f, 0.0f, 0.0f);
        renderWest(state, 0.0, 0.0, 0.0, block->getTexture(Direction::WEST, state));
        builder->end();
        builder->begin();
        builder->normal(1.0f, 0.0f, 0.0f);
        renderEast(state, 0.0, 0.0, 0.0, block->getTexture(Direction::EAST, state));
        builder->end();
        GlStateManager::translatef(0.5f, 0.5f, 0.5f);
        setShape(0.0f, 0.0f, 0.0f, 1.0f, 1.0f, 1.0f);
    }
    else if (shape == BLOCK_RENDER_CHORUS_FLOWER)
    {
        ChorusFlowerBlock* chorusFlower = static_cast<ChorusFlowerBlock*>(state->getBlock());
        GlStateManager::translatef(-0.5f, -0.5f, -0.5f);
        setShape(ChorusFlowerBlock::getDownShape());
        chorusFlower->setCulledFace(NULL);
        renderAllFaces(state);
        setShape(ChorusFlowerBlock::getUpShape());
        chorusFlower->setCulledFace(Direction::DOWN);
        renderAllFaces(state);
        setShape(ChorusFlowerBlock::getNorthShape());
        chorusFlower->setCulledFace(Direction::SOUTH);
        renderAllFaces(state);
        setShape(ChorusFlowerBlock::getSouthShape());
        chorusFlower->setCulledFace(Direction::NORTH);
        renderAllFaces(state);
        setShape(ChorusFlowerBlock::getEastShape());
        chorusFlower->setCulledFace(Direction::WEST);
        renderAllFaces(state);
        setShape(ChorusFlowerBlock::getWestShape());
        chorusFlower->setCulledFace(Direction::EAST);
        renderAllFaces(state);
    }
    else if (shape == BLOCK_RENDER_CHORUS_PLANT)
    {
        GlStateManager::translatef(-0.5f, -0.5f, -0.5f);
        setShape(ChorusFlowerBlock::getDownShape());
        renderAllFaces(state);
        setShape(ChorusFlowerBlock::getUpShape());
        renderAllFaces(state);
        setShape(ChorusFlowerBlock::getNorthShape());
        renderAllFaces(state);
        setShape(ChorusFlowerBlock::getSouthShape());
        renderAllFaces(state);
        setShape(ChorusFlowerBlock::getEastShape());
        renderAllFaces(state);
        setShape(ChorusFlowerBlock::getWestShape());
        renderAllFaces(state);
    }
    else if (shape == BLOCK_RENDER_END_ROD)
    {
        GlStateManager::translatef(-0.5f, -0.5f, -0.5f);
        builder->begin();
#ifdef MINECRAFT_PC
        builder->normal(0.0f, 1.0f, 0.0f);
#endif
        tesselateEndRod(state, 0.0f, 0.0f, 0.0f);
        builder->end();
        GlStateManager::translatef(0.5f, 0.5f, 0.5f);
    }
    else if (shape == BLOCK_RENDER_TURTLE_EGG)
    {
        BoxDesc box = TURTLE_EGG_BOX;
        TextureAtlasSprite* texture = getTexture(state, Direction::UP);
        BoxUV uv;
        uv.down[0] = (float)texture->getU(0.0);
        uv.down[1] = (float)texture->getU(4.0);
        uv.down[2] = (float)texture->getV(0.0);
        uv.down[3] = (float)texture->getV(4.0);
        uv.up[0] = (float)texture->getU(0.0);
        uv.up[1] = (float)texture->getU(4.0);
        uv.up[2] = (float)texture->getV(0.0);
        uv.up[3] = (float)texture->getV(4.0);
        uv.north[0] = (float)texture->getU(1.0);
        uv.north[1] = (float)texture->getU(5.0);
        uv.north[2] = (float)texture->getV(4.0);
        uv.north[3] = (float)texture->getV(11.0);
        uv.south[0] = (float)texture->getU(1.0);
        uv.south[1] = (float)texture->getU(5.0);
        uv.south[2] = (float)texture->getV(4.0);
        uv.south[3] = (float)texture->getV(11.0);
        uv.east[0] = (float)texture->getU(1.0);
        uv.east[1] = (float)texture->getU(5.0);
        uv.east[2] = (float)texture->getV(4.0);
        uv.east[3] = (float)texture->getV(11.0);
        uv.west[0] = (float)texture->getU(1.0);
        uv.west[1] = (float)texture->getU(5.0);
        uv.west[2] = (float)texture->getV(4.0);
        uv.west[3] = (float)texture->getV(11.0);
        fjRenderCubes(state, &box, 1, &uv);
    }

    builder->setMipmapEnable(oldMipmap);
    mFaceRotation0 = 0;
    mFaceRotation1 = 0;
    mFaceRotation2 = 0;
    mFaceRotation3 = 0;
    mFaceRotation4 = 0;
    mFaceRotation5 = 0;
    mFaceTransform0 = 0;
    mFaceTransform1 = 0;
    mFaceTransform2 = 0;
    mFaceTransform3 = 0;
    mFaceTransform4 = 0;
    mFaceTransform5 = 0;
}

bool BlockRenderer::tesselateTrapDoorInWorld(const BlockState* state, const BlockPos& pos)
{
    for (int i = 0; i < 6; ++i)
    {
        (&mFaceRotation0)[i] = 0;
        (&mFaceTransform0)[i] = 0;
    }

    const int direction = state->getValue(TrapDoorBlock::FACING)->get3DDataValue();
    const bool open = state->getValue(TrapDoorBlock::OPEN);
    const bool bottom = state->getValue(TrapDoorBlock::HALF) == TrapDoorBlock::BOTTOM;

    if (!open && bottom)
    {
        switch (direction)
        {
        case 3:
            mFaceRotation3 = 3;
            mFaceTransform2 = 1;
            mFaceRotation0 = 3;
            mFaceTransform3 = 1;
            mFaceRotation2 = 3;
            mFaceTransform1 = 1;
            mFaceTransform0 = 1;
            break;
        case 4:
            mFaceRotation2 = 3;
            mFaceTransform2 = 1;
            mFaceRotation4 = 1;
            mFaceRotation1 = 3;
            mFaceRotation0 = 3;
            mFaceTransform3 = 1;
            mFaceRotation5 = 2;
            mFaceTransform1 = 1;
            mFaceTransform0 = 1;
            break;
        case 5:
            mFaceRotation1 = 3;
            mFaceTransform3 = 1;
            mFaceRotation5 = 1;
            mFaceTransform2 = 1;
            mFaceRotation3 = 3;
            mFaceRotation0 = 3;
            mFaceTransform0 = 1;
            mFaceRotation4 = 2;
            mFaceTransform1 = 1;
            break;
        default:
            mFaceRotation3 = 3;
            mFaceRotation2 = 3;
            mFaceRotation4 = 3;
            mFaceRotation1 = 3;
            mFaceRotation5 = 3;
            mFaceTransform0 = 1;
            mFaceTransform2 = 1;
            mFaceTransform1 = 1;
            mFaceTransform3 = 1;
            break;
        }
    }
    else if (!open)
    {
        if (direction == 2)
        {
            mFaceRotation0 = 3;
            mFaceRotation4 = 3;
            mFaceRotation5 = 3;
        }
        else if (direction == 3)
        {
            mFaceRotation1 = 3;
        }
        else if (direction == 4)
        {
            mFaceRotation3 = 3;
            mFaceRotation4 = 1;
            mFaceRotation5 = 2;
        }
        else if (direction == 5)
        {
            mFaceRotation2 = 3;
            mFaceRotation4 = 2;
            mFaceRotation5 = 1;
        }
    }
    else if (bottom)
    {
        if (direction == 2)
        {
            mFaceRotation0 = 3;
            mFaceRotation3 = 1;
            mFaceRotation2 = 2;
            mFaceTransform0 = 2;
            mFaceRotation1 = 3;
            mFaceRotation5 = 3;
        }
        else if (direction == 3)
        {
            mFaceRotation3 = 2;
            mFaceTransform1 = 2;
            mFaceRotation4 = 3;
            mFaceRotation1 = 3;
            mFaceRotation2 = 1;
            mFaceRotation0 = 3;
        }
        else if (direction == 4)
        {
            mFaceRotation0 = 2;
            mFaceRotation1 = 1;
            mFaceRotation2 = 3;
            mFaceRotation3 = 3;
            mFaceRotation4 = 2;
            mFaceRotation5 = 2;
            mFaceTransform3 = 2;
        }
        else if (direction == 5)
        {
            mFaceRotation0 = 1;
            mFaceRotation1 = 2;
            mFaceRotation2 = 3;
            mFaceRotation3 = 3;
            mFaceRotation4 = 1;
            mFaceRotation5 = 1;
            mFaceTransform0 = 1;
            mFaceTransform2 = 2;
        }
    }
    else
    {
        if (direction == 2)
        {
            mFaceTransform2 = 1;
            mFaceRotation1 = 3;
            mFaceRotation4 = 3;
            mFaceRotation2 = 2;
            mFaceTransform1 = 1;
            mFaceTransform4 = 1;
            mFaceRotation3 = 1;
            mFaceTransform3 = 1;
        }
        else if (direction == 3)
        {
            mFaceRotation2 = 1;
            mFaceTransform2 = 1;
            mFaceRotation5 = 3;
            mFaceRotation3 = 2;
            mFaceTransform0 = 2;
            mFaceTransform3 = 1;
            mFaceTransform5 = 1;
            mFaceTransform4 = 1;
        }
        else if (direction == 4)
        {
            mFaceRotation4 = 1;
            mFaceTransform2 = 2;
            mFaceTransform4 = 1;
        }
        else if (direction == 5)
        {
            mFaceRotation0 = 1;
            mFaceRotation1 = 2;
            mFaceRotation4 = 2;
            mFaceRotation5 = 1;
            mFaceTransform5 = 1;
            mFaceTransform1 = 1;
            mFaceTransform4 = 1;
            mFaceTransform3 = 2;
        }
    }

    Block* block = state->getBlock();
    int faces = block->getFaceFlags(state, mLevelSource, pos, mRenderAllFaces);
    if (faces == 0)
        return false;

    setShape(block->getShape(state, mLevelSource, pos));
    bool rendered = tesselateBlockInWorld(state, pos, faces);

    for (int i = 0; i < 6; ++i)
    {
        (&mFaceRotation0)[i] = 0;
        (&mFaceTransform0)[i] = 0;
    }
    return rendered;
}

bool BlockRenderer::tesselateTreeInWorld(const BlockState* state, const BlockPos& pos)
{
    Direction::Axis* axis = static_cast<RotatedPillarBlock*>(state->getBlock())->getAxis(state);
    if (axis == Direction::Axis::X)
    {
        mFaceRotation0 = 1;
        mFaceRotation1 = 1;
        mFaceRotation4 = 1;
        mFaceRotation5 = 1;
    }
    else if (axis == Direction::Axis::Z)
    {
        mFaceRotation2 = 1;
        mFaceRotation3 = 1;
    }
    bool rendered = tesselateBlockInWorld(state, pos);
    mFaceRotation2 = 0;
    mFaceRotation0 = 0;
    mFaceRotation1 = 0;
    mFaceRotation3 = 0;
    mFaceRotation4 = 0;
    mFaceRotation5 = 0;
    return rendered;
}

bool BlockRenderer::tesselateQuartzInWorld(const BlockState* state, const BlockPos& pos)
{
    QuartzBlock::Variant* variant = state->getValue(QuartzBlock::VARIANT);
    if (variant == QuartzBlock::Variant::LINES_X)
    {
        mFaceRotation0 = 1;
        mFaceRotation4 = 1;
        mFaceRotation5 = 1;
        mFaceRotation1 = 1;
    }
    else if (variant == QuartzBlock::Variant::LINES_Z)
    {
        mFaceRotation2 = 1;
        mFaceRotation3 = 1;
    }
    bool rendered = tesselateBlockInWorld(state, pos);
    mFaceRotation2 = 0;
    mFaceRotation0 = 0;
    mFaceRotation1 = 0;
    mFaceRotation3 = 0;
    mFaceRotation4 = 0;
    mFaceRotation5 = 0;
    return rendered;
}

bool BlockRenderer::tesselateHorizontalDirectionalInWorld(const BlockState* state, const BlockPos& pos)
{
    const Direction* facing = state->getValue<const Direction*>(HorizontalDirectionalBlock::FACING_PROPERTY);
    if (facing == Direction::SOUTH)
    {
        mFaceTransform3 = 0;
        mFaceRotation4 = 0;
        mFaceTransform5 = 1;
        mFaceRotation1 = 2;
        mFaceRotation0 = 1;
        mFaceRotation5 = 0;
        mFaceTransform2 = 3;
        mFaceTransform0 = 3;
        bool rendered = tesselateBlockInWorld(state, pos);
        mFaceTransform0 = 0;
        mFaceTransform3 = 0;
        mFaceTransform4 = 0;
        mFaceRotation0 = 0;
        mFaceRotation1 = 0;
        mFaceTransform5 = 0;
        mFaceRotation4 = 0;
        mFaceTransform1 = 0;
        mFaceRotation2 = 0;
        mFaceTransform2 = 0;
        mFaceRotation3 = 0;
        mFaceRotation5 = 0;
        return rendered;
    }
    else if (facing == Direction::NORTH)
    {
        mFaceTransform2 = 0;
        mFaceTransform0 = 2;
        mFaceRotation1 = 1;
        mFaceRotation5 = 3;
        mFaceRotation0 = 2;
        mFaceTransform3 = 3;
        mFaceTransform5 = 1;
        mFaceRotation4 = 3;
        bool rendered = tesselateBlockInWorld(state, pos);
        mFaceTransform2 = 0;
        mFaceTransform4 = 0;
        mFaceRotation2 = 0;
        mFaceTransform1 = 0;
        mFaceRotation3 = 0;
        mFaceTransform0 = 0;
        mFaceRotation4 = 0;
        mFaceRotation5 = 0;
        mFaceRotation0 = 0;
        mFaceTransform3 = 0;
        mFaceTransform5 = 0;
        mFaceRotation1 = 0;
        return rendered;
    }
    else if (facing == Direction::WEST)
    {
        mFaceTransform2 = 2;
        mFaceRotation5 = 2;
        mFaceRotation3 = 2;
        mFaceTransform5 = 1;
        mFaceTransform1 = 3;
        mFaceRotation2 = 1;
        mFaceTransform0 = 0;
        mFaceRotation4 = 1;
        bool rendered = tesselateBlockInWorld(state, pos);
        mFaceTransform0 = 0;
        mFaceRotation0 = 0;
        mFaceRotation2 = 0;
        mFaceRotation1 = 0;
        mFaceTransform2 = 0;
        mFaceTransform5 = 0;
        mFaceTransform1 = 0;
        mFaceRotation5 = 0;
        mFaceRotation4 = 0;
        mFaceRotation3 = 0;
        mFaceTransform3 = 0;
        mFaceTransform4 = 0;
        return rendered;
    }
    else
    {
        if (facing == Direction::EAST)
        {
            mFaceTransform1 = 0;
            mFaceRotation5 = 1;
            mFaceTransform0 = 3;
            mFaceTransform2 = 2;
            mFaceTransform5 = 1;
            mFaceRotation4 = 2;
            mFaceRotation3 = 1;
            mFaceRotation2 = 2;
        }
        bool rendered = tesselateBlockInWorld(state, pos);
        mFaceTransform4 = 0;
        mFaceTransform2 = 0;
        mFaceTransform1 = 0;
        mFaceRotation2 = 0;
        mFaceTransform0 = 0;
        mFaceRotation3 = 0;
        mFaceRotation5 = 0;
        mFaceRotation4 = 0;
        mFaceTransform3 = 0;
        mFaceRotation0 = 0;
        mFaceRotation1 = 0;
        mFaceTransform5 = 0;
        return rendered;
    }
}

float BlockRenderer::getWaterHeight(const BlockPos& pos, Material* material)
{
    float total = 0.0f;
    int weight = 0;
    MutableBlockPos samplePos(0, 0, 0);
    for (int i = 0; i < 4; ++i)
    {
        int x = pos.x - (i & 1);
        int y = pos.y;
        int z = pos.z - ((i >> 1) & 1);
        samplePos.set(x, y + 1, z);
        Block* aboveLiquid = mLevelSource->getExtraBlock(samplePos);
        if (mLevelSource->getBlockState(samplePos)->getBlock()->getCraftingMaterial() == material
            || (aboveLiquid != Blocks::air && aboveLiquid->getCraftingMaterial() == material))
            return 1.0f;

        samplePos.set(x, y, z);
        const BlockState* state = mLevelSource->getBlockState(samplePos);
        Material* stateMaterial = state->getBlock()->getCraftingMaterial();
        const BlockState* liquidState = mLevelSource->getExtraBlockState(samplePos);
        Material* liquidMaterial = NULL;
        if (liquidState->getBlock() != Blocks::air)
            liquidMaterial = liquidState->getMaterial();
        if (stateMaterial == material)
        {
            int level = state->getValue(LiquidBlock::LEVEL);
            if (level >= 8 || level == 0)
            {
                total += LiquidBlock::getHeightFromData(level) * 10.0f;
                weight += 10;
            }
            total += LiquidBlock::getHeightFromData(level);
            ++weight;
        }
        else if (liquidMaterial == NULL || liquidMaterial != material)
        {
            if (!stateMaterial->isSolid() || (liquidMaterial != NULL && !liquidMaterial->isSolid()))
            {
                total += 1.0f;
                ++weight;
            }
        }
        else
        {
            int level = liquidState->getValue(LiquidBlock::LEVEL);
            if (level >= 8 || level == 0)
            {
                total += LiquidBlock::getHeightFromData(level) * 10.0f;
                weight += 10;
            }
            total += LiquidBlock::getHeightFromData(level);
            ++weight;
        }
    }
    return 1.0f - total / static_cast<float>(weight);
}

bool BlockRenderer::tesselateWaterInWorld(const BlockState* state, const BlockPos& pos)
{
    BufferBuilder* builder = Tesselator::getInstance()->getBuilder();
    Block* block = state->getBlock();
    int color = mBlockColors->getColor(state, mLevelSource, &pos, 0);
    float alpha = (float)((unsigned int)color >> 24) / 255.0f;
    float red = (float)(color >> 16 & 255) / 255.0f;
    float green = (float)(color >> 8 & 255) / 255.0f;
    float blue = (float)(color & 255) / 255.0f;
    bool top = block->shouldRenderFace(state, mLevelSource, pos, Direction::UP);
    bool bottom = block->shouldRenderFace(state, mLevelSource, pos, Direction::DOWN);
    const Direction* sides[4] = {Direction::NORTH, Direction::SOUTH, Direction::WEST, Direction::EAST};
    bool visibleSides[4];
    for (int i = 0; i < 4; ++i)
        visibleSides[i] = block->shouldRenderFace(state, mLevelSource, pos, sides[i]);
    const int buckets[4] = {2, 4, 5, 3};
    if (!top && !bottom && !visibleSides[0] && !visibleSides[1] && !visibleSides[2] && !visibleSides[3])
        return false;

    bool rendered = false;
    Material* material = block->mMaterial;
    float height[4] = {getWaterHeight(pos, material), getWaterHeight(pos.south(), material),
                       getWaterHeight(pos.offset(1, 0, 1), material), getWaterHeight(pos.east(), material)};
    int level = state->getValue(LiquidBlock::LEVEL);

    float maxHeight = height[0];
    for (int i = 1; i < 4; ++i)
    {
        if (maxHeight < height[i])
            maxHeight = height[i];
    }
    if (maxHeight <= 0.9375f)
    {
        const BlockPos neighbours[4] = {pos.north(), pos.south(), pos.west(), pos.east()};
        for (int i = 0; i < 4; ++i)
        {
            if (mLevelSource->getBlockState(neighbours[i])->getBlock() == Blocks::farmland)
                visibleSides[i] = false;
        }
    }

    const float inset = 0.0009765625f;
    float x = (float)pos.x;
    float y = (float)pos.y;
    float z = (float)pos.z;
    Block* blockAbove = Block::byId(LiquidBlock::getLiquidElement(mLevelSource, pos.above()) >> 4);
    if (blockAbove->mMaterial != block->mMaterial || mRenderAllFaces)
    {
        rendered = true;
        float slope = (float)LiquidBlock::getSlopeAngle(mLevelSource, pos, material, state);
        TextureAtlasSprite* texture = getTexture(state, slope <= -999.0f ? Direction::UP : Direction::NORTH);
        for (int i = 0; i < 4; ++i)
            height[i] -= inset;
        float u[4];
        float v[4];
        if (slope < -999.0f)
        {
            u[0] = (float)texture->getU0(true);
            v[0] = (float)texture->getV0(true);
            u[1] = u[0];
            v[1] = (float)texture->getV1(true);
            u[2] = (float)texture->getU1(true);
            v[2] = v[1];
            u[3] = u[2];
            v[3] = v[0];
        }
        else
        {
            float sine = Mth::sin(slope) * 0.25f;
            float cosine = Mth::cos(slope) * 0.25f;
            u[0] = (float)texture->getU(8.0f + (-cosine - sine) * 16.0f);
            v[0] = (float)texture->getV(8.0f + (-cosine + sine) * 16.0f);
            u[1] = (float)texture->getU(8.0f + (-cosine + sine) * 16.0f);
            v[1] = (float)texture->getV(8.0f + (cosine + sine) * 16.0f);
            u[2] = (float)texture->getU(8.0f + (cosine + sine) * 16.0f);
            v[2] = (float)texture->getV(8.0f + (cosine - sine) * 16.0f);
            u[3] = (float)texture->getU(8.0f + (cosine - sine) * 16.0f);
            v[3] = (float)texture->getV(8.0f + (-cosine - sine) * 16.0f);
        }
        builder->tex2(getLightColor(block, mLevelSource, pos));
        builder->bucket(level == 0 ? 1 : 0);
        builder->color(red, green, blue, alpha);
        builder->vertexUV(x, y + height[0], z, u[0], v[0]);
        builder->vertexUV(x, y + height[1], z + 1.0f, u[1], v[1]);
        builder->vertexUV(x + 1.0f, y + height[2], z + 1.0f, u[2], v[2]);
        builder->vertexUV(x + 1.0f, y + height[3], z, u[3], v[3]);
        builder->bucket(6);
        if (static_cast<LiquidBlock*>(block)->shouldRenderBackwardUpFace(mLevelSource, pos.above()))
        {
            builder->vertexUV(x, y + height[0], z, u[0], v[0]);
            builder->vertexUV(x + 1.0f, y + height[3], z, u[3], v[3]);
            builder->vertexUV(x + 1.0f, y + height[2], z + 1.0f, u[2], v[2]);
            builder->vertexUV(x, y + height[1], z + 1.0f, u[1], v[1]);
        }
    }
    if (bottom || mRenderAllFaces)
    {
        builder->tex2(getLightColor(block, mLevelSource, pos.below()));
        builder->color(red * 0.5f, green * 0.5f, blue * 0.5f, (float)(alpha * 0.75));
        renderFaceDown(state, pos.x, y + inset, pos.z, getTexture(block, Direction::DOWN));
        rendered = true;
    }
    for (int i = 0; i < 4; ++i)
    {
        BlockPos neighbour = pos.relative(sides[i]);
        int adjacentId = Block::getId(mLevelSource->getBlock(neighbour));
        bool stillSide = adjacentId == BlockId::GLASS || adjacentId == BlockId::ICE
                         || adjacentId == BlockId::STAINED_GLASS || adjacentId == BlockId::SLIME;
        TextureAtlasSprite* texture = getTexture(state, stillSide ? Direction::UP : Direction::from3DDataValue(i + 2));
        float sideAlpha = stillSide ? alpha * 0.75f : alpha;
        if (!visibleSides[i] && !mRenderAllFaces)
            continue;

        rendered = true;
        int first = i == 0 ? 0 : i == 1 ? 2 : i == 2 ? 1 : 3;
        int second = i == 0 ? 3 : i == 1 ? 1 : i == 2 ? 0 : 2;
        float x0;
        float z0;
        float x1;
        float z1;
        if (i == 0)
        {
            x0 = x;
            z0 = z + inset;
            x1 = x + 1.0f;
            z1 = z0;
        }
        else if (i == 1)
        {
            x0 = x + 1.0f;
            z0 = z + 1.0f - inset;
            x1 = x;
            z1 = z0;
        }
        else if (i == 2)
        {
            x0 = x + inset;
            z0 = z + 1.0f;
            x1 = x0;
            z1 = z;
        }
        else
        {
            x0 = x + 1.0f - inset;
            z0 = z;
            x1 = x0;
            z1 = z + 1.0f;
        }

        float textureScale = stillSide ? 1.0f : 0.5f;
        float u0 = (float)texture->getU(0.0f);
        float u1 = (float)texture->getU(16.0f * textureScale);
        texture->adjustU(u0, u1);
        float vFirst = (float)texture->getV((1.0f - height[first]) * 16.0f * textureScale);
        float vSecond = (float)texture->getV((1.0f - height[second]) * 16.0f * textureScale);
        float vBottom = (float)texture->getV(16.0f * textureScale);
        float vInset = 0.01f / (float)texture->getHeight();
        if (stillSide)
            vInset *= 0.5f;
        vBottom -= vInset;

        builder->tex2(getLightColor(block, mLevelSource, neighbour));
        float shade = i < 2 ? 0.8f : 0.6f;
        builder->bucket(buckets[i]);
        builder->color(red * shade, green * shade, blue * shade, sideAlpha);
        builder->vertexUV(x0, y + height[first], z0, u0, vFirst + vInset);
        builder->vertexUV(x1, y + height[second], z1, u1, vSecond + vInset);
        builder->vertexUV(x1, y, z1, u1, vBottom);
        builder->vertexUV(x0, y, z0, u0, vBottom);
        if (!stillSide)
        {
            builder->bucket(buckets[(i + 2) % 4]);
            builder->vertexUV(x0, y, z0, u0, vBottom);
            builder->vertexUV(x1, y, z1, u1, vBottom);
            builder->vertexUV(x1, y + height[second], z1, u1, vSecond + vInset);
            builder->vertexUV(x0, y + height[first], z0, u0, vFirst + vInset);
        }
    }
    mMinY = 0.0f;
    mMaxY = 1.0f;
    return rendered;
}

bool BlockRenderer::tesselateCactusInWorld(const BlockState* state, const BlockPos& pos, float red, float green,
                                           float blue)
{
    BufferBuilder* builder = Tesselator::getInstance()->getBuilder();
    Block* block = state->getBlock();
    int lightAtPos = block->getLightColor(state, mLevelSource, pos, -1);

    if (mRenderAllFaces || block->shouldRenderFace(state, mLevelSource, pos, Direction::DOWN))
    {
        if (mMinY <= 0.0f)
        {
            BlockPos lightPos = pos.offset(0, -1, 0);
            builder->tex2(block->getLightColor(state, mLevelSource, lightPos, -1));
        }
        else
        {
            builder->tex2(lightAtPos);
        }
        builder->color(red * 0.5f, green * 0.5f, blue * 0.5f);
        renderFaceDown(state, pos.x, pos.y, pos.z, getTexture(state, mLevelSource, pos, Direction::DOWN));
    }

    if (mRenderAllFaces || block->shouldRenderFace(state, mLevelSource, pos, Direction::UP))
    {
        if (1.0f <= mMaxY)
        {
            BlockPos lightPos = pos.offset(0, 1, 0);
            builder->tex2(block->getLightColor(state, mLevelSource, lightPos, -1));
        }
        else
        {
            builder->tex2(lightAtPos);
        }
        builder->color(red, green, blue);
        renderFaceUp(state, pos.x, pos.y, pos.z, getTexture(state, mLevelSource, pos, Direction::UP));
    }

    builder->tex2(lightAtPos);
    builder->color(red * 0.8f, green * 0.8f, blue * 0.8f);
    builder->addOffset(0.0f, 0.0f, 0.0625f);
    renderNorth(state, pos.x, pos.y, pos.z, getTexture(state, mLevelSource, pos, Direction::NORTH));
    builder->addOffset(0.0f, 0.0f, -0.0625f);
    builder->addOffset(0.0f, 0.0f, -0.0625f);
    renderSouth(state, pos.x, pos.y, pos.z, getTexture(state, mLevelSource, pos, Direction::SOUTH));
    builder->addOffset(0.0f, 0.0f, 0.0625f);

    builder->color(red * 0.6f, green * 0.6f, blue * 0.6f);
    builder->addOffset(0.0625f, 0.0f, 0.0f);
    renderWest(state, pos.x, pos.y, pos.z, getTexture(state, mLevelSource, pos, Direction::WEST));
    builder->addOffset(-0.0625f, 0.0f, 0.0f);
    builder->addOffset(-0.0625f, 0.0f, 0.0f);
    renderEast(state, pos.x, pos.y, pos.z, getTexture(state, mLevelSource, pos, Direction::EAST));
    builder->addOffset(0.0625f, 0.0f, 0.0f);

    return true;
}

void BlockRenderer::tesselateSunflowerCrossTexture(const BlockState* state, float x, float y, float z)
{
    BufferBuilder* builder = Tesselator::getInstance()->getBuilder();
    TextureAtlasSprite* texture = getTexture(state, Direction::UP);
    if (hasFixedTexture())
        texture = mFixedTexture;
    float u0 = (float)texture->getU0(true);
    float vMid = (float)texture->getV(8.0);
    float u1 = (float)texture->getU1(true);
    float v1 = (float)texture->getV1(true);
    double centerX = x + 0.5;
    double centerZ = z + 0.5;
    float x0 = (float)(centerX - 0.45f);
    float y1 = y + 0.5f;
    float z0 = (float)(centerZ - 0.45f);
    float z1 = (float)(centerZ + 0.45f);
    float x1 = (float)(centerX + 0.45f);
    builder->vertexUV(x0, y1, z0, u1, vMid);
    builder->vertexUV(x0, y, z0, u1, v1);
    builder->vertexUV(x1, y, z1, u0, v1);
    builder->vertexUV(x1, y1, z1, u0, vMid);
    builder->vertexUV(x1, y1, z1, u0, vMid);
    builder->vertexUV(x1, y, z1, u0, v1);
    builder->vertexUV(x0, y, z0, u1, v1);
    builder->vertexUV(x0, y1, z0, u1, vMid);
    builder->vertexUV(x0, y1, z1, u1, vMid);
    builder->vertexUV(x0, y, z1, u1, v1);
    builder->vertexUV(x1, y, z0, u0, v1);
    builder->vertexUV(x1, y1, z0, u0, vMid);
    builder->vertexUV(x1, y1, z0, u0, vMid);
    builder->vertexUV(x1, y, z0, u0, v1);
    builder->vertexUV(x0, y, z1, u1, v1);
    builder->vertexUV(x0, y1, z1, u1, vMid);
}

void BlockRenderer::tesselateSunflowerDirTexture(const BlockState* state, float x, float y, float z)
{
    BufferBuilder* builder = Tesselator::getInstance()->getBuilder();
    DoublePlantBlock* block = static_cast<DoublePlantBlock*>(state->getBlock());
    TextureAtlasSprite* sideTexture = block->getTexture(state, DoublePlantBlock::Variant::SIDE);
    TextureAtlasSprite* inventoryTexture = block->getTexture(state, DoublePlantBlock::Variant::INVENTORY);
    float sideU0 = (float)sideTexture->getU0(true);
    float sideV0 = (float)sideTexture->getV0(true);
    float sideU1 = (float)sideTexture->getU1(true);
    float sideV1 = (float)sideTexture->getV1(true);
    float invU0 = (float)inventoryTexture->getU0(true);
    float invV0 = (float)inventoryTexture->getV0(true);
    float invU1 = (float)inventoryTexture->getU1(true);
    float invV1 = (float)inventoryTexture->getV1(true);
    float angle = Mth::DEG_TO_RAD * 22.5f;
    float cosAngle = Mth::cos(angle);
    float sinAngle = Mth::sin(angle);
    float sinOffset = 0.100000024f * sinAngle;
    float cosOffset = 0.100000024f * cosAngle;
    float centerX = x + 0.5f;
    float centerY = y + 0.5f;
    float centerZ = z + 0.5f;
    float z0 = centerZ + -0.4375f;
    float leftY = centerY + (-0.5625f * cosAngle + sinOffset);
    float leftX = centerX + (cosOffset - -0.5625f * sinAngle);
    float rightX = centerX + (cosOffset - 0.4375f * sinAngle);
    float rightY = centerY + (0.4375f * cosAngle + sinOffset);
    float z1 = centerZ + 0.4375f;
    builder->vertexUV(leftX, leftY, z0, sideU0, sideV1);
    builder->vertexUV(rightX, rightY, z0, sideU0, sideV0);
    builder->vertexUV(rightX, rightY, z1, sideU1, sideV0);
    builder->vertexUV(leftX, leftY, z1, sideU1, sideV1);
    builder->vertexUV(leftX, leftY, z1, invU0, invV1);
    builder->vertexUV(rightX, rightY, z1, invU0, invV0);
    builder->vertexUV(rightX, rightY, z0, invU1, invV0);
    builder->vertexUV(leftX, leftY, z0, invU1, invV1);
}

bool BlockRenderer::tesselateDoublePlantInWorld(const BlockState* state, const BlockPos& pos)
{
    if (state->getValue(DoublePlantBlock::HALF) == DoublePlantBlock::HALVES[DoublePlantBlock::Half::UPPER])
    {
        const BlockState* below = mLevelSource->getBlockState(pos.below());
        if (below->getBlock() != Blocks::double_plant)
            return false;
    }

    BufferBuilder* builder = Tesselator::getInstance()->getBuilder();
    builder->tex2(getLightColor(state->getBlock(), mLevelSource, pos));

    int color = mBlockColors->getColor(state, mLevelSource, &pos, 0);
    float red = (float)(color >> 16 & 255) / 255.0f;
    float green = (float)(color >> 8 & 255) / 255.0f;
    float blue = (float)(color & 255) / 255.0f;
    if (GameRenderer::anaglyph3d)
    {
        float redPart = red * 30.0f;
        float newRed = (blue * 11.0f + (green * 59.0f + redPart)) / 100.0f;
        float newGreen = (green * 70.0f + redPart) / 100.0f;
        float newBlue = (blue * 70.0f + redPart) / 100.0f;
        red = newRed;
        green = newGreen;
        blue = newBlue;
    }
    builder->color(red, green, blue);

    float x = (float)pos.x;
    float y = (float)pos.y;
    float z = (float)pos.z;

    DoublePlantBlock* block = static_cast<DoublePlantBlock*>(state->getBlock());
    const BlockState* virtualState = block->fillVirtualBlockStateProperties(state, mLevelSource, pos);
    if (virtualState->getValue(DoublePlantBlock::VARIANT) == DoublePlantBlock::Variant::SUNFLOWER
        && virtualState->getValue(DoublePlantBlock::HALF) == DoublePlantBlock::HALVES[DoublePlantBlock::Half::UPPER])
    {
        tesselateSunflowerCrossTexture(virtualState, x, y, z);
        tesselateSunflowerDirTexture(virtualState, x, y, z);
    }
    else
    {
        tesselateCrossTexture(virtualState, x, y, z, 1.0f);
    }
    return true;
}

bool BlockRenderer::tesselateKelpInWorld(const BlockState* state, const BlockPos& pos)
{
    BufferBuilder* builder = Tesselator::getInstance()->getBuilder();
    builder->tex2(getLightColor(state->getBlock(), mLevelSource, pos));

    int color = mBlockColors->getColor(state, mLevelSource, &pos, 0);
    float red = (float)(color >> 16 & 255) / 255.0f;
    float green = (float)(color >> 8 & 255) / 255.0f;
    float blue = (float)(color & 255) / 255.0f;
    if (GameRenderer::anaglyph3d)
    {
        float redPart = red * 30.0f;
        float newRed = (blue * 11.0f + (green * 59.0f + redPart)) / 100.0f;
        float newGreen = (green * 70.0f + redPart) / 100.0f;
        float newBlue = (blue * 70.0f + redPart) / 100.0f;
        red = newRed;
        green = newGreen;
        blue = newBlue;
    }
    builder->color(red, green, blue);

    float x = (float)pos.x;
    float y = (float)pos.y;
    float z = (float)pos.z;

    int aboveId = mLevelSource->getBlockId((int)pos.x, (int)pos.y + 1, (int)pos.z);
    if (aboveId == 258)
    {
        int dir1 = std::abs(pos.getRandomSeed()) % 4;
        int dir2 = (dir1 + std::abs(pos.getRandomSeed()) % 3 + 1) % 4;
        TextureAtlasSprite* texture1 = getTexture(state, Direction::from3DDataValue(dir1));
        TextureAtlasSprite* texture2 = getTexture(state, Direction::from3DDataValue(dir2));
        tesselateCrossTexture(texture1, texture2, x, y, z, 1.0f);
    }
    else
    {
        TextureAtlasSprite* texture = getTexture(state, Direction::WEST);
        tesselateCrossTexture(texture, texture, x, y, z, 1.0f);
    }
    return true;
}

bool BlockRenderer::tessellateCoralFanInWorld(const BlockState* state, const BlockPos& pos)
{
    BufferBuilder* builder = Tesselator::getInstance()->getBuilder();
    builder->tex2(getLightColor(state->getBlock(), mLevelSource, pos));
    builder->color(1.0f, 1.0f, 1.0f);
    TextureAtlasSprite* texture = getTexture(state, Direction::UP);

    Vec3* origin = Vec3::newTemp(&pos);
    int seed = pos.randomSeed();
    float offsetX = ((float)(seed >> 16 & 15) / 15.0f - 0.5f) * 0.18f;
    float offsetZ = ((float)(seed >> 24 & 15) / 15.0f - 0.5f) * 0.18f;
    origin->x += offsetX;
    origin->z += offsetZ;

    float u0 = (float)texture->getU0(true);
    float v0 = (float)texture->getV0(true);
    float u1 = (float)texture->getU1(true);
    float v1 = (float)texture->getV1(true);

    float x0 = (float)origin->x;
    float z0 = (float)origin->z;
    float xMid = (float)(origin->x + 0.5);
    float zMid = (float)(origin->z + 0.5);
    float z1 = (float)(origin->z + 1.0);
    float x1 = (float)(origin->x + 1.0);
    float zLeanPositive = zMid + 0.8f;
    float xLeanNegative = xMid - 0.8f;
    float xLeanPositive = xMid + 0.8f;
    float zLeanNegative = zMid - 0.8f;

    if (state->getValue(CoralFan::DIRECTION) == 0)
    {
        builder->vertexUV(x0, (float)(origin->y + 0.35f), zLeanPositive, u0, v0);
        builder->vertexUV(x0, (float)origin->y, zMid, u0, v1);
        builder->vertexUV(x1, (float)origin->y, zMid, u1, v1);
        builder->vertexUV(x1, (float)(origin->y + 0.35f), zLeanPositive, u1, v0);
        builder->vertexUV(x0, (float)(origin->y + 0.35f), zLeanNegative, u0, v0);
        builder->vertexUV(x0, (float)origin->y, zMid, u0, v1);
        builder->vertexUV(x1, (float)origin->y, zMid, u1, v1);
        builder->vertexUV(x1, (float)(origin->y + 0.35f), zLeanNegative, u1, v0);
        builder->vertexUV(xLeanPositive, (float)(origin->y + 0.55f), z0, u0, v0);
        builder->vertexUV(xMid, (float)origin->y, z0, u0, v1);
        builder->vertexUV(xMid, (float)origin->y, z1, u1, v1);
        builder->vertexUV(xLeanPositive, (float)(origin->y + 0.55f), z1, u1, v0);
        builder->vertexUV(xLeanNegative, (float)(origin->y + 0.55f), z0, u0, v0);
        builder->vertexUV(xMid, (float)origin->y, z0, u0, v1);
        builder->vertexUV(xMid, (float)origin->y, z1, u1, v1);
        builder->vertexUV(xLeanNegative, (float)(origin->y + 0.55f), z1, u1, v0);

        builder->vertexUV(x1, (float)(origin->y + 0.35f), zLeanPositive, u1, v0);
        builder->vertexUV(x1, (float)origin->y, zMid, u1, v1);
        builder->vertexUV(x0, (float)origin->y, zMid, u0, v1);
        builder->vertexUV(x0, (float)(origin->y + 0.35f), zLeanPositive, u0, v0);
        builder->vertexUV(x1, (float)(origin->y + 0.35f), zLeanNegative, u1, v0);
        builder->vertexUV(x1, (float)origin->y, zMid, u1, v1);
        builder->vertexUV(x0, (float)origin->y, zMid, u0, v1);
        builder->vertexUV(x0, (float)(origin->y + 0.35f), zLeanNegative, u0, v0);
        builder->vertexUV(xLeanPositive, (float)(origin->y + 0.55f), z1, u1, v0);
        builder->vertexUV(xMid, (float)origin->y, z1, u1, v1);
        builder->vertexUV(xMid, (float)origin->y, z0, u0, v1);
        builder->vertexUV(xLeanPositive, (float)(origin->y + 0.55f), z0, u0, v0);
        builder->vertexUV(xLeanNegative, (float)(origin->y + 0.55f), z1, u1, v0);
        builder->vertexUV(xMid, (float)origin->y, z1, u1, v1);
        builder->vertexUV(xMid, (float)origin->y, z0, u0, v1);
        builder->vertexUV(xLeanNegative, (float)(origin->y + 0.55f), z0, u0, v0);
    }
    else
    {
        builder->vertexUV(x0, (float)(origin->y + 0.55f), zLeanPositive, u0, v0);
        builder->vertexUV(x0, (float)origin->y, zMid, u0, v1);
        builder->vertexUV(x1, (float)origin->y, zMid, u1, v1);
        builder->vertexUV(x1, (float)(origin->y + 0.55f), zLeanPositive, u1, v0);
        builder->vertexUV(x0, (float)(origin->y + 0.55f), zLeanNegative, u0, v0);
        builder->vertexUV(x0, (float)origin->y, zMid, u0, v1);
        builder->vertexUV(x1, (float)origin->y, zMid, u1, v1);
        builder->vertexUV(x1, (float)(origin->y + 0.55f), zLeanNegative, u1, v0);
        builder->vertexUV(xLeanPositive, (float)(origin->y + 0.35f), z0, u0, v0);
        builder->vertexUV(xMid, (float)origin->y, z0, u0, v1);
        builder->vertexUV(xMid, (float)origin->y, z1, u1, v1);
        builder->vertexUV(xLeanPositive, (float)(origin->y + 0.35f), z1, u1, v0);
        builder->vertexUV(xLeanNegative, (float)(origin->y + 0.35f), z0, u0, v0);
        builder->vertexUV(xMid, (float)origin->y, z0, u0, v1);
        builder->vertexUV(xMid, (float)origin->y, z1, u1, v1);
        builder->vertexUV(xLeanNegative, (float)(origin->y + 0.35f), z1, u1, v0);

        builder->vertexUV(x1, (float)(origin->y + 0.55f), zLeanPositive, u1, v0);
        builder->vertexUV(x1, (float)origin->y, zMid, u1, v1);
        builder->vertexUV(x0, (float)origin->y, zMid, u0, v1);
        builder->vertexUV(x0, (float)(origin->y + 0.55f), zLeanPositive, u0, v0);
        builder->vertexUV(x1, (float)(origin->y + 0.55f), zLeanNegative, u1, v0);
        builder->vertexUV(x1, (float)origin->y, zMid, u1, v1);
        builder->vertexUV(x0, (float)origin->y, zMid, u0, v1);
        builder->vertexUV(x0, (float)(origin->y + 0.55f), zLeanNegative, u0, v0);
        builder->vertexUV(xLeanPositive, (float)(origin->y + 0.35f), z1, u1, v0);
        builder->vertexUV(xMid, (float)origin->y, z1, u1, v1);
        builder->vertexUV(xMid, (float)origin->y, z0, u0, v1);
        builder->vertexUV(xLeanPositive, (float)(origin->y + 0.35f), z0, u0, v0);
        builder->vertexUV(xLeanNegative, (float)(origin->y + 0.35f), z1, u1, v0);
        builder->vertexUV(xMid, (float)origin->y, z1, u1, v1);
        builder->vertexUV(xMid, (float)origin->y, z0, u0, v1);
        builder->vertexUV(xLeanNegative, (float)(origin->y + 0.35f), z0, u0, v0);
    }
    return true;
}

bool BlockRenderer::tessellateCoralFanHangInWorld(const BlockState* state, const BlockPos& pos)
{
    BufferBuilder* builder = Tesselator::getInstance()->getBuilder();
    builder->tex2(getLightColor(state->getBlock(), mLevelSource, pos));
    builder->color(1.0f, 1.0f, 1.0f);
    TextureAtlasSprite* texture = getTexture(state, Direction::UP);
    MutableBlockPos position(pos);

    float u0 = (float)texture->getU0(true);
    float v0 = (float)texture->getV0(true);
    float u1 = (float)texture->getU1(true);
    float v1 = (float)texture->getV1(true);

    int direction = state->getValue(CoralFanHang::DIRECTION);
    const float farEdge = 0.95f;
    const float nearEdge = 1.0f - farEdge;
    float rootHeight = 0.5f;
    float tipHeight = 0.65f;
    if (direction == 0)
    {
        if (mLevelSource->getBlockState(pos.east())->getBlock() != Blocks::coral_block)
        {
            tipHeight = 0.85f;
            rootHeight = 0.75f;
        }
        float rootX = (float)position.x + 0.99f;
        float tipX = (float)(rootX - 0.65f);
        float z0 = (float)position.z + nearEdge;
        float z1 = (float)position.z + farEdge;
        float tipY = (float)position.y + tipHeight;
        float rootY = (float)position.y + rootHeight;
        builder->vertexUV(tipX, tipY, z0, u0, v0);
        builder->vertexUV(rootX, rootY, z0, u0, v1);
        builder->vertexUV(rootX, rootY, z1, u1, v1);
        builder->vertexUV(tipX, tipY, z1, u1, v0);
        builder->vertexUV(tipX, tipY, z1, u1, v0);
        builder->vertexUV(rootX, rootY, z1, u1, v1);
        builder->vertexUV(rootX, rootY, z0, u0, v1);
        builder->vertexUV(tipX, tipY, z0, u0, v0);
    }
    else if (direction == 1)
    {
        if (mLevelSource->getBlockState(pos.west())->getBlock() != Blocks::coral_block)
        {
            tipHeight = 0.85f;
            rootHeight = 0.75f;
        }
        float rootX = (float)position.x + 0.01f;
        float tipX = (float)(rootX + 0.65f);
        float z0 = (float)position.z + nearEdge;
        float z1 = (float)position.z + farEdge;
        float tipY = (float)position.y + tipHeight;
        float rootY = (float)position.y + rootHeight;
        builder->vertexUV(tipX, tipY, z0, u0, v0);
        builder->vertexUV(rootX, rootY, z0, u0, v1);
        builder->vertexUV(rootX, rootY, z1, u1, v1);
        builder->vertexUV(tipX, tipY, z1, u1, v0);
        builder->vertexUV(tipX, tipY, z1, u1, v0);
        builder->vertexUV(rootX, rootY, z1, u1, v1);
        builder->vertexUV(rootX, rootY, z0, u0, v1);
        builder->vertexUV(tipX, tipY, z0, u0, v0);
    }
    else if (direction == 2)
    {
        if (mLevelSource->getBlockState(pos.south())->getBlock() != Blocks::coral_block)
        {
            tipHeight = 0.85f;
            rootHeight = 0.75f;
        }
        float x0 = (float)position.x + farEdge;
        float rootZ = (float)position.z + 0.99f;
        float tipZ = (float)(rootZ - 0.65f);
        float x1 = (float)position.x + nearEdge;
        float tipY = (float)position.y + tipHeight;
        float rootY = (float)position.y + rootHeight;
        builder->vertexUV(x0, tipY, tipZ, u0, v0);
        builder->vertexUV(x0, rootY, rootZ, u0, v1);
        builder->vertexUV(x1, rootY, rootZ, u1, v1);
        builder->vertexUV(x1, tipY, tipZ, u1, v0);
        builder->vertexUV(x1, tipY, tipZ, u1, v0);
        builder->vertexUV(x1, rootY, rootZ, u1, v1);
        builder->vertexUV(x0, rootY, rootZ, u0, v1);
        builder->vertexUV(x0, tipY, tipZ, u0, v0);
    }
    else
    {
        if (mLevelSource->getBlockState(pos.north())->getBlock() != Blocks::coral_block)
        {
            tipHeight = 0.85f;
            rootHeight = 0.75f;
        }
        float x0 = (float)position.x + farEdge;
        float rootZ = (float)position.z + 0.01f;
        float tipZ = (float)(rootZ + 0.65f);
        float x1 = (float)position.x + nearEdge;
        float tipY = (float)position.y + tipHeight;
        float rootY = (float)position.y + rootHeight;
        builder->vertexUV(x0, tipY, tipZ, u0, v0);
        builder->vertexUV(x0, rootY, rootZ, u0, v1);
        builder->vertexUV(x1, rootY, rootZ, u1, v1);
        builder->vertexUV(x1, tipY, tipZ, u1, v0);
        builder->vertexUV(x1, tipY, tipZ, u1, v0);
        builder->vertexUV(x1, rootY, rootZ, u1, v1);
        builder->vertexUV(x0, rootY, rootZ, u0, v1);
        builder->vertexUV(x0, tipY, tipZ, u0, v0);
    }
    return true;
}

void BlockRenderer::tesselateStemDirTexture(const BlockState* state, const Direction* direction, float height, float x,
                                            float y, float z)
{
    BufferBuilder* builder = Tesselator::getInstance()->getBuilder();
    StemBlock* stemBlock = static_cast<StemBlock*>(state->getBlock());
    TextureAtlasSprite* texture = stemBlock->getAngledTexture();
    if (hasFixedTexture())
        texture = mFixedTexture;

    float u0 = (float)texture->getU0(true);
    float v0 = (float)texture->getV0(true);
    float u1 = (float)texture->getU1(true);
    float v1 = (float)texture->getV1(true);

    float xPlus1 = x + 1.0f;
    float xHalf = x + 0.5f;
    float zHalf = z + 0.5f;
    float zPlus1 = z + 1.0f;
    float yTop = height + y;

    float ua = u0;
    float ub = u1;
    if (direction == Direction::NORTH || direction == Direction::EAST)
    {
        ua = u1;
        ub = u0;
    }

    if (direction == Direction::WEST || direction == Direction::EAST)
    {
        builder->vertexUV(x, yTop, zHalf, ua, v0);
        builder->vertexUV(x, y, zHalf, ua, v1);
        builder->vertexUV(xPlus1, y, zHalf, ub, v1);
        builder->vertexUV(xPlus1, yTop, zHalf, ub, v0);
        builder->vertexUV(xPlus1, yTop, zHalf, ub, v0);
        builder->vertexUV(xPlus1, y, zHalf, ub, v1);
        builder->vertexUV(x, y, zHalf, ua, v1);
        builder->vertexUV(x, yTop, zHalf, ua, v0);
    }
    else
    {
        builder->vertexUV(xHalf, yTop, zPlus1, ua, v0);
        builder->vertexUV(xHalf, y, zPlus1, ua, v1);
        builder->vertexUV(xHalf, y, z, ub, v1);
        builder->vertexUV(xHalf, yTop, z, ub, v0);
        builder->vertexUV(xHalf, yTop, z, ub, v0);
        builder->vertexUV(xHalf, y, z, ub, v1);
        builder->vertexUV(xHalf, y, zPlus1, ua, v1);
        builder->vertexUV(xHalf, yTop, zPlus1, ua, v0);
    }
}

bool BlockRenderer::tesselateStemInWorld(const BlockState* state, const BlockPos& pos)
{
    Block* block = state->getBlock();
    BufferBuilder* builder = Tesselator::getInstance()->getBuilder();
    builder->tex2(getLightColor(block, mLevelSource, pos));

    int color = mBlockColors->getColor(state, mLevelSource, &pos, 0);
    float red = (float)(color >> 16 & 255) / 255.0f;
    float green = (float)(color >> 8 & 255) / 255.0f;
    float blue = (float)(color & 255) / 255.0f;
    if (GameRenderer::anaglyph3d)
    {
        float redPart = red * 30.0f;
        float newRed = (blue * 11.0f + (green * 59.0f + redPart)) / 100.0f;
        float newGreen = (green * 70.0f + redPart) / 100.0f;
        float newBlue = (blue * 70.0f + redPart) / 100.0f;
        red = newRed;
        green = newGreen;
        blue = newBlue;
    }
    builder->color(red, green, blue);

    float x = (float)pos.x;
    float y = (float)pos.y - 0.0625f;
    float z = (float)pos.z;

    const Direction* direction = state->getValue<const Direction*>(StemBlock::FACING);
    if (direction == Direction::UP)
    {
        tesselateStemTexture(state, mMaxY, x, y, z);
    }
    else
    {
        tesselateStemTexture(state, 0.5f, x, y, z);
        tesselateStemDirTexture(state, direction, mMaxY, x, y, z);
    }
    return true;
}

bool BlockRenderer::tesselateLilypadInWorld(const BlockState* state, const BlockPos& pos)
{
    BufferBuilder* builder = Tesselator::getInstance()->getBuilder();
    TextureAtlasSprite* texture = getTexture(state, Direction::UP);
    if (hasFixedTexture())
        texture = mFixedTexture;

    float u0 = (float)texture->getU0(true);
    float v0 = (float)texture->getV0(true);
    float u1 = (float)texture->getU1(true);
    float v1 = (float)texture->getV1(true);

    unsigned int hash = (unsigned int)pos.x * 0x2fc20fu ^ (unsigned int)pos.z * 0x6ebfff5u ^ (unsigned int)pos.y;
    hash = hash * hash * 0x285b825u + hash * 0xbu >> 0x10;
    unsigned int rotation = hash & 3;
    unsigned int rotation1 = rotation + 1;

    builder->tex2(getLightColor(state->getBlock(), mLevelSource, pos));

    float x0 = (float)pos.x + 0.5f;
    float z0 = (float)pos.z + 0.5f;
    float y = (float)pos.y + 0.015625f;
    float xOff = (float)(int)(rotation & 1) * 0.5f * (float)((int)(rotation >> 1) * -2 + 1);
    float zOff = (float)(int)(rotation1 & 1) * 0.5f * (float)((int)((rotation1 >> 1) & 1) * -2 + 1);

    float a = x0 + xOff;
    float b = z0 + xOff;
    float c = x0 - xOff;
    float d = z0 - xOff;

    int color = mBlockColors->getColor(state, mLevelSource, &pos, 0);
    builder->color(color);
    builder->vertexUV(a - zOff, y, b + zOff, u0, v0);
    builder->vertexUV(a + zOff, y, d + zOff, u1, v0);
    builder->vertexUV(c + zOff, y, d - zOff, u1, v1);
    builder->vertexUV(c - zOff, y, b - zOff, u0, v1);

    color = mBlockColors->getColor(state, mLevelSource, &pos, 0);
    builder->color((color & 0xfefefe) >> 1);
    builder->vertexUV(c - zOff, y, b - zOff, u0, v1);
    builder->vertexUV(c + zOff, y, d - zOff, u1, v1);
    builder->vertexUV(a + zOff, y, d + zOff, u1, v0);
    builder->vertexUV(a - zOff, y, b + zOff, u0, v0);

    return true;
}

bool BlockRenderer::tesselateTorchInWorld(const BlockState* state, const BlockPos& pos)
{
    const Direction* facing = state->getValue(TorchBlock::FACING);
    BufferBuilder* builder = Tesselator::getInstance()->getBuilder();
    builder->tex2(getLightColor(state->getBlock(), mLevelSource, pos));
    builder->color(1.0f, 1.0f, 1.0f);
    const float x = (float)pos.x;
    const float y = (float)pos.y;
    const float z = (float)pos.z;
    const float offset = 0.099999994f;
    if (facing == Direction::EAST)
        tesselateTorch(state, x - offset, y + 0.2f, z, -0.4f, 0.0f);
    else if (facing == Direction::WEST)
        tesselateTorch(state, x + offset, y + 0.2f, z, 0.4f, 0.0f);
    else if (facing == Direction::SOUTH)
        tesselateTorch(state, x, y + 0.2f, z - offset, 0.0f, -0.4f);
    else if (facing == Direction::NORTH)
        tesselateTorch(state, x, y + 0.2f, z + offset, 0.0f, 0.4f);
    else
        tesselateTorch(state, x, y, z, 0.0f, 0.0f);
    return true;
}

bool BlockRenderer::tesselateFireInWorld(const BlockState* state, const BlockPos& pos)
{
    BufferBuilder* builder = Tesselator::getInstance()->getBuilder();
    FireBlock* block = static_cast<FireBlock*>(state->getBlock());
    TextureAtlasSprite* firstTex = block->getTextureLayer(0);
    TextureAtlasSprite* secondTex = block->getTextureLayer(1);
    TextureAtlasSprite* tex = firstTex;
    if (hasFixedTexture())
        tex = mFixedTexture;

    builder->color(1.0f, 1.0f, 1.0f);
    builder->tex2(getLightColor(block, mLevelSource, pos));
    float u0 = tex->getU0(true);
    float v0 = tex->getV0(true);
    float u1 = tex->getU1(true);
    float v1 = tex->getV1(true);
    float h = 1.4f;
    int x = pos.x;
    int y = pos.y;
    int z = pos.z;

    if (mLevelSource->getBlockState(pos.below())->isTopSolidBlocking()
        || static_cast<FireBlock*>(Blocks::fire)->canBurn(mLevelSource, pos.below()))
    {
        float x0 = x + (0.5f + 0.2f);
        float x1 = x + (0.5f - 0.2f);
        float z0 = z + (0.5f + 0.2f);
        float z1 = z + (0.5f - 0.2f);
        float x2 = x + (0.5f - 0.3f);
        float x3 = x + (0.5f + 0.3f);
        float z2 = z + (0.5f - 0.3f);
        float z3 = z + (0.5f + 0.3f);
        builder->vertexUV(x2, y + h, z + 1, u1, v0);
        builder->vertexUV(x0, y + 0.0f, z + 1, u1, v1);
        builder->vertexUV(x0, y + 0.0f, z + 0, u0, v1);
        builder->vertexUV(x2, y + h, z + 0, u0, v0);

        builder->vertexUV(x3, y + h, z + 0, u1, v0);
        builder->vertexUV(x1, y + 0.0f, z + 0, u1, v1);
        builder->vertexUV(x1, y + 0.0f, z + 1, u0, v1);
        builder->vertexUV(x3, y + h, z + 1, u0, v0);

        u0 = secondTex->getU0(true);
        v0 = secondTex->getV0(true);
        u1 = secondTex->getU1(true);
        v1 = secondTex->getV1(true);

        builder->vertexUV(x + 1, y + h, z3, u1, v0);
        builder->vertexUV(x + 1, y + 0.0f, z1, u1, v1);
        builder->vertexUV(x + 0, y + 0.0f, z1, u0, v1);
        builder->vertexUV(x + 0, y + h, z3, u0, v0);

        builder->vertexUV(x + 0, y + h, z2, u1, v0);
        builder->vertexUV(x + 0, y + 0.0f, z0, u1, v1);
        builder->vertexUV(x + 1, y + 0.0f, z0, u0, v1);
        builder->vertexUV(x + 1, y + h, z2, u0, v0);

        x0 = x + (0.5f - 0.4f);
        x1 = x + (0.5f + 0.4f);
        z0 = z + (0.5f - 0.4f);
        z1 = z + (0.5f + 0.4f);

        builder->vertexUV(x0, y + h, z + 0, u0, v0);
        builder->vertexUV(x + 0, y + 0.0f, z + 0, u0, v1);
        builder->vertexUV(x + 0, y + 0.0f, z + 1, u1, v1);
        builder->vertexUV(x0, y + h, z + 1, u1, v0);

        builder->vertexUV(x1, y + h, z + 1, u0, v0);
        builder->vertexUV(x + 1.0f, y + 0.0f, z + 1, u0, v1);
        builder->vertexUV(x + 1.0f, y + 0.0f, z + 0, u1, v1);
        builder->vertexUV(x1, y + h, z + 0, u1, v0);

        u0 = firstTex->getU0(true);
        v0 = firstTex->getV0(true);
        u1 = firstTex->getU1(true);
        v1 = firstTex->getV1(true);

        builder->vertexUV(x + 0, y + h, z1, u0, v0);
        builder->vertexUV(x + 0, y + 0.0f, z + 1.0f, u0, v1);
        builder->vertexUV(x + 1, y + 0.0f, z + 1.0f, u1, v1);
        builder->vertexUV(x + 1, y + h, z1, u1, v0);

        builder->vertexUV(x + 1, y + h, z0, u0, v0);
        builder->vertexUV(x + 1, y + 0.0f, z + 0, u0, v1);
        builder->vertexUV(x + 0, y + 0.0f, z + 0, u1, v1);
        builder->vertexUV(x + 0, y + h, z0, u1, v0);
    }
    else
    {
        float r = 0.2f;
        float yo = 1.0f / 16.0f;
        if (((x + y + z) & 1) == 1)
        {
            u0 = secondTex->getU0(true);
            v0 = secondTex->getV0(true);
            u1 = secondTex->getU1(true);
            v1 = secondTex->getV1(true);
        }
        if (((x / 2 + y / 2 + z / 2) & 1) == 1)
        {
            float tmp = u1;
            u1 = u0;
            u0 = tmp;
        }

        if (static_cast<FireBlock*>(Blocks::fire)->canBurn(mLevelSource, pos.west()))
        {
            builder->vertexUV(x + r, y + h + yo, z + 1.0f, u1, v0);
            builder->vertexUV(x + 0.0f, y + 0.0f + yo, z + 1.0f, u1, v1);
            builder->vertexUV(x + 0.0f, y + 0.0f + yo, z + 0.0f, u0, v1);
            builder->vertexUV(x + r, y + h + yo, z + 0.0f, u0, v0);

            builder->vertexUV(x + r, y + h + yo, z + 0.0f, u0, v0);
            builder->vertexUV(x + 0.0f, y + 0.0f + yo, z + 0.0f, u0, v1);
            builder->vertexUV(x + 0.0f, y + 0.0f + yo, z + 1.0f, u1, v1);
            builder->vertexUV(x + r, y + h + yo, z + 1.0f, u1, v0);
        }
        if (static_cast<FireBlock*>(Blocks::fire)->canBurn(mLevelSource, pos.east()))
        {
            builder->vertexUV(x + 1 - r, y + h + yo, z + 0.0f, u0, v0);
            builder->vertexUV(x + 1 - 0.0f, y + 0.0f + yo, z + 0.0f, u0, v1);
            builder->vertexUV(x + 1 - 0.0f, y + 0.0f + yo, z + 1.0f, u1, v1);
            builder->vertexUV(x + 1 - r, y + h + yo, z + 1.0f, u1, v0);

            builder->vertexUV(x + 1.0f - r, y + h + yo, z + 1.0f, u1, v0);
            builder->vertexUV(x + 1.0f - 0.0f, y + 0.0f + yo, z + 1.0f, u1, v1);
            builder->vertexUV(x + 1.0f - 0.0f, y + 0.0f + yo, z + 0.0f, u0, v1);
            builder->vertexUV(x + 1.0f - r, y + h + yo, z + 0.0f, u0, v0);
        }
        if (static_cast<FireBlock*>(Blocks::fire)->canBurn(mLevelSource, pos.north()))
        {
            builder->vertexUV(x + 0.0f, y + h + yo, z + r, u1, v0);
            builder->vertexUV(x + 0.0f, y + 0.0f + yo, z + 0.0f, u1, v1);
            builder->vertexUV(x + 1.0f, y + 0.0f + yo, z + 0.0f, u0, v1);
            builder->vertexUV(x + 1.0f, y + h + yo, z + r, u0, v0);

            builder->vertexUV(x + 1.0f, y + h + yo, z + r, u0, v0);
            builder->vertexUV(x + 1.0f, y + 0.0f + yo, z + 0.0f, u0, v1);
            builder->vertexUV(x + 0.0f, y + 0.0f + yo, z + 0.0f, u1, v1);
            builder->vertexUV(x + 0.0f, y + h + yo, z + r, u1, v0);
        }
        if (static_cast<FireBlock*>(Blocks::fire)->canBurn(mLevelSource, pos.south()))
        {
            builder->vertexUV(x + 1.0f, y + h + yo, z + 1.0f - r, u0, v0);
            builder->vertexUV(x + 1.0f, y + 0.0f + yo, z + 1.0f - 0.0f, u0, v1);
            builder->vertexUV(x + 0.0f, y + 0.0f + yo, z + 1.0f - 0.0f, u1, v1);
            builder->vertexUV(x + 0.0f, y + h + yo, z + 1.0f - r, u1, v0);

            builder->vertexUV(x + 0.0f, y + h + yo, z + 1.0f - r, u1, v0);
            builder->vertexUV(x + 0.0f, y + 0.0f + yo, z + 1.0f - 0.0f, u1, v1);
            builder->vertexUV(x + 1.0f, y + 0.0f + yo, z + 1.0f - 0.0f, u0, v1);
            builder->vertexUV(x + 1.0f, y + h + yo, z + 1.0f - r, u0, v0);
        }
        if (static_cast<FireBlock*>(Blocks::fire)->canBurn(mLevelSource, pos.above()))
        {
            float x0 = x + 1.0f;
            float x1 = x + 0.0f;
            float z0 = z + 0.0f;
            float z1 = z + 1.0f;
            u0 = firstTex->getU0(true);
            v0 = firstTex->getV0(true);
            u1 = firstTex->getU1(true);
            v1 = firstTex->getV1(true);
            float yt = y + 1;
            float yb = yt + -0.2f;
            if (((x + (y + 1) + z) & 1) == 0)
            {
                builder->vertexUV(x1, yb, z0, u1, v0);
                builder->vertexUV(x0, yt, z0, u1, v1);
                builder->vertexUV(x0, yt, z + 1, u0, v1);
                builder->vertexUV(x1, yb, z + 1, u0, v0);
                u0 = secondTex->getU0(true);
                v0 = secondTex->getV0(true);
                u1 = secondTex->getU1(true);
                v1 = secondTex->getV1(true);
                builder->vertexUV(x0, yb, z1, u1, v0);
                builder->vertexUV(x1, yt, z1, u1, v1);
                builder->vertexUV(x1, yt, z0, u0, v1);
                builder->vertexUV(x0, yb, z0, u0, v0);
            }
            else
            {
                builder->vertexUV(x1, yb, z1, u1, v0);
                builder->vertexUV(x1, yt, z0, u1, v1);
                builder->vertexUV(x0, yt, z0, u0, v1);
                builder->vertexUV(x0, yb, z1, u0, v0);
                u0 = secondTex->getU0(true);
                v0 = secondTex->getV0(true);
                u1 = secondTex->getU1(true);
                v1 = secondTex->getV1(true);
                builder->vertexUV(x0, yb, z0, u1, v0);
                builder->vertexUV(x0, yt, z1, u1, v1);
                builder->vertexUV(x1, yt, z1, u0, v1);
                builder->vertexUV(x1, yb, z0, u0, v0);
            }
        }
    }
    return true;
}

bool BlockRenderer::tesselateDustInWorld(const BlockState* state, const BlockPos& pos)
{
    BufferBuilder* builder = Tesselator::getInstance()->getBuilder();
    const BlockState* wireState =
        static_cast<RedStoneWireBlock*>(state->getBlock())->fillVirtualBlockStateProperties(state, mLevelSource, pos);
    TextureAtlasSprite* cross = RedStoneWireBlock::getTexture(RedStoneWireBlock::TEXTURE_CROSS);
    TextureAtlasSprite* line = RedStoneWireBlock::getTexture(RedStoneWireBlock::TEXTURE_LINE);
    TextureAtlasSprite* crossOverlay = RedStoneWireBlock::getTexture(RedStoneWireBlock::TEXTURE_CROSS_OVERLAY);
    TextureAtlasSprite* lineOverlay = RedStoneWireBlock::getTexture(RedStoneWireBlock::TEXTURE_LINE_OVERLAY);
    builder->tex2(getLightColor(wireState->getBlock(), mLevelSource, pos));

    const int power = wireState->getValue(RedStoneWireBlock::POWER);
    float red;
    float green;
    float blue;
    if (power == 0)
    {
        const unsigned int colour = Minecraft::GetInstance()->getColourTable()->getColour(
            eMinecraftColour_Tile_RedstoneDustUnlit);
        red = (float)(colour >> 16 & 0xff) / 255.0f;
        green = (float)(colour >> 8 & 0xff) / 255.0f;
        blue = (float)(colour & 0xff) / 255.0f;
    }
    else
    {
        const unsigned int minColour = Minecraft::GetInstance()->getColourTable()->getColour(
            eMinecraftColour_Tile_RedstoneDustLitMin);
        const unsigned int maxColour = Minecraft::GetInstance()->getColourTable()->getColour(
            eMinecraftColour_Tile_RedstoneDustLitMax);
        const float fraction = (float)(power - 1) / 14.0f;
        red = (float)((int)((float)((maxColour >> 16) & 0xff - (minColour >> 16) & 0xff) * fraction
                            + (float)(minColour >> 16 & 0xff))
                      & 0xff)
              / 255.0f;
        green = (float)((int)((float)((maxColour >> 8) & 0xff - (minColour >> 8) & 0xff) * fraction
                              + (float)(minColour >> 8 & 0xff))
                        & 0xff)
                / 255.0f;
        blue = (float)((int)((float)(maxColour & 0xff - minColour & 0xff) * fraction + (float)(minColour & 0xff))
                       & 0xff)
               / 255.0f;
    }
    builder->color(red, green, blue);

    RedStoneWireBlock::ConnectingSide* westSide = wireState->getValue(RedStoneWireBlock::WEST);
    const bool westUp = westSide == RedStoneWireBlock::UP;
    const bool west = westSide == RedStoneWireBlock::SIDE || westUp;
    RedStoneWireBlock::ConnectingSide* eastSide = wireState->getValue(RedStoneWireBlock::EAST);
    const bool eastUp = eastSide == RedStoneWireBlock::UP;
    const bool east = eastSide == RedStoneWireBlock::SIDE || eastUp;
    RedStoneWireBlock::ConnectingSide* northSide = wireState->getValue(RedStoneWireBlock::NORTH);
    const bool northUp = northSide == RedStoneWireBlock::UP;
    const bool north = northSide == RedStoneWireBlock::SIDE || northUp;
    RedStoneWireBlock::ConnectingSide* southSide = wireState->getValue(RedStoneWireBlock::SOUTH);
    const bool southUp = southSide == RedStoneWireBlock::UP;
    const bool south = southSide == RedStoneWireBlock::SIDE || southUp;

    const float wallOffset = 0.015625f;
    float x0 = (float)pos.x;
    float z0 = (float)pos.z;
    float x1 = x0 + 1.0f;
    float z1 = z0 + 1.0f;

    int mode = 0;
    if (north)
    {
        if (!east && !west)
            mode = 2;
    }
    else if (!west && !east)
    {
        if (south)
            mode = 2;
    }
    else if (!south)
    {
        mode = 1;
    }

    const float y = (float)pos.y;
    const float top = y + wallOffset;
    if (mode == 0)
    {
        int u0Texel = 0;
        int u1Texel = 16;
        int v0Texel = 0;
        int v1Texel = 16;
        if (!west)
        {
            x0 += 0.3125f;
            u0Texel = 5;
        }
        if (!east)
        {
            x1 -= 0.3125f;
            u1Texel = 11;
        }
        if (!north)
        {
            z0 += 0.3125f;
            v0Texel = 5;
        }
        if (!south)
        {
            z1 -= 0.3125f;
            v1Texel = 11;
        }

        float u0 = (float)cross->getU((double)u0Texel);
        float u1 = (float)cross->getU((double)u1Texel);
        float v0 = (float)cross->getV((double)v0Texel);
        float v1 = (float)cross->getV((double)v1Texel);
        cross->adjustUVs(u0, u1, v0, v1);
        builder->vertexUV(x1, top, z1, u1, v1);
        builder->vertexUV(x1, top, z0, u1, v0);
        builder->vertexUV(x0, top, z0, u0, v0);
        builder->vertexUV(x0, top, z1, u0, v1);

        float overlayU0 = (float)crossOverlay->getU((double)u0Texel);
        float overlayU1 = (float)crossOverlay->getU((double)u1Texel);
        float overlayV0 = (float)crossOverlay->getV((double)v0Texel);
        float overlayV1 = (float)crossOverlay->getV((double)v1Texel);
        crossOverlay->adjustUVs(overlayU0, overlayU1, overlayV0, overlayV1);
        builder->color(1.0f, 1.0f, 1.0f);
        builder->vertexUV(x1, top, z1, overlayU1, overlayV1);
        builder->vertexUV(x1, top, z0, overlayU1, overlayV0);
        builder->vertexUV(x0, top, z0, overlayU0, overlayV0);
        builder->vertexUV(x0, top, z1, overlayU0, overlayV1);
    }
    else if (mode == 1)
    {
        builder->vertexUV(x1, top, z1, (float)line->getU1(true), (float)line->getV1(true));
        builder->vertexUV(x1, top, z0, (float)line->getU1(true), (float)line->getV0(true));
        builder->vertexUV(x0, top, z0, (float)line->getU0(true), (float)line->getV0(true));
        builder->vertexUV(x0, top, z1, (float)line->getU0(true), (float)line->getV1(true));
        builder->color(1.0f, 1.0f, 1.0f);
        builder->vertexUV(x1, top, z1, (float)lineOverlay->getU1(true), (float)lineOverlay->getV1(true));
        builder->vertexUV(x1, top, z0, (float)lineOverlay->getU1(true), (float)lineOverlay->getV0(true));
        builder->vertexUV(x0, top, z0, (float)lineOverlay->getU0(true), (float)lineOverlay->getV0(true));
        builder->vertexUV(x0, top, z1, (float)lineOverlay->getU0(true), (float)lineOverlay->getV1(true));
    }
    else
    {
        builder->vertexUV(x1, top, z1, (float)line->getU1(true), (float)line->getV1(true));
        builder->vertexUV(x1, top, z0, (float)line->getU0(true), (float)line->getV1(true));
        builder->vertexUV(x0, top, z0, (float)line->getU0(true), (float)line->getV0(true));
        builder->vertexUV(x0, top, z1, (float)line->getU1(true), (float)line->getV0(true));
        builder->color(1.0f, 1.0f, 1.0f);
        builder->vertexUV(x1, top, z1, (float)lineOverlay->getU1(true), (float)lineOverlay->getV1(true));
        builder->vertexUV(x1, top, z0, (float)lineOverlay->getU0(true), (float)lineOverlay->getV1(true));
        builder->vertexUV(x0, top, z0, (float)lineOverlay->getU0(true), (float)lineOverlay->getV0(true));
        builder->vertexUV(x0, top, z1, (float)lineOverlay->getU1(true), (float)lineOverlay->getV0(true));
    }

    const float wallTop = (float)(pos.y + 1) + 0.021875f;
    if (westUp)
    {
        const float wallX = (float)pos.x + wallOffset;
        const float wallZ0 = (float)pos.z;
        const float wallZ1 = (float)(pos.z + 1);
        builder->color(red, green, blue);
        builder->vertexUV(wallX, wallTop, wallZ1, (float)line->getU1(true), (float)line->getV0(true));
        builder->vertexUV(wallX, y, wallZ1, (float)line->getU0(true), (float)line->getV0(true));
        builder->vertexUV(wallX, y, wallZ0, (float)line->getU0(true), (float)line->getV1(true));
        builder->vertexUV(wallX, wallTop, wallZ0, (float)line->getU1(true), (float)line->getV1(true));
        builder->color(1.0f, 1.0f, 1.0f);
        builder->vertexUV(wallX, wallTop, wallZ1, (float)lineOverlay->getU1(true), (float)lineOverlay->getV0(true));
        builder->vertexUV(wallX, y, wallZ1, (float)lineOverlay->getU0(true), (float)lineOverlay->getV0(true));
        builder->vertexUV(wallX, y, wallZ0, (float)lineOverlay->getU0(true), (float)lineOverlay->getV1(true));
        builder->vertexUV(wallX, wallTop, wallZ0, (float)lineOverlay->getU1(true), (float)lineOverlay->getV1(true));
    }
    if (eastUp)
    {
        const float wallX = (float)(pos.x + 1) - wallOffset;
        const float wallZ0 = (float)pos.z;
        const float wallZ1 = (float)(pos.z + 1);
        builder->color(red, green, blue);
        builder->vertexUV(wallX, y, wallZ1, (float)line->getU0(true), (float)line->getV1(true));
        builder->vertexUV(wallX, wallTop, wallZ1, (float)line->getU1(true), (float)line->getV1(true));
        builder->vertexUV(wallX, wallTop, wallZ0, (float)line->getU1(true), (float)line->getV0(true));
        builder->vertexUV(wallX, y, wallZ0, (float)line->getU0(true), (float)line->getV0(true));
        builder->color(1.0f, 1.0f, 1.0f);
        builder->vertexUV(wallX, y, wallZ1, (float)lineOverlay->getU0(true), (float)lineOverlay->getV1(true));
        builder->vertexUV(wallX, wallTop, wallZ1, (float)lineOverlay->getU1(true), (float)lineOverlay->getV1(true));
        builder->vertexUV(wallX, wallTop, wallZ0, (float)lineOverlay->getU1(true), (float)lineOverlay->getV0(true));
        builder->vertexUV(wallX, y, wallZ0, (float)lineOverlay->getU0(true), (float)lineOverlay->getV0(true));
    }
    if (northUp)
    {
        const float wallX0 = (float)pos.x;
        const float wallX1 = (float)(pos.x + 1);
        const float wallZ = (float)pos.z + wallOffset;
        builder->color(red, green, blue);
        builder->vertexUV(wallX1, y, wallZ, (float)line->getU0(true), (float)line->getV1(true));
        builder->vertexUV(wallX1, wallTop, wallZ, (float)line->getU1(true), (float)line->getV1(true));
        builder->vertexUV(wallX0, wallTop, wallZ, (float)line->getU1(true), (float)line->getV0(true));
        builder->vertexUV(wallX0, y, wallZ, (float)line->getU0(true), (float)line->getV0(true));
        builder->color(1.0f, 1.0f, 1.0f);
        builder->vertexUV(wallX1, y, wallZ, (float)lineOverlay->getU0(true), (float)lineOverlay->getV1(true));
        builder->vertexUV(wallX1, wallTop, wallZ, (float)lineOverlay->getU1(true), (float)lineOverlay->getV1(true));
        builder->vertexUV(wallX0, wallTop, wallZ, (float)lineOverlay->getU1(true), (float)lineOverlay->getV0(true));
        builder->vertexUV(wallX0, y, wallZ, (float)lineOverlay->getU0(true), (float)lineOverlay->getV0(true));
    }
    if (southUp)
    {
        const float wallX0 = (float)pos.x;
        const float wallX1 = (float)(pos.x + 1);
        const float wallZ = (float)(pos.z + 1) - wallOffset;
        builder->color(red, green, blue);
        builder->vertexUV(wallX1, wallTop, wallZ, (float)line->getU1(true), (float)line->getV0(true));
        builder->vertexUV(wallX1, y, wallZ, (float)line->getU0(true), (float)line->getV0(true));
        builder->vertexUV(wallX0, y, wallZ, (float)line->getU0(true), (float)line->getV1(true));
        builder->vertexUV(wallX0, wallTop, wallZ, (float)line->getU1(true), (float)line->getV1(true));
        builder->color(1.0f, 1.0f, 1.0f);
        builder->vertexUV(wallX1, wallTop, wallZ, (float)lineOverlay->getU1(true), (float)lineOverlay->getV0(true));
        builder->vertexUV(wallX1, y, wallZ, (float)lineOverlay->getU0(true), (float)lineOverlay->getV0(true));
        builder->vertexUV(wallX0, y, wallZ, (float)lineOverlay->getU0(true), (float)lineOverlay->getV1(true));
        builder->vertexUV(wallX0, wallTop, wallZ, (float)lineOverlay->getU1(true), (float)lineOverlay->getV1(true));
    }
    return true;
}

bool BlockRenderer::tesselateLadderInWorld(const BlockState* state, const BlockPos& pos)
{
    BufferBuilder* builder = Tesselator::getInstance()->getBuilder();
    TextureAtlasSprite* texture = getTexture(state, NULL);
    if (hasFixedTexture())
        texture = mFixedTexture;

    builder->tex2(getLightColor(state->getBlock(), mLevelSource, pos));
    builder->color(1.0f, 1.0f, 1.0f);
    float u0 = texture->getU0(true);
    float v0 = texture->getV0(true);
    float u1 = texture->getU1(true);
    float v1 = texture->getV1(true);
    const Direction* facing = state->getValue(LadderBlock::FACING);
    float r = 0.05f;
    int x = pos.x;
    int y = pos.y;
    int z = pos.z;

    if (facing == Direction::EAST)
    {
        builder->vertexUV((float)x + r, (float)(y + 1), (float)(z + 1), u0, v0);
        builder->vertexUV((float)x + r, (float)y, (float)(z + 1), u0, v1);
        builder->vertexUV((float)x + r, (float)y, (float)z, u1, v1);
        builder->vertexUV((float)x + r, (float)(y + 1), (float)z, u1, v0);
    }
    if (facing == Direction::WEST)
    {
        builder->vertexUV((float)(x + 1) - r, (float)y, (float)(z + 1), u1, v1);
        builder->vertexUV((float)(x + 1) - r, (float)(y + 1), (float)(z + 1), u1, v0);
        builder->vertexUV((float)(x + 1) - r, (float)(y + 1), (float)z, u0, v0);
        builder->vertexUV((float)(x + 1) - r, (float)y, (float)z, u0, v1);
    }
    if (facing == Direction::SOUTH)
    {
        builder->vertexUV((float)(x + 1), (float)y, (float)z + r, u1, v1);
        builder->vertexUV((float)(x + 1), (float)(y + 1), (float)z + r, u1, v0);
        builder->vertexUV((float)x, (float)(y + 1), (float)z + r, u0, v0);
        builder->vertexUV((float)x, (float)y, (float)z + r, u0, v1);
    }
    if (facing == Direction::NORTH)
    {
        builder->vertexUV((float)(x + 1), (float)(y + 1), (float)(z + 1) - r, u0, v0);
        builder->vertexUV((float)(x + 1), (float)y, (float)(z + 1) - r, u0, v1);
        builder->vertexUV((float)x, (float)y, (float)(z + 1) - r, u1, v1);
        builder->vertexUV((float)x, (float)(y + 1), (float)(z + 1) - r, u1, v0);
    }
    return true;
}

bool BlockRenderer::tesselateDoorInWorld(const BlockState* state, const BlockPos& pos)
{
    if (state->getValue(DoorBlock::HALF) == DoorBlock::UPPER)
    {
        if (mLevelSource->getBlockState(pos.below())->getBlock() != state->getBlock())
            return false;
    }
    else if (mLevelSource->getBlockState(pos.above())->getBlock() != state->getBlock())
    {
        return false;
    }

    Block* block = state->getBlock();
    bool rightHinge = state->getValue(DoorBlock::HINGE) == DoorBlock::RIGHT;
    const Direction* facing = state->getValue(DoorBlock::FACING);
    unsigned int direction = facing->get3DDataValue();
    bool open = state->getValue(DoorBlock::OPEN);
    bool rendered;

    if (!open)
    {
        int* transformA = NULL;
        int* transformB = NULL;
        int rotation4Alt = 0;
        int rotation4Value = 0;

        if (direction == Direction::eDirection_North || direction == Direction::eDirection_South)
        {
            transformA = &mFaceTransform2;
            rotation4Value = 3;
            transformB = &mFaceTransform3;
            mFaceRotation3 = 3;
        }
        if (direction == Direction::eDirection_West || direction == Direction::eDirection_East)
        {
            transformA = &mFaceTransform0;
            transformB = &mFaceTransform1;
            rotation4Alt = 2;
            rotation4Value = 1;
            mFaceRotation1 = 3;
        }

        direction = facing->get3DDataValue();
        if (direction > Direction::eDirection_Up)
        {
            if (direction == Direction::eDirection_North)
            {
                mFaceRotation4 = rotation4Alt;
                if (rightHinge)
                {
                    *transformA = 2;
                    *transformB = 3;
                    mFaceTransform4 = 2;
                    setShape(block->getShape(state, mLevelSource, pos));
                    rendered = tesselateBlockInWorld(state, pos);
                    goto resetAndReturn;
                }
                *transformB = 1;
                setShape(block->getShape(state, mLevelSource, pos));
                rendered = tesselateBlockInWorld(state, pos);
                goto resetAndReturn;
            }
            if (direction > Direction::eDirection_South)
            {
                if (direction == Direction::eDirection_West)
                {
                    mFaceRotation4 = rotation4Alt;
                    if (rightHinge)
                    {
                        *transformA = 2;
                        *transformB = 3;
                        mFaceTransform4 = 2;
                        setShape(block->getShape(state, mLevelSource, pos));
                        rendered = tesselateBlockInWorld(state, pos);
                        goto resetAndReturn;
                    }
                    *transformB = 1;
                    setShape(block->getShape(state, mLevelSource, pos));
                    rendered = tesselateBlockInWorld(state, pos);
                    goto resetAndReturn;
                }
                if (direction != Direction::eDirection_East)
                {
                    setShape(block->getShape(state, mLevelSource, pos));
                    rendered = tesselateBlockInWorld(state, pos);
                    goto resetAndReturn;
                }
            }
            mFaceRotation4 = rotation4Value;
            if (rightHinge)
            {
                *transformB = 1;
                mFaceTransform4 = 2;
                setShape(block->getShape(state, mLevelSource, pos));
                rendered = tesselateBlockInWorld(state, pos);
                goto resetAndReturn;
            }
            *transformA = 2;
            *transformB = 3;
        }
    }
    else if (direction > Direction::eDirection_Up)
    {
        if (direction > Direction::eDirection_South)
        {
            if (direction > Direction::eDirection_East)
            {
                setShape(block->getShape(state, mLevelSource, pos));
                rendered = tesselateBlockInWorld(state, pos);
                goto resetAndReturn;
            }

            if (rightHinge)
            {
                mFaceTransform2 = 2;
                mFaceTransform3 = 1;
                mFaceRotation3 = 3;
                mFaceTransform4 = 1;
                if (facing->get3DDataValue() == Direction::eDirection_West)
                {
                    mFaceRotation4 = 3;
                    setShape(block->getShape(state, mLevelSource, pos));
                    rendered = tesselateBlockInWorld(state, pos);
                    goto resetAndReturn;
                }
            }
            else
            {
                mFaceTransform2 = 3;
                mFaceRotation2 = 3;
                mFaceTransform4 = 3;
                if (facing->get3DDataValue() != Direction::eDirection_West)
                {
                    mFaceRotation4 = 3;
                    setShape(block->getShape(state, mLevelSource, pos));
                    rendered = tesselateBlockInWorld(state, pos);
                    goto resetAndReturn;
                }
            }
            mFaceRotation4 = 0;
            setShape(block->getShape(state, mLevelSource, pos));
            rendered = tesselateBlockInWorld(state, pos);
            goto resetAndReturn;
        }

        if (rightHinge)
        {
            mFaceTransform0 = 2;
            mFaceTransform1 = 1;
            mFaceRotation1 = 3;
            mFaceTransform4 = 1;
            if (direction != Direction::eDirection_North)
            {
                mFaceRotation4 = 1;
                setShape(block->getShape(state, mLevelSource, pos));
                rendered = tesselateBlockInWorld(state, pos);
                goto resetAndReturn;
            }
        }
        else
        {
            mFaceTransform4 = 3;
            mFaceTransform0 = 3;
            mFaceRotation0 = 3;
            if (direction == Direction::eDirection_North)
            {
                mFaceRotation4 = 1;
                setShape(block->getShape(state, mLevelSource, pos));
                rendered = tesselateBlockInWorld(state, pos);
                goto resetAndReturn;
            }
        }
        mFaceRotation4 = 2;
        setShape(block->getShape(state, mLevelSource, pos));
        rendered = tesselateBlockInWorld(state, pos);
        goto resetAndReturn;
    }

    setShape(block->getShape(state, mLevelSource, pos));
    rendered = tesselateBlockInWorld(state, pos);

resetAndReturn:
    for (int i = 0; i < 6; ++i)
    {
        (&mFaceRotation0)[i] = 0;
        (&mFaceTransform0)[i] = 0;
    }
    return rendered;
}

bool BlockRenderer::tesselateRailInWorld(const BlockState* state, const BlockPos& pos)
{
    BufferBuilder* builder = Tesselator::getInstance()->getBuilder();
    TextureAtlasSprite* texture = getTexture(state, Direction::UP);
    if (hasFixedTexture())
        texture = mFixedTexture;

    BaseRailBlock* rail = static_cast<BaseRailBlock*>(state->getBlock());
    BaseRailBlock::RailShape* shape = state->getValue(rail->getShapeProperty());

    builder->tex2(getLightColor(rail, mLevelSource, pos));
    builder->color(1.0f, 1.0f, 1.0f);
    float u0 = texture->getU0(true);
    float v0 = texture->getV0(true);
    float u1 = texture->getU1(true);
    float v1 = texture->getV1(true);

    float x0 = (float)(pos.x + 1);
    float x1 = (float)(pos.x + 1);
    float x2 = (float)pos.x;
    float x3 = (float)pos.x;
    float z0 = (float)pos.z;
    float z1 = (float)(pos.z + 1);
    float z2 = (float)(pos.z + 1);
    float z3 = (float)pos.z;
    float y0 = (float)pos.y + 0.0625f;
    float y1 = y0;
    float y2 = y0;
    float y3 = y0;

    if (shape->getData() == 1 || shape->getData() == 2 || shape->getData() == 3 || shape->getData() == 7)
    {
        x0 = (float)(pos.x + 1);
        x3 = (float)(pos.x + 1);
        x1 = (float)pos.x;
        x2 = (float)pos.x;
        z0 = (float)(pos.z + 1);
        z1 = (float)(pos.z + 1);
        z2 = (float)pos.z;
        z3 = (float)pos.z;
    }
    else if (shape->getData() == 8)
    {
        x0 = (float)pos.x;
        x1 = (float)pos.x;
        x2 = (float)(pos.x + 1);
        x3 = (float)(pos.x + 1);
        z0 = (float)(pos.z + 1);
        z3 = (float)(pos.z + 1);
        z1 = (float)pos.z;
        z2 = (float)pos.z;
    }
    else if (shape->getData() == 9)
    {
        x0 = (float)pos.x;
        x3 = (float)pos.x;
        x1 = (float)(pos.x + 1);
        x2 = (float)(pos.x + 1);
        z0 = (float)pos.z;
        z1 = (float)pos.z;
        z2 = (float)(pos.z + 1);
        z3 = (float)(pos.z + 1);
    }

    if (shape->getData() == 2 || shape->getData() == 4)
    {
        y0 += 1.0f;
        y3 += 1.0f;
    }
    else if (shape->getData() == 3 || shape->getData() == 5)
    {
        y1 += 1.0f;
        y2 += 1.0f;
    }

    builder->vertexUV(x0, y0, z0, u1, v0);
    builder->vertexUV(x1, y1, z1, u1, v1);
    builder->vertexUV(x2, y2, z2, u0, v1);
    builder->vertexUV(x3, y3, z3, u0, v0);
    builder->vertexUV(x3, y3, z3, u0, v0);
    builder->vertexUV(x2, y2, z2, u0, v1);
    builder->vertexUV(x1, y1, z1, u1, v1);
    builder->vertexUV(x0, y0, z0, u1, v0);
    return true;
}

bool BlockRenderer::tesselateStairsInWorld(const BlockState* state, const BlockPos& pos)
{
    Block* block = state->getBlock();
    const arrayWithLength<const AABB*>* shapes = block->getShapes(state, mLevelSource, pos);

    for (unsigned int i = 0; i < shapes->length; i++)
    {
        if (shapes->data[i] != NULL)
        {
            setShape(shapes->data[i]);
            tesselateBlockInWorld(state, pos);
        }
    }

    return true;
}

bool BlockRenderer::tesselateEggInWorld(const BlockState* state, const BlockPos& pos)
{
    int depth = 0;
    for (int layer = 0; layer < 8; ++layer)
    {
        int halfWidth = 0;
        int height = 1;
        if (layer == 0)
            halfWidth = 2;
        if (layer == 1)
            halfWidth = 3;
        if (layer == 2)
            halfWidth = 4;
        if (layer == 3)
        {
            halfWidth = 5;
            height = 2;
        }
        if (layer == 4)
        {
            halfWidth = 6;
            height = 3;
        }
        if (layer == 5)
        {
            halfWidth = 7;
            height = 5;
        }
        if (layer == 6)
        {
            halfWidth = 6;
            height = 2;
        }
        if (layer == 7)
            halfWidth = 3;

        float half = (float)halfWidth / 16.0f;
        float top = 1.0f - (float)depth / 16.0f;
        float bottom = 1.0f - (float)(depth + height) / 16.0f;
        depth += height;
        setShape(0.5f - half, bottom, 0.5f - half, 0.5f + half, top, 0.5f + half);
        tesselateBlockInWorld(state, pos);
    }
    setShape(0.0f, 0.0f, 0.0f, 1.0f, 1.0f, 1.0f);
    return true;
}

bool BlockRenderer::tesselateFenceInWorld(const BlockState* state, const BlockPos& pos)
{
    setShape(0.375f, 0.0f, 0.375f, 0.625f, 1.0f, 0.625f);
    tesselateBlockInWorld(state, pos);

    bool alongZ = false;
    bool alongX = false;
    FenceBlock* fence = static_cast<FenceBlock*>(state->getBlock());
    if (fence->connectsTo(mLevelSource, pos.offset(-1, 0, 0), Direction::EAST)
        || fence->connectsTo(mLevelSource, pos.offset(1, 0, 0), Direction::WEST))
        alongX = true;
    if (fence->connectsTo(mLevelSource, pos.offset(0, 0, -1), Direction::SOUTH)
        || fence->connectsTo(mLevelSource, pos.offset(0, 0, 1), Direction::NORTH))
        alongZ = true;

    bool west = fence->connectsTo(mLevelSource, pos.offset(-1, 0, 0), Direction::EAST);
    bool east = fence->connectsTo(mLevelSource, pos.offset(1, 0, 0), Direction::WEST);
    bool north = fence->connectsTo(mLevelSource, pos.offset(0, 0, -1), Direction::SOUTH);
    bool south = fence->connectsTo(mLevelSource, pos.offset(0, 0, 1), Direction::NORTH);
    if (!alongX && !alongZ)
        alongX = true;

    float minX = west ? 0.0f : 0.4375f;
    float maxX = east ? 1.0f : 0.5625f;
    float minZ = north ? 0.0f : 0.4375f;
    float maxZ = south ? 1.0f : 0.5625f;
    if (alongX)
    {
        setShape(minX, 0.75f, 0.4375f, maxX, 0.9375f, 0.5625f);
        tesselateBlockInWorld(state, pos);
    }
    if (alongZ)
    {
        setShape(0.4375f, 0.75f, minZ, 0.5625f, 0.9375f, maxZ);
        tesselateBlockInWorld(state, pos);
    }
    if (alongX)
    {
        setShape(minX, 0.375f, 0.4375f, maxX, 0.5625f, 0.5625f);
        tesselateBlockInWorld(state, pos);
    }
    if (alongZ)
    {
        setShape(0.4375f, 0.375f, minZ, 0.5625f, 0.5625f, maxZ);
        tesselateBlockInWorld(state, pos);
    }
    return true;
}

bool BlockRenderer::tesselateWallInWorld(const BlockState* state, const BlockPos& pos)
{
    WallBlock* wallBlock = static_cast<WallBlock*>(state->getBlock());
    bool east = wallBlock->connectsTo(mLevelSource, pos.offset(-1, 0, 0), Direction::EAST);
    bool west = wallBlock->connectsTo(mLevelSource, pos.offset(1, 0, 0), Direction::WEST);
    bool south = wallBlock->connectsTo(mLevelSource, pos.offset(0, 0, -1), Direction::SOUTH);
    bool north = wallBlock->connectsTo(mLevelSource, pos.offset(0, 0, 1), Direction::NORTH);

    bool northSouthThrough = south && north && !east && !west;
    bool eastWestThrough = !south && !north && east && west;
    bool aboveEmpty = mLevelSource->isEmptyBlock(pos.offset(0, 1, 0));

    if (!(northSouthThrough || eastWestThrough) || !aboveEmpty)
    {
        float postMin = 0.5f - WallBlock::POST_HALF_WIDTH;
        float postMax = WallBlock::POST_HALF_WIDTH + 0.5f;
        setShape(postMin, 0.0f, postMin, postMax, WallBlock::POST_HEIGHT, postMax);
        tesselateBlockInWorld(state, pos);
        if (east)
        {
            setShape(0.0f, 0.0f, 0.5f - WallBlock::ARM_HALF_WIDTH, postMin, WallBlock::ARM_HEIGHT,
                     WallBlock::ARM_HALF_WIDTH + 0.5f);
            tesselateBlockInWorld(state, pos);
        }
        if (west)
        {
            setShape(postMax, 0.0f, 0.5f - WallBlock::ARM_HALF_WIDTH, 1.0f, WallBlock::ARM_HEIGHT,
                     WallBlock::ARM_HALF_WIDTH + 0.5f);
            tesselateBlockInWorld(state, pos);
        }
        if (south)
        {
            setShape(0.5f - WallBlock::ARM_HALF_WIDTH, 0.0f, 0.0f, WallBlock::ARM_HALF_WIDTH + 0.5f,
                     WallBlock::ARM_HEIGHT, postMin);
            tesselateBlockInWorld(state, pos);
        }
        if (north)
        {
            setShape(0.5f - WallBlock::ARM_HALF_WIDTH, 0.0f, postMax, WallBlock::ARM_HALF_WIDTH + 0.5f,
                     WallBlock::ARM_HEIGHT, 1.0f);
            tesselateBlockInWorld(state, pos);
        }
    }
    else
    {
        float armMin = 0.5f - WallBlock::ARM_HALF_WIDTH;
        if (northSouthThrough)
        {
            setShape(armMin, 0.0f, 0.0f, WallBlock::ARM_HALF_WIDTH + 0.5f, WallBlock::ARM_HEIGHT, 1.0f);
            tesselateBlockInWorld(state, pos);
        }
        else
        {
            setShape(0.0f, 0.0f, armMin, 1.0f, WallBlock::ARM_HEIGHT, WallBlock::ARM_HALF_WIDTH + 0.5f);
            tesselateBlockInWorld(state, pos);
        }
    }
    return true;
}

bool BlockRenderer::tesselateLeverInWorld(const BlockState* state, const BlockPos& pos)
{
    LeverBlock::LeverDirection* facing = state->getValue(LeverBlock::FACING);
    bool powered = state->getValue(LeverBlock::POWERED);
    BufferBuilder* builder = Tesselator::getInstance()->getBuilder();

    bool hadFixedTexture = hasFixedTexture();
    if (!hadFixedTexture)
        setFixedTexture(getTexture(Blocks::cobblestone, Direction::UP));

    float halfLength = 0.25f;
    float halfWidth = 0.1875f;
    float height = 0.1875f;
    if (facing == LeverBlock::LeverDirection::UP_Z)
        setShape(0.5f - halfWidth, 0.0f, 0.5f - halfLength, 0.5f + halfWidth, height, 0.5f + halfLength);
    else if (facing == LeverBlock::LeverDirection::UP_X)
        setShape(0.5f - halfLength, 0.0f, 0.5f - halfWidth, 0.5f + halfLength, height, 0.5f + halfWidth);
    else if (facing == LeverBlock::LeverDirection::NORTH)
        setShape(0.5f - halfWidth, 0.5f - halfLength, 1.0f - height, 0.5f + halfWidth, 0.5f + halfLength, 1.0f);
    else if (facing == LeverBlock::LeverDirection::SOUTH)
        setShape(0.5f - halfWidth, 0.5f - halfLength, 0.0f, 0.5f + halfWidth, 0.5f + halfLength, height);
    else if (facing == LeverBlock::LeverDirection::WEST)
        setShape(1.0f - height, 0.5f - halfLength, 0.5f - halfWidth, 1.0f, 0.5f + halfLength, 0.5f + halfWidth);
    else if (facing == LeverBlock::LeverDirection::EAST)
        setShape(0.0f, 0.5f - halfLength, 0.5f - halfWidth, height, 0.5f + halfLength, 0.5f + halfWidth);
    else if (facing == LeverBlock::LeverDirection::DOWN_X)
        setShape(0.5f - halfLength, 1.0f - height, 0.5f - halfWidth, 0.5f + halfLength, 1.0f, 0.5f + halfWidth);
    else if (facing == LeverBlock::LeverDirection::DOWN_Z)
        setShape(0.5f - halfWidth, 1.0f - height, 0.5f - halfLength, 0.5f + halfWidth, 1.0f, 0.5f + halfLength);

    tesselateBlockInWorld(state, pos);
    if (!hadFixedTexture)
        clearFixedTexture();

    builder->tex2(getLightColor(state->getBlock(), mLevelSource, pos));
    state->getBlock()->defaultBlockState()->getLightEmission();
    builder->color(1.0f, 1.0f, 1.0f);
    TextureAtlasSprite* texture = getTexture(state, NULL);
    if (hasFixedTexture())
        texture = mFixedTexture;
    float u0 = texture->getU0(true);
    float v0 = texture->getV0(true);
    float u1 = texture->getU1(true);
    float v1 = texture->getV1(true);

    Vec3* corners[8];
    float handleX = 0.0625f;
    float handleZ = 0.0625f;
    float handleHeight = 0.625f;
    corners[0] = Vec3::newTemp(-handleX, 0.0, -handleZ);
    corners[1] = Vec3::newTemp(handleX, 0.0, -handleZ);
    corners[2] = Vec3::newTemp(handleX, 0.0, handleZ);
    corners[3] = Vec3::newTemp(-handleX, 0.0, handleZ);
    corners[4] = Vec3::newTemp(-handleX, handleHeight, -handleZ);
    corners[5] = Vec3::newTemp(handleX, handleHeight, -handleZ);
    corners[6] = Vec3::newTemp(handleX, handleHeight, handleZ);
    corners[7] = Vec3::newTemp(-handleX, handleHeight, handleZ);

    for (int i = 0; i < 8; ++i)
    {
        if (powered)
        {
            corners[i]->z -= 0.0625;
            corners[i] = corners[i]->xRot(0.69813174f);
        }
        else
        {
            corners[i]->z += 0.0625;
            corners[i] = corners[i]->xRot(-0.69813174f);
        }

        if (facing == LeverBlock::LeverDirection::DOWN_X || facing == LeverBlock::LeverDirection::DOWN_Z)
            corners[i] = corners[i]->zRot(3.1415927f);
        if (facing == LeverBlock::LeverDirection::UP_X || facing == LeverBlock::LeverDirection::DOWN_X)
            corners[i] = corners[i]->yRot(1.5707964f);

        if (facing == LeverBlock::LeverDirection::DOWN_X || facing == LeverBlock::LeverDirection::DOWN_Z)
        {
            corners[i]->x += (double)pos.x + 0.5;
            corners[i]->y += (double)((float)pos.y + 0.875f);
            corners[i]->z += (double)pos.z + 0.5;
        }
        else if (facing == LeverBlock::LeverDirection::UP_X || facing == LeverBlock::LeverDirection::UP_Z)
        {
            corners[i]->x += (double)pos.x + 0.5;
            corners[i]->y += (double)((float)pos.y + 0.125f);
            corners[i]->z += (double)pos.z + 0.5;
        }
        else
        {
            corners[i]->y -= 0.375;
            corners[i] = corners[i]->xRot(1.5707964f);
            if (facing == LeverBlock::LeverDirection::NORTH)
                corners[i] = corners[i]->yRot(0.0f);
            if (facing == LeverBlock::LeverDirection::SOUTH)
                corners[i] = corners[i]->yRot(3.1415927f);
            if (facing == LeverBlock::LeverDirection::WEST)
                corners[i] = corners[i]->yRot(1.5707964f);
            if (facing == LeverBlock::LeverDirection::EAST)
                corners[i] = corners[i]->yRot(-1.5707964f);
            corners[i]->x += (double)pos.x + 0.5;
            corners[i]->y += (double)((float)pos.y + 0.5f);
            corners[i]->z += (double)pos.z + 0.5;
        }
    }

    Vec3* c0 = NULL;
    Vec3* c1 = NULL;
    Vec3* c2 = NULL;
    Vec3* c3 = NULL;
    for (int face = 0; face < 6; ++face)
    {
        if (face == 0)
        {
            u0 = texture->getU(7.0);
            v0 = texture->getV(6.0);
            u1 = texture->getU(9.0);
            v1 = texture->getV(8.0);
        }
        else if (face == 2)
        {
            u0 = texture->getU(7.0);
            v0 = texture->getV(6.0);
            u1 = texture->getU(9.0);
            v1 = texture->getV1(false);
        }
        texture->adjustUVs(u0, u1, v0, v1);

        if (face == 0)
        {
            c0 = corners[0];
            c1 = corners[1];
            c2 = corners[2];
            c3 = corners[3];
        }
        else if (face == 1)
        {
            c0 = corners[7];
            c1 = corners[6];
            c2 = corners[5];
            c3 = corners[4];
        }
        else if (face == 2)
        {
            c0 = corners[1];
            c1 = corners[0];
            c2 = corners[4];
            c3 = corners[5];
        }
        else if (face == 3)
        {
            c0 = corners[2];
            c1 = corners[1];
            c2 = corners[5];
            c3 = corners[6];
        }
        else if (face == 4)
        {
            c0 = corners[3];
            c1 = corners[2];
            c2 = corners[6];
            c3 = corners[7];
        }
        else if (face == 5)
        {
            c0 = corners[0];
            c1 = corners[3];
            c2 = corners[7];
            c3 = corners[4];
        }

        builder->vertexUV((float)c0->x, (float)c0->y, (float)c0->z, u0, v1);
        builder->vertexUV((float)c1->x, (float)c1->y, (float)c1->z, u1, v1);
        builder->vertexUV((float)c2->x, (float)c2->y, (float)c2->z, u1, v0);
        builder->vertexUV((float)c3->x, (float)c3->y, (float)c3->z, u0, v0);
    }
    return true;
}

bool BlockRenderer::tesselateTripwireSourceInWorld(const BlockState* state, const BlockPos& pos)
{
    BufferBuilder* builder = Tesselator::getInstance()->getBuilder();
    const Direction* facing = state->getValue(TripWireHookBlock::FACING);
    bool attached = state->getValue(TripWireHookBlock::ATTACHED);
    bool powered = state->getValue(TripWireHookBlock::POWERED);
    bool suspended = !mLevelSource->getBlockState(pos.below())->isTopSolidBlocking();
    bool hadFixedTexture = hasFixedTexture();
    if (!hadFixedTexture)
        setFixedTexture(getTexture(Blocks::planks, Direction::UP));

    float baseHalfHeight = 0.25f;
    float baseHalfWidth = 0.125f;
    float baseDepth = 0.125f;
    float baseBottom = 0.3f - baseHalfHeight;
    float baseTop = 0.3f + baseHalfHeight;
    if (facing == Direction::NORTH)
        setShape(0.5f - baseHalfWidth, baseBottom, 1.0f - baseDepth, 0.5f + baseHalfWidth, baseTop, 1.0f);
    else if (facing == Direction::SOUTH)
        setShape(0.5f - baseHalfWidth, baseBottom, 0.0f, 0.5f + baseHalfWidth, baseTop, baseDepth);
    else if (facing == Direction::WEST)
        setShape(1.0f - baseDepth, baseBottom, 0.5f - baseHalfWidth, 1.0f, baseTop, 0.5f + baseHalfWidth);
    else if (facing == Direction::EAST)
        setShape(0.0f, baseBottom, 0.5f - baseHalfWidth, baseDepth, baseTop, 0.5f + baseHalfWidth);
    tesselateBlockInWorld(state, pos);
    if (!hadFixedTexture)
        clearFixedTexture();

    builder->tex2(state->getLightColor(mLevelSource, pos));
    float brightness = 1.0f;
    if (state->getBlock()->defaultBlockState()->getLightEmission() > 0)
        brightness = 1.0f;
    builder->color(brightness, brightness, brightness);

    TextureAtlasSprite* texture = getTexture(state->getBlock(), NULL);
    if (hasFixedTexture())
        texture = mFixedTexture;
    double u0 = texture->getU0(false);
    double v0 = texture->getV0(false);
    double u1 = texture->getU1(false);
    double v1 = texture->getV1(false);

    Vec3* corners[8];
    float stickHalfWidth = 0.046875f;
    float stickHalfDepth = 0.046875f;
    float stickLength = 0.3125f;
    corners[0] = Vec3::newTemp(-stickHalfWidth, 0.0, -stickHalfDepth);
    corners[1] = Vec3::newTemp(stickHalfWidth, 0.0, -stickHalfDepth);
    corners[2] = Vec3::newTemp(stickHalfWidth, 0.0, stickHalfDepth);
    corners[3] = Vec3::newTemp(-stickHalfWidth, 0.0, stickHalfDepth);
    corners[4] = Vec3::newTemp(-stickHalfWidth, stickLength, -stickHalfDepth);
    corners[5] = Vec3::newTemp(stickHalfWidth, stickLength, -stickHalfDepth);
    corners[6] = Vec3::newTemp(stickHalfWidth, stickLength, stickHalfDepth);
    corners[7] = Vec3::newTemp(-stickHalfWidth, stickLength, stickHalfDepth);

    for (int i = 0; i < 8; ++i)
    {
        corners[i]->z += 0.0625;
        if (powered)
        {
            corners[i] = corners[i]->xRot(0.5235988f);
            corners[i]->y -= 0.4375;
        }
        else if (attached)
        {
            corners[i] = corners[i]->xRot(0.08726647f);
            corners[i]->y -= 0.4375;
        }
        else
        {
            corners[i] = corners[i]->xRot(-(3.1415927f * 2.0f / 9.0f));
            corners[i]->y -= 0.375;
        }
        corners[i] = corners[i]->xRot(1.5707964f);
        if (facing == Direction::NORTH)
            corners[i] = corners[i]->yRot(0.0f);
        if (facing == Direction::SOUTH)
            corners[i] = corners[i]->yRot(3.1415927f);
        if (facing == Direction::WEST)
            corners[i] = corners[i]->yRot(1.5707964f);
        if (facing == Direction::EAST)
            corners[i] = corners[i]->yRot(-1.5707964f);
        corners[i]->x += (double)pos.x + 0.5;
        corners[i]->y += (double)((float)pos.y + 0.3125f);
        corners[i]->z += (double)pos.z + 0.5;
    }

    Vec3* a = NULL;
    Vec3* b = NULL;
    Vec3* c = NULL;
    Vec3* d = NULL;
    int stickU0 = 7;
    int stickU1 = 9;
    int stickV0 = 9;
    int stickV1 = 16;
    for (int face = 0; face < 6; ++face)
    {
        if (face == 0)
        {
            a = corners[0];
            b = corners[1];
            c = corners[2];
            d = corners[3];
            u0 = texture->getU(stickU0);
            v0 = texture->getV(stickV0);
            u1 = texture->getU(stickU1);
            v1 = texture->getV(stickV0 + 2);
        }
        else if (face == 1)
        {
            a = corners[7];
            b = corners[6];
            c = corners[5];
            d = corners[4];
        }
        else if (face == 2)
        {
            a = corners[1];
            b = corners[0];
            c = corners[4];
            d = corners[5];
            u0 = texture->getU(stickU0);
            v0 = texture->getV(stickV0);
            u1 = texture->getU(stickU1);
            v1 = texture->getV(stickV1);
        }
        else if (face == 3)
        {
            a = corners[2];
            b = corners[1];
            c = corners[5];
            d = corners[6];
        }
        else if (face == 4)
        {
            a = corners[3];
            b = corners[2];
            c = corners[6];
            d = corners[7];
        }
        else if (face == 5)
        {
            a = corners[0];
            b = corners[3];
            c = corners[7];
            d = corners[4];
        }
        builder->vertexUV((float)a->x, (float)a->y, (float)a->z, (float)u0, (float)v1);
        builder->vertexUV((float)b->x, (float)b->y, (float)b->z, (float)u1, (float)v1);
        builder->vertexUV((float)c->x, (float)c->y, (float)c->z, (float)u1, (float)v0);
        builder->vertexUV((float)d->x, (float)d->y, (float)d->z, (float)u0, (float)v0);
    }

    float ringHalfWidth = 0.09375f;
    float ringHalfDepth = 0.09375f;
    float ringHeight = 0.03125f;
    corners[0] = Vec3::newTemp(-ringHalfWidth, 0.0, -ringHalfDepth);
    corners[1] = Vec3::newTemp(ringHalfWidth, 0.0, -ringHalfDepth);
    corners[2] = Vec3::newTemp(ringHalfWidth, 0.0, ringHalfDepth);
    corners[3] = Vec3::newTemp(-ringHalfWidth, 0.0, ringHalfDepth);
    corners[4] = Vec3::newTemp(-ringHalfWidth, ringHeight, -ringHalfDepth);
    corners[5] = Vec3::newTemp(ringHalfWidth, ringHeight, -ringHalfDepth);
    corners[6] = Vec3::newTemp(ringHalfWidth, ringHeight, ringHalfDepth);
    corners[7] = Vec3::newTemp(-ringHalfWidth, ringHeight, ringHalfDepth);

    for (int i = 0; i < 8; ++i)
    {
        corners[i]->z += 0.21875;
        if (powered)
        {
            corners[i]->y -= 0.09375;
            corners[i]->z -= 0.1625;
            corners[i] = corners[i]->xRot(0.0f);
        }
        else if (attached)
        {
            corners[i]->y += 0.015625;
            corners[i]->z -= 0.171875;
            corners[i] = corners[i]->xRot(0.17453294f);
        }
        else
        {
            corners[i] = corners[i]->xRot(0.87266463f);
        }
        if (facing == Direction::NORTH)
            corners[i] = corners[i]->yRot(0.0f);
        if (facing == Direction::SOUTH)
            corners[i] = corners[i]->yRot(3.1415927f);
        if (facing == Direction::WEST)
            corners[i] = corners[i]->yRot(1.5707964f);
        if (facing == Direction::EAST)
            corners[i] = corners[i]->yRot(-1.5707964f);
        corners[i]->x += (double)pos.x + 0.5;
        corners[i]->y += (double)((float)pos.y + 0.3125f);
        corners[i]->z += (double)pos.z + 0.5;
    }

    int ringU0 = 5;
    int ringU1 = 11;
    int ringV0 = 3;
    int ringV1 = 9;
    for (int face = 0; face < 6; ++face)
    {
        if (face == 0)
        {
            a = corners[0];
            b = corners[1];
            c = corners[2];
            d = corners[3];
            u0 = texture->getU(ringU0);
            v0 = texture->getV(ringV0);
            u1 = texture->getU(ringU1);
            v1 = texture->getV(ringV1);
        }
        else if (face == 1)
        {
            a = corners[7];
            b = corners[6];
            c = corners[5];
            d = corners[4];
        }
        else if (face == 2)
        {
            a = corners[1];
            b = corners[0];
            c = corners[4];
            d = corners[5];
            u0 = texture->getU(ringU0);
            v0 = texture->getV(ringV0);
            u1 = texture->getU(ringU1);
            v1 = texture->getV(ringV0 + 2);
        }
        else if (face == 3)
        {
            a = corners[2];
            b = corners[1];
            c = corners[5];
            d = corners[6];
        }
        else if (face == 4)
        {
            a = corners[3];
            b = corners[2];
            c = corners[6];
            d = corners[7];
        }
        else if (face == 5)
        {
            a = corners[0];
            b = corners[3];
            c = corners[7];
            d = corners[4];
        }
        builder->vertexUV((float)a->x, (float)a->y, (float)a->z, (float)u0, (float)v1);
        builder->vertexUV((float)b->x, (float)b->y, (float)b->z, (float)u1, (float)v1);
        builder->vertexUV((float)c->x, (float)c->y, (float)c->z, (float)u1, (float)v0);
        builder->vertexUV((float)d->x, (float)d->y, (float)d->z, (float)u0, (float)v0);
    }

    if (attached)
    {
        double hookY = corners[0]->y;
        float wireWidth = 0.03125f;
        float wireMin = 0.5f - wireWidth / 2.0f;
        float wireMax = wireMin + wireWidth;
        TextureAtlasSprite* wireTexture = getTexture(Blocks::tripwire, Direction::UP);
        double wireU0 = wireTexture->getU0(false);
        double wireV0 = wireTexture->getV(attached ? 2.0 : 0.0);
        double wireU1 = wireTexture->getU1(false);
        double wireV1 = wireTexture->getV(attached ? 4.0 : 2.0);
        double wireHeight = (suspended ? 3.5 : 1.5) * 0.0625;
        float wireBrightness = mLevelSource->getBrightness(pos) * 0.75f;
        builder->color(wireBrightness, wireBrightness, wireBrightness);

        float wireY = (float)((double)pos.y + wireHeight);
        if (facing == Direction::NORTH)
        {
            builder->vertexUV((float)pos.x + wireMin, wireY, (float)((double)pos.z + 0.25), (float)wireU0,
                              (float)wireV0);
            builder->vertexUV((float)pos.x + wireMax, wireY, (float)((double)pos.z + 0.25), (float)wireU0,
                              (float)wireV1);
            builder->vertexUV((float)pos.x + wireMax, wireY, (float)pos.z, (float)wireU1, (float)wireV1);
            builder->vertexUV((float)pos.x + wireMin, wireY, (float)pos.z, (float)wireU1, (float)wireV0);
            builder->vertexUV((float)pos.x + wireMin, (float)hookY, (float)((double)pos.z + 0.5), (float)wireU0,
                              (float)wireV0);
            builder->vertexUV((float)pos.x + wireMax, (float)hookY, (float)((double)pos.z + 0.5), (float)wireU0,
                              (float)wireV1);
            builder->vertexUV((float)pos.x + wireMax, wireY, (float)((double)pos.z + 0.25), (float)wireU1,
                              (float)wireV1);
            builder->vertexUV((float)pos.x + wireMin, wireY, (float)((double)pos.z + 0.25), (float)wireU1,
                              (float)wireV0);
        }
        else if (facing == Direction::SOUTH)
        {
            builder->vertexUV((float)pos.x + wireMin, wireY, (float)((double)pos.z + 0.75), (float)wireU0,
                              (float)wireV0);
            builder->vertexUV((float)pos.x + wireMax, wireY, (float)((double)pos.z + 0.75), (float)wireU0,
                              (float)wireV1);
            builder->vertexUV((float)pos.x + wireMax, (float)hookY, (float)((double)pos.z + 0.5), (float)wireU1,
                              (float)wireV1);
            builder->vertexUV((float)pos.x + wireMin, (float)hookY, (float)((double)pos.z + 0.5), (float)wireU1,
                              (float)wireV0);
            builder->vertexUV((float)pos.x + wireMin, wireY, (float)(pos.z + 1), (float)wireU0, (float)wireV0);
            builder->vertexUV((float)pos.x + wireMax, wireY, (float)(pos.z + 1), (float)wireU0, (float)wireV1);
            builder->vertexUV((float)pos.x + wireMax, wireY, (float)((double)pos.z + 0.75), (float)wireU1,
                              (float)wireV1);
            builder->vertexUV((float)pos.x + wireMin, wireY, (float)((double)pos.z + 0.75), (float)wireU1,
                              (float)wireV0);
        }
        else if (facing == Direction::WEST)
        {
            builder->vertexUV((float)pos.x, wireY, (float)pos.z + wireMax, (float)wireU0, (float)wireV1);
            builder->vertexUV((float)((double)pos.x + 0.25), wireY, (float)pos.z + wireMax, (float)wireU1,
                              (float)wireV1);
            builder->vertexUV((float)((double)pos.x + 0.25), wireY, (float)pos.z + wireMin, (float)wireU1,
                              (float)wireV0);
            builder->vertexUV((float)pos.x, wireY, (float)pos.z + wireMin, (float)wireU0, (float)wireV0);
            builder->vertexUV((float)((double)pos.x + 0.25), wireY, (float)pos.z + wireMax, (float)wireU0,
                              (float)wireV1);
            builder->vertexUV((float)((double)pos.x + 0.5), (float)hookY, (float)pos.z + wireMax, (float)wireU1,
                              (float)wireV1);
            builder->vertexUV((float)((double)pos.x + 0.5), (float)hookY, (float)pos.z + wireMin, (float)wireU1,
                              (float)wireV0);
            builder->vertexUV((float)((double)pos.x + 0.25), wireY, (float)pos.z + wireMin, (float)wireU0,
                              (float)wireV0);
        }
        else
        {
            builder->vertexUV((float)((double)pos.x + 0.5), (float)hookY, (float)pos.z + wireMax, (float)wireU0,
                              (float)wireV1);
            builder->vertexUV((float)((double)pos.x + 0.75), wireY, (float)pos.z + wireMax, (float)wireU1,
                              (float)wireV1);
            builder->vertexUV((float)((double)pos.x + 0.75), wireY, (float)pos.z + wireMin, (float)wireU1,
                              (float)wireV0);
            builder->vertexUV((float)((double)pos.x + 0.5), (float)hookY, (float)pos.z + wireMin, (float)wireU0,
                              (float)wireV0);
            builder->vertexUV((float)((double)pos.x + 0.75), wireY, (float)pos.z + wireMax, (float)wireU0,
                              (float)wireV1);
            builder->vertexUV((float)(pos.x + 1), wireY, (float)pos.z + wireMax, (float)wireU1, (float)wireV1);
            builder->vertexUV((float)(pos.x + 1), wireY, (float)pos.z + wireMin, (float)wireU1, (float)wireV0);
            builder->vertexUV((float)((double)pos.x + 0.75), wireY, (float)pos.z + wireMin, (float)wireU0,
                              (float)wireV0);
        }
    }
    return true;
}

bool BlockRenderer::tesselateTripwireInWorld(const BlockState* state, const BlockPos& pos)
{
    BufferBuilder* builder = Tesselator::getInstance()->getBuilder();
    TextureAtlasSprite* texture = getTexture(state, NULL);
    bool attached = state->getValue(TripWireBlock::ATTACHED);
    bool suspended = state->getValue(TripWireBlock::SUSPENDED);
    if (hasFixedTexture())
        texture = mFixedTexture;

    builder->tex2(state->getLightColor(mLevelSource, pos));
    float brightness = mLevelSource->getBrightness(pos) * 0.75f;
    builder->color(brightness, brightness, brightness);

    double u0 = texture->getU0(false);
    double v0 = texture->getV(attached ? 2.0 : 0.0);
    double u1 = texture->getU1(false);
    double v1 = texture->getV(attached ? 4.0 : 2.0);
    double height = (suspended ? 3.5 : 1.5) * 0.0625;

    bool west = TripWireBlock::shouldConnectTo(mLevelSource, pos, state, Direction::WEST);
    bool east = TripWireBlock::shouldConnectTo(mLevelSource, pos, state, Direction::EAST);
    bool north = TripWireBlock::shouldConnectTo(mLevelSource, pos, state, Direction::NORTH);
    bool south = TripWireBlock::shouldConnectTo(mLevelSource, pos, state, Direction::SOUTH);

    float wireMin = 0.484375f;
    float wireMax = 0.515625f;
    if (!north && !east && !south && !west)
    {
        north = true;
        south = true;
    }

    float y = (float)((double)pos.y + height);
    if (north)
    {
        builder->vertexUV((float)pos.x + wireMin, y, (float)((double)pos.z + 0.25), (float)u0, (float)v0);
        builder->vertexUV((float)pos.x + wireMax, y, (float)((double)pos.z + 0.25), (float)u0, (float)v1);
        builder->vertexUV((float)pos.x + wireMax, y, (float)pos.z, (float)u1, (float)v1);
        builder->vertexUV((float)pos.x + wireMin, y, (float)pos.z, (float)u1, (float)v0);
        builder->vertexUV((float)pos.x + wireMin, y, (float)pos.z, (float)u1, (float)v0);
        builder->vertexUV((float)pos.x + wireMax, y, (float)pos.z, (float)u1, (float)v1);
        builder->vertexUV((float)pos.x + wireMax, y, (float)((double)pos.z + 0.25), (float)u0, (float)v1);
        builder->vertexUV((float)pos.x + wireMin, y, (float)((double)pos.z + 0.25), (float)u0, (float)v0);
    }
    if (north || (south && !east && !west))
    {
        builder->vertexUV((float)pos.x + wireMin, y, (float)((double)pos.z + 0.5), (float)u0, (float)v0);
        builder->vertexUV((float)pos.x + wireMax, y, (float)((double)pos.z + 0.5), (float)u0, (float)v1);
        builder->vertexUV((float)pos.x + wireMax, y, (float)((double)pos.z + 0.25), (float)u1, (float)v1);
        builder->vertexUV((float)pos.x + wireMin, y, (float)((double)pos.z + 0.25), (float)u1, (float)v0);
        builder->vertexUV((float)pos.x + wireMin, y, (float)((double)pos.z + 0.25), (float)u1, (float)v0);
        builder->vertexUV((float)pos.x + wireMax, y, (float)((double)pos.z + 0.25), (float)u1, (float)v1);
        builder->vertexUV((float)pos.x + wireMax, y, (float)((double)pos.z + 0.5), (float)u0, (float)v1);
        builder->vertexUV((float)pos.x + wireMin, y, (float)((double)pos.z + 0.5), (float)u0, (float)v0);
    }
    if (south || (north && !east && !west))
    {
        builder->vertexUV((float)pos.x + wireMin, y, (float)((double)pos.z + 0.75), (float)u0, (float)v0);
        builder->vertexUV((float)pos.x + wireMax, y, (float)((double)pos.z + 0.75), (float)u0, (float)v1);
        builder->vertexUV((float)pos.x + wireMax, y, (float)((double)pos.z + 0.5), (float)u1, (float)v1);
        builder->vertexUV((float)pos.x + wireMin, y, (float)((double)pos.z + 0.5), (float)u1, (float)v0);
        builder->vertexUV((float)pos.x + wireMin, y, (float)((double)pos.z + 0.5), (float)u1, (float)v0);
        builder->vertexUV((float)pos.x + wireMax, y, (float)((double)pos.z + 0.5), (float)u1, (float)v1);
        builder->vertexUV((float)pos.x + wireMax, y, (float)((double)pos.z + 0.75), (float)u0, (float)v1);
        builder->vertexUV((float)pos.x + wireMin, y, (float)((double)pos.z + 0.75), (float)u0, (float)v0);
    }
    if (south)
    {
        builder->vertexUV((float)pos.x + wireMin, y, (float)(pos.z + 1), (float)u0, (float)v0);
        builder->vertexUV((float)pos.x + wireMax, y, (float)(pos.z + 1), (float)u0, (float)v1);
        builder->vertexUV((float)pos.x + wireMax, y, (float)((double)pos.z + 0.75), (float)u1, (float)v1);
        builder->vertexUV((float)pos.x + wireMin, y, (float)((double)pos.z + 0.75), (float)u1, (float)v0);
        builder->vertexUV((float)pos.x + wireMin, y, (float)((double)pos.z + 0.75), (float)u1, (float)v0);
        builder->vertexUV((float)pos.x + wireMax, y, (float)((double)pos.z + 0.75), (float)u1, (float)v1);
        builder->vertexUV((float)pos.x + wireMax, y, (float)(pos.z + 1), (float)u0, (float)v1);
        builder->vertexUV((float)pos.x + wireMin, y, (float)(pos.z + 1), (float)u0, (float)v0);
    }
    if (west)
    {
        builder->vertexUV((float)pos.x, y, (float)pos.z + wireMax, (float)u0, (float)v1);
        builder->vertexUV((float)((double)pos.x + 0.25), y, (float)pos.z + wireMax, (float)u1, (float)v1);
        builder->vertexUV((float)((double)pos.x + 0.25), y, (float)pos.z + wireMin, (float)u1, (float)v0);
        builder->vertexUV((float)pos.x, y, (float)pos.z + wireMin, (float)u0, (float)v0);
        builder->vertexUV((float)pos.x, y, (float)pos.z + wireMin, (float)u0, (float)v0);
        builder->vertexUV((float)((double)pos.x + 0.25), y, (float)pos.z + wireMin, (float)u1, (float)v0);
        builder->vertexUV((float)((double)pos.x + 0.25), y, (float)pos.z + wireMax, (float)u1, (float)v1);
        builder->vertexUV((float)pos.x, y, (float)pos.z + wireMax, (float)u0, (float)v1);
    }
    if (west || (east && !north && !south))
    {
        builder->vertexUV((float)((double)pos.x + 0.25), y, (float)pos.z + wireMax, (float)u0, (float)v1);
        builder->vertexUV((float)((double)pos.x + 0.5), y, (float)pos.z + wireMax, (float)u1, (float)v1);
        builder->vertexUV((float)((double)pos.x + 0.5), y, (float)pos.z + wireMin, (float)u1, (float)v0);
        builder->vertexUV((float)((double)pos.x + 0.25), y, (float)pos.z + wireMin, (float)u0, (float)v0);
        builder->vertexUV((float)((double)pos.x + 0.25), y, (float)pos.z + wireMin, (float)u0, (float)v0);
        builder->vertexUV((float)((double)pos.x + 0.5), y, (float)pos.z + wireMin, (float)u1, (float)v0);
        builder->vertexUV((float)((double)pos.x + 0.5), y, (float)pos.z + wireMax, (float)u1, (float)v1);
        builder->vertexUV((float)((double)pos.x + 0.25), y, (float)pos.z + wireMax, (float)u0, (float)v1);
    }
    if (east || (west && !north && !south))
    {
        builder->vertexUV((float)((double)pos.x + 0.5), y, (float)pos.z + wireMax, (float)u0, (float)v1);
        builder->vertexUV((float)((double)pos.x + 0.75), y, (float)pos.z + wireMax, (float)u1, (float)v1);
        builder->vertexUV((float)((double)pos.x + 0.75), y, (float)pos.z + wireMin, (float)u1, (float)v0);
        builder->vertexUV((float)((double)pos.x + 0.5), y, (float)pos.z + wireMin, (float)u0, (float)v0);
        builder->vertexUV((float)((double)pos.x + 0.5), y, (float)pos.z + wireMin, (float)u0, (float)v0);
        builder->vertexUV((float)((double)pos.x + 0.75), y, (float)pos.z + wireMin, (float)u1, (float)v0);
        builder->vertexUV((float)((double)pos.x + 0.75), y, (float)pos.z + wireMax, (float)u1, (float)v1);
        builder->vertexUV((float)((double)pos.x + 0.5), y, (float)pos.z + wireMax, (float)u0, (float)v1);
    }
    if (east)
    {
        builder->vertexUV((float)((double)pos.x + 0.75), y, (float)pos.z + wireMax, (float)u0, (float)v1);
        builder->vertexUV((float)(pos.x + 1), y, (float)pos.z + wireMax, (float)u1, (float)v1);
        builder->vertexUV((float)(pos.x + 1), y, (float)pos.z + wireMin, (float)u1, (float)v0);
        builder->vertexUV((float)((double)pos.x + 0.75), y, (float)pos.z + wireMin, (float)u0, (float)v0);
        builder->vertexUV((float)((double)pos.x + 0.75), y, (float)pos.z + wireMin, (float)u0, (float)v0);
        builder->vertexUV((float)(pos.x + 1), y, (float)pos.z + wireMin, (float)u1, (float)v0);
        builder->vertexUV((float)(pos.x + 1), y, (float)pos.z + wireMax, (float)u1, (float)v1);
        builder->vertexUV((float)((double)pos.x + 0.75), y, (float)pos.z + wireMax, (float)u0, (float)v1);
    }
    return true;
}

bool BlockRenderer::tesselateBedInWorld(const BlockState* state, const BlockPos& pos)
{
    BufferBuilder* builder = Tesselator::getInstance()->getBuilder();
    Block* block = state->getBlock();
    const Direction* facing = state->getValue(HorizontalDirectionalBlock::FACING_PROPERTY);
    bool isHead = state->getValue(BedBlock::PART) == &gBedHead;

    float bottomBrightness = 0.5f;
    float topBrightness = 1.0f;
    float northSouthBrightness = 0.8f;
    float westEastBrightness = 0.6f;

    int lightColor = getLightColor(block, mLevelSource, pos);
    builder->tex2(lightColor);
    builder->color(bottomBrightness, bottomBrightness, bottomBrightness);
    TextureAtlasSprite* texture = getTexture(state, Direction::DOWN);
    float u0 = texture->getU0(true);
    float u1 = texture->getU1(true);
    float v0 = texture->getV0(true);
    float v1 = texture->getV1(true);
    float x0 = (float)pos.x + mMinX;
    float x1 = (float)pos.x + mMaxX;
    float y = (float)pos.y + mMinY + 0.1875f;
    float z0 = (float)pos.z + mMinZ;
    float z1 = (float)pos.z + mMaxZ;
    builder->vertexUV(x0, y, z1, u0, v1);
    builder->vertexUV(x0, y, z0, u0, v0);
    builder->vertexUV(x1, y, z0, u1, v0);
    builder->vertexUV(x1, y, z1, u1, v1);

    builder->tex2(getLightColor(block, mLevelSource, pos.above()));
    builder->color(topBrightness, topBrightness, topBrightness);
    texture = getTexture(state, Direction::UP);
    u0 = texture->getU0(true);
    u1 = texture->getU1(true);
    v0 = texture->getV0(true);
    v1 = texture->getV1(true);
    float uA = u0;
    float uB = u1;
    float vA = v0;
    float vB = v0;
    float uC = u0;
    float uD = u1;
    float vC = v1;
    float vD = v1;
    if (facing == Direction::SOUTH)
    {
        uB = u0;
        vA = v1;
        uC = u1;
        vD = v0;
    }
    else if (facing == Direction::NORTH)
    {
        uA = u1;
        vB = v1;
        uD = u0;
        vC = v0;
    }
    else if (facing == Direction::EAST)
    {
        uA = u1;
        vB = v1;
        uD = u0;
        vC = v0;
        uB = u0;
        vA = v1;
        uC = u1;
        vD = v0;
    }
    x1 = (float)pos.x + mMaxX;
    x0 = (float)pos.x + mMinX;
    float yTop = (float)pos.y + mMaxY;
    z1 = (float)pos.z + mMaxZ;
    z0 = (float)pos.z + mMinZ;
    builder->vertexUV(x1, yTop, z1, uC, vC);
    builder->vertexUV(x1, yTop, z0, uA, vA);
    builder->vertexUV(x0, yTop, z0, uB, vB);
    builder->vertexUV(x0, yTop, z1, uD, vD);

    const Direction* joinedFace = facing;
    if (isHead)
        joinedFace = facing->getOpposite();

    const Direction* flippedFace = Direction::WEST;
    switch (facing->get3DDataValue())
    {
    case 3:
        flippedFace = Direction::EAST;
        break;
    case 4:
        flippedFace = Direction::SOUTH;
        break;
    case 5:
        flippedFace = Direction::NORTH;
        break;
    }

    if (joinedFace != Direction::NORTH
        && (mRenderAllFaces || block->shouldRenderFace(state, mLevelSource, pos, Direction::NORTH)))
    {
        if (mMinZ <= 0.0f)
            builder->tex2(getLightColor(block, mLevelSource, pos.north()));
        else
            builder->tex2(lightColor);
        builder->color(northSouthBrightness, northSouthBrightness, northSouthBrightness);
        mFlipTexture = flippedFace == Direction::NORTH;
        renderNorth(state, (double)pos.x, (double)pos.y, (double)pos.z, getTexture(state, Direction::NORTH));
    }
    if (joinedFace != Direction::SOUTH
        && (mRenderAllFaces || block->shouldRenderFace(state, mLevelSource, pos, Direction::SOUTH)))
    {
        if (mMaxZ >= 1.0f)
            builder->tex2(getLightColor(block, mLevelSource, pos.south()));
        else
            builder->tex2(lightColor);
        builder->color(northSouthBrightness, northSouthBrightness, northSouthBrightness);
        mFlipTexture = flippedFace == Direction::SOUTH;
        renderSouth(state, (double)pos.x, (double)pos.y, (double)pos.z, getTexture(state, Direction::SOUTH));
    }
    if (joinedFace != Direction::WEST
        && (mRenderAllFaces || block->shouldRenderFace(state, mLevelSource, pos, Direction::WEST)))
    {
        if (mMinZ <= 0.0f)
            builder->tex2(getLightColor(block, mLevelSource, pos.west()));
        else
            builder->tex2(lightColor);
        builder->color(westEastBrightness, westEastBrightness, westEastBrightness);
        mFlipTexture = flippedFace == Direction::WEST;
        renderWest(state, (double)pos.x, (double)pos.y, (double)pos.z, getTexture(state, Direction::WEST));
    }
    if (joinedFace != Direction::EAST
        && (mRenderAllFaces || block->shouldRenderFace(state, mLevelSource, pos, Direction::EAST)))
    {
        if (mMaxZ >= 1.0f)
            builder->tex2(getLightColor(block, mLevelSource, pos.east()));
        else
            builder->tex2(lightColor);
        builder->color(westEastBrightness, westEastBrightness, westEastBrightness);
        mFlipTexture = flippedFace == Direction::EAST;
        renderEast(state, (double)pos.x, (double)pos.y, (double)pos.z, getTexture(state, Direction::EAST));
    }
    mFlipTexture = false;
    return true;
}

void BlockRenderer::tesselateDiodeInWorld(const BlockState* state, const BlockPos& pos, const Direction* direction)
{
    tesselateBlockInWorld(state, pos);

    BufferBuilder* builder = Tesselator::getInstance()->getBuilder();
    builder->tex2(getLightColor(state->getBlock(), mLevelSource, pos));
    builder->color(1.0f, 1.0f, 1.0f);

    TextureAtlasSprite* texture = getTexture(state, Direction::UP);
    float u0 = (float)texture->getU0(true);
    float u1 = (float)texture->getU1(true);
    float v0 = (float)texture->getV0(true);
    float v1 = (float)texture->getV1(true);

    float x0 = (float)pos.x;
    float x1 = x0 + 1.0f;
    float z0 = (float)pos.z;
    float z1 = z0 + 1.0f;
    float y = (float)pos.y + 0.125f;

    if (direction == Direction::NORTH)
    {
        builder->vertexUV(x1, y, z1, u0, v0);
        builder->vertexUV(x1, y, z0, u0, v1);
        builder->vertexUV(x0, y, z0, u1, v1);
        builder->vertexUV(x0, y, z1, u1, v0);
    }
    else if (direction == Direction::EAST)
    {
        builder->vertexUV(x0, y, z1, u0, v0);
        builder->vertexUV(x1, y, z1, u0, v1);
        builder->vertexUV(x1, y, z0, u1, v1);
        builder->vertexUV(x0, y, z0, u1, v0);
    }
    else if (direction == Direction::WEST)
    {
        builder->vertexUV(x1, y, z0, u0, v0);
        builder->vertexUV(x0, y, z0, u0, v1);
        builder->vertexUV(x0, y, z1, u1, v1);
        builder->vertexUV(x1, y, z1, u1, v0);
    }
    else
    {
        builder->vertexUV(x0, y, z0, u0, v0);
        builder->vertexUV(x0, y, z1, u0, v1);
        builder->vertexUV(x1, y, z1, u1, v1);
        builder->vertexUV(x1, y, z0, u1, v0);
    }
}

bool BlockRenderer::tesselateDiodeInWorld(const BlockState* state, const BlockPos& pos)
{
    const Direction* direction = state->getValue<const Direction*>(HorizontalDirectionalBlock::FACING_PROPERTY);
    tesselateDiodeInWorld(state, pos, direction);
    return true;
}

bool BlockRenderer::tesselateRepeaterInWorld(const BlockState* state, const BlockPos& pos)
{
    const Direction* facing = state->getValue(HorizontalDirectionalBlock::FACING_PROPERTY);
    int delayIndex = state->getValue(RepeaterBlock::DELAY) - 1;
    BufferBuilder* builder = Tesselator::getInstance()->getBuilder();
    builder->tex2(state->getLightColor(mLevelSource, pos));
    builder->color(1.0f, 1.0f, 1.0f);

    double torchY = -0.1875;
    bool locked = static_cast<RepeaterBlock*>(state->getBlock())->isLocked(mLevelSource, pos, state);
    double delayX = 0.0;
    double delayZ = 0.0;
    double fixedX = 0.0;
    double fixedZ = 0.0;
    switch (facing->get3DDataValue())
    {
    case 2:
        delayZ = -RepeaterBlock::DELAY_TORCH_OFFSETS[delayIndex];
        fixedZ = 0.3125;
        break;
    case 3:
        delayZ = RepeaterBlock::DELAY_TORCH_OFFSETS[delayIndex];
        fixedZ = -0.3125;
        break;
    case 4:
        delayX = -RepeaterBlock::DELAY_TORCH_OFFSETS[delayIndex];
        fixedX = 0.3125;
        break;
    case 5:
        delayX = RepeaterBlock::DELAY_TORCH_OFFSETS[delayIndex];
        fixedX = -0.3125;
        break;
    }

    if (!locked)
    {
        tesselateTorch(state, (float)((double)pos.x + delayX), (float)((double)pos.y + torchY),
                       (float)((double)pos.z + delayZ), 0.0f, 0.0f);
    }
    else
    {
        TextureAtlasSprite* bedrock = getTexture(Blocks::bedrock, Direction::UP);
        setFixedTexture(bedrock);
        float barX0 = 2.0f;
        float barX1 = 14.0f;
        float barZ0 = 7.0f;
        float barZ1 = 9.0f;
        int direction = facing->get3DDataValue();
        if (direction > 3 && direction < 6)
        {
            barX0 = 7.0f;
            barX1 = 9.0f;
            barZ0 = 2.0f;
            barZ1 = 14.0f;
        }
        float x0 = barX0 / 16.0f;
        float x1 = barX1 / 16.0f;
        float z0 = barZ0 / 16.0f;
        float z1 = barZ1 / 16.0f;
        setShape(x0 + (float)delayX, 0.125f, z0 + (float)delayZ, x1 + (float)delayX, 0.25f, z1 + (float)delayZ);
        float u0 = (float)bedrock->getU(barX0);
        float v0 = (float)bedrock->getV(barZ0);
        float u1 = (float)bedrock->getU(barX1);
        float v1 = (float)bedrock->getV(barZ1);
        float y = (float)pos.y + 0.25f;
        builder->vertexUV((float)((double)((float)pos.x + x0) + delayX), y,
                          (float)((double)((float)pos.z + z0) + delayZ), u0, v0);
        builder->vertexUV((float)((double)((float)pos.x + x0) + delayX), y,
                          (float)((double)((float)pos.z + z1) + delayZ), u0, v1);
        builder->vertexUV((float)((double)((float)pos.x + x1) + delayX), y,
                          (float)((double)((float)pos.z + z1) + delayZ), u1, v1);
        builder->vertexUV((float)((double)((float)pos.x + x1) + delayX), y,
                          (float)((double)((float)pos.z + z0) + delayZ), u1, v0);
        tesselateBlockInWorld(state, pos);
        setShape(0.0f, 0.0f, 0.0f, 1.0f, 0.125f, 1.0f);
        clearFixedTexture();
    }

    builder->tex2(state->getLightColor(mLevelSource, pos));
    builder->color(1.0f, 1.0f, 1.0f);
    tesselateTorch(state, (float)((double)pos.x + fixedX), (float)((double)pos.y + torchY),
                   (float)((double)pos.z + fixedZ), 0.0f, 0.0f);
    tesselateDiodeInWorld(state, pos);
    return true;
}

bool BlockRenderer::tesselateComparatorInWorld(const BlockState* state, const BlockPos& pos)
{
    BufferBuilder* builder = Tesselator::getInstance()->getBuilder();
    builder->tex2(state->getLightColor(mLevelSource, pos));
    builder->color(1.0f, 1.0f, 1.0f);
    const Direction* facing = state->getValue(HorizontalDirectionalBlock::FACING_PROPERTY);

    double frontX = 0.0;
    double frontY = -0.1875;
    double frontZ = 0.0;
    double sideX = 0.0;
    double sideZ = 0.0;
    TextureAtlasSprite* frontTexture;
    if (state->getValue(ComparatorBlock::MODE) == ComparatorBlock::ComparatorMode::SUBTRACT)
    {
        frontTexture = Blocks::redstone_torch->getTexture(Direction::DOWN);
    }
    else
    {
        frontY = -0.375;
        frontTexture = Blocks::unlit_redstone_torch->getTexture(Direction::DOWN);
    }

    switch (facing->get3DDataValue())
    {
    case 2:
        frontZ = 0.3125;
        sideZ = -1.0;
        break;
    case 3:
        frontZ = -0.3125;
        sideZ = 1.0;
        break;
    case 4:
        frontX = 0.3125;
        sideX = -1.0;
        break;
    case 5:
        frontX = -0.3125;
        sideX = 1.0;
        break;
    }

    float x = (float)pos.x;
    float y = (float)pos.y;
    float z = (float)pos.z;
    float torchY = y - 0.1875f;
    tesselateTorch(state, (float)(x + 0.25 * sideX + 0.1875 * sideZ), torchY,
                   (float)(z + 0.25 * sideZ + 0.1875 * sideX), 0.0f, 0.0f);
    tesselateTorch(state, (float)(x + 0.25 * sideX + -0.1875 * sideZ), torchY,
                   (float)(z + 0.25 * sideZ + -0.1875 * sideX), 0.0f, 0.0f);
    setFixedTexture(frontTexture);
    tesselateTorch(state, (float)(x + frontX), (float)(y + frontY), (float)(z + frontZ), 0.0f, 0.0f);
    clearFixedTexture();
    tesselateDiodeInWorld(state, pos, facing);
    return true;
}

bool BlockRenderer::tesselatePistonBaseInWorld(const BlockState* state, const BlockPos& pos, bool forceExtended,
                                               const BlockState* actualState)
{
    const BlockState* renderState = actualState == NULL ? state : actualState;
    bool extended = forceExtended || renderState->getValue(PistonBaseBlock::EXTENDED);
    const Direction* facing = renderState->getValue(PistonBaseBlock::FACING);
    float headThickness = PistonBaseBlock::PLATFORM_THICKNESS;

    if (extended)
    {
        switch (facing->get3DDataValue())
        {
        case 0:
            mFaceRotation1 = 3;
            mFaceRotation0 = 3;
            mFaceRotation2 = 3;
            mFaceRotation3 = 3;
            setShape(0.0f, headThickness, 0.0f, 1.0f, 1.0f, 1.0f);
            break;
        case 1:
            setShape(0.0f, 0.0f, 0.0f, 1.0f, 1.0f - headThickness, 1.0f);
            break;
        case 2:
            mFaceRotation3 = 2;
            mFaceRotation2 = 1;
            setShape(0.0f, 0.0f, headThickness, 1.0f, 1.0f, 1.0f);
            break;
        case 3:
            mFaceRotation4 = 3;
            mFaceRotation5 = 3;
            mFaceRotation2 = 2;
            mFaceRotation3 = 1;
            setShape(0.0f, 0.0f, 0.0f, 1.0f, 1.0f, 1.0f - headThickness);
            break;
        case 4:
            mFaceRotation1 = 2;
            mFaceRotation5 = 1;
            mFaceRotation4 = 2;
            mFaceTransform0 = 1;
            mFaceRotation0 = 1;
            setShape(headThickness, 0.0f, 0.0f, 1.0f, 1.0f, 1.0f);
            break;
        case 5:
            mFaceRotation1 = 1;
            mFaceRotation4 = 1;
            mFaceRotation5 = 2;
            mFaceRotation0 = 2;
            setShape(0.0f, 0.0f, 0.0f, 1.0f - headThickness, 1.0f, 1.0f);
            break;
        }
        mRenderFromInside = true;
        setShape(mMinX, mMinY, mMinZ, mMaxX, mMaxY, mMaxZ);
        tesselateBlockInWorld(renderState, pos);
        mFaceRotation1 = 0;
        mFaceRotation3 = 0;
        mFaceRotation4 = 0;
        mFaceRotation2 = 0;
        mFaceTransform0 = 0;
        mFaceRotation5 = 0;
        mFaceRotation0 = 0;
        setShape(mMinX, mMinY, mMinZ, mMaxX, mMaxY, mMaxZ);
        mRenderFromInside = false;
    }
    else
    {
        switch (facing->get3DDataValue())
        {
        case 0:
            mFaceRotation0 = 3;
            mFaceRotation1 = 3;
            mFaceRotation3 = 3;
            mFaceRotation2 = 3;
            break;
        case 2:
            mFaceRotation2 = 1;
            mFaceRotation3 = 2;
            break;
        case 3:
            mFaceRotation2 = 2;
            mFaceRotation4 = 3;
            mFaceRotation5 = 3;
            mFaceRotation3 = 1;
            break;
        case 4:
            mFaceRotation0 = 1;
            mFaceRotation1 = 2;
            mFaceTransform0 = 1;
            mFaceRotation4 = 2;
            mFaceRotation5 = 1;
            break;
        case 5:
            mFaceRotation4 = 1;
            mFaceRotation1 = 1;
            mFaceRotation5 = 2;
            mFaceRotation0 = 2;
            break;
        }
        tesselateBlockInWorld(renderState, pos);
        mFaceTransform0 = 0;
        mFaceRotation0 = 0;
        mFaceRotation1 = 0;
        mFaceRotation2 = 0;
        mFaceRotation3 = 0;
        mFaceRotation4 = 0;
        mFaceRotation5 = 0;
    }
    return true;
}

void BlockRenderer::renderPistonArmUpDown(float x0, float x1, float y0, float y1, float z0, float z1, float shade,
                                          float length)
{
    TextureAtlasSprite* texture = PistonBaseBlock::getTexture(PistonBaseBlock::TEXTURE_SIDE);
    if (hasFixedTexture())
        texture = mFixedTexture;
    BufferBuilder* builder = Tesselator::getInstance()->getBuilder();
    float u0 = (float)texture->getU0(false);
    float v0 = (float)texture->getV0(false);
    float u1 = (float)texture->getU(length);
    float v1 = (float)texture->getV(PistonBaseBlock::PLATFORM_THICKNESS);
    texture->adjustUVs(u0, u1, v0, v1);
    builder->color(shade, shade, shade);
    builder->vertexUV(x0, y1, z0, u1, v0);
    builder->vertexUV(x0, y0, z0, u0, v0);
    builder->vertexUV(x1, y0, z1, u0, v1);
    builder->vertexUV(x1, y1, z1, u1, v1);
}

void BlockRenderer::renderPistonArmNorthSouth(float x0, float x1, float y0, float y1, float z0, float z1, float shade,
                                              float length)
{
    TextureAtlasSprite* texture = PistonBaseBlock::getTexture(PistonBaseBlock::TEXTURE_SIDE);
    if (hasFixedTexture())
        texture = mFixedTexture;
    BufferBuilder* builder = Tesselator::getInstance()->getBuilder();
    float u0 = (float)texture->getU0(false);
    float v0 = (float)texture->getV0(false);
    float u1 = (float)texture->getU(length);
    float v1 = (float)texture->getV(PistonBaseBlock::PLATFORM_THICKNESS);
    texture->adjustUVs(u0, u1, v0, v1);
    builder->color(shade, shade, shade);
    builder->vertexUV(x0, y0, z1, u1, v0);
    builder->vertexUV(x0, y0, z0, u0, v0);
    builder->vertexUV(x1, y1, z0, u0, v1);
    builder->vertexUV(x1, y1, z1, u1, v1);
}

void BlockRenderer::renderPistonArmEastWest(float x0, float x1, float y0, float y1, float z0, float z1, float shade,
                                            float length)
{
    TextureAtlasSprite* texture = PistonBaseBlock::getTexture(PistonBaseBlock::TEXTURE_SIDE);
    if (hasFixedTexture())
        texture = mFixedTexture;
    BufferBuilder* builder = Tesselator::getInstance()->getBuilder();
    float u0 = (float)texture->getU0(false);
    float v0 = (float)texture->getV0(false);
    float u1 = (float)texture->getU(length);
    float v1 = (float)texture->getV(PistonBaseBlock::PLATFORM_THICKNESS);
    texture->adjustUVs(u0, u1, v0, v1);
    builder->color(shade, shade, shade);
    builder->vertexUV(x1, y0, z0, u1, v0);
    builder->vertexUV(x0, y0, z0, u0, v0);
    builder->vertexUV(x0, y1, z1, u0, v1);
    builder->vertexUV(x1, y1, z1, u1, v1);
}

bool BlockRenderer::tesselatePistonExtensionInWorld(const BlockState* state, const BlockPos& pos, bool fullArm,
                                                    const BlockState* actualState)
{
    mRenderFromInside = true;
    const BlockState* renderState = actualState == NULL ? state : actualState;
    const Direction* facing = renderState->getValue(PistonHeadBlock::FACING);

    float headThickness = PistonBaseBlock::PLATFORM_THICKNESS;
    float rodNear = 0.5f - headThickness * 0.5f;
    float rodFar = headThickness * 0.5f + 0.5f;
    float armLength;
    float textureLength;
    if (!fullArm)
    {
        armLength = 0.5f;
        textureLength = 8.0f;
    }
    else
    {
        armLength = 1.0f;
        textureLength = 16.0f;
    }

    BufferBuilder* builder = Tesselator::getInstance()->getBuilder();
    float x = (float)pos.x;
    float y = (float)pos.y;
    float z = (float)pos.z;
    switch (facing->get3DDataValue())
    {
    case 0:
    {
        mFaceRotation0 = 3;
        mFaceRotation1 = 3;
        mFaceRotation2 = 3;
        mFaceRotation3 = 3;
        setShape(0.0f, 0.0f, 0.0f, 1.0f, headThickness, 1.0f);
        tesselateBlockInWorld(renderState, pos);
        builder->tex2(getLightColor(state->getBlock(), mLevelSource, pos));
        float y0 = y + headThickness;
        float y1 = y0 + armLength;
        renderPistonArmUpDown(x + rodNear, x + rodFar, y0, y1, z + rodFar, z + rodFar, 0.8f, textureLength);
        renderPistonArmUpDown(x + rodFar, x + rodNear, y0, y1, z + rodNear, z + rodNear, 0.8f, textureLength);
        renderPistonArmUpDown(x + rodNear, x + rodNear, y0, y1, z + rodNear, z + rodFar, 0.6f, textureLength);
        renderPistonArmUpDown(x + rodFar, x + rodFar, y0, y1, z + rodFar, z + rodNear, 0.6f, textureLength);
        break;
    }
    case 1:
    {
        setShape(0.0f, 1.0f - headThickness, 0.0f, 1.0f, 1.0f, 1.0f);
        tesselateBlockInWorld(renderState, pos);
        builder->tex2(getLightColor(state->getBlock(), mLevelSource, pos));
        float y1 = y - headThickness + 1.0f;
        float y0 = y1 - armLength;
        renderPistonArmUpDown(x + rodNear, x + rodFar, y0, y1, z + rodFar, z + rodFar, 0.8f, textureLength);
        renderPistonArmUpDown(x + rodFar, x + rodNear, y0, y1, z + rodNear, z + rodNear, 0.8f, textureLength);
        renderPistonArmUpDown(x + rodNear, x + rodNear, y0, y1, z + rodNear, z + rodFar, 0.6f, textureLength);
        renderPistonArmUpDown(x + rodFar, x + rodFar, y0, y1, z + rodFar, z + rodNear, 0.6f, textureLength);
        break;
    }
    case 2:
    {
        mFaceRotation2 = 1;
        mFaceRotation3 = 2;
        setShape(0.0f, 0.0f, 0.0f, 1.0f, 1.0f, headThickness);
        tesselateBlockInWorld(renderState, pos);
        builder->tex2(getLightColor(state->getBlock(), mLevelSource, pos));
        float z0 = z + headThickness;
        float z1 = z0 + armLength;
        renderPistonArmNorthSouth(x + rodNear, x + rodNear, y + rodFar, y + rodNear, z0, z1, 0.6f, textureLength);
        renderPistonArmNorthSouth(x + rodFar, x + rodFar, y + rodNear, y + rodFar, z0, z1, 0.6f, textureLength);
        renderPistonArmNorthSouth(x + rodNear, x + rodFar, y + rodNear, y + rodNear, z0, z1, 0.5f, textureLength);
        renderPistonArmNorthSouth(x + rodFar, x + rodNear, y + rodFar, y + rodFar, z0, z1, 1.0f, textureLength);
        break;
    }
    case 3:
    {
        mFaceRotation2 = 2;
        mFaceRotation3 = 1;
        mFaceRotation4 = 3;
        mFaceRotation5 = 3;
        setShape(0.0f, 0.0f, 1.0f - headThickness, 1.0f, 1.0f, 1.0f);
        tesselateBlockInWorld(renderState, pos);
        builder->tex2(getLightColor(state->getBlock(), mLevelSource, pos));
        float z1 = z - headThickness + 1.0f;
        float z0 = z1 - armLength;
        renderPistonArmNorthSouth(x + rodNear, x + rodNear, y + rodFar, y + rodNear, z0, z1, 0.6f, textureLength);
        renderPistonArmNorthSouth(x + rodFar, x + rodFar, y + rodNear, y + rodFar, z0, z1, 0.6f, textureLength);
        renderPistonArmNorthSouth(x + rodNear, x + rodFar, y + rodNear, y + rodNear, z0, z1, 0.5f, textureLength);
        renderPistonArmNorthSouth(x + rodFar, x + rodNear, y + rodFar, y + rodFar, z0, z1, 1.0f, textureLength);
        break;
    }
    case 4:
    {
        mFaceRotation0 = 1;
        mFaceRotation1 = 2;
        mFaceRotation4 = 2;
        mFaceRotation5 = 1;
        setShape(0.0f, 0.0f, 0.0f, headThickness, 1.0f, 1.0f);
        tesselateBlockInWorld(renderState, pos);
        builder->tex2(getLightColor(state->getBlock(), mLevelSource, pos));
        float x0 = x + headThickness;
        float x1 = x0 + armLength;
        renderPistonArmEastWest(x0, x1, y + rodNear, y + rodNear, z + rodFar, z + rodNear, 0.5f, textureLength);
        renderPistonArmEastWest(x0, x1, y + rodFar, y + rodFar, z + rodNear, z + rodFar, 1.0f, textureLength);
        renderPistonArmEastWest(x0, x1, y + rodNear, y + rodFar, z + rodNear, z + rodNear, 0.6f, textureLength);
        renderPistonArmEastWest(x0, x1, y + rodFar, y + rodNear, z + rodFar, z + rodFar, 0.6f, textureLength);
        break;
    }
    case 5:
    {
        mFaceRotation0 = 2;
        mFaceRotation1 = 1;
        mFaceRotation4 = 1;
        mFaceRotation5 = 2;
        setShape(1.0f - headThickness, 0.0f, 0.0f, 1.0f, 1.0f, 1.0f);
        tesselateBlockInWorld(renderState, pos);
        builder->tex2(getLightColor(state->getBlock(), mLevelSource, pos));
        float x1 = x - headThickness + 1.0f;
        float x0 = x1 - armLength;
        renderPistonArmEastWest(x0, x1, y + rodNear, y + rodNear, z + rodFar, z + rodNear, 0.5f, textureLength);
        renderPistonArmEastWest(x0, x1, y + rodFar, y + rodFar, z + rodNear, z + rodFar, 1.0f, textureLength);
        renderPistonArmEastWest(x0, x1, y + rodNear, y + rodFar, z + rodNear, z + rodNear, 0.6f, textureLength);
        renderPistonArmEastWest(x0, x1, y + rodFar, y + rodNear, z + rodFar, z + rodFar, 0.6f, textureLength);
        break;
    }
    }

    mFaceRotation0 = 0;
    mFaceRotation1 = 0;
    mFaceRotation2 = 0;
    mFaceRotation3 = 0;
    mFaceRotation4 = 0;
    mFaceRotation5 = 0;
    setShape(0.0f, 0.0f, 0.0f, 1.0f, 1.0f, 1.0f);
    mRenderFromInside = false;
    return true;
}

bool BlockRenderer::tesselateThinFenceInWorld(const BlockState* state, const BlockPos& pos)
{
    BufferBuilder* builder = Tesselator::getInstance()->getBuilder();
    Block* block = state->getBlock();
    builder->tex2(state->getLightColor(mLevelSource, pos));

    int color = mBlockColors->getColor(state, mLevelSource, &pos, 0);
    float red = (float)(color >> 16 & 255) / 255.0f;
    float green = (float)(color >> 8 & 255) / 255.0f;
    float blue = (float)(color & 255) / 255.0f;
    if (GameRenderer::anaglyph3d)
    {
        float anaglyphRed = (blue * 11.0f + (green * 59.0f + red * 30.0f)) / 100.0f;
        float anaglyphGreen = (green * 70.0f + red * 30.0f) / 100.0f;
        float anaglyphBlue = (blue * 70.0f + red * 30.0f) / 100.0f;
        red = anaglyphRed;
        green = anaglyphGreen;
        blue = anaglyphBlue;
    }
    builder->color(red, green, blue);

    bool stainedPane = dynamic_cast<StainedGlassPaneBlock*>(block) != NULL;
    TextureAtlasSprite* texture;
    TextureAtlasSprite* edgeTexture;
    if (hasFixedTexture())
    {
        texture = mFixedTexture;
        edgeTexture = texture;
    }
    else
    {
        texture = getTexture(state, Direction::UP);
        edgeTexture = stainedPane ? static_cast<StainedGlassPaneBlock*>(block)->getEdgeTexture(state)
                                  : static_cast<ThinFenceBlock*>(block)->getEdgeTexture();
    }

    double u0 = texture->getU0(false);
    double u7 = texture->getU(7.0);
    double u9 = texture->getU(9.0);
    double u16 = texture->getU1(false);
    double v0 = texture->getV0(false);
    double v16 = texture->getV1(false);
    double edgeU7 = edgeTexture->getU(7.0);
    double edgeU9 = edgeTexture->getU(9.0);
    double edgeV0 = edgeTexture->getV0(false);
    double edgeV16 = edgeTexture->getV1(false);
    double edgeV7 = edgeTexture->getV(7.0);
    double edgeV9 = edgeTexture->getV(9.0);

    double xMin = pos.x;
    double xCoreMin = xMin + 0.4375;
    double xMax = pos.x + 1;
    double zMin = pos.z;
    double zMax = pos.z + 1;
    double xCoreMax = xMin + 0.5625;
    double zCoreMax = zMin + 0.5625;
    double zCoreMin = zMin + 0.4375;

    bool north = state->getValue(THIN_FENCE_NORTH);
    bool south = state->getValue(THIN_FENCE_SOUTH);
    bool west = state->getValue(THIN_FENCE_WEST);
    bool east = state->getValue(THIN_FENCE_EAST);
    float yTop = pos.y + 0.999;
    float yBottom = pos.y + 0.001;

    if (west)
    {
        if (north)
        {
            builder->vertexUV(xCoreMin, yTop, zCoreMin, u7, v0);
            builder->vertexUV(xCoreMin, yBottom, zCoreMin, u7, v16);
            builder->vertexUV(xMin, yBottom, zCoreMin, u0, v16);
            builder->vertexUV(xMin, yTop, zCoreMin, u0, v0);
            if (east)
            {
                builder->vertexUV(xMax, yTop, zCoreMin, u16, v0);
                builder->vertexUV(xMax, yBottom, zCoreMin, u16, v16);
                builder->vertexUV(xCoreMax, yBottom, zCoreMin, u9, v16);
                builder->vertexUV(xCoreMax, yTop, zCoreMin, u9, v0);
            }
        }
        else if (east)
        {
            builder->vertexUV(xMax, yTop, zCoreMin, u16, v0);
            builder->vertexUV(xMax, yBottom, zCoreMin, u16, v16);
            builder->vertexUV(xMin, yBottom, zCoreMin, u0, v16);
            builder->vertexUV(xMin, yTop, zCoreMin, u0, v0);
        }
        else
        {
            builder->vertexUV(xCoreMax, yTop, zCoreMin, u9, v0);
            builder->vertexUV(xCoreMax, yBottom, zCoreMin, u9, v16);
            builder->vertexUV(xMin, yBottom, zCoreMin, u0, v16);
            builder->vertexUV(xMin, yTop, zCoreMin, u0, v0);
        }
        if (south)
        {
            builder->vertexUV(xMin, yTop, zCoreMax, u0, v0);
            builder->vertexUV(xMin, yBottom, zCoreMax, u0, v16);
            builder->vertexUV(xCoreMin, yBottom, zCoreMax, u7, v16);
            builder->vertexUV(xCoreMin, yTop, zCoreMax, u7, v0);
            if (east)
            {
                builder->vertexUV(xCoreMax, yTop, zCoreMax, u9, v0);
                builder->vertexUV(xCoreMax, yBottom, zCoreMax, u9, v16);
                builder->vertexUV(xMax, yBottom, zCoreMax, u16, v16);
                builder->vertexUV(xMax, yTop, zCoreMax, u16, v0);
            }
        }
        else if (east)
        {
            builder->vertexUV(xMin, yTop, zCoreMax, u0, v0);
            builder->vertexUV(xMin, yBottom, zCoreMax, u0, v16);
            builder->vertexUV(xMax, yBottom, zCoreMax, u16, v16);
            builder->vertexUV(xMax, yTop, zCoreMax, u16, v0);
        }
        else
        {
            builder->vertexUV(xMin, yTop, zCoreMax, u0, v0);
            builder->vertexUV(xMin, yBottom, zCoreMax, u0, v16);
            builder->vertexUV(xCoreMax, yBottom, zCoreMax, u9, v16);
            builder->vertexUV(xCoreMax, yTop, zCoreMax, u9, v0);
        }
        if (east)
        {
            builder->vertexUV(xMin, yTop, zCoreMax, edgeU9, edgeV0);
            builder->vertexUV(xMax, yTop, zCoreMax, edgeU9, edgeV16);
            builder->vertexUV(xMax, yTop, zCoreMin, edgeU7, edgeV16);
            builder->vertexUV(xMin, yTop, zCoreMin, edgeU7, edgeV0);
            builder->vertexUV(xMax, yBottom, zCoreMax, edgeU7, edgeV16);
            builder->vertexUV(xMin, yBottom, zCoreMax, edgeU7, edgeV0);
            builder->vertexUV(xMin, yBottom, zCoreMin, edgeU9, edgeV0);
            builder->vertexUV(xMax, yBottom, zCoreMin, edgeU9, edgeV16);
        }
        else
        {
            builder->vertexUV(xMin, yTop, zCoreMax, edgeU9, edgeV0);
            builder->vertexUV(xCoreMin, yTop, zCoreMax, edgeU9, edgeV7);
            builder->vertexUV(xCoreMin, yTop, zCoreMin, edgeU7, edgeV7);
            builder->vertexUV(xMin, yTop, zCoreMin, edgeU7, edgeV0);
            builder->vertexUV(xCoreMin, yBottom, zCoreMax, edgeU7, edgeV7);
            builder->vertexUV(xMin, yBottom, zCoreMax, edgeU7, edgeV0);
            builder->vertexUV(xMin, yBottom, zCoreMin, edgeU9, edgeV0);
            builder->vertexUV(xCoreMin, yBottom, zCoreMin, edgeU9, edgeV7);
        }
        if (!east && !north && !south)
        {
            builder->vertexUV(xCoreMax, yTop, zCoreMax, u9, v0);
            builder->vertexUV(xCoreMax, yBottom, zCoreMax, u9, v16);
            builder->vertexUV(xCoreMax, yBottom, zCoreMin, u7, v16);
            builder->vertexUV(xCoreMax, yTop, zCoreMin, u7, v0);
        }
    }
    else if (east)
    {
        if (!north && !south)
        {
            builder->vertexUV(xCoreMin, yTop, zCoreMin, u9, v0);
            builder->vertexUV(xCoreMin, yBottom, zCoreMin, u9, v16);
            builder->vertexUV(xCoreMin, yBottom, zCoreMax, u7, v16);
            builder->vertexUV(xCoreMin, yTop, zCoreMax, u7, v0);
        }
        if (south)
        {
            builder->vertexUV(xCoreMax, yTop, zCoreMax, u9, v0);
            builder->vertexUV(xCoreMax, yBottom, zCoreMax, u9, v16);
            builder->vertexUV(xMax, yBottom, zCoreMax, u16, v16);
            builder->vertexUV(xMax, yTop, zCoreMax, u16, v0);
        }
        else
        {
            builder->vertexUV(xCoreMin, yTop, zCoreMax, u7, v0);
            builder->vertexUV(xCoreMin, yBottom, zCoreMax, u7, v16);
            builder->vertexUV(xMax, yBottom, zCoreMax, u16, v16);
            builder->vertexUV(xMax, yTop, zCoreMax, u16, v0);
        }
        if (north)
        {
            builder->vertexUV(xMax, yTop, zCoreMin, u16, v0);
            builder->vertexUV(xMax, yBottom, zCoreMin, u16, v16);
            builder->vertexUV(xCoreMax, yBottom, zCoreMin, u9, v16);
            builder->vertexUV(xCoreMax, yTop, zCoreMin, u9, v0);
        }
        else
        {
            builder->vertexUV(xMax, yTop, zCoreMin, u16, v0);
            builder->vertexUV(xMax, yBottom, zCoreMin, u16, v16);
            builder->vertexUV(xCoreMin, yBottom, zCoreMin, u7, v16);
            builder->vertexUV(xCoreMin, yTop, zCoreMin, u7, v0);
        }
        builder->vertexUV(xCoreMax, yTop, zCoreMax, edgeU9, edgeV9);
        builder->vertexUV(xMax, yTop, zCoreMax, edgeU9, edgeV0);
        builder->vertexUV(xMax, yTop, zCoreMin, edgeU7, edgeV0);
        builder->vertexUV(xCoreMax, yTop, zCoreMin, edgeU7, edgeV9);
        builder->vertexUV(xMax, yBottom, zCoreMax, edgeU7, edgeV16);
        builder->vertexUV(xCoreMax, yBottom, zCoreMax, edgeU7, edgeV9);
        builder->vertexUV(xCoreMax, yBottom, zCoreMin, edgeU9, edgeV9);
        builder->vertexUV(xMax, yBottom, zCoreMin, edgeU9, edgeV16);
    }
    else
    {
        if (!north && !south)
        {
            builder->vertexUV(xCoreMin, yTop, zCoreMin, u9, v0);
            builder->vertexUV(xCoreMin, yBottom, zCoreMin, u9, v16);
            builder->vertexUV(xCoreMin, yBottom, zCoreMax, u7, v16);
            builder->vertexUV(xCoreMin, yTop, zCoreMax, u7, v0);
            builder->vertexUV(xCoreMax, yTop, zCoreMax, u9, v0);
            builder->vertexUV(xCoreMax, yBottom, zCoreMax, u9, v16);
            builder->vertexUV(xCoreMax, yBottom, zCoreMin, u7, v16);
            builder->vertexUV(xCoreMax, yTop, zCoreMin, u7, v0);
        }
        if (!north)
        {
            builder->vertexUV(xCoreMax, yTop, zCoreMin, u9, v0);
            builder->vertexUV(xCoreMax, yBottom, zCoreMin, u9, v16);
            builder->vertexUV(xCoreMin, yBottom, zCoreMin, u7, v16);
            builder->vertexUV(xCoreMin, yTop, zCoreMin, u7, v0);
        }
    }

    if (north || south)
    {
        if (west)
        {
            if (north)
            {
                builder->vertexUV(xCoreMin, yTop, zMin, u0, v0);
                builder->vertexUV(xCoreMin, yBottom, zMin, u0, v16);
                builder->vertexUV(xCoreMin, yBottom, zCoreMin, u7, v16);
                builder->vertexUV(xCoreMin, yTop, zCoreMin, u7, v0);
            }
            if (south)
            {
                builder->vertexUV(xCoreMin, yTop, zCoreMax, u9, v0);
                builder->vertexUV(xCoreMin, yBottom, zCoreMax, u9, v16);
                builder->vertexUV(xCoreMin, yBottom, zMax, u16, v16);
                builder->vertexUV(xCoreMin, yTop, zMax, u16, v0);
            }
        }
        else
        {
            builder->vertexUV(xCoreMin, yTop, north ? zMin : zCoreMin, north ? u0 : u7, v0);
            builder->vertexUV(xCoreMin, yBottom, north ? zMin : zCoreMin, north ? u0 : u7, v16);
            builder->vertexUV(xCoreMin, yBottom, south ? zMax : zCoreMax, south ? u16 : u9, v16);
            builder->vertexUV(xCoreMin, yTop, south ? zMax : zCoreMax, south ? u16 : u9, v0);
        }
        if (east)
        {
            if (north)
            {
                builder->vertexUV(xCoreMax, yTop, zCoreMin, u7, v0);
                builder->vertexUV(xCoreMax, yBottom, zCoreMin, u7, v16);
                builder->vertexUV(xCoreMax, yBottom, zMin, u0, v16);
                builder->vertexUV(xCoreMax, yTop, zMin, u0, v0);
            }
            if (south)
            {
                builder->vertexUV(xCoreMax, yTop, zMax, u16, v0);
                builder->vertexUV(xCoreMax, yBottom, zMax, u16, v16);
                builder->vertexUV(xCoreMax, yBottom, zCoreMax, u9, v16);
                builder->vertexUV(xCoreMax, yTop, zCoreMax, u9, v0);
            }
        }
        else
        {
            builder->vertexUV(xCoreMax, yTop, south ? zMax : zCoreMax, south ? u16 : u9, v0);
            builder->vertexUV(xCoreMax, yBottom, south ? zMax : zCoreMax, south ? u16 : u9, v16);
            builder->vertexUV(xCoreMax, yBottom, north ? zMin : zCoreMin, north ? u0 : u7, v16);
            builder->vertexUV(xCoreMax, yTop, north ? zMin : zCoreMin, north ? u0 : u7, v0);
        }
        builder->vertexUV(xCoreMax, yTop, north ? zMin : zCoreMax, edgeU9, north ? edgeV0 : edgeV9);
        builder->vertexUV(xCoreMin, yTop, north ? zMin : zCoreMax, edgeU7, north ? edgeV0 : edgeV9);
        builder->vertexUV(xCoreMin, yTop, south ? zMax : zCoreMin, edgeU7, south ? edgeV16 : edgeV7);
        builder->vertexUV(xCoreMax, yTop, south ? zMax : zCoreMin, edgeU9, south ? edgeV16 : edgeV7);
        builder->vertexUV(xCoreMin, yBottom, north ? zMin : zCoreMax, edgeU7, north ? edgeV0 : edgeV9);
        builder->vertexUV(xCoreMax, yBottom, north ? zMin : zCoreMax, edgeU9, north ? edgeV0 : edgeV9);
        builder->vertexUV(xCoreMax, yBottom, south ? zMax : zCoreMin, edgeU9, south ? edgeV16 : edgeV7);
        builder->vertexUV(xCoreMin, yBottom, south ? zMax : zCoreMin, edgeU7, south ? edgeV16 : edgeV7);
    }

    if (!south && !west && !east)
    {
        builder->vertexUV(xCoreMin, yTop, zCoreMax, u9, v0);
        builder->vertexUV(xCoreMin, yBottom, zCoreMax, u9, v16);
        builder->vertexUV(xCoreMax, yBottom, zCoreMax, u7, v16);
        builder->vertexUV(xCoreMax, yTop, zCoreMax, u7, v0);
    }
    builder->vertexUV(xCoreMax, yTop, zCoreMin, edgeU9, edgeV7);
    builder->vertexUV(xCoreMin, yTop, zCoreMin, edgeU7, edgeV7);
    builder->vertexUV(xCoreMin, yTop, zCoreMax, edgeU7, edgeV9);
    builder->vertexUV(xCoreMax, yTop, zCoreMax, edgeU9, edgeV9);
    builder->vertexUV(xCoreMin, yBottom, zCoreMin, edgeU7, edgeV7);
    builder->vertexUV(xCoreMax, yBottom, zCoreMin, edgeU9, edgeV7);
    builder->vertexUV(xCoreMax, yBottom, zCoreMax, edgeU9, edgeV9);
    builder->vertexUV(xCoreMin, yBottom, zCoreMax, edgeU7, edgeV9);
    return true;
}

bool BlockRenderer::tesselateVineInWorld(const BlockState* state, const BlockPos& pos)
{
    BufferBuilder* builder = Tesselator::getInstance()->getBuilder();
    TextureAtlasSprite* tex = getTexture(state, NULL);
    if (hasFixedTexture())
        tex = mFixedTexture;

    builder->tex2(getLightColor(state->getBlock(), mLevelSource, pos));
    int color = mBlockColors->getColor(state, mLevelSource, &pos, 0);
    builder->color((float)(color >> 16 & 255) / 255.0f, (float)(color >> 8 & 255) / 255.0f,
                   (float)(color & 255) / 255.0f);

    float u0 = tex->getU0(true);
    float v0 = tex->getV0(true);
    float u1 = tex->getU1(true);
    float v1 = tex->getV1(true);
    int x = pos.x;
    int y = pos.y;
    int z = pos.z;
    float r = 0.05f;

    if (state->getValue(VineBlock::WEST))
    {
        builder->vertexUV(x + r, y + 1, z + 1, u0, v0);
        builder->vertexUV(x + r, y + 0, z + 1, u0, v1);
        builder->vertexUV(x + r, y + 0, z + 0, u1, v1);
        builder->vertexUV(x + r, y + 1, z + 0, u1, v0);

        builder->vertexUV(x + r, y + 1, z + 0, u1, v0);
        builder->vertexUV(x + r, y + 0, z + 0, u1, v1);
        builder->vertexUV(x + r, y + 0, z + 1, u0, v1);
        builder->vertexUV(x + r, y + 1, z + 1, u0, v0);
    }
    if (state->getValue(VineBlock::EAST))
    {
        builder->vertexUV(x + 1 - r, y + 0, z + 1, u1, v1);
        builder->vertexUV(x + 1 - r, y + 1, z + 1, u1, v0);
        builder->vertexUV(x + 1 - r, y + 1, z + 0, u0, v0);
        builder->vertexUV(x + 1 - r, y + 0, z + 0, u0, v1);

        builder->vertexUV(x + 1 - r, y + 0, z + 0, u0, v1);
        builder->vertexUV(x + 1 - r, y + 1, z + 0, u0, v0);
        builder->vertexUV(x + 1 - r, y + 1, z + 1, u1, v0);
        builder->vertexUV(x + 1 - r, y + 0, z + 1, u1, v1);
    }
    if (state->getValue(VineBlock::NORTH))
    {
        builder->vertexUV(x + 1, y + 0, z + r, u1, v1);
        builder->vertexUV(x + 1, y + 1, z + r, u1, v0);
        builder->vertexUV(x + 0, y + 1, z + r, u0, v0);
        builder->vertexUV(x + 0, y + 0, z + r, u0, v1);

        builder->vertexUV(x + 0, y + 0, z + r, u0, v1);
        builder->vertexUV(x + 0, y + 1, z + r, u0, v0);
        builder->vertexUV(x + 1, y + 1, z + r, u1, v0);
        builder->vertexUV(x + 1, y + 0, z + r, u1, v1);
    }
    if (state->getValue(VineBlock::SOUTH))
    {
        builder->vertexUV(x + 1, y + 1, z + 1 - r, u0, v0);
        builder->vertexUV(x + 1, y + 0, z + 1 - r, u0, v1);
        builder->vertexUV(x + 0, y + 0, z + 1 - r, u1, v1);
        builder->vertexUV(x + 0, y + 1, z + 1 - r, u1, v0);

        builder->vertexUV(x + 0, y + 1, z + 1 - r, u1, v0);
        builder->vertexUV(x + 0, y + 0, z + 1 - r, u1, v1);
        builder->vertexUV(x + 1, y + 0, z + 1 - r, u0, v1);
        builder->vertexUV(x + 1, y + 1, z + 1 - r, u0, v0);
    }
    if (mLevelSource->getBlockState(pos.above())->isSolidBlockingCubeAndNotSignalSource())
    {
        builder->vertexUV(x + 1, y + 1 - r, z + 0, u0, v0);
        builder->vertexUV(x + 1, y + 1 - r, z + 1, u0, v1);
        builder->vertexUV(x + 0, y + 1 - r, z + 1, u1, v1);
        builder->vertexUV(x + 0, y + 1 - r, z + 0, u1, v0);
    }
    return true;
}

bool BlockRenderer::tesselateFenceGateInWorld(const BlockState* state, const BlockPos& pos)
{
    bool open = state->getValue(FenceGateBlock::OPEN);
    const Direction* facing = state->getValue(HorizontalDirectionalBlock::FACING_PROPERTY);

    float gateTop = 0.9375f;
    float gateBottom = 0.375f;
    float upperBottom = 0.75f;
    float postBottom = 0.3125f;
    float postTop;
    float middleTop;
    if (((facing == Direction::NORTH || facing == Direction::SOUTH)
         && mLevelSource->getBlockState(pos.offset(-1, 0, 0))->getBlock() == Blocks::cobblestone_wall
         && mLevelSource->getBlockState(pos.offset(1, 0, 0))->getBlock() == Blocks::cobblestone_wall)
        || ((facing == Direction::EAST || facing == Direction::WEST)
            && mLevelSource->getBlockState(pos.offset(0, 0, -1))->getBlock() == Blocks::cobblestone_wall
            && mLevelSource->getBlockState(pos.offset(0, 0, 1))->getBlock() == Blocks::cobblestone_wall))
    {
        postTop = 0.8125f;
        gateTop = 0.75f;
        upperBottom = 0.5625f;
        gateBottom = 0.1875f;
        postBottom = 0.125f;
        middleTop = 0.375f;
    }
    else
    {
        postTop = 1.0f;
        middleTop = 0.5625f;
    }

    mRenderAllFaces = true;
    if (facing == Direction::EAST || facing == Direction::WEST)
    {
        mFaceRotation4 = 1;
        setShape(0.4375f, postBottom, 0.0f, 0.5625f, postTop, 0.125f);
        tesselateBlockInWorld(state, pos);
        setShape(0.4375f, postBottom, 0.875f, 0.5625f, postTop, 1.0f);
        tesselateBlockInWorld(state, pos);
        mFaceRotation4 = 0;
    }
    else
    {
        setShape(0.0f, postBottom, 0.4375f, 0.125f, postTop, 0.5625f);
        tesselateBlockInWorld(state, pos);
        setShape(0.875f, postBottom, 0.4375f, 1.0f, postTop, 0.5625f);
        tesselateBlockInWorld(state, pos);
    }

    if (!open)
    {
        if (facing == Direction::EAST || facing == Direction::WEST)
        {
            mFaceRotation4 = 1;
            setShape(0.4375f, gateBottom, 0.375f, 0.5625f, gateTop, 0.5f);
            tesselateBlockInWorld(state, pos);
            setShape(0.4375f, gateBottom, 0.5f, 0.5625f, gateTop, 0.625f);
            tesselateBlockInWorld(state, pos);
            setShape(0.4375f, gateBottom, 0.625f, 0.5625f, middleTop, 0.875f);
            tesselateBlockInWorld(state, pos);
            setShape(0.4375f, upperBottom, 0.625f, 0.5625f, gateTop, 0.875f);
            tesselateBlockInWorld(state, pos);
            setShape(0.4375f, gateBottom, 0.125f, 0.5625f, middleTop, 0.375f);
            tesselateBlockInWorld(state, pos);
            setShape(0.4375f, upperBottom, 0.125f, 0.5625f, gateTop, 0.375f);
            tesselateBlockInWorld(state, pos);
        }
        else
        {
            setShape(0.375f, gateBottom, 0.4375f, 0.5f, gateTop, 0.5625f);
            tesselateBlockInWorld(state, pos);
            setShape(0.5f, gateBottom, 0.4375f, 0.625f, gateTop, 0.5625f);
            tesselateBlockInWorld(state, pos);
            setShape(0.625f, gateBottom, 0.4375f, 0.875f, middleTop, 0.5625f);
            tesselateBlockInWorld(state, pos);
            setShape(0.625f, upperBottom, 0.4375f, 0.875f, gateTop, 0.5625f);
            tesselateBlockInWorld(state, pos);
            setShape(0.125f, gateBottom, 0.4375f, 0.375f, middleTop, 0.5625f);
            tesselateBlockInWorld(state, pos);
            setShape(0.125f, upperBottom, 0.4375f, 0.375f, gateTop, 0.5625f);
            tesselateBlockInWorld(state, pos);
        }
    }
    else
    {
        if (facing == Direction::NORTH || facing == Direction::SOUTH)
            mFaceRotation4 = 1;

        if (facing == Direction::EAST)
        {
            setShape(0.8125f, gateBottom, 0.0f, 0.9375f, gateTop, 0.125f);
            tesselateBlockInWorld(state, pos);
            setShape(0.8125f, gateBottom, 0.875f, 0.9375f, gateTop, 1.0f);
            tesselateBlockInWorld(state, pos);
            setShape(0.5625f, gateBottom, 0.0f, 0.8125f, middleTop, 0.125f);
            tesselateBlockInWorld(state, pos);
            setShape(0.5625f, gateBottom, 0.875f, 0.8125f, middleTop, 1.0f);
            tesselateBlockInWorld(state, pos);
            setShape(0.5625f, upperBottom, 0.0f, 0.8125f, gateTop, 0.125f);
            tesselateBlockInWorld(state, pos);
            setShape(0.5625f, upperBottom, 0.875f, 0.8125f, gateTop, 1.0f);
            tesselateBlockInWorld(state, pos);
        }
        else if (facing == Direction::WEST)
        {
            setShape(0.0625f, gateBottom, 0.0f, 0.1875f, gateTop, 0.125f);
            tesselateBlockInWorld(state, pos);
            setShape(0.0625f, gateBottom, 0.875f, 0.1875f, gateTop, 1.0f);
            tesselateBlockInWorld(state, pos);
            setShape(0.1875f, gateBottom, 0.0f, 0.4375f, middleTop, 0.125f);
            tesselateBlockInWorld(state, pos);
            setShape(0.1875f, gateBottom, 0.875f, 0.4375f, middleTop, 1.0f);
            tesselateBlockInWorld(state, pos);
            setShape(0.1875f, upperBottom, 0.0f, 0.4375f, gateTop, 0.125f);
            tesselateBlockInWorld(state, pos);
            setShape(0.1875f, upperBottom, 0.875f, 0.4375f, gateTop, 1.0f);
            tesselateBlockInWorld(state, pos);
        }
        else if (facing == Direction::SOUTH)
        {
            setShape(0.0f, gateBottom, 0.8125f, 0.125f, gateTop, 0.9375f);
            tesselateBlockInWorld(state, pos);
            setShape(0.875f, gateBottom, 0.8125f, 1.0f, gateTop, 0.9375f);
            tesselateBlockInWorld(state, pos);
            setShape(0.0f, gateBottom, 0.5625f, 0.125f, middleTop, 0.8125f);
            tesselateBlockInWorld(state, pos);
            setShape(0.875f, gateBottom, 0.5625f, 1.0f, middleTop, 0.8125f);
            tesselateBlockInWorld(state, pos);
            setShape(0.0f, upperBottom, 0.5625f, 0.125f, gateTop, 0.8125f);
            tesselateBlockInWorld(state, pos);
            setShape(0.875f, upperBottom, 0.5625f, 1.0f, gateTop, 0.8125f);
            tesselateBlockInWorld(state, pos);
        }
        else if (facing == Direction::NORTH)
        {
            setShape(0.0f, gateBottom, 0.0625f, 0.125f, gateTop, 0.1875f);
            tesselateBlockInWorld(state, pos);
            setShape(0.875f, gateBottom, 0.0625f, 1.0f, gateTop, 0.1875f);
            tesselateBlockInWorld(state, pos);
            setShape(0.0f, gateBottom, 0.1875f, 0.125f, middleTop, 0.4375f);
            tesselateBlockInWorld(state, pos);
            setShape(0.875f, gateBottom, 0.1875f, 1.0f, middleTop, 0.4375f);
            tesselateBlockInWorld(state, pos);
            setShape(0.0f, upperBottom, 0.1875f, 0.125f, gateTop, 0.4375f);
            tesselateBlockInWorld(state, pos);
            setShape(0.875f, upperBottom, 0.1875f, 1.0f, gateTop, 0.4375f);
            tesselateBlockInWorld(state, pos);
        }
    }

    mRenderAllFaces = false;
    mFaceRotation4 = 0;
    setShape(0.0f, 0.0f, 0.0f, 1.0f, 1.0f, 1.0f);
    return true;
}

bool BlockRenderer::tesselateCauldronInWorld(const BlockState* state, const BlockPos& pos)
{
    Block* block = state->getBlock();
    tesselateBlockInWorld(state, pos);

    BufferBuilder* builder = Tesselator::getInstance()->getBuilder();
    builder->tex2(getLightColor(block, mLevelSource, pos));
    int color = mBlockColors->getColor(state, mLevelSource, &pos, 0);
    builder->color((float)(color >> 16 & 255) / 255.0f, (float)(color >> 8 & 255) / 255.0f,
                   (float)(color & 255) / 255.0f);

    TextureAtlasSprite* side = block->getTexture(Direction::NORTH);
    float inset = 0.1171875f;
    renderEast(state, (double)((float)pos.x - 1.0f + inset), (double)pos.y, (double)pos.z, side);
    renderWest(state, (double)((float)pos.x + 1.0f - inset), (double)pos.y, (double)pos.z, side);
    renderSouth(state, (double)pos.x, (double)pos.y, (double)((float)pos.z - 1.0f + inset), side);
    renderNorth(state, (double)pos.x, (double)pos.y, (double)((float)pos.z + 1.0f - inset), side);

    TextureAtlasSprite* inner = CauldronBlock::getTexture(CauldronBlock::TEXTURE_INNER);
    renderFaceUp(state, (double)pos.x, (double)((float)pos.y + -0.75f), (double)pos.z, inner);
    renderFaceDown(state, (double)pos.x, (double)((float)pos.y + 0.25f), (double)pos.z, inner);

    int level = state->getValue(CauldronBlock::LEVEL);
    if (level > 0)
    {
        TextureAtlasSprite* water = CauldronBlock::getTexture(CauldronBlock::TEXTURE_WATER);
        if (level > 6)
            level = 6;

        unsigned int liquidColor = Minecraft::GetInstance()->getColourTable()->getColour(
            eMinecraftColour_Cauldron_Water);
        boost::shared_ptr<BlockEntity> blockEntity = mLevelSource->getBlockEntity(pos);
        if (blockEntity != NULL && eTYPE_DERIVED_FROM(BlockEntityType::CAULDRON, blockEntity->GetType()))
        {
            boost::shared_ptr<CauldronBlockEntity> cauldron = boost::static_pointer_cast<CauldronBlockEntity>(
                blockEntity);
            if (cauldron->getPotionId() == L"")
            {
                if (cauldron->mHasCustomColor)
                    liquidColor = cauldron->getCustomColor();
            }
            else
            {
                liquidColor = cauldron->getPotionColor();
            }
        }

        float brightness = 1.0f;
        builder->color(brightness * ((float)(liquidColor >> 16 & 255) / 255.0f),
                       brightness * ((float)(liquidColor >> 8 & 255) / 255.0f),
                       brightness * ((float)(liquidColor & 255) / 255.0f));
        float height = (float)level;
        renderFaceUp(state, (double)pos.x, (double)((height + height + 3.0f) * 0.0625f + ((float)pos.y - 1.0f)),
                     (double)pos.z, water);
    }
    return true;
}

bool BlockRenderer::tesselateFlowerPotInWorld(const BlockState* state, const BlockPos& pos)
{
    if (state->getBlock()->defaultBlockState()->getLightEmission() == 0)
        tesselateBlockInWorldWithAmbienceOcclusionTexLighting(state, pos, 1.0f, 1.0f, 1.0f, 0, mShapeOutside);
    else
        tesselateBlockInWorld(state, pos, 1.0f, 1.0f, 1.0f);

    BufferBuilder* builder = Tesselator::getInstance()->getBuilder();
    builder->tex2(state->getLightColor(mLevelSource, pos));
    int color = mBlockColors->getColor(state, mLevelSource, &pos, 0);
    TextureAtlasSprite* texture = getTexture(state, NULL);
    float red = (float)(color >> 16 & 255) / 255.0f;
    float green = (float)(color >> 8 & 255) / 255.0f;
    float blue = (float)(color & 255) / 255.0f;
    if (GameRenderer::anaglyph3d)
    {
        float anaglyphRed = (red * 30.0f + green * 59.0f + blue * 11.0f) / 100.0f;
        float anaglyphGreen = (red * 30.0f + green * 70.0f) / 100.0f;
        float anaglyphBlue = (red * 30.0f + blue * 70.0f) / 100.0f;
        red = anaglyphRed;
        green = anaglyphGreen;
        blue = anaglyphBlue;
    }
    builder->color(red, green, blue);

    double inset = FlowerPotBlock::POT_WIDTH * 0.5;
    renderEast(state, (double)pos.x - 0.5 + inset, (double)pos.y, (double)pos.z, texture);
    renderWest(state, (double)pos.x + 0.5 - inset, (double)pos.y + 0.003125, (double)pos.z, texture);
    renderSouth(state, (double)pos.x, (double)pos.y, (double)pos.z - 0.5 + inset, texture);
    renderNorth(state, (double)pos.x, (double)pos.y + 0.003125, (double)pos.z + 0.5 - inset, texture);
    renderFaceUp(state, (double)pos.x, (double)pos.y - 0.5 + inset + 0.1875, (double)pos.z,
                 getTexture(Blocks::dirt, Direction::UP));

    const FlowerPotBlock::Contents* contents = state->getValue(FlowerPotBlock::CONTENTS);
    if (contents == FlowerPotBlock::Contents::EMPTY)
        return true;

    float offsetX = 0.0f;
    float offsetY = 4.0f;
    float offsetZ = 0.0f;
    mUseInventoryTint = true;

    Block* plant = NULL;
    switch (contents->mContents)
    {
    case FlowerPotBlock::Contents::eContentsPoppy:
    case FlowerPotBlock::Contents::eContentsBlueOrchid:
    case FlowerPotBlock::Contents::eContentsAllium:
    case FlowerPotBlock::Contents::eContentsHoustonia:
    case FlowerPotBlock::Contents::eContentsRedTulip:
    case FlowerPotBlock::Contents::eContentsOrangeTulip:
    case FlowerPotBlock::Contents::eContentsWhiteTulip:
    case FlowerPotBlock::Contents::eContentsPinkTulip:
    case FlowerPotBlock::Contents::eContentsOxeyeDaisy:
        plant = Blocks::red_flower;
        break;
    case FlowerPotBlock::Contents::eContentsDandelion:
        plant = Blocks::yellow_flower;
        break;
    case FlowerPotBlock::Contents::eContentsMushroomRed:
        plant = Blocks::red_mushroom;
        break;
    case FlowerPotBlock::Contents::eContentsMushroomBrown:
        plant = Blocks::brown_mushroom;
        break;
    default:
        break;
    }
    builder->addOffset(offsetX / 16.0f, offsetY / 16.0f, offsetZ / 16.0f);

    if (plant != NULL)
    {
        boost::shared_ptr<BlockEntity> blockEntity = mLevelSource->getBlockEntity(pos);
        const BlockState* plantState;
        if (eTYPE_DERIVED_FROM(BlockEntityType::FLOWER_POT, blockEntity->GetType()))
        {
            boost::shared_ptr<FlowerPotBlockEntity> pot = boost::static_pointer_cast<FlowerPotBlockEntity>(blockEntity);
            plantState = plant->getBlockState(pot->getItemData());
        }
        else
        {
            plantState = plant->defaultBlockState();
        }
        tesselateInWorld(plantState, pos, NULL, boost::shared_ptr<BlockEntity>());
    }
    else if (contents == FlowerPotBlock::Contents::CACTUS)
    {
        mRenderAllFaces = true;
        setShape(0.375f, 0.0f, 0.375f, 0.625f, 0.25f, 0.625f);
        tesselateBlockInWorld(Blocks::cactus->defaultBlockState(), pos);
        setShape(0.375f, 0.25f, 0.375f, 0.625f, 0.5f, 0.625f);
        tesselateBlockInWorld(Blocks::cactus->defaultBlockState(), pos);
        setShape(0.375f, 0.5f, 0.375f, 0.625f, 0.75f, 0.625f);
        tesselateBlockInWorld(Blocks::cactus->defaultBlockState(), pos);
        mRenderAllFaces = false;
        setShape(0.0f, 0.0f, 0.0f, 1.0f, 1.0f, 1.0f);
    }
    else if (contents == FlowerPotBlock::Contents::OAK_SAPLING)
    {
        TypedBoxed<PlanksBlock::Variant*> variant(PlanksBlock::Variant::OAK);
        tesselateCrossTexture(Blocks::sapling->defaultBlockState()->setBoxedValue(SaplingBlock::TYPE, &variant),
                              (float)pos.x, (float)pos.y, (float)pos.z, 0.75f);
    }
    else if (contents == FlowerPotBlock::Contents::BIRCH_SAPLING)
    {
        TypedBoxed<PlanksBlock::Variant*> variant(PlanksBlock::Variant::BIRCH);
        tesselateCrossTexture(Blocks::sapling->defaultBlockState()->setBoxedValue(SaplingBlock::TYPE, &variant),
                              (float)pos.x, (float)pos.y, (float)pos.z, 0.75f);
    }
    else if (contents == FlowerPotBlock::Contents::SPRUCE_SAPLING)
    {
        TypedBoxed<PlanksBlock::Variant*> variant(PlanksBlock::Variant::SPRUCE);
        tesselateCrossTexture(Blocks::sapling->defaultBlockState()->setBoxedValue(SaplingBlock::TYPE, &variant),
                              (float)pos.x, (float)pos.y, (float)pos.z, 0.75f);
    }
    else if (contents == FlowerPotBlock::Contents::JUNGLE_SAPLING)
    {
        TypedBoxed<PlanksBlock::Variant*> variant(PlanksBlock::Variant::JUNGLE);
        tesselateCrossTexture(Blocks::sapling->defaultBlockState()->setBoxedValue(SaplingBlock::TYPE, &variant),
                              (float)pos.x, (float)pos.y, (float)pos.z, 0.75f);
    }
    else if (contents == FlowerPotBlock::Contents::ACACIA_SAPLING)
    {
        TypedBoxed<PlanksBlock::Variant*> variant(PlanksBlock::Variant::ACACIA);
        tesselateCrossTexture(Blocks::sapling->defaultBlockState()->setBoxedValue(SaplingBlock::TYPE, &variant),
                              (float)pos.x, (float)pos.y, (float)pos.z, 0.75f);
    }
    else if (contents == FlowerPotBlock::Contents::DARK_OAK_SAPLING)
    {
        TypedBoxed<PlanksBlock::Variant*> variant(PlanksBlock::Variant::DARK_OAK);
        tesselateCrossTexture(Blocks::sapling->defaultBlockState()->setBoxedValue(SaplingBlock::TYPE, &variant),
                              (float)pos.x, (float)pos.y, (float)pos.z, 0.75f);
    }
    else if (contents == FlowerPotBlock::Contents::FERN)
    {
        int fernColor = mBlockColors->getColor(Blocks::tallgrass->defaultBlockState(), mLevelSource, &pos, 0);
        builder->color((float)(fernColor >> 16 & 255) / 255.0f, (float)(fernColor >> 8 & 255) / 255.0f,
                       (float)(fernColor & 255) / 255.0f);
        TypedBoxed<TallGrassBlock::Variant*> variant(TallGrassBlock::Variant::FERN);
        tesselateCrossTexture(Blocks::tallgrass->defaultBlockState()->setBoxedValue(TallGrassBlock::VARIANT, &variant),
                              (float)pos.x, (float)pos.y, (float)pos.z, 0.75f);
    }
    else if (contents == FlowerPotBlock::Contents::DEAD_BUSH)
    {
        tesselateCrossTexture(Blocks::deadbush->defaultBlockState(), (float)pos.x, (float)pos.y, (float)pos.z, 0.75f);
    }

    builder->addOffset(-offsetX / 16.0f, -offsetY / 16.0f, -offsetZ / 16.0f);
    mUseInventoryTint = false;
    return true;
}

bool BlockRenderer::tesselateAnvilInWorld(const BlockState* state, const BlockPos& pos)
{
    BufferBuilder* builder = Tesselator::getInstance()->getBuilder();
    builder->tex2(state->getLightColor(mLevelSource, pos));
    int color = mBlockColors->getColor(state, mLevelSource, &pos, 0);
    float red = (float)(color >> 16 & 255) / 255.0f;
    float green = (float)(color >> 8 & 255) / 255.0f;
    float blue = (float)(color & 255) / 255.0f;
    if (GameRenderer::anaglyph3d)
    {
        float anaglyphRed = (red * 30.0f + green * 59.0f + blue * 11.0f) / 100.0f;
        float anaglyphGreen = (red * 30.0f + green * 70.0f) / 100.0f;
        float anaglyphBlue = (red * 30.0f + blue * 70.0f) / 100.0f;
        red = anaglyphRed;
        green = anaglyphGreen;
        blue = anaglyphBlue;
    }
    builder->color(red, green, blue);
    return tesselateAnvilInWorld(state, pos, false);
}

bool BlockRenderer::tesselateBrewingStandInWorld(const BlockState* state, const BlockPos& pos)
{
    BrewingStandBlock* block = static_cast<BrewingStandBlock*>(state->getBlock());
    const float bottom = 0.0f;
    const float postEdge = 0.4375f;
    const float postOtherEdge = 0.5625f;

    setShape(postEdge, bottom, postEdge, postOtherEdge, 0.875f, postOtherEdge);
    tesselateBlockInWorld(state, pos);

    setFixedTexture(block->getBaseTexture());
    mRenderAllFaces = true;
    setShape(postOtherEdge, bottom, 0.3125f, 0.9375f, 0.125f, 0.6875f);
    tesselateBlockInWorld(state, pos);
    setShape(0.125f, bottom, 0.0625f, 0.5f, 0.125f, 0.4375f);
    tesselateBlockInWorld(state, pos);
    setShape(0.125f, bottom, 0.5625f, 0.5f, 0.125f, 0.9375f);
    tesselateBlockInWorld(state, pos);
    mRenderAllFaces = false;
    clearFixedTexture();

    BufferBuilder* builder = Tesselator::getInstance()->getBuilder();
    builder->tex2(getLightColor(block, mLevelSource, pos));
    int color = mBlockColors->getColor(state, mLevelSource, &pos, 0);
    builder->color((float)(color >> 16 & 255) / 255.0f, (float)(color >> 8 & 255) / 255.0f,
                   (float)(color & 255) / 255.0f);

    TextureAtlasSprite* texture = getTexture(state, Direction::UP);
    if (hasFixedTexture())
        texture = mFixedTexture;

    const double vTop = texture->getV0(true);
    const double vBottom = texture->getV1(true);
    const float pi = 3.1415927f;
    AbstractProperty<bool>* const bottleProperties[3] = {
        BrewingStandBlock::HAS_BOTTLE_0, BrewingStandBlock::HAS_BOTTLE_1, BrewingStandBlock::HAS_BOTTLE_2};

    for (int i = 0; i < 3; ++i)
    {
        float angle = (float)i * pi;
        angle = (angle + angle) / 3.0f + pi * 0.5f;
        float u0 = (float)texture->getU(8.0);
        float u1 = (float)texture->getU1(false);
        if (state->getValue(bottleProperties[i]))
            u1 = (float)texture->getU0(false);
        texture->adjustU(u0, u1);

        float x0 = (float)pos.x + 0.5f;
        float z0 = (float)pos.z + 0.5f;
        float x1 = std::sin(angle) * 8.0f * 0.0625f + ((float)pos.x + 0.5f);
        float z1 = std::cos(angle) * 8.0f * 0.0625f + ((float)pos.z + 0.5f);
        float y0 = (float)pos.y;
        float y1 = (float)pos.y + 1.0f;

        builder->vertexUV(x0, y1, z0, u0, (float)vTop);
        builder->vertexUV(x0, y0, z0, u0, (float)vBottom);
        builder->vertexUV(x1, y0, z1, u1, (float)vBottom);
        builder->vertexUV(x1, y1, z1, u1, (float)vTop);
        builder->vertexUV(x1, y1, z1, u1, (float)vTop);
        builder->vertexUV(x1, y0, z1, u1, (float)vBottom);
        builder->vertexUV(x0, y0, z0, u0, (float)vBottom);
        builder->vertexUV(x0, y1, z0, u0, (float)vTop);
    }
    return true;
}

bool BlockRenderer::tesselateAirPortalFrameInWorld(const BlockState* state, const BlockPos& pos)
{
    const Direction* facing = state->getValue(EndPortalFrameBlock::FACING);
    if (facing == Direction::SOUTH)
        mFaceRotation4 = 3;
    else if (facing == Direction::EAST)
        mFaceRotation4 = 1;
    else if (facing == Direction::WEST)
        mFaceRotation4 = 2;

    if (!state->getValue(EndPortalFrameBlock::EYE))
    {
        setShape(0.0f, 0.0f, 0.0f, 1.0f, 0.8125f, 1.0f);
        tesselateBlockInWorld(state, pos);
        mFaceRotation4 = 0;
    }
    else
    {
        EndPortalFrameBlock* frame = static_cast<EndPortalFrameBlock*>(state->getBlock());
        mRenderAllFaces = true;
        setShape(0.0f, 0.0f, 0.0f, 1.0f, 0.8125f, 1.0f);
        tesselateBlockInWorld(state, pos);
        setFixedTexture(frame->getEye());
        mRenderFromInside = true;
        setShape(0.25f, 0.8125f, 0.25f, 0.75f, 1.0f, 0.75f);
        tesselateBlockInWorld(state, pos);
        mRenderAllFaces = false;
        mRenderFromInside = false;
        clearFixedTexture();
        mFaceRotation4 = 0;
    }
    return true;
}

bool BlockRenderer::tesselateCocoaInWorld(const BlockState* state, const BlockPos& pos)
{
    BufferBuilder* builder = Tesselator::getInstance()->getBuilder();
    CocoaBlock* block = static_cast<CocoaBlock*>(state->getBlock());
    builder->tex2(getLightColor(block, mLevelSource, pos));
    builder->color(1.0f, 1.0f, 1.0f);

    const Direction* facing = state->getValue(HorizontalDirectionalBlock::FACING_PROPERTY);
    int age = state->getValue(CocoaBlock::AGE);
    TextureAtlasSprite* texture = block->getTextureForAge(age);

    int width = age * 2 + 4;
    int height = age * 2 + 5;
    float podLeft = 15.0f - (float)width;
    float u0 = (float)texture->getU(podLeft);
    float u1 = (float)texture->getU(15.0);
    float v0 = (float)texture->getV(4.0);
    float v1 = (float)texture->getV((float)height + 4.0f);
    texture->adjustUVs(u0, u1, v0, v1);

    float offsetX = 0.0f;
    float offsetZ = 0.0f;
    switch (facing->get3DDataValue())
    {
    case 2:
        offsetX = 8.0f - (float)(width / 2);
        offsetZ = 1.0f;
        break;
    case 3:
        offsetX = 8.0f - (float)(width / 2);
        offsetZ = podLeft;
        break;
    case 4:
        offsetX = 1.0f;
        offsetZ = 8.0f - (float)(width / 2);
        break;
    case 5:
        offsetX = podLeft;
        offsetZ = 8.0f - (float)(width / 2);
        break;
    }

    float x = (float)pos.x;
    float y = (float)pos.y;
    float z = (float)pos.z;
    float x0 = (float)(offsetX * 0.0625 + x);
    float x1 = (float)((offsetX + (float)width) * 0.0625 + x);
    float y0 = (float)((12.0 - height) * 0.0625 + y);
    float y1 = (float)(y + 0.75);
    float z0 = (float)(offsetZ * 0.0625 + z);
    float z1 = (float)((offsetZ + (float)width) * 0.0625 + z);

    builder->vertexUV(x0, y0, z0, u0, v1);
    builder->vertexUV(x0, y0, z1, u1, v1);
    builder->vertexUV(x0, y1, z1, u1, v0);
    builder->vertexUV(x0, y1, z0, u0, v0);

    builder->vertexUV(x1, y0, z1, u0, v1);
    builder->vertexUV(x1, y0, z0, u1, v1);
    builder->vertexUV(x1, y1, z0, u1, v0);
    builder->vertexUV(x1, y1, z1, u0, v0);

    builder->vertexUV(x1, y0, z0, u0, v1);
    builder->vertexUV(x0, y0, z0, u1, v1);
    builder->vertexUV(x0, y1, z0, u1, v0);
    builder->vertexUV(x1, y1, z0, u0, v0);

    builder->vertexUV(x0, y0, z1, u0, v1);
    builder->vertexUV(x1, y0, z1, u1, v1);
    builder->vertexUV(x1, y1, z1, u1, v0);
    builder->vertexUV(x0, y1, z1, u0, v0);

    int topSize = width;
    if (age >= 2)
        topSize = width - 1;
    u0 = (float)texture->getU0(false);
    u1 = (float)texture->getU(topSize);
    v0 = (float)texture->getV0(false);
    v1 = (float)texture->getV(topSize);
    texture->adjustUVs(u0, u1, v0, v1);

    builder->vertexUV(x0, y1, z1, u0, v1);
    builder->vertexUV(x1, y1, z1, u1, v1);
    builder->vertexUV(x1, y1, z0, u1, v0);
    builder->vertexUV(x0, y1, z0, u0, v0);

    builder->vertexUV(x0, y0, z0, u0, v0);
    builder->vertexUV(x1, y0, z0, u1, v0);
    builder->vertexUV(x1, y0, z1, u1, v1);
    builder->vertexUV(x0, y0, z1, u0, v1);

    u0 = (float)texture->getU(12.0);
    u1 = (float)texture->getU1(false);
    v0 = (float)texture->getV0(false);
    v1 = (float)texture->getV(4.0);
    texture->adjustUVs(u0, u1, v0, v1);

    float stemX = 8.0f;
    float stemZ = 0.0f;
    switch (facing->get3DDataValue())
    {
    case 3:
    {
        float swap = u0;
        u0 = u1;
        u1 = swap;
        stemZ = 12.0f;
        break;
    }
    case 4:
        stemX = 0.0f;
        stemZ = 8.0f;
        break;
    case 5:
    {
        float swap = u0;
        u0 = u1;
        u1 = swap;
        stemX = 12.0f;
        stemZ = 8.0f;
        break;
    }
    }

    float stemZ0 = (float)(stemZ * 0.0625 + z);
    float stemZ1 = (float)((stemZ + 4.0) * 0.0625 + z);
    float stemTop = (float)(y + 1.0);
    float stemX0 = (float)(stemX * 0.0625 + x);
    if (facing == Direction::NORTH || facing == Direction::SOUTH)
    {
        builder->vertexUV(stemX0, y1, stemZ0, u1, v1);
        builder->vertexUV(stemX0, y1, stemZ1, u0, v1);
        builder->vertexUV(stemX0, stemTop, stemZ1, u0, v0);
        builder->vertexUV(stemX0, stemTop, stemZ0, u1, v0);
        builder->vertexUV(stemX0, y1, stemZ1, u0, v1);
        builder->vertexUV(stemX0, y1, stemZ0, u1, v1);
        builder->vertexUV(stemX0, stemTop, stemZ0, u1, v0);
        builder->vertexUV(stemX0, stemTop, stemZ1, u0, v0);
    }
    else if (facing == Direction::WEST || facing == Direction::EAST)
    {
        float stemX1 = (float)((stemX + 4.0) * 0.0625 + x);
        builder->vertexUV(stemX1, y1, stemZ0, u0, v1);
        builder->vertexUV(stemX0, y1, stemZ0, u1, v1);
        builder->vertexUV(stemX0, stemTop, stemZ0, u1, v0);
        builder->vertexUV(stemX1, stemTop, stemZ0, u0, v0);
        builder->vertexUV(stemX0, y1, stemZ0, u1, v1);
        builder->vertexUV(stemX1, y1, stemZ0, u0, v1);
        builder->vertexUV(stemX1, stemTop, stemZ0, u0, v0);
        builder->vertexUV(stemX0, stemTop, stemZ0, u1, v0);
    }
    return true;
}

bool BlockRenderer::tesselateBeaconInWorld(const BlockState* state, const BlockPos& pos)
{
    if (!hasFixedTexture())
    {
        setFixedTexture(getTexture(Blocks::glass, Direction::UP));
        setShape(0.0f, 0.0f, 0.0f, 1.0f, 1.0f, 1.0f);
        tesselateBlockInWorld(state, pos);
        mRenderAllFaces = true;
        setFixedTexture(getTexture(Blocks::obsidian, Direction::UP));
        setShape(0.125f, 0.00625f, 0.125f, 0.875f, 0.1875f, 0.875f);
        tesselateBlockInWorld(state, pos);
        setFixedTexture(getTexture(Blocks::beacon, Direction::UP));
        setShape(0.1875f, 0.1875f, 0.1875f, 0.8125f, 0.875f, 0.8125f);
        tesselateBlockInWorld(state, pos);
        mRenderAllFaces = false;
        clearFixedTexture();
    }
    else
    {
        setShape(0.0f, 0.0f, 0.0f, 1.0f, 1.0f, 1.0f);
        tesselateBlockInWorld(state, pos);
    }
    return true;
}

bool BlockRenderer::tesselateHopperInWorld(const BlockState* state, const BlockPos& pos)
{
    BufferBuilder* builder = Tesselator::getInstance()->getBuilder();
    Block* block = state->getBlock();
    builder->tex2(block->getLightColor(state, mLevelSource, pos, -1));
    int color = mBlockColors->getColor(state, mLevelSource, &pos, 0);
    float red = (float)(color >> 16 & 255) / 255.0f;
    float green = (float)(color >> 8 & 255) / 255.0f;
    float blue = (float)(color & 255) / 255.0f;
    if (GameRenderer::anaglyph3d)
    {
        float anaglyphRed = (red * 30.0f + green * 59.0f + blue * 11.0f) / 100.0f;
        float anaglyphGreen = (red * 30.0f + green * 70.0f) / 100.0f;
        float anaglyphBlue = (red * 30.0f + blue * 70.0f) / 100.0f;
        red = anaglyphRed;
        green = anaglyphGreen;
        blue = anaglyphBlue;
    }
    builder->color(red, green, blue);
    return tesselateHopperInWorld(state, pos, false);
}

bool BlockRenderer::tesselateChorusFlowerInWorld(const BlockState* state, const BlockPos& pos)
{
    mRenderFromInside = true;
    ChorusFlowerBlock* flower = static_cast<ChorusFlowerBlock*>(state->getBlock());

    setShape(ChorusFlowerBlock::getDownShape());
    flower->setCulledFace(0);
    bool rendered = tesselateBlockInWorld(state, pos);

    setShape(ChorusFlowerBlock::getUpShape());
    flower->setCulledFace(Direction::DOWN);
    rendered |= tesselateBlockInWorld(state, pos);

    setShape(ChorusFlowerBlock::getNorthShape());
    flower->setCulledFace(Direction::SOUTH);
    rendered |= tesselateBlockInWorld(state, pos);

    setShape(ChorusFlowerBlock::getSouthShape());
    flower->setCulledFace(Direction::NORTH);
    rendered |= tesselateBlockInWorld(state, pos);

    setShape(ChorusFlowerBlock::getEastShape());
    flower->setCulledFace(Direction::WEST);
    rendered |= tesselateBlockInWorld(state, pos);

    setShape(ChorusFlowerBlock::getWestShape());
    flower->setCulledFace(Direction::EAST);
    rendered |= tesselateBlockInWorld(state, pos);

    flower->setCulledFace(0);
    mRenderFromInside = false;

    return rendered;
}

bool BlockRenderer::tesselateChorusPlantInWorld(const BlockState* state, const BlockPos& pos)
{
    int positionMix = (pos.x * pos.y + pos.y) * pos.z + pos.z;
    long long levelSeed = mMinecraft->mLevel->getSeed();
    long long mixedSeed = levelSeed + positionMix;
    Random random(mixedSeed);
    int variantGate = random.nextInt(5);

    mRenderFromInside = true;
    ChorusPlantBlock* plant = static_cast<ChorusPlantBlock*>(state->getBlock());
    plant->fillVirtualBlockStateProperties(state, mLevelSource, pos);

    setShape(plant->getVerticalShape(state, mLevelSource, pos));
    bool rendered = true;
    rendered |= tesselateBlockInWorld(state, pos);

    if (state->getValue(ChorusPlantBlock::NORTH_PROPERTY))
    {
        setShape(plant->getHorizontalShape(state, Direction::NORTH));
        rendered |= tesselateBlockInWorld(state, pos);
    }

    if (state->getValue(ChorusPlantBlock::SOUTH_PROPERTY))
    {
        setShape(plant->getHorizontalShape(state, Direction::SOUTH));
        rendered |= tesselateBlockInWorld(state, pos);
    }

    if (state->getValue(ChorusPlantBlock::EAST_PROPERTY))
    {
        setShape(plant->getHorizontalShape(state, Direction::EAST));
        rendered |= tesselateBlockInWorld(state, pos);
    }

    if (state->getValue(ChorusPlantBlock::WEST_PROPERTY))
    {
        setShape(plant->getHorizontalShape(state, Direction::WEST));
        rendered |= tesselateBlockInWorld(state, pos);
    }

    int northNodule = random.nextInt(3);
    int southNodule = random.nextInt(3);
    int eastNodule = random.nextInt(3);
    int westNodule = random.nextInt(3);

    if (variantGate > 1)
    {
        if (!state->getValue(ChorusPlantBlock::NORTH_PROPERTY))
        {
            setShape(plant->getRandomNoduleShape(northNodule == 1, state, Direction::NORTH));
            rendered |= tesselateBlockInWorld(state, pos);
        }

        if (!state->getValue(ChorusPlantBlock::SOUTH_PROPERTY))
        {
            setShape(plant->getRandomNoduleShape(southNodule == 1, state, Direction::SOUTH));
            rendered |= tesselateBlockInWorld(state, pos);
        }

        if (!state->getValue(ChorusPlantBlock::EAST_PROPERTY))
        {
            setShape(plant->getRandomNoduleShape(eastNodule == 1, state, Direction::EAST));
            rendered |= tesselateBlockInWorld(state, pos);
        }

        if (!state->getValue(ChorusPlantBlock::WEST_PROPERTY))
        {
            setShape(plant->getRandomNoduleShape(westNodule == 1, state, Direction::WEST));
            rendered |= tesselateBlockInWorld(state, pos);
        }
    }

    mRenderFromInside = false;
    return rendered;
}

bool BlockRenderer::tesselateObserverInWorld(const BlockState* state, const BlockPos& pos)
{
    switch (state->getValue<const Direction*>(DirectionalBlock::FACING_PROPERTY)->get3DDataValue())
    {
    case Direction::eDirection_Down:
        mFaceRotation4 = 3;
        mFaceRotation5 = 3;
        tesselateBlockInWorld(state, pos);
        break;
    case Direction::eDirection_Up:
        mFaceTransform1 = 1;
        mFaceRotation4 = 3;
        mFaceTransform0 = 1;
        tesselateBlockInWorld(state, pos);
        break;
    case Direction::eDirection_North:
        mFaceRotation5 = 3;
        mFaceRotation4 = 3;
        tesselateBlockInWorld(state, pos);
        break;
    case Direction::eDirection_South:
        mFaceRotation4 = 0;
        mFaceRotation5 = 0;
        tesselateBlockInWorld(state, pos);
        break;
    case Direction::eDirection_West:
        mFaceRotation4 = 1;
        mFaceRotation5 = 2;
        tesselateBlockInWorld(state, pos);
        break;
    case Direction::eDirection_East:
        mFaceRotation4 = 2;
        mFaceRotation5 = 1;
        tesselateBlockInWorld(state, pos);
        break;
    default:
        tesselateBlockInWorld(state, pos);
        break;
    }
    mFaceTransform2 = 0;
    mFaceTransform4 = 0;
    mFaceRotation2 = 0;
    mFaceTransform1 = 0;
    mFaceRotation3 = 0;
    mFaceTransform0 = 0;
    mFaceRotation4 = 0;
    mFaceRotation5 = 0;
    mFaceRotation0 = 0;
    mFaceTransform3 = 0;
    mFaceTransform5 = 0;
    mFaceRotation1 = 0;
    return true;
}

void BlockRenderer::tesselateTurtleEgg(const BlockState* state, const BlockPos& pos, float red, float green, float blue)
{
    BufferBuilder* builder = Tesselator::getInstance()->getBuilder();
    TextureAtlasSprite* texture = getTexture(state, Direction::DOWN);
    float zRed = 0.8f * red;
    float zGreen = 0.8f * green;
    float zBlue = 0.8f * blue;
    float xRed = 0.6f * red;
    float xGreen = 0.6f * green;
    float xBlue = 0.6f * blue;
    float bottomRed = 0.5f * red;
    float bottomGreen = 0.5f * green;
    float bottomBlue = 0.5f * blue;

    int eggCount = state->getValue(TurtleEggBlock::EGG_COUNT);
    if (eggCount >= 0)
    {
        float x0 = (float)pos.x + 0.3125f;
        float x1 = (float)pos.x + 0.625f;
        float y0 = (float)pos.y + 0.0001f;
        float y1 = (float)pos.y + 0.4375f;
        float z0 = (float)pos.z + 0.25f;
        float z1 = (float)pos.z + 0.5625f;
        float topU0 = (float)texture->getU0(true);
        float topU1 = (float)texture->getU(4.0);
        float topV0 = (float)texture->getV0(true);
        float topV1 = (float)texture->getV(4.0);
        texture->adjustUVs(topU0, topU1, topV0, topV1);
        float sideU0 = (float)texture->getU(1.0);
        float sideU1 = (float)texture->getU(5.0);
        float sideV0 = (float)texture->getV(4.0);
        float sideV1 = (float)texture->getV(11.0);
        texture->adjustUVs(sideU0, sideU1, sideV0, sideV1);

        builder->color(bottomRed, bottomGreen, bottomBlue);
        builder->vertexUV(x0, y0, z1, topU1, topV0);
        builder->vertexUV(x0, y0, z0, topU1, topV1);
        builder->vertexUV(x1, y0, z0, topU0, topV1);
        builder->vertexUV(x1, y0, z1, topU0, topV0);
        builder->color(red, green, blue);
        builder->vertexUV(x1, y1, z1, topU1, topV0);
        builder->vertexUV(x1, y1, z0, topU1, topV1);
        builder->vertexUV(x0, y1, z0, topU0, topV1);
        builder->vertexUV(x0, y1, z1, topU0, topV0);
        builder->color(xRed, xGreen, xBlue);
        builder->vertexUV(x1, y0, z0, sideU1, sideV1);
        builder->vertexUV(x1, y1, z0, sideU1, sideV0);
        builder->vertexUV(x1, y1, z1, sideU0, sideV0);
        builder->vertexUV(x1, y0, z1, sideU0, sideV1);
        builder->color(xRed, xGreen, xBlue);
        builder->vertexUV(x0, y0, z1, sideU1, sideV0);
        builder->vertexUV(x0, y1, z1, sideU1, sideV1);
        builder->vertexUV(x0, y1, z0, sideU0, sideV1);
        builder->vertexUV(x0, y0, z0, sideU0, sideV0);
        builder->color(zRed, zGreen, zBlue);
        builder->vertexUV(x0, y0, z0, sideU1, sideV1);
        builder->vertexUV(x0, y1, z0, sideU1, sideV0);
        builder->vertexUV(x1, y1, z0, sideU0, sideV0);
        builder->vertexUV(x1, y0, z0, sideU0, sideV1);
        builder->color(zRed, zGreen, zBlue);
        builder->vertexUV(x1, y0, z1, sideU1, sideV0);
        builder->vertexUV(x1, y1, z1, sideU1, sideV1);
        builder->vertexUV(x0, y1, z1, sideU0, sideV1);
        builder->vertexUV(x0, y0, z1, sideU0, sideV0);
    }

    if (eggCount >= 1)
    {
        float x0 = (float)pos.x + 0.0625f;
        float x1 = (float)pos.x + 0.3125f;
        float y0 = (float)pos.y + 0.0001f;
        float y1 = (float)pos.y + 0.3125f;
        float z0 = (float)pos.z + 0.4375f;
        float z1 = (float)pos.z + 0.6875f;
        float topU0 = (float)texture->getU(6.0);
        float topU1 = (float)texture->getU(10.0);
        float topV0 = (float)texture->getV(7.0);
        float topV1 = (float)texture->getV(11.0);
        texture->adjustUVs(topU0, topU1, topV0, topV1);
        float sideU0 = (float)texture->getU(10.0);
        float sideU1 = (float)texture->getU(14.0);
        float sideV0 = (float)texture->getV(10.0);
        float sideV1 = (float)texture->getV(15.0);
        texture->adjustUVs(sideU0, sideU1, sideV0, sideV1);

        builder->color(red, green, blue);
        builder->vertexUV(x1, y1, z1, topU1, topV0);
        builder->vertexUV(x1, y1, z0, topU1, topV1);
        builder->vertexUV(x0, y1, z0, topU0, topV1);
        builder->vertexUV(x0, y1, z1, topU0, topV0);
        builder->color(bottomRed, bottomGreen, bottomBlue);
        builder->vertexUV(x0, y0, z1, topU1, topV0);
        builder->vertexUV(x0, y0, z0, topU1, topV1);
        builder->vertexUV(x1, y0, z0, topU0, topV1);
        builder->vertexUV(x1, y0, z1, topU0, topV0);
        builder->color(xRed, xGreen, xBlue);
        builder->vertexUV(x0, y1, z0, sideU0, sideV0);
        builder->vertexUV(x0, y0, z0, sideU0, sideV1);
        builder->vertexUV(x0, y0, z1, sideU1, sideV1);
        builder->vertexUV(x0, y1, z1, sideU1, sideV0);
        builder->color(xRed, xGreen, xBlue);
        builder->vertexUV(x1, y1, z1, sideU1, sideV0);
        builder->vertexUV(x1, y0, z1, sideU1, sideV1);
        builder->vertexUV(x1, y0, z0, sideU0, sideV1);
        builder->vertexUV(x1, y1, z0, sideU0, sideV0);
        builder->color(zRed, zGreen, zBlue);
        builder->vertexUV(x0, y1, z1, sideU0, sideV0);
        builder->vertexUV(x0, y0, z1, sideU0, sideV1);
        builder->vertexUV(x1, y0, z1, sideU1, sideV1);
        builder->vertexUV(x1, y1, z1, sideU1, sideV0);
        builder->color(zRed, zGreen, zBlue);
        builder->vertexUV(x1, y1, z0, sideU1, sideV0);
        builder->vertexUV(x1, y0, z0, sideU1, sideV1);
        builder->vertexUV(x0, y0, z0, sideU0, sideV1);
        builder->vertexUV(x0, y1, z0, sideU0, sideV0);
    }

    if (eggCount >= 2)
    {
        float x0 = (float)pos.x + 0.6875f;
        float x1 = (float)pos.x + 0.875f;
        float y0 = (float)pos.y + 0.0001f;
        float y1 = (float)pos.y + 0.25f;
        float z0 = (float)pos.z + 0.4375f;
        float z1 = (float)pos.z + 0.625f;
        float topU0 = (float)texture->getU(5.0);
        float topU1 = (float)texture->getU(8.0);
        float topV0 = (float)texture->getV0(true);
        float topV1 = (float)texture->getV(3.0);
        texture->adjustUVs(topU0, topU1, topV0, topV1);
        float sideU0 = (float)texture->getU(8.0);
        float sideU1 = (float)texture->getU(11.0);
        float sideV0 = (float)texture->getV(3.0);
        float sideV1 = (float)texture->getV(7.0);
        texture->adjustUVs(sideU0, sideU1, sideV0, sideV1);

        builder->color(red, green, blue);
        builder->vertexUV(x1, y1, z1, topU1, topV0);
        builder->vertexUV(x1, y1, z0, topU1, topV1);
        builder->vertexUV(x0, y1, z0, topU0, topV1);
        builder->vertexUV(x0, y1, z1, topU0, topV0);
        builder->color(bottomRed, bottomGreen, bottomBlue);
        builder->vertexUV(x0, y0, z1, topU1, topV0);
        builder->vertexUV(x0, y0, z0, topU1, topV1);
        builder->vertexUV(x1, y0, z0, topU0, topV1);
        builder->vertexUV(x1, y0, z1, topU0, topV0);
        builder->color(xRed, xGreen, xBlue);
        builder->vertexUV(x0, y1, z0, sideU0, sideV0);
        builder->vertexUV(x0, y0, z0, sideU0, sideV1);
        builder->vertexUV(x0, y0, z1, sideU1, sideV1);
        builder->vertexUV(x0, y1, z1, sideU1, sideV0);
        builder->color(xRed, xGreen, xBlue);
        builder->vertexUV(x1, y1, z1, sideU1, sideV0);
        builder->vertexUV(x1, y0, z1, sideU1, sideV1);
        builder->vertexUV(x1, y0, z0, sideU0, sideV1);
        builder->vertexUV(x1, y1, z0, sideU0, sideV0);
        builder->color(zRed, zGreen, zBlue);
        builder->vertexUV(x0, y1, z1, sideU0, sideV0);
        builder->vertexUV(x0, y0, z1, sideU0, sideV1);
        builder->vertexUV(x1, y0, z1, sideU1, sideV1);
        builder->vertexUV(x1, y1, z1, sideU1, sideV0);
        builder->color(zRed, zGreen, zBlue);
        builder->vertexUV(x1, y1, z0, sideU1, sideV0);
        builder->vertexUV(x1, y0, z0, sideU1, sideV1);
        builder->vertexUV(x0, y0, z0, sideU0, sideV1);
        builder->vertexUV(x0, y1, z0, sideU0, sideV0);
    }

    if (eggCount >= 3)
    {
        float x0 = (float)pos.x + 0.4375f;
        float x1 = (float)pos.x + 0.625f;
        float y0 = (float)pos.y + 0.0001f;
        float y1 = (float)pos.y + 0.1875f;
        float z0 = (float)pos.z + 0.625f;
        float z1 = (float)pos.z + 0.8125f;
        float topU0 = (float)texture->getU0(true);
        float topU1 = (float)texture->getU(4.0);
        float topV0 = (float)texture->getV(11.0);
        float topV1 = (float)texture->getV(15.0);
        texture->adjustUVs(topU0, topU1, topV0, topV1);
        float sideU0 = (float)texture->getU(4.0);
        float sideU1 = (float)texture->getU(8.0);
        float sideV0 = (float)texture->getV(11.0);
        float sideV1 = (float)texture->getV(15.0);
        texture->adjustUVs(sideU0, sideU1, sideV0, sideV1);

        builder->color(red, green, blue);
        builder->vertexUV(x1, y1, z1, topU1, topV0);
        builder->vertexUV(x1, y1, z0, topU1, topV1);
        builder->vertexUV(x0, y1, z0, topU0, topV1);
        builder->vertexUV(x0, y1, z1, topU0, topV0);
        builder->color(bottomRed, bottomGreen, bottomBlue);
        builder->vertexUV(x0, y0, z1, topU1, topV0);
        builder->vertexUV(x0, y0, z0, topU1, topV1);
        builder->vertexUV(x1, y0, z0, topU0, topV1);
        builder->vertexUV(x1, y0, z1, topU0, topV0);
        builder->color(xRed, xGreen, xBlue);
        builder->vertexUV(x0, y1, z0, sideU0, sideV0);
        builder->vertexUV(x0, y0, z0, sideU0, sideV1);
        builder->vertexUV(x0, y0, z1, sideU1, sideV1);
        builder->vertexUV(x0, y1, z1, sideU1, sideV0);
        builder->color(xRed, xGreen, xBlue);
        builder->vertexUV(x1, y1, z1, sideU1, sideV0);
        builder->vertexUV(x1, y0, z1, sideU1, sideV1);
        builder->vertexUV(x1, y0, z0, sideU0, sideV1);
        builder->vertexUV(x1, y1, z0, sideU0, sideV0);
        builder->color(zRed, zGreen, zBlue);
        builder->vertexUV(x0, y1, z1, sideU0, sideV0);
        builder->vertexUV(x0, y0, z1, sideU0, sideV1);
        builder->vertexUV(x1, y0, z1, sideU1, sideV1);
        builder->vertexUV(x1, y1, z1, sideU1, sideV0);
        builder->color(zRed, zGreen, zBlue);
        builder->vertexUV(x1, y1, z0, sideU1, sideV0);
        builder->vertexUV(x1, y0, z0, sideU1, sideV1);
        builder->vertexUV(x0, y0, z0, sideU0, sideV1);
        builder->vertexUV(x0, y1, z0, sideU0, sideV0);
    }
}

bool BlockRenderer::tessellateSeagrassInWorld(const BlockState* state, const BlockPos& pos)
{
    BufferBuilder* builder = Tesselator::getInstance()->getBuilder();
    Block* block = state->getBlock();
    builder->tex2(getLightColor(block, mLevelSource, pos));
    builder->color(0.5f, 0.5f, 0.5f);
    TextureAtlasSprite* texture = getTexture(state, Direction::DOWN);

    float x = (float)pos.x;
    float y = (float)pos.y - 0.0625f;
    float z = (float)pos.z;

    BlockPos seedPos = pos;
    if (Block::getId(mLevelSource->getBlock(pos.below())) == BlockId::SEA_GRASS)
        seedPos = pos.below();
    Random random(seedPos.getRandomSeed());
    bool swap = random.nextBoolean();

    float u0 = (float)texture->getU0(true);
    float u1 = (float)texture->getU1(true);
    float v0 = (float)texture->getV0(true);
    float v1 = (float)texture->getV1(true);

    float uA = swap ? u1 : u0;
    float uB = swap ? u0 : u1;

    float x1 = x + 0.75f;
    float x0 = x + 0.25f;
    float yTop = y + 1.0f;
    float zFull1 = z + 1.0f;

    builder->vertexUV(x0, yTop, z, uA, v0);
    builder->vertexUV(x0, y, z, uA, v1);
    builder->vertexUV(x0, y, zFull1, uB, v1);
    builder->vertexUV(x0, yTop, zFull1, uB, v0);
    builder->vertexUV(x1, yTop, zFull1, uA, v0);
    builder->vertexUV(x1, y, zFull1, uA, v1);
    builder->vertexUV(x1, y, z, uB, v1);
    builder->vertexUV(x1, yTop, z, uB, v0);

    builder->vertexUV(x0, yTop, zFull1, uB, v0);
    builder->vertexUV(x0, y, zFull1, uB, v1);
    builder->vertexUV(x0, y, z, uA, v1);
    builder->vertexUV(x0, yTop, z, uA, v0);
    builder->vertexUV(x1, yTop, z, uB, v0);
    builder->vertexUV(x1, y, z, uB, v1);
    builder->vertexUV(x1, y, zFull1, uA, v1);
    builder->vertexUV(x1, yTop, zFull1, uA, v0);

    float z0 = z + 0.25f;
    float xFull1 = x + 1.0f;
    float z1 = z + 0.75f;

    builder->vertexUV(x, yTop, z0, uA, v0);
    builder->vertexUV(x, y, z0, uA, v1);
    builder->vertexUV(xFull1, y, z0, uB, v1);
    builder->vertexUV(xFull1, yTop, z0, uB, v0);
    builder->vertexUV(xFull1, yTop, z1, uA, v0);
    builder->vertexUV(xFull1, y, z1, uA, v1);
    builder->vertexUV(x, y, z1, uB, v1);
    builder->vertexUV(x, yTop, z1, uB, v0);

    builder->vertexUV(xFull1, yTop, z0, uB, v0);
    builder->vertexUV(xFull1, y, z0, uB, v1);
    builder->vertexUV(x, y, z0, uA, v1);
    builder->vertexUV(x, yTop, z0, uA, v0);
    builder->vertexUV(x, yTop, z1, uB, v0);
    builder->vertexUV(x, y, z1, uB, v1);
    builder->vertexUV(xFull1, y, z1, uA, v1);
    builder->vertexUV(xFull1, yTop, z1, uA, v0);

    return true;
}

bool BlockRenderer::tessellateIndividualSeaPickleInWorld(const BlockState* state, const BlockPos& pos, double height,
                                                         const Vec3* offset)
{
    BufferBuilder* builder = Tesselator::getInstance()->getBuilder();
    builder->tex2(getLightColor(state->getBlock(), mLevelSource, pos));
    int color = mBlockColors->getColor(state, mLevelSource, &pos, 0);
    builder->color((float)(color >> 16 & 255) / 255.0f, (float)(color >> 8 & 255) / 255.0f,
                   (float)(color & 255) / 255.0f);
    TextureAtlasSprite* texture = getTexture(state, Direction::DOWN);
    if (hasFixedTexture())
        texture = mFixedTexture;

    float y0 = (float)(double)pos.y;
    float y1 = (float)((double)pos.y + height);
    double sideTop = 16.0 - height * 16.0;

    float u0 = (float)texture->getU0(true);
    float u1 = (float)texture->getU(4.0);
    float v0 = (float)texture->getV(sideTop);
    float v1 = (float)texture->getV1(true);
    double centerX = (double)pos.x + offset->x;
    double centerZ = (double)pos.z + offset->z;
    float x0 = (float)(centerX - 0.125);
    float x1 = (float)(centerX + 0.125);
    float z0 = (float)(centerZ - 0.125);
    float z1 = (float)(centerZ + 0.125);
    builder->vertexUV(x0, y1, z0, u0, v0);
    builder->vertexUV(x0, y0, z0, u0, v1);
    builder->vertexUV(x0, y0, z1, u1, v1);
    builder->vertexUV(x0, y1, z1, u1, v0);
    builder->vertexUV(x0, y1, z1, u1, v0);
    builder->vertexUV(x0, y0, z1, u1, v1);
    builder->vertexUV(x0, y0, z0, u0, v1);
    builder->vertexUV(x0, y1, z0, u0, v0);

    u0 = (float)texture->getU(4.0);
    u1 = (float)texture->getU(8.0);
    v0 = (float)texture->getV(sideTop);
    v1 = (float)texture->getV1(true);
    builder->vertexUV(x1, y1, z1, u0, v0);
    builder->vertexUV(x1, y0, z1, u0, v1);
    builder->vertexUV(x1, y0, z0, u1, v1);
    builder->vertexUV(x1, y1, z0, u1, v0);
    builder->vertexUV(x1, y1, z0, u1, v0);
    builder->vertexUV(x1, y0, z0, u1, v1);
    builder->vertexUV(x1, y0, z1, u0, v1);
    builder->vertexUV(x1, y1, z1, u0, v0);

    u0 = (float)texture->getU(8.0);
    u1 = (float)texture->getU(12.0);
    v0 = (float)texture->getV(sideTop);
    v1 = (float)texture->getV1(true);
    builder->vertexUV(x1, y1, z0, u1, v0);
    builder->vertexUV(x1, y0, z0, u1, v1);
    builder->vertexUV(x0, y0, z0, u0, v1);
    builder->vertexUV(x0, y1, z0, u0, v0);
    builder->vertexUV(x0, y1, z0, u0, v0);
    builder->vertexUV(x0, y0, z0, u0, v1);
    builder->vertexUV(x1, y0, z0, u1, v1);
    builder->vertexUV(x1, y1, z0, u1, v0);

    u0 = (float)texture->getU(12.0);
    u1 = (float)texture->getU1(true);
    v0 = (float)texture->getV(sideTop);
    v1 = (float)texture->getV1(true);
    builder->vertexUV(x0, y1, z1, u1, v0);
    builder->vertexUV(x0, y0, z1, u1, v1);
    builder->vertexUV(x1, y0, z1, u0, v1);
    builder->vertexUV(x1, y1, z1, u0, v0);
    builder->vertexUV(x1, y1, z1, u0, v0);
    builder->vertexUV(x1, y0, z1, u0, v1);
    builder->vertexUV(x0, y0, z1, u1, v1);
    builder->vertexUV(x0, y1, z1, u1, v0);

    u0 = (float)texture->getU(4.0);
    u1 = (float)texture->getU(8.0);
    v0 = (float)texture->getV(5.0);
    v1 = (float)texture->getV(1.0);
    builder->vertexUV(x0, y1, z1, u1, v0);
    builder->vertexUV(x1, y1, z1, u1, v1);
    builder->vertexUV(x1, y1, z0, u0, v1);
    builder->vertexUV(x0, y1, z0, u0, v0);

    u0 = (float)texture->getU(8.0);
    u1 = (float)texture->getU(12.0);
    v0 = (float)texture->getV(5.0);
    v1 = (float)texture->getV(1.0);
    float innerTop = (float)((double)pos.y + height - 0.05f);
    builder->vertexUV(x0, innerTop, z1, u1, v0);
    builder->vertexUV(x1, innerTop, z1, u1, v1);
    builder->vertexUV(x1, innerTop, z0, u0, v1);
    builder->vertexUV(x0, innerTop, z0, u0, v0);
    builder->vertexUV(x0, y0, z1, u1, v0);
    builder->vertexUV(x1, y0, z1, u1, v1);
    builder->vertexUV(x1, y0, z0, u0, v1);
    builder->vertexUV(x0, y0, z0, u0, v0);

    if (state->getValue(SeaPickle::FLANGE) == 0)
    {
        Vec3* crown = Vec3::newTemp((double)pos.x + offset->x, (double)pos.y + height - 0.05f,
                                    (double)pos.z + offset->z);
        float leafU0 = (float)texture->getU0(true);
        float leafU1 = (float)texture->getU(4.0);
        float leafV0 = (float)texture->getV0(true);
        float leafV1 = (float)texture->getV(5.0);
        float crossU0 = (float)texture->getU(12.0);
        float crossU1 = (float)texture->getU1(true);
        float crossV0 = (float)texture->getV0(true);
        float crossV1 = (float)texture->getV(5.0);
        float crownX0 = (float)(crown->x - 0.04f);
        float crownX1 = (float)(crown->x + 0.04f);
        float crownZ0 = (float)(crown->z - 0.04f);
        float crownZ1 = (float)(crown->z + 0.04f);
        builder->vertexUV(crownX0, (float)(crown->y + 0.2f), crownZ0, leafU0, leafV0);
        builder->vertexUV(crownX0, (float)crown->y, crownZ0, leafU0, leafV1);
        builder->vertexUV(crownX1, (float)crown->y, crownZ1, leafU1, leafV1);
        builder->vertexUV(crownX1, (float)(crown->y + 0.2f), crownZ1, leafU1, leafV0);
        builder->vertexUV(crownX0, (float)(crown->y + 0.2f), crownZ1, crossU0, crossV0);
        builder->vertexUV(crownX0, (float)crown->y, crownZ1, crossU0, crossV1);
        builder->vertexUV(crownX1, (float)crown->y, crownZ0, crossU1, crossV1);
        builder->vertexUV(crownX1, (float)(crown->y + 0.2f), crownZ0, crossU1, crossV0);
        builder->vertexUV(crownX1, (float)(crown->y + 0.2f), crownZ1, leafU1, leafV0);
        builder->vertexUV(crownX1, (float)crown->y, crownZ1, leafU1, leafV1);
        builder->vertexUV(crownX0, (float)crown->y, crownZ0, leafU0, leafV1);
        builder->vertexUV(crownX0, (float)(crown->y + 0.2f), crownZ0, leafU0, leafV0);
        builder->vertexUV(crownX1, (float)(crown->y + 0.2f), crownZ0, crossU1, crossV0);
        builder->vertexUV(crownX1, (float)crown->y, crownZ0, crossU1, crossV1);
        builder->vertexUV(crownX0, (float)crown->y, crownZ1, crossU0, crossV1);
        builder->vertexUV(crownX0, (float)(crown->y + 0.2f), crownZ1, crossU0, crossV0);
    }
    return true;
}

bool BlockRenderer::tessellateSeaPickleInWorld(const BlockState* state, const BlockPos& pos)
{
    unsigned int count = state->getValue(SeaPickle::COUNT);
    switch (count)
    {
    case 0:
        tessellateIndividualSeaPickleInWorld(state, pos, 0.375, Vec3::newTemp(0.5, 0.0, 0.5));
        break;
    case 1:
        tessellateIndividualSeaPickleInWorld(state, pos, 0.375, Vec3::newTemp(0.35f, 0.0, 0.35f));
        tessellateIndividualSeaPickleInWorld(state, pos, 0.245f, Vec3::newTemp(0.65f, 0.0, 0.65f));
        break;
    case 2:
        tessellateIndividualSeaPickleInWorld(state, pos, 0.245f, Vec3::newTemp(0.5, 0.0, 0.7f));
        tessellateIndividualSeaPickleInWorld(state, pos, 0.375 + 0.06f, Vec3::newTemp(0.3f, 0.0, 0.3f));
        tessellateIndividualSeaPickleInWorld(state, pos, 0.375, Vec3::newTemp(0.6f, 0.0, 0.35f));
        break;
    case 3:
        tessellateIndividualSeaPickleInWorld(state, pos, 0.375, Vec3::newTemp(0.29f, 0.0, 0.31f));
        tessellateIndividualSeaPickleInWorld(state, pos, 0.245f, Vec3::newTemp(0.71f, 0.0, 0.76f));
        tessellateIndividualSeaPickleInWorld(state, pos, 0.375 - 0.07f, Vec3::newTemp(0.69f, 0.0, 0.27f));
        tessellateIndividualSeaPickleInWorld(state, pos, 0.375 + 0.06f, Vec3::newTemp(0.25, 0.0, 0.67f));
        break;
    }
    return true;
}

bool BlockRenderer::tesselateCommandBlockInWorld(const BlockState* state, const BlockPos& pos)
{
    unsigned int direction = state->getValue(CommandBlock::FACING)->get3DDataValue();
    bool rendered;

    switch (direction)
    {
    case Direction::eDirection_Down:
        mFaceRotation4 = 3;
        mFaceRotation5 = 3;
        rendered = tesselateBlockInWorld(state, pos);
        break;
    case Direction::eDirection_Up:
        mFaceRotation4 = 3;
        mFaceTransform0 = 1;
        mFaceTransform1 = 1;
        mFaceTransform2 = 1;
        mFaceTransform3 = 1;
        rendered = tesselateBlockInWorld(state, pos);
        break;
    case Direction::eDirection_North:
        mFaceRotation2 = 2;
        mFaceRotation3 = 1;
        mFaceRotation4 = 3;
        mFaceRotation5 = 3;
        rendered = tesselateBlockInWorld(state, pos);
        break;
    case Direction::eDirection_South:
        mFaceRotation2 = 1;
        mFaceRotation3 = 2;
        mFaceRotation4 = 0;
        mFaceRotation5 = 0;
        rendered = tesselateBlockInWorld(state, pos);
        break;
    case Direction::eDirection_West:
        mFaceRotation0 = 2;
        mFaceRotation1 = 1;
        mFaceRotation4 = 1;
        mFaceRotation5 = 2;
        rendered = tesselateBlockInWorld(state, pos);
        break;
    case Direction::eDirection_East:
        mFaceRotation0 = 5;
        mFaceRotation1 = 2;
        mFaceRotation4 = 2;
        mFaceRotation5 = 1;
        rendered = tesselateBlockInWorld(state, pos);
        break;
    default:
        rendered = tesselateBlockInWorld(state, pos);
        break;
    }

    for (int i = 0; i < 6; ++i)
    {
        (&mFaceRotation0)[i] = 0;
        (&mFaceTransform0)[i] = 0;
    }
    return rendered;
}
