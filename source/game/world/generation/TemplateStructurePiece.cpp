#include "game/world/generation/TemplateStructurePiece.hpp"

#include <boost/unordered_map.hpp>

#include "game/core/BoundingBox.hpp"
#include "game/core/DataInputStream.hpp"
#include "game/core/DataOutputStream.hpp"
#include "game/core/Direction.hpp"
#include "game/core/Mirror.hpp"
#include "game/core/Rotation.hpp"
#include "game/world/Level.hpp"
#include "game/world/block/Block.hpp"
#include "game/world/block/Blocks.hpp"
#include "game/world/generation/StructureManager.hpp"
#include "game/world/generation/StructurePlaceSettings.hpp"
#include "game/world/generation/StructureTemplate.hpp"

static StructurePlaceSettings* DEFAULT_PLACE_SETTINGS = new StructurePlaceSettings();

TemplateStructurePiece::TemplateStructurePiece() : StructurePiece(), mTemplatePosition()
{
    mPlaceSettings = DEFAULT_PLACE_SETTINGS->setIgnoreEntities(true)->setIgnoreBlock(Blocks::air);
    mTemplate = NULL;
}

TemplateStructurePiece::TemplateStructurePiece(int genDepth) : StructurePiece(genDepth), mTemplatePosition()
{
    mPlaceSettings = DEFAULT_PLACE_SETTINGS->setIgnoreEntities(true)->setIgnoreBlock(Blocks::air);
    mTemplate = NULL;
}

TemplateStructurePiece::~TemplateStructurePiece()
{
    if (mPlaceSettings != DEFAULT_PLACE_SETTINGS)
        delete mPlaceSettings;
}

void TemplateStructurePiece::setBoundingBoxFromTemplate()
{
    Rotation* rotation = mPlaceSettings->getRotation();
    BlockPos size = mTemplate->getSize(rotation);
    Mirror* mirror = mPlaceSettings->getMirror();
    delete mBoundingBox;
    mBoundingBox = new BoundingBox(0, 0, 0, size.x, size.y - 1, size.z);

    switch (rotation->getType())
    {
    case Rotation::eTYPE_Clockwise90:
        mBoundingBox->move(-size.x, 0, 0);
        break;
    case Rotation::eTYPE_Clockwise180:
        mBoundingBox->move(-size.x, 0, -size.z);
        break;
    case Rotation::eTYPE_CounterClockwise90:
        mBoundingBox->move(0, 0, -size.z);
        break;
    default:
        break;
    }

    Mirror::eTYPE mirrorType = mirror->getType();
    if (mirrorType == Mirror::eTYPE_LeftRight)
    {
        BlockPos offset = BlockPos::ZERO;
        if (rotation == Rotation::CLOCKWISE_90 || rotation == Rotation::COUNTERCLOCKWISE_90)
            offset = offset.relative(rotation->rotate(Direction::NORTH), size.x);
        else if (rotation == Rotation::CLOCKWISE_180)
            offset = offset.relative(Direction::SOUTH, size.z);
        else
            offset = offset.relative(Direction::NORTH, size.z);
        mBoundingBox->move(offset.x, 0, offset.z);
    }
    else if (mirrorType == Mirror::eTYPE_FrontBack)
    {
        BlockPos offset = BlockPos::ZERO;
        if (rotation == Rotation::CLOCKWISE_90 || rotation == Rotation::COUNTERCLOCKWISE_90)
        {
            offset = offset.relative(rotation->rotate(Direction::WEST), size.z);
            mBoundingBox->move(offset.x, 0, offset.z);
        }
        else if (rotation == Rotation::CLOCKWISE_180)
        {
            offset = offset.relative(Direction::EAST, size.x);
            mBoundingBox->move(offset.x, 0, offset.z);
        }
        else
        {
            offset = offset.relative(Direction::WEST, size.x);
            mBoundingBox->move(offset.x, 0, offset.z);
        }
    }

    mBoundingBox->move(mTemplatePosition.x, mTemplatePosition.y, mTemplatePosition.z);
}

void TemplateStructurePiece::setup(const StructureTemplate* templateObject, const BlockPos& templatePosition,
                                   StructurePlaceSettings* placeSettings)
{
    mTemplate = const_cast<StructureTemplate*>(templateObject);
    setOrientation(Direction::NORTH);
    mTemplatePosition = templatePosition;
    mPlaceSettings = placeSettings;
    setBoundingBoxFromTemplate();
}

void TemplateStructurePiece::addAdditionalSaveData(DataOutputStream* output)
{
    output->writeInt(mTemplatePosition.x);
    output->writeInt(mTemplatePosition.y);
    output->writeInt(mTemplatePosition.z);
}

void TemplateStructurePiece::readAdditionalSaveData(DataInputStream* input, StructureManager*)
{
    int x = input->readInt();
    int y = input->readInt();
    int z = input->readInt();
    mTemplatePosition = BlockPos(x, y, z);
}

bool TemplateStructurePiece::postProcess(Level* level, Random& random, BoundingBox* box)
{
    mPlaceSettings->setBoundingBox(box);
    mTemplate->placeInWorld(level, mTemplatePosition, mPlaceSettings, 18);
    StructureTemplate::MarkerMap markers = mTemplate->getMarkers(mTemplatePosition, mPlaceSettings);

    for (StructureTemplate::MarkerMap::const_iterator it = markers.begin(); it != markers.end(); ++it)
    {
        std::wstring marker = it->second;
        handleDataMarker(marker, it->first, level, random, box);
    }

    return true;
}

void TemplateStructurePiece::move(int x, int y, int z)
{
    StructurePiece::move(x, y, z);
    mTemplatePosition = mTemplatePosition.offset(x, y, z);
}

Rotation* TemplateStructurePiece::getRotation()
{
    return mPlaceSettings->getRotation();
}

BlockPos TemplateStructurePiece::getTemplatePosition()
{
    return mTemplatePosition;
}

StructureTemplate* TemplateStructurePiece::getStructureTemplate()
{
    return mTemplate;
}

StructurePlaceSettings* TemplateStructurePiece::getPlaceSettings()
{
    return mPlaceSettings;
}
