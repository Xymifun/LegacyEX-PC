#include "game/core/OceanMonumentPieces.hpp"

#include <algorithm>
#include <string>

#include "game/core/BoundingBox.hpp"
#include "game/core/DifficultyInstance.hpp"
#include "game/core/Direction.hpp"
#include "game/core/ElderGuardian.hpp"
#include "game/core/Mth.hpp"
#include "game/entity/mob/Entity.hpp"
#include "game/util/Random.hpp"
#include "game/world/Level.hpp"
#include "game/world/block/Block.hpp"
#include "game/world/block/BlockPos.hpp"
#include "game/world/block/Blocks.hpp"
#include "game/world/block/Material.hpp"
#include "game/world/block/PrismarineBlock.hpp"
#include "game/world/generation/StructureFeatureIO.hpp"

const BlockState* OceanMonumentPieces::OceanMonumentPiece::BASE_GRAY;
const BlockState* OceanMonumentPieces::OceanMonumentPiece::BASE_LIGHT;
const BlockState* OceanMonumentPieces::OceanMonumentPiece::BASE_BLACK;
const BlockState* OceanMonumentPieces::OceanMonumentPiece::LAMP_BLOCK;
const BlockState* OceanMonumentPieces::OceanMonumentPiece::FILL_BLOCK;
const BlockState* OceanMonumentPieces::OceanMonumentPiece::DOT_DECO_DATA =
    OceanMonumentPieces::OceanMonumentPiece::BASE_LIGHT;
const int OceanMonumentPieces::OceanMonumentPiece::GRIDROOM_SOURCE_INDEX =
    OceanMonumentPieces::OceanMonumentPiece::getRoomIndex(2, 0, 0);
const int OceanMonumentPieces::OceanMonumentPiece::GRIDROOM_TOP_CONNECT_INDEX =
    OceanMonumentPieces::OceanMonumentPiece::getRoomIndex(2, 2, 0);
const int OceanMonumentPieces::OceanMonumentPiece::GRIDROOM_LEFTWING_CONNECT_INDEX =
    OceanMonumentPieces::OceanMonumentPiece::getRoomIndex(0, 1, 0);
const int OceanMonumentPieces::OceanMonumentPiece::GRIDROOM_RIGHTWING_CONNECT_INDEX =
    OceanMonumentPieces::OceanMonumentPiece::getRoomIndex(4, 1, 0);

OceanMonumentPieces::OceanMonumentPieces()
{
}

void OceanMonumentPieces::loadStatic()
{
    StructureFeatureIO::setPieceId(eStructurePiece_MonumentBuilding, &MonumentBuilding::Create, L"OMB");
    StructureFeatureIO::setPieceId(eStructurePiece_OceanMonumentCoreRoom, &OceanMonumentCoreRoom::Create, L"OMCR");
    StructureFeatureIO::setPieceId(eStructurePiece_OceanMonumentDoubleXRoom, &OceanMonumentDoubleXRoom::Create,
                                   L"OMDXR");
    StructureFeatureIO::setPieceId(eStructurePiece_OceanMonumentDoubleXYRoom, &OceanMonumentDoubleXYRoom::Create,
                                   L"OMDXYR");
    StructureFeatureIO::setPieceId(eStructurePiece_OceanMonumentDoubleYRoom, &OceanMonumentDoubleYRoom::Create,
                                   L"OMDYR");
    StructureFeatureIO::setPieceId(eStructurePiece_OceanMonumentDoubleYZRoom, &OceanMonumentDoubleYZRoom::Create,
                                   L"OMDYZR");
    StructureFeatureIO::setPieceId(eStructurePiece_OceanMonumentDoubleZRoom, &OceanMonumentDoubleZRoom::Create,
                                   L"OMDZR");
    StructureFeatureIO::setPieceId(eStructurePiece_OceanMonumentEntryRoom, &OceanMonumentEntryRoom::Create, L"OMEntry");
    StructureFeatureIO::setPieceId(eStructurePiece_OceanMonumentPenthouse, &OceanMonumentPenthouse::Create,
                                   L"OMPenthouse");
    StructureFeatureIO::setPieceId(eStructurePiece_OceanMonumentSimpleRoom, &OceanMonumentSimpleRoom::Create,
                                   L"OMSimple");
    StructureFeatureIO::setPieceId(eStructurePiece_OceanMonumentSimpleTopRoom, &OceanMonumentSimpleTopRoom::Create,
                                   L"OMSimpleT");
}

OceanMonumentPieces::RoomDefinition::RoomDefinition(int index)
{
    mIndex = index;
    mClaimed = false;
}

void OceanMonumentPieces::RoomDefinition::setConnection(const Direction* direction,
                                                        const boost::shared_ptr<RoomDefinition>& connection)
{
    mConnections[direction->get3DDataValue()] = connection;
    connection->mConnections[direction->getOpposite()->get3DDataValue()] = shared_from_this();
}

boost::shared_ptr<OceanMonumentPieces::RoomDefinition> OceanMonumentPieces::RoomDefinition::fjConnection(
    int index) const
{
    return mConnections[index].lock();
}

boost::shared_ptr<OceanMonumentPieces::RoomDefinition> OceanMonumentPieces::RoomDefinition::fjConnection(
    const Direction* direction) const
{
    return fjConnection(direction->get3DDataValue());
}

void OceanMonumentPieces::RoomDefinition::updateOpenings()
{
    for (int i = 0; i < 6; ++i)
    {
        mHasOpening[i] = mConnections[i].lock() != NULL;
    }
}

bool OceanMonumentPieces::RoomDefinition::findSource(int scanIndex)
{
    if (mIsSource)
    {
        return true;
    }
    mScanIndex = scanIndex;
    for (int i = 0; i < 6; ++i)
    {
        boost::shared_ptr<RoomDefinition> connection = mConnections[i].lock();
        if (connection != NULL && mHasOpening[i] && connection->mScanIndex != scanIndex
            && connection->findSource(scanIndex))
        {
            return true;
        }
    }
    return false;
}

bool OceanMonumentPieces::RoomDefinition::isSpecial()
{
    return mIndex >= 75;
}

int OceanMonumentPieces::RoomDefinition::countOpenings()
{
    int count = 0;
    for (int i = 0; i < 6; ++i)
    {
        if (mHasOpening[i])
        {
            ++count;
        }
    }
    return count;
}

OceanMonumentPieces::OceanMonumentPiece::OceanMonumentPiece(int genDepth) : StructurePiece(genDepth)
{
    mRoomDefinition = boost::shared_ptr<RoomDefinition>();
}

OceanMonumentPieces::OceanMonumentPiece::OceanMonumentPiece(const Direction* orientation, BoundingBox* boundingBox)
    : StructurePiece(1)
{
    setOrientation(orientation);
    mBoundingBox = boundingBox;
    mRoomDefinition = boost::shared_ptr<RoomDefinition>();
}

OceanMonumentPieces::OceanMonumentPiece::OceanMonumentPiece(int genDepth, const Direction* orientation,
                                                            const boost::shared_ptr<RoomDefinition>& roomDefinition,
                                                            int roomWidth, int roomHeight, int roomDepth)
    : StructurePiece(genDepth)
{
    setOrientation(orientation);
    mRoomDefinition = roomDefinition;
    int index = mRoomDefinition->mIndex;
    int roomX = index % 5;
    int roomZ = index / 5 % 5;
    int roomY = index / 25;
    if (orientation == Direction::NORTH || orientation == Direction::SOUTH)
    {
        mBoundingBox = new BoundingBox(0, 0, 0, roomWidth * 8 - 1, roomHeight * 4 - 1, roomDepth * 8 - 1);
    }
    else
    {
        mBoundingBox = new BoundingBox(0, 0, 0, roomDepth * 8 - 1, roomHeight * 4 - 1, roomWidth * 8 - 1);
    }
    switch (orientation->get3DDataValue())
    {
    case 2:
        mBoundingBox->move(roomX * 8, roomY * 4, -(roomZ + roomDepth) * 8 + 1);
        break;
    case 3:
        mBoundingBox->move(roomX * 8, roomY * 4, roomZ * 8);
        break;
    case 4:
        mBoundingBox->move(-(roomZ + roomDepth) * 8 + 1, roomY * 4, roomX * 8);
        break;
    default:
        mBoundingBox->move(roomZ * 8, roomY * 4, roomX * 8);
        break;
    }
}

OceanMonumentPieces::OceanMonumentPiece::~OceanMonumentPiece()
{
    mRoomDefinition = boost::shared_ptr<RoomDefinition>();
}

void OceanMonumentPieces::OceanMonumentPiece::staticCtor()
{
    BASE_GRAY = Blocks::prismarine->getBlockState(PrismarineBlock::ROUGH_DATA);
    BASE_LIGHT = Blocks::prismarine->getBlockState(PrismarineBlock::BRICKS_DATA);
    BASE_BLACK = Blocks::prismarine->getBlockState(PrismarineBlock::DARK_DATA);
    DOT_DECO_DATA = BASE_LIGHT;
    LAMP_BLOCK = Blocks::sea_lantern->defaultBlockState();
    FILL_BLOCK = Blocks::water->defaultBlockState();
}

int OceanMonumentPieces::OceanMonumentPiece::getRoomIndex(int roomX, int roomY, int roomZ)
{
    return roomY * 25 + roomZ * 5 + roomX;
}

int OceanMonumentPieces::OceanMonumentPiece::getRoomX(int roomIndex)
{
    return roomIndex % 5;
}

int OceanMonumentPieces::OceanMonumentPiece::getRoomY(int roomIndex)
{
    return roomIndex / 25;
}

int OceanMonumentPieces::OceanMonumentPiece::getRoomZ(int roomIndex)
{
    return roomIndex / 5 % 5;
}

void OceanMonumentPieces::OceanMonumentPiece::fjKeepReference(const boost::shared_ptr<RoomDefinition>& roomDefinition)
{
    mKeptRooms.push_back(roomDefinition);
}

void OceanMonumentPieces::OceanMonumentPiece::addAdditionalSaveData(DataOutputStream* output)
{
}

void OceanMonumentPieces::OceanMonumentPiece::readAdditionalSaveData(DataInputStream* input,
                                                                     StructureManager* structureManager)
{
}

void OceanMonumentPieces::OceanMonumentPiece::generateWaterBox(Level* level, BoundingBox* box, int x0, int y0, int z0,
                                                               int x1, int y1, int z1, bool skipAir)
{
    for (int y = y0; y <= y1; ++y)
    {
        for (int x = x0; x <= x1; ++x)
        {
            for (int z = z0; z <= z1; ++z)
            {
                if (!skipAir || getBlock(level, x, y, z, box)->getBlock()->mMaterial != Material::AIR)
                {
                    if (getWorldY(y) < level->getSeaLevel())
                    {
                        placeBlock(level, FILL_BLOCK, x, y, z, box);
                    }
                    else
                    {
                        placeBlock(level, Blocks::air->defaultBlockState(), x, y, z, box);
                    }
                }
            }
        }
    }
}

void OceanMonumentPieces::OceanMonumentPiece::generateDefaultFloor(Level* level, BoundingBox* box, int x, int z,
                                                                   bool hasOpening)
{
    if (hasOpening)
    {
        generateBox(level, box, x, 0, z, x + 2, 0, z + 7, BASE_GRAY, BASE_GRAY, false);
        generateBox(level, box, x + 5, 0, z, x + 7, 0, z + 7, BASE_GRAY, BASE_GRAY, false);
        generateBox(level, box, x + 3, 0, z, x + 4, 0, z + 2, BASE_GRAY, BASE_GRAY, false);
        generateBox(level, box, x + 3, 0, z + 5, x + 4, 0, z + 7, BASE_GRAY, BASE_GRAY, false);
        generateBox(level, box, x + 3, 0, z + 2, x + 4, 0, z + 2, BASE_LIGHT, BASE_LIGHT, false);
        generateBox(level, box, x + 3, 0, z + 5, x + 4, 0, z + 5, BASE_LIGHT, BASE_LIGHT, false);
        generateBox(level, box, x + 2, 0, z + 3, x + 2, 0, z + 4, BASE_LIGHT, BASE_LIGHT, false);
        generateBox(level, box, x + 5, 0, z + 3, x + 5, 0, z + 4, BASE_LIGHT, BASE_LIGHT, false);
    }
    else
    {
        generateBox(level, box, x, 0, z, x + 7, 0, z + 7, BASE_GRAY, BASE_GRAY, false);
    }
}

void OceanMonumentPieces::OceanMonumentPiece::generateBoxOnFillOnly(Level* level, BoundingBox* box, int x0, int y0,
                                                                    int z0, int x1, int y1, int z1,
                                                                    const BlockState* state)
{
    for (int y = y0; y <= y1; ++y)
    {
        for (int x = x0; x <= x1; ++x)
        {
            for (int z = z0; z <= z1; ++z)
            {
                if (getBlock(level, x, y, z, box) == FILL_BLOCK)
                {
                    placeBlock(level, state, x, y, z, box);
                }
            }
        }
    }
}

bool OceanMonumentPieces::OceanMonumentPiece::chunkIntersects(BoundingBox* box, int x0, int z0, int x1, int z1)
{
    int worldX0 = getWorldX(x0, z0);
    int worldZ0 = getWorldZ(x0, z0);
    int worldX1 = getWorldX(x1, z1);
    int worldZ1 = getWorldZ(x1, z1);
    return box->intersects(std::min(worldX0, worldX1), std::min(worldZ0, worldZ1), std::max(worldX0, worldX1),
                           std::max(worldZ1, worldZ0));
}

bool OceanMonumentPieces::OceanMonumentPiece::spawnElder(Level* level, BoundingBox* box, int x, int y, int z)
{
    int worldX = getWorldX(x, z);
    int worldY = getWorldY(y);
    int worldZ = getWorldZ(x, z);
    BlockPos pos(worldX, worldY, worldZ);
    if (!box->isInside(pos))
    {
        return false;
    }
    boost::shared_ptr<ElderGuardian> elder(new ElderGuardian(level));
    elder->heal(elder->getMaxHealth());
    elder->moveTo(worldX + 0.5, worldY, worldZ + 0.5, 0.0f, 0.0f);
    elder->finalizeMobSpawn(level->getCurrentDifficultyAt(BlockPos(elder)), NULL, L"");
    level->addEntity(elder);
    return true;
}

OceanMonumentPieces::MonumentRoomFitter::~MonumentRoomFitter()
{
}

OceanMonumentPieces::FitSimpleRoom::~FitSimpleRoom()
{
}

bool OceanMonumentPieces::FitSimpleRoom::fits(const boost::shared_ptr<RoomDefinition>& roomDefinition)
{
    return true;
}

OceanMonumentPieces::OceanMonumentPiece* OceanMonumentPieces::FitSimpleRoom::create(
    const Direction* orientation, const boost::shared_ptr<RoomDefinition>& roomDefinition, Random& random)
{
    roomDefinition->mClaimed = true;
    return new OceanMonumentSimpleRoom(orientation, roomDefinition, random);
}

OceanMonumentPieces::FitSimpleTopRoom::~FitSimpleTopRoom()
{
}

bool OceanMonumentPieces::FitSimpleTopRoom::fits(const boost::shared_ptr<RoomDefinition>& roomDefinition)
{
    return !roomDefinition->mHasOpening[Direction::WEST->get3DDataValue()]
           && !roomDefinition->mHasOpening[Direction::EAST->get3DDataValue()]
           && !roomDefinition->mHasOpening[Direction::NORTH->get3DDataValue()]
           && !roomDefinition->mHasOpening[Direction::SOUTH->get3DDataValue()]
           && !roomDefinition->mHasOpening[Direction::UP->get3DDataValue()];
}

OceanMonumentPieces::OceanMonumentPiece* OceanMonumentPieces::FitSimpleTopRoom::create(
    const Direction* orientation, const boost::shared_ptr<RoomDefinition>& roomDefinition, Random& random)
{
    roomDefinition->mClaimed = true;
    return new OceanMonumentSimpleTopRoom(orientation, roomDefinition, random);
}

OceanMonumentPieces::FitDoubleYRoom::~FitDoubleYRoom()
{
}

bool OceanMonumentPieces::FitDoubleYRoom::fits(const boost::shared_ptr<RoomDefinition>& roomDefinition)
{
    return roomDefinition->mHasOpening[Direction::UP->get3DDataValue()]
           && !roomDefinition->fjConnection(Direction::UP)->mClaimed;
}

OceanMonumentPieces::OceanMonumentPiece* OceanMonumentPieces::FitDoubleYRoom::create(
    const Direction* orientation, const boost::shared_ptr<RoomDefinition>& roomDefinition, Random& random)
{
    roomDefinition->mClaimed = true;
    roomDefinition->fjConnection(Direction::UP)->mClaimed = true;
    return new OceanMonumentDoubleYRoom(orientation, roomDefinition, random);
}

OceanMonumentPieces::FitDoubleXRoom::~FitDoubleXRoom()
{
}

bool OceanMonumentPieces::FitDoubleXRoom::fits(const boost::shared_ptr<RoomDefinition>& roomDefinition)
{
    return roomDefinition->mHasOpening[Direction::EAST->get3DDataValue()]
           && !roomDefinition->fjConnection(Direction::EAST)->mClaimed;
}

OceanMonumentPieces::OceanMonumentPiece* OceanMonumentPieces::FitDoubleXRoom::create(
    const Direction* orientation, const boost::shared_ptr<RoomDefinition>& roomDefinition, Random& random)
{
    roomDefinition->mClaimed = true;
    roomDefinition->fjConnection(Direction::EAST)->mClaimed = true;
    return new OceanMonumentDoubleXRoom(orientation, roomDefinition, random);
}

OceanMonumentPieces::FitDoubleZRoom::~FitDoubleZRoom()
{
}

bool OceanMonumentPieces::FitDoubleZRoom::fits(const boost::shared_ptr<RoomDefinition>& roomDefinition)
{
    return roomDefinition->mHasOpening[Direction::NORTH->get3DDataValue()]
           && !roomDefinition->fjConnection(Direction::NORTH)->mClaimed;
}

OceanMonumentPieces::OceanMonumentPiece* OceanMonumentPieces::FitDoubleZRoom::create(
    const Direction* orientation, const boost::shared_ptr<RoomDefinition>& roomDefinition, Random& random)
{
    boost::shared_ptr<RoomDefinition> room = roomDefinition;
    if (!roomDefinition->mHasOpening[Direction::NORTH->get3DDataValue()]
        || roomDefinition->fjConnection(Direction::NORTH)->mClaimed)
    {
        room = roomDefinition->fjConnection(Direction::SOUTH);
    }
    room->mClaimed = true;
    room->fjConnection(Direction::NORTH)->mClaimed = true;
    return new OceanMonumentDoubleZRoom(orientation, room, random);
}

OceanMonumentPieces::FitDoubleXYRoom::~FitDoubleXYRoom()
{
}

bool OceanMonumentPieces::FitDoubleXYRoom::fits(const boost::shared_ptr<RoomDefinition>& roomDefinition)
{
    if (roomDefinition->mHasOpening[Direction::EAST->get3DDataValue()]
        && !roomDefinition->fjConnection(Direction::EAST)->mClaimed
        && roomDefinition->mHasOpening[Direction::UP->get3DDataValue()]
        && !roomDefinition->fjConnection(Direction::UP)->mClaimed)
    {
        boost::shared_ptr<RoomDefinition> east = roomDefinition->fjConnection(Direction::EAST);
        return east->mHasOpening[Direction::UP->get3DDataValue()] && !east->fjConnection(Direction::UP)->mClaimed;
    }
    return false;
}

OceanMonumentPieces::OceanMonumentPiece* OceanMonumentPieces::FitDoubleXYRoom::create(
    const Direction* orientation, const boost::shared_ptr<RoomDefinition>& roomDefinition, Random& random)
{
    roomDefinition->mClaimed = true;
    roomDefinition->fjConnection(Direction::EAST)->mClaimed = true;
    roomDefinition->fjConnection(Direction::UP)->mClaimed = true;
    roomDefinition->fjConnection(Direction::EAST)->fjConnection(Direction::UP)->mClaimed = true;
    return new OceanMonumentDoubleXYRoom(orientation, roomDefinition, random);
}

OceanMonumentPieces::FitDoubleYZRoom::~FitDoubleYZRoom()
{
}

bool OceanMonumentPieces::FitDoubleYZRoom::fits(const boost::shared_ptr<RoomDefinition>& roomDefinition)
{
    if (roomDefinition->mHasOpening[Direction::NORTH->get3DDataValue()]
        && !roomDefinition->fjConnection(Direction::NORTH)->mClaimed
        && roomDefinition->mHasOpening[Direction::UP->get3DDataValue()]
        && !roomDefinition->fjConnection(Direction::UP)->mClaimed)
    {
        boost::shared_ptr<RoomDefinition> north = roomDefinition->fjConnection(Direction::NORTH);
        return north->mHasOpening[Direction::UP->get3DDataValue()] && !north->fjConnection(Direction::UP)->mClaimed;
    }
    return false;
}

OceanMonumentPieces::OceanMonumentPiece* OceanMonumentPieces::FitDoubleYZRoom::create(
    const Direction* orientation, const boost::shared_ptr<RoomDefinition>& roomDefinition, Random& random)
{
    roomDefinition->mClaimed = true;
    roomDefinition->fjConnection(Direction::NORTH)->mClaimed = true;
    roomDefinition->fjConnection(Direction::UP)->mClaimed = true;
    roomDefinition->fjConnection(Direction::NORTH)->fjConnection(Direction::UP)->mClaimed = true;
    return new OceanMonumentDoubleYZRoom(orientation, roomDefinition, random);
}

OceanMonumentPieces::MonumentBuilding::MonumentBuilding() : OceanMonumentPiece(0)
{
}

OceanMonumentPieces::MonumentBuilding::MonumentBuilding(Random& random, int x, int z, const Direction* orientation)
    : OceanMonumentPiece(0)
{
    delete mBoundingBox;
    setOrientation(orientation);
    const Direction* direction = getOrientation();
    if (direction->getAxis() == Direction::Axis::Z)
    {
        mBoundingBox = new BoundingBox(x, 39, z, x + 57, 61, z + 57);
    }
    else
    {
        mBoundingBox = new BoundingBox(x, 39, z, x + 57, 61, z + 57);
    }

    std::vector<boost::shared_ptr<RoomDefinition> >* rooms = generateRoomGraph(random);
    mSourceRoom->mClaimed = true;
    OceanMonumentPiece* piece = new OceanMonumentEntryRoom(direction, mSourceRoom);
    mChildPieces.push_back(piece);
    piece = new OceanMonumentCoreRoom(direction, mCoreRoom, random);
    mChildPieces.push_back(piece);

    std::vector<MonumentRoomFitter*> fitters;
    fitters.push_back(new FitDoubleXYRoom());
    fitters.push_back(new FitDoubleYZRoom());
    fitters.push_back(new FitDoubleZRoom());
    fitters.push_back(new FitDoubleXRoom());
    fitters.push_back(new FitDoubleYRoom());
    fitters.push_back(new FitSimpleTopRoom());
    fitters.push_back(new FitSimpleRoom());

    for (std::vector<boost::shared_ptr<RoomDefinition> >::iterator it = rooms->begin(); it != rooms->end(); ++it)
    {
        boost::shared_ptr<RoomDefinition>& room = *it;
        if (!room->mClaimed && !room->isSpecial())
        {
            for (std::vector<MonumentRoomFitter*>::iterator fitterIt = fitters.begin(); fitterIt != fitters.end();
                 ++fitterIt)
            {
                MonumentRoomFitter* fitter = *fitterIt;
                if (fitter->fits(room))
                {
                    mChildPieces.push_back(fitter->create(direction, room, random));
                    break;
                }
            }
        }
    }

    for (std::vector<MonumentRoomFitter*>::iterator fitter = fitters.begin(); fitter != fitters.end(); ++fitter)
    {
        delete *fitter;
    }
    delete rooms;

    int y = mBoundingBox->y0;
    int worldX = getWorldX(9, 22);
    int worldZ = getWorldZ(9, 22);
    for (std::vector<OceanMonumentPiece*>::iterator it = mChildPieces.begin(); it != mChildPieces.end(); ++it)
    {
        (*it)->getBoundingBox()->move(worldX, y, worldZ);
    }

    BoundingBox* leftWingBox = BoundingBox::createProper(getWorldX(1, 1), getWorldY(1), getWorldZ(1, 1),
                                                         getWorldX(23, 21), getWorldY(8), getWorldZ(23, 21));
    BoundingBox* rightWingBox = BoundingBox::createProper(getWorldX(34, 1), getWorldY(1), getWorldZ(34, 1),
                                                          getWorldX(56, 21), getWorldY(8), getWorldZ(56, 21));
    BoundingBox* penthouseBox = BoundingBox::createProper(getWorldX(22, 22), getWorldY(13), getWorldZ(22, 22),
                                                          getWorldX(35, 35), getWorldY(17), getWorldZ(35, 35));
    int wingValue = random.nextInt();
    mChildPieces.push_back(new OceanMonumentWingRoom(direction, leftWingBox, wingValue++));
    mChildPieces.push_back(new OceanMonumentWingRoom(direction, rightWingBox, wingValue++));
    mChildPieces.push_back(new OceanMonumentPenthouse(direction, penthouseBox));
}

OceanMonumentPieces::MonumentBuilding::~MonumentBuilding()
{
    std::vector<OceanMonumentPiece*>::iterator end = mChildPieces.end();
    for (std::vector<OceanMonumentPiece*>::iterator it = mChildPieces.begin(); it != end; ++it)
    {
        delete *it;
    }
}

std::vector<boost::shared_ptr<OceanMonumentPieces::RoomDefinition> >* OceanMonumentPieces::MonumentBuilding::
    generateRoomGraph(Random& random)
{
    boost::shared_ptr<RoomDefinition> rooms[75];

    for (int x = 0; x < 5; ++x)
    {
        for (int z = 0; z < 4; ++z)
        {
            int index = getRoomIndex(x, 0, z);
            rooms[index] = boost::shared_ptr<RoomDefinition>(new RoomDefinition(index));
        }
    }
    for (int x = 0; x < 5; ++x)
    {
        for (int z = 0; z < 4; ++z)
        {
            int index = getRoomIndex(x, 1, z);
            rooms[index] = boost::shared_ptr<RoomDefinition>(new RoomDefinition(index));
        }
    }
    for (int x = 1; x < 4; ++x)
    {
        for (int z = 0; z < 2; ++z)
        {
            int index = getRoomIndex(x, 2, z);
            rooms[index] = boost::shared_ptr<RoomDefinition>(new RoomDefinition(index));
        }
    }

    mSourceRoom = rooms[GRIDROOM_SOURCE_INDEX];

    for (int x = 0; x < 5; ++x)
    {
        for (int z = 0; z < 5; ++z)
        {
            for (int y = 0; y < 3; ++y)
            {
                int index = getRoomIndex(x, y, z);
                if (rooms[index] == NULL)
                {
                    continue;
                }
                for (std::vector<const Direction*>::iterator it = Direction::VALUES.begin();
                     it != Direction::VALUES.end(); ++it)
                {
                    const Direction* direction = *it;
                    int neighbourX = x + direction->getStepX();
                    int neighbourY = y + direction->getStepY();
                    int neighbourZ = z + direction->getStepZ();
                    if (neighbourX >= 0 && neighbourX < 5 && neighbourZ >= 0 && neighbourZ < 5 && neighbourY >= 0
                        && neighbourY < 3)
                    {
                        int neighbourIndex = getRoomIndex(neighbourX, neighbourY, neighbourZ);
                        if (rooms[neighbourIndex] != NULL)
                        {
                            if (neighbourZ == z)
                            {
                                rooms[index]->setConnection(direction, rooms[neighbourIndex]);
                            }
                            else
                            {
                                rooms[index]->setConnection(direction->getOpposite(), rooms[neighbourIndex]);
                            }
                        }
                    }
                }
            }
        }
    }

    boost::shared_ptr<RoomDefinition> topRoom(new RoomDefinition(1003));
    boost::shared_ptr<RoomDefinition> leftWingRoom(new RoomDefinition(1001));
    boost::shared_ptr<RoomDefinition> rightWingRoom(new RoomDefinition(1002));
    rooms[GRIDROOM_TOP_CONNECT_INDEX]->setConnection(Direction::UP, topRoom);
    rooms[GRIDROOM_LEFTWING_CONNECT_INDEX]->setConnection(Direction::SOUTH, leftWingRoom);
    rooms[GRIDROOM_RIGHTWING_CONNECT_INDEX]->setConnection(Direction::SOUTH, rightWingRoom);
    topRoom->mClaimed = true;
    leftWingRoom->mClaimed = true;
    rightWingRoom->mClaimed = true;
    mSourceRoom->mIsSource = true;

    mCoreRoom = rooms[getRoomIndex(random.nextInt(4), 0, 2)];
    mCoreRoom->mClaimed = true;
    mCoreRoom->fjConnection(Direction::EAST)->mClaimed = true;
    mCoreRoom->fjConnection(Direction::NORTH)->mClaimed = true;
    mCoreRoom->fjConnection(Direction::EAST)->fjConnection(Direction::NORTH)->mClaimed = true;
    mCoreRoom->fjConnection(Direction::UP)->mClaimed = true;
    mCoreRoom->fjConnection(Direction::EAST)->fjConnection(Direction::UP)->mClaimed = true;
    mCoreRoom->fjConnection(Direction::NORTH)->fjConnection(Direction::UP)->mClaimed = true;
    mCoreRoom->fjConnection(Direction::EAST)->fjConnection(Direction::NORTH)->fjConnection(Direction::UP)->mClaimed =
        true;

    std::vector<boost::shared_ptr<RoomDefinition> >* roomList = new std::vector<boost::shared_ptr<RoomDefinition> >();
    for (int i = 0; i < 75; ++i)
    {
        if (rooms[i] != NULL)
        {
            rooms[i]->updateOpenings();
            roomList->push_back(rooms[i]);
        }
    }
    topRoom->updateOpenings();
    Mth::shuffle(roomList, random);

    int scanIndex = 1;
    for (std::vector<boost::shared_ptr<RoomDefinition> >::iterator it = roomList->begin(); it != roomList->end(); ++it)
    {
        boost::shared_ptr<RoomDefinition> room = *it;
        int closedOpenings = 0;
        int attempts = 0;
        while (closedOpenings < 2 && attempts < 5)
        {
            ++attempts;
            int opening = random.nextInt(6);
            if (room->mHasOpening[opening])
            {
                boost::shared_ptr<RoomDefinition> connection = room->fjConnection(opening);
                int opposite = Direction::from3DDataValue(opening)->getOpposite()->get3DDataValue();
                room->mHasOpening[opening] = false;
                connection->mHasOpening[opposite] = false;
                if (room->findSource(scanIndex++) && connection->findSource(scanIndex++))
                {
                    ++closedOpenings;
                }
                else
                {
                    room->mHasOpening[opening] = true;
                    connection->mHasOpening[opposite] = true;
                }
            }
        }
    }

    roomList->push_back(topRoom);
    roomList->push_back(leftWingRoom);
    roomList->push_back(rightWingRoom);
    return roomList;
}

bool OceanMonumentPieces::MonumentBuilding::postProcess(Level* level, Random& random, BoundingBox* box)
{
    int waterTop = std::max(level->getSeaLevel(), 64) - mBoundingBox->y0;
    generateWaterBox(level, box, 0, 0, 0, 58, waterTop, 58, false);
    generateWing(false, 0, level, random, box);
    generateWing(true, 33, level, random, box);
    generateEntranceArchs(level, random, box);
    generateEntranceWall(level, random, box);
    generateRoofPiece(level, random, box);
    generateLowerWall(level, random, box);
    generateMiddleWall(level, random, box);
    generateUpperWall(level, random, box);

    for (int column = 0; column < 7; ++column)
    {
        int row = 0;
        while (row < 7)
        {
            if (row == 0 && column == 3)
            {
                row = 6;
            }
            int x = column * 9;
            int z = row * 9;
            for (int dx = 0; dx < 4; ++dx)
            {
                for (int dz = 0; dz < 4; ++dz)
                {
                    placeBlock(level, BASE_LIGHT, x + dx, 0, z + dz, box);
                    fillColumnDown(level, BASE_LIGHT, x + dx, -1, z + dz, box);
                }
            }
            if (column == 0 || column == 6)
            {
                ++row;
            }
            else
            {
                row += 6;
            }
        }
    }

    for (int i = 0; i < 5; ++i)
    {
        generateWaterBox(level, box, -1 - i, 0 + i * 2, -1 - i, -1 - i, 23, 58 + i, false);
        generateWaterBox(level, box, 58 + i, 0 + i * 2, -1 - i, 58 + i, 23, 58 + i, false);
        generateWaterBox(level, box, 0 - i, 0 + i * 2, -1 - i, 57 + i, 23, -1 - i, false);
        generateWaterBox(level, box, 0 - i, 0 + i * 2, 58 + i, 57 + i, 23, 58 + i, false);
    }

    for (std::vector<OceanMonumentPiece*>::iterator it = mChildPieces.begin(); it != mChildPieces.end(); ++it)
    {
        OceanMonumentPiece* piece = *it;
        if (piece->getBoundingBox()->intersects(box))
        {
            piece->postProcess(level, random, box);
        }
    }
    return true;
}

void OceanMonumentPieces::MonumentBuilding::generateWing(bool flipped, int xOffset, Level* level, Random& random,
                                                         BoundingBox* box)
{
    if (!chunkIntersects(box, xOffset, 0, xOffset + 23, 20))
    {
        return;
    }
    generateBox(level, box, xOffset, 0, 0, xOffset + 24, 0, 20, BASE_GRAY, BASE_GRAY, false);
    generateWaterBox(level, box, xOffset, 1, 0, xOffset + 24, 10, 20, false);
    for (int i = 0; i < 4; ++i)
    {
        generateBox(level, box, xOffset + i, i + 1, i, xOffset + i, i + 1, 20, BASE_LIGHT, BASE_LIGHT, false);
        generateBox(level, box, xOffset + i + 7, i + 5, i + 7, xOffset + i + 7, i + 5, 20, BASE_LIGHT, BASE_LIGHT,
                    false);
        generateBox(level, box, xOffset + 17 - i, i + 5, i + 7, xOffset + 17 - i, i + 5, 20, BASE_LIGHT, BASE_LIGHT,
                    false);
        generateBox(level, box, xOffset + 24 - i, i + 1, i, xOffset + 24 - i, i + 1, 20, BASE_LIGHT, BASE_LIGHT, false);
        generateBox(level, box, xOffset + i + 1, i + 1, i, xOffset + 23 - i, i + 1, i, BASE_LIGHT, BASE_LIGHT, false);
        generateBox(level, box, xOffset + i + 8, i + 5, i + 7, xOffset + 16 - i, i + 5, i + 7, BASE_LIGHT, BASE_LIGHT,
                    false);
    }
    generateBox(level, box, xOffset + 4, 4, 4, xOffset + 6, 4, 20, BASE_GRAY, BASE_GRAY, false);
    generateBox(level, box, xOffset + 7, 4, 4, xOffset + 17, 4, 6, BASE_GRAY, BASE_GRAY, false);
    generateBox(level, box, xOffset + 18, 4, 4, xOffset + 20, 4, 20, BASE_GRAY, BASE_GRAY, false);
    generateBox(level, box, xOffset + 11, 8, 11, xOffset + 13, 8, 20, BASE_GRAY, BASE_GRAY, false);
    placeBlock(level, DOT_DECO_DATA, xOffset + 12, 9, 12, box);
    placeBlock(level, DOT_DECO_DATA, xOffset + 12, 9, 15, box);
    placeBlock(level, DOT_DECO_DATA, xOffset + 12, 9, 18, box);

    int outerX = xOffset + (flipped ? 19 : 5);
    int innerX = xOffset + (flipped ? 5 : 19);
    for (int z = 20; z >= 5; z -= 3)
    {
        placeBlock(level, DOT_DECO_DATA, outerX, 5, z, box);
    }
    for (int z = 19; z >= 7; z -= 3)
    {
        placeBlock(level, DOT_DECO_DATA, innerX, 5, z, box);
    }
    for (int i = 0; i < 4; ++i)
    {
        int x = flipped ? xOffset + 24 - (17 - i * 3) : xOffset + 17 - i * 3;
        placeBlock(level, DOT_DECO_DATA, x, 5, 5, box);
    }
    placeBlock(level, DOT_DECO_DATA, innerX, 5, 5, box);
    generateBox(level, box, xOffset + 11, 1, 12, xOffset + 13, 7, 12, BASE_GRAY, BASE_GRAY, false);
    generateBox(level, box, xOffset + 12, 1, 11, xOffset + 12, 7, 13, BASE_GRAY, BASE_GRAY, false);
}

void OceanMonumentPieces::MonumentBuilding::generateEntranceArchs(Level* level, Random& random, BoundingBox* box)
{
    if (!chunkIntersects(box, 22, 5, 35, 17))
    {
        return;
    }
    generateWaterBox(level, box, 25, 0, 0, 32, 8, 20, false);
    for (int i = 0; i < 4; ++i)
    {
        generateBox(level, box, 24, 2, 5 + i * 4, 24, 4, 5 + i * 4, BASE_LIGHT, BASE_LIGHT, false);
        generateBox(level, box, 22, 4, 5 + i * 4, 23, 4, 5 + i * 4, BASE_LIGHT, BASE_LIGHT, false);
        placeBlock(level, BASE_LIGHT, 25, 5, 5 + i * 4, box);
        placeBlock(level, BASE_LIGHT, 26, 6, 5 + i * 4, box);
        placeBlock(level, LAMP_BLOCK, 26, 5, 5 + i * 4, box);
        generateBox(level, box, 33, 2, 5 + i * 4, 33, 4, 5 + i * 4, BASE_LIGHT, BASE_LIGHT, false);
        generateBox(level, box, 34, 4, 5 + i * 4, 35, 4, 5 + i * 4, BASE_LIGHT, BASE_LIGHT, false);
        placeBlock(level, BASE_LIGHT, 32, 5, 5 + i * 4, box);
        placeBlock(level, BASE_LIGHT, 31, 6, 5 + i * 4, box);
        placeBlock(level, LAMP_BLOCK, 31, 5, 5 + i * 4, box);
        generateBox(level, box, 27, 6, 5 + i * 4, 30, 6, 5 + i * 4, BASE_GRAY, BASE_GRAY, false);
    }
}

void OceanMonumentPieces::MonumentBuilding::generateEntranceWall(Level* level, Random& random, BoundingBox* box)
{
    if (!chunkIntersects(box, 15, 20, 42, 21))
    {
        return;
    }
    generateBox(level, box, 15, 0, 21, 42, 0, 21, BASE_GRAY, BASE_GRAY, false);
    generateWaterBox(level, box, 26, 1, 21, 31, 3, 21, false);
    generateBox(level, box, 21, 12, 21, 36, 12, 21, BASE_GRAY, BASE_GRAY, false);
    generateBox(level, box, 17, 11, 21, 40, 11, 21, BASE_GRAY, BASE_GRAY, false);
    generateBox(level, box, 16, 10, 21, 41, 10, 21, BASE_GRAY, BASE_GRAY, false);
    generateBox(level, box, 15, 7, 21, 42, 9, 21, BASE_GRAY, BASE_GRAY, false);
    generateBox(level, box, 16, 6, 21, 41, 6, 21, BASE_GRAY, BASE_GRAY, false);
    generateBox(level, box, 17, 5, 21, 40, 5, 21, BASE_GRAY, BASE_GRAY, false);
    generateBox(level, box, 21, 4, 21, 36, 4, 21, BASE_GRAY, BASE_GRAY, false);
    generateBox(level, box, 22, 3, 21, 26, 3, 21, BASE_GRAY, BASE_GRAY, false);
    generateBox(level, box, 31, 3, 21, 35, 3, 21, BASE_GRAY, BASE_GRAY, false);
    generateBox(level, box, 23, 2, 21, 25, 2, 21, BASE_GRAY, BASE_GRAY, false);
    generateBox(level, box, 32, 2, 21, 34, 2, 21, BASE_GRAY, BASE_GRAY, false);
    generateBox(level, box, 28, 4, 20, 29, 4, 21, BASE_LIGHT, BASE_LIGHT, false);
    placeBlock(level, BASE_LIGHT, 27, 3, 21, box);
    placeBlock(level, BASE_LIGHT, 30, 3, 21, box);
    placeBlock(level, BASE_LIGHT, 26, 2, 21, box);
    placeBlock(level, BASE_LIGHT, 31, 2, 21, box);
    placeBlock(level, BASE_LIGHT, 25, 1, 21, box);
    placeBlock(level, BASE_LIGHT, 32, 1, 21, box);
    for (int i = 0; i < 7; ++i)
    {
        placeBlock(level, BASE_BLACK, 28 - i, 6 + i, 21, box);
        placeBlock(level, BASE_BLACK, 29 + i, 6 + i, 21, box);
    }
    for (int i = 0; i < 4; ++i)
    {
        placeBlock(level, BASE_BLACK, 28 - i, 9 + i, 21, box);
        placeBlock(level, BASE_BLACK, 29 + i, 9 + i, 21, box);
    }
    placeBlock(level, BASE_BLACK, 28, 12, 21, box);
    placeBlock(level, BASE_BLACK, 29, 12, 21, box);
    for (int i = 0; i < 3; ++i)
    {
        placeBlock(level, BASE_BLACK, 22 - i * 2, 8, 21, box);
        placeBlock(level, BASE_BLACK, 22 - i * 2, 9, 21, box);
        placeBlock(level, BASE_BLACK, 35 + i * 2, 8, 21, box);
        placeBlock(level, BASE_BLACK, 35 + i * 2, 9, 21, box);
    }
    generateWaterBox(level, box, 15, 13, 21, 42, 15, 21, false);
    generateWaterBox(level, box, 15, 1, 21, 15, 6, 21, false);
    generateWaterBox(level, box, 16, 1, 21, 16, 5, 21, false);
    generateWaterBox(level, box, 17, 1, 21, 20, 4, 21, false);
    generateWaterBox(level, box, 21, 1, 21, 21, 3, 21, false);
    generateWaterBox(level, box, 22, 1, 21, 22, 2, 21, false);
    generateWaterBox(level, box, 23, 1, 21, 24, 1, 21, false);
    generateWaterBox(level, box, 42, 1, 21, 42, 6, 21, false);
    generateWaterBox(level, box, 41, 1, 21, 41, 5, 21, false);
    generateWaterBox(level, box, 37, 1, 21, 40, 4, 21, false);
    generateWaterBox(level, box, 36, 1, 21, 36, 3, 21, false);
    generateWaterBox(level, box, 33, 1, 21, 34, 1, 21, false);
    generateWaterBox(level, box, 35, 1, 21, 35, 2, 21, false);
}

void OceanMonumentPieces::MonumentBuilding::generateRoofPiece(Level* level, Random& random, BoundingBox* box)
{
    if (!chunkIntersects(box, 21, 21, 36, 36))
    {
        return;
    }
    generateBox(level, box, 21, 0, 22, 36, 0, 36, BASE_GRAY, BASE_GRAY, false);
    generateWaterBox(level, box, 21, 1, 22, 36, 23, 36, false);
    for (int i = 0; i < 4; ++i)
    {
        generateBox(level, box, 21 + i, 13 + i, 21 + i, 36 - i, 13 + i, 21 + i, BASE_LIGHT, BASE_LIGHT, false);
        generateBox(level, box, 21 + i, 13 + i, 36 - i, 36 - i, 13 + i, 36 - i, BASE_LIGHT, BASE_LIGHT, false);
        generateBox(level, box, 21 + i, 13 + i, 22 + i, 21 + i, 13 + i, 35 - i, BASE_LIGHT, BASE_LIGHT, false);
        generateBox(level, box, 36 - i, 13 + i, 22 + i, 36 - i, 13 + i, 35 - i, BASE_LIGHT, BASE_LIGHT, false);
    }
    generateBox(level, box, 25, 16, 25, 32, 16, 32, BASE_GRAY, BASE_GRAY, false);
    generateBox(level, box, 25, 17, 25, 25, 19, 25, BASE_LIGHT, BASE_LIGHT, false);
    generateBox(level, box, 32, 17, 25, 32, 19, 25, BASE_LIGHT, BASE_LIGHT, false);
    generateBox(level, box, 25, 17, 32, 25, 19, 32, BASE_LIGHT, BASE_LIGHT, false);
    generateBox(level, box, 32, 17, 32, 32, 19, 32, BASE_LIGHT, BASE_LIGHT, false);
    placeBlock(level, BASE_LIGHT, 26, 20, 26, box);
    placeBlock(level, BASE_LIGHT, 27, 21, 27, box);
    placeBlock(level, LAMP_BLOCK, 27, 20, 27, box);
    placeBlock(level, BASE_LIGHT, 26, 20, 31, box);
    placeBlock(level, BASE_LIGHT, 27, 21, 30, box);
    placeBlock(level, LAMP_BLOCK, 27, 20, 30, box);
    placeBlock(level, BASE_LIGHT, 31, 20, 31, box);
    placeBlock(level, BASE_LIGHT, 30, 21, 30, box);
    placeBlock(level, LAMP_BLOCK, 30, 20, 30, box);
    placeBlock(level, BASE_LIGHT, 31, 20, 26, box);
    placeBlock(level, BASE_LIGHT, 30, 21, 27, box);
    placeBlock(level, LAMP_BLOCK, 30, 20, 27, box);
    generateBox(level, box, 28, 21, 27, 29, 21, 27, BASE_GRAY, BASE_GRAY, false);
    generateBox(level, box, 27, 21, 28, 27, 21, 29, BASE_GRAY, BASE_GRAY, false);
    generateBox(level, box, 28, 21, 30, 29, 21, 30, BASE_GRAY, BASE_GRAY, false);
    generateBox(level, box, 30, 21, 28, 30, 21, 29, BASE_GRAY, BASE_GRAY, false);
}

void OceanMonumentPieces::MonumentBuilding::generateLowerWall(Level* level, Random& random, BoundingBox* box)
{
    if (chunkIntersects(box, 0, 21, 6, 58))
    {
        generateBox(level, box, 0, 0, 21, 6, 0, 57, BASE_GRAY, BASE_GRAY, false);
        generateWaterBox(level, box, 0, 1, 21, 6, 7, 57, false);
        generateBox(level, box, 4, 4, 21, 6, 4, 53, BASE_GRAY, BASE_GRAY, false);
        for (int i = 0; i < 4; ++i)
        {
            generateBox(level, box, i, i + 1, 21, i, i + 1, 57 - i, BASE_LIGHT, BASE_LIGHT, false);
        }
        for (int z = 23; z < 53; z += 3)
        {
            placeBlock(level, DOT_DECO_DATA, 5, 5, z, box);
        }
        placeBlock(level, DOT_DECO_DATA, 5, 5, 52, box);
        for (int i = 0; i < 4; ++i)
        {
            generateBox(level, box, i, i + 1, 21, i, i + 1, 57 - i, BASE_LIGHT, BASE_LIGHT, false);
        }
        generateBox(level, box, 4, 1, 52, 6, 3, 52, BASE_GRAY, BASE_GRAY, false);
        generateBox(level, box, 5, 1, 51, 5, 3, 53, BASE_GRAY, BASE_GRAY, false);
    }
    if (chunkIntersects(box, 51, 21, 58, 58))
    {
        generateBox(level, box, 51, 0, 21, 57, 0, 57, BASE_GRAY, BASE_GRAY, false);
        generateWaterBox(level, box, 51, 1, 21, 57, 7, 57, false);
        generateBox(level, box, 51, 4, 21, 53, 4, 53, BASE_GRAY, BASE_GRAY, false);
        for (int i = 0; i < 4; ++i)
        {
            generateBox(level, box, 57 - i, i + 1, 21, 57 - i, i + 1, 57 - i, BASE_LIGHT, BASE_LIGHT, false);
        }
        for (int z = 23; z < 53; z += 3)
        {
            placeBlock(level, DOT_DECO_DATA, 52, 5, z, box);
        }
        placeBlock(level, DOT_DECO_DATA, 52, 5, 52, box);
        generateBox(level, box, 51, 1, 52, 53, 3, 52, BASE_GRAY, BASE_GRAY, false);
        generateBox(level, box, 52, 1, 51, 52, 3, 53, BASE_GRAY, BASE_GRAY, false);
    }
    if (chunkIntersects(box, 0, 51, 57, 57))
    {
        generateBox(level, box, 7, 0, 51, 50, 0, 57, BASE_GRAY, BASE_GRAY, false);
        generateWaterBox(level, box, 7, 1, 51, 50, 10, 57, false);
        for (int i = 0; i < 4; ++i)
        {
            generateBox(level, box, i + 1, i + 1, 57 - i, 56 - i, i + 1, 57 - i, BASE_LIGHT, BASE_LIGHT, false);
        }
    }
}

void OceanMonumentPieces::MonumentBuilding::generateMiddleWall(Level* level, Random& random, BoundingBox* box)
{
    if (chunkIntersects(box, 7, 21, 13, 50))
    {
        generateBox(level, box, 7, 0, 21, 13, 0, 50, BASE_GRAY, BASE_GRAY, false);
        generateWaterBox(level, box, 7, 1, 21, 13, 10, 50, false);
        generateBox(level, box, 11, 8, 21, 13, 8, 53, BASE_GRAY, BASE_GRAY, false);
        for (int i = 0; i < 4; ++i)
        {
            generateBox(level, box, i + 7, i + 5, 21, i + 7, i + 5, 54, BASE_LIGHT, BASE_LIGHT, false);
        }
        for (int z = 21; z <= 45; z += 3)
        {
            placeBlock(level, DOT_DECO_DATA, 12, 9, z, box);
        }
    }
    if (chunkIntersects(box, 44, 21, 50, 54))
    {
        generateBox(level, box, 44, 0, 21, 50, 0, 50, BASE_GRAY, BASE_GRAY, false);
        generateWaterBox(level, box, 44, 1, 21, 50, 10, 50, false);
        generateBox(level, box, 44, 8, 21, 46, 8, 53, BASE_GRAY, BASE_GRAY, false);
        for (int i = 0; i < 4; ++i)
        {
            generateBox(level, box, 50 - i, i + 5, 21, 50 - i, i + 5, 54, BASE_LIGHT, BASE_LIGHT, false);
        }
        for (int z = 21; z <= 45; z += 3)
        {
            placeBlock(level, DOT_DECO_DATA, 45, 9, z, box);
        }
    }
    if (chunkIntersects(box, 8, 44, 49, 54))
    {
        generateBox(level, box, 14, 0, 44, 43, 0, 50, BASE_GRAY, BASE_GRAY, false);
        generateWaterBox(level, box, 14, 1, 44, 43, 10, 50, false);
        for (int x = 12; x <= 45; x += 3)
        {
            placeBlock(level, DOT_DECO_DATA, x, 9, 45, box);
            placeBlock(level, DOT_DECO_DATA, x, 9, 52, box);
            if (x == 12 || x == 18 || x == 24 || x == 33 || x == 39 || x == 45)
            {
                placeBlock(level, DOT_DECO_DATA, x, 9, 47, box);
                placeBlock(level, DOT_DECO_DATA, x, 9, 50, box);
                placeBlock(level, DOT_DECO_DATA, x, 10, 45, box);
                placeBlock(level, DOT_DECO_DATA, x, 10, 46, box);
                placeBlock(level, DOT_DECO_DATA, x, 10, 51, box);
                placeBlock(level, DOT_DECO_DATA, x, 10, 52, box);
                placeBlock(level, DOT_DECO_DATA, x, 11, 47, box);
                placeBlock(level, DOT_DECO_DATA, x, 11, 50, box);
                placeBlock(level, DOT_DECO_DATA, x, 12, 48, box);
                placeBlock(level, DOT_DECO_DATA, x, 12, 49, box);
            }
        }
        for (int i = 0; i < 3; ++i)
        {
            generateBox(level, box, 8 + i, 5 + i, 54, 49 - i, 5 + i, 54, BASE_GRAY, BASE_GRAY, false);
        }
        generateBox(level, box, 11, 8, 54, 46, 8, 54, BASE_LIGHT, BASE_LIGHT, false);
        generateBox(level, box, 14, 8, 44, 43, 8, 53, BASE_GRAY, BASE_GRAY, false);
    }
}

void OceanMonumentPieces::MonumentBuilding::generateUpperWall(Level* level, Random& random, BoundingBox* box)
{
    if (chunkIntersects(box, 14, 21, 20, 43))
    {
        generateBox(level, box, 14, 0, 21, 20, 0, 43, BASE_GRAY, BASE_GRAY, false);
        generateWaterBox(level, box, 14, 1, 22, 20, 14, 43, false);
        generateBox(level, box, 18, 12, 22, 20, 12, 39, BASE_GRAY, BASE_GRAY, false);
        generateBox(level, box, 18, 12, 21, 20, 12, 21, BASE_LIGHT, BASE_LIGHT, false);
        for (int i = 0; i < 4; ++i)
        {
            generateBox(level, box, i + 14, i + 9, 21, i + 14, i + 9, 43 - i, BASE_LIGHT, BASE_LIGHT, false);
        }
        for (int z = 23; z <= 39; z += 3)
        {
            placeBlock(level, DOT_DECO_DATA, 19, 13, z, box);
        }
    }
    if (chunkIntersects(box, 37, 21, 43, 43))
    {
        generateBox(level, box, 37, 0, 21, 43, 0, 43, BASE_GRAY, BASE_GRAY, false);
        generateWaterBox(level, box, 37, 1, 22, 43, 14, 43, false);
        generateBox(level, box, 37, 12, 22, 39, 12, 39, BASE_GRAY, BASE_GRAY, false);
        generateBox(level, box, 37, 12, 21, 39, 12, 21, BASE_LIGHT, BASE_LIGHT, false);
        for (int i = 0; i < 4; ++i)
        {
            generateBox(level, box, 43 - i, i + 9, 21, 43 - i, i + 9, 43 - i, BASE_LIGHT, BASE_LIGHT, false);
        }
        for (int z = 23; z <= 39; z += 3)
        {
            placeBlock(level, DOT_DECO_DATA, 38, 13, z, box);
        }
    }
    if (chunkIntersects(box, 15, 37, 42, 43))
    {
        generateBox(level, box, 21, 0, 37, 36, 0, 43, BASE_GRAY, BASE_GRAY, false);
        generateWaterBox(level, box, 21, 1, 37, 36, 14, 43, false);
        generateBox(level, box, 21, 12, 37, 36, 12, 39, BASE_GRAY, BASE_GRAY, false);
        for (int i = 0; i < 4; ++i)
        {
            generateBox(level, box, 15 + i, i + 9, 43 - i, 42 - i, i + 9, 43 - i, BASE_LIGHT, BASE_LIGHT, false);
        }
        for (int x = 21; x <= 36; x += 3)
        {
            placeBlock(level, DOT_DECO_DATA, x, 13, 38, box);
        }
    }
}

StructurePiece* OceanMonumentPieces::MonumentBuilding::Create()
{
    return new MonumentBuilding();
}

EStructurePiece OceanMonumentPieces::MonumentBuilding::GetType()
{
    return eStructurePiece_MonumentBuilding;
}

OceanMonumentPieces::OceanMonumentEntryRoom::OceanMonumentEntryRoom() : OceanMonumentPiece(0)
{
}

OceanMonumentPieces::OceanMonumentEntryRoom::OceanMonumentEntryRoom(
    const Direction* orientation, const boost::shared_ptr<RoomDefinition>& roomDefinition)
    : OceanMonumentPiece(1, orientation, roomDefinition, 1, 1, 1)
{
}

bool OceanMonumentPieces::OceanMonumentEntryRoom::postProcess(Level* level, Random& random, BoundingBox* box)
{
    generateBox(level, box, 0, 3, 0, 2, 3, 7, BASE_LIGHT, BASE_LIGHT, false);
    generateBox(level, box, 5, 3, 0, 7, 3, 7, BASE_LIGHT, BASE_LIGHT, false);
    generateBox(level, box, 0, 2, 0, 1, 2, 7, BASE_LIGHT, BASE_LIGHT, false);
    generateBox(level, box, 6, 2, 0, 7, 2, 7, BASE_LIGHT, BASE_LIGHT, false);
    generateBox(level, box, 0, 1, 0, 0, 1, 7, BASE_LIGHT, BASE_LIGHT, false);
    generateBox(level, box, 7, 1, 0, 7, 1, 7, BASE_LIGHT, BASE_LIGHT, false);
    generateBox(level, box, 0, 1, 7, 7, 3, 7, BASE_LIGHT, BASE_LIGHT, false);
    generateBox(level, box, 1, 1, 0, 2, 3, 0, BASE_LIGHT, BASE_LIGHT, false);
    generateBox(level, box, 5, 1, 0, 6, 3, 0, BASE_LIGHT, BASE_LIGHT, false);
    if (mRoomDefinition->mHasOpening[Direction::NORTH->get3DDataValue()])
    {
        generateWaterBox(level, box, 3, 1, 7, 4, 2, 7, false);
    }
    if (mRoomDefinition->mHasOpening[Direction::WEST->get3DDataValue()])
    {
        generateWaterBox(level, box, 0, 1, 3, 1, 2, 4, false);
    }
    if (mRoomDefinition->mHasOpening[Direction::EAST->get3DDataValue()])
    {
        generateWaterBox(level, box, 6, 1, 3, 7, 2, 4, false);
    }
    return true;
}

StructurePiece* OceanMonumentPieces::OceanMonumentEntryRoom::Create()
{
    return new OceanMonumentEntryRoom();
}

EStructurePiece OceanMonumentPieces::OceanMonumentEntryRoom::GetType()
{
    return eStructurePiece_OceanMonumentEntryRoom;
}

OceanMonumentPieces::OceanMonumentSimpleRoom::OceanMonumentSimpleRoom() : OceanMonumentPiece(0)
{
}

OceanMonumentPieces::OceanMonumentSimpleRoom::OceanMonumentSimpleRoom(
    const Direction* orientation, const boost::shared_ptr<RoomDefinition>& roomDefinition, Random& random)
    : OceanMonumentPiece(1, orientation, roomDefinition, 1, 1, 1)
{
    mMainDesign = random.nextInt(3);
    fjKeepReference(mRoomDefinition->fjConnection(Direction::UP));
}

bool OceanMonumentPieces::OceanMonumentSimpleRoom::postProcess(Level* level, Random& random, BoundingBox* box)
{
    if (mRoomDefinition->mIndex / 25 > 0)
    {
        generateDefaultFloor(level, box, 0, 0, mRoomDefinition->mHasOpening[Direction::DOWN->get3DDataValue()]);
    }
    if (!mRoomDefinition->fjConnection(Direction::UP))
    {
        generateBoxOnFillOnly(level, box, 1, 4, 1, 6, 4, 6, BASE_GRAY);
    }

    bool centrePillar = mMainDesign != 0 && random.nextBoolean()
                        && !mRoomDefinition->mHasOpening[Direction::DOWN->get3DDataValue()]
                        && !mRoomDefinition->mHasOpening[Direction::UP->get3DDataValue()]
                        && mRoomDefinition->countOpenings() > 1;

    if (mMainDesign == 0)
    {
        generateBox(level, box, 0, 1, 0, 2, 1, 2, BASE_LIGHT, BASE_LIGHT, false);
        generateBox(level, box, 0, 3, 0, 2, 3, 2, BASE_LIGHT, BASE_LIGHT, false);
        generateBox(level, box, 0, 2, 0, 0, 2, 2, BASE_GRAY, BASE_GRAY, false);
        generateBox(level, box, 1, 2, 0, 2, 2, 0, BASE_GRAY, BASE_GRAY, false);
        placeBlock(level, LAMP_BLOCK, 1, 2, 1, box);
        generateBox(level, box, 5, 1, 0, 7, 1, 2, BASE_LIGHT, BASE_LIGHT, false);
        generateBox(level, box, 5, 3, 0, 7, 3, 2, BASE_LIGHT, BASE_LIGHT, false);
        generateBox(level, box, 7, 2, 0, 7, 2, 2, BASE_GRAY, BASE_GRAY, false);
        generateBox(level, box, 5, 2, 0, 6, 2, 0, BASE_GRAY, BASE_GRAY, false);
        placeBlock(level, LAMP_BLOCK, 6, 2, 1, box);
        generateBox(level, box, 0, 1, 5, 2, 1, 7, BASE_LIGHT, BASE_LIGHT, false);
        generateBox(level, box, 0, 3, 5, 2, 3, 7, BASE_LIGHT, BASE_LIGHT, false);
        generateBox(level, box, 0, 2, 5, 0, 2, 7, BASE_GRAY, BASE_GRAY, false);
        generateBox(level, box, 1, 2, 7, 2, 2, 7, BASE_GRAY, BASE_GRAY, false);
        placeBlock(level, LAMP_BLOCK, 1, 2, 6, box);
        generateBox(level, box, 5, 1, 5, 7, 1, 7, BASE_LIGHT, BASE_LIGHT, false);
        generateBox(level, box, 5, 3, 5, 7, 3, 7, BASE_LIGHT, BASE_LIGHT, false);
        generateBox(level, box, 7, 2, 5, 7, 2, 7, BASE_GRAY, BASE_GRAY, false);
        generateBox(level, box, 5, 2, 7, 6, 2, 7, BASE_GRAY, BASE_GRAY, false);
        placeBlock(level, LAMP_BLOCK, 6, 2, 6, box);
        if (mRoomDefinition->mHasOpening[Direction::SOUTH->get3DDataValue()])
        {
            generateBox(level, box, 3, 3, 0, 4, 3, 0, BASE_LIGHT, BASE_LIGHT, false);
        }
        else
        {
            generateBox(level, box, 3, 3, 0, 4, 3, 1, BASE_LIGHT, BASE_LIGHT, false);
            generateBox(level, box, 3, 2, 0, 4, 2, 0, BASE_GRAY, BASE_GRAY, false);
            generateBox(level, box, 3, 1, 0, 4, 1, 1, BASE_LIGHT, BASE_LIGHT, false);
        }
        if (mRoomDefinition->mHasOpening[Direction::NORTH->get3DDataValue()])
        {
            generateBox(level, box, 3, 3, 7, 4, 3, 7, BASE_LIGHT, BASE_LIGHT, false);
        }
        else
        {
            generateBox(level, box, 3, 3, 6, 4, 3, 7, BASE_LIGHT, BASE_LIGHT, false);
            generateBox(level, box, 3, 2, 7, 4, 2, 7, BASE_GRAY, BASE_GRAY, false);
            generateBox(level, box, 3, 1, 6, 4, 1, 7, BASE_LIGHT, BASE_LIGHT, false);
        }
        if (mRoomDefinition->mHasOpening[Direction::WEST->get3DDataValue()])
        {
            generateBox(level, box, 0, 3, 3, 0, 3, 4, BASE_LIGHT, BASE_LIGHT, false);
        }
        else
        {
            generateBox(level, box, 0, 3, 3, 1, 3, 4, BASE_LIGHT, BASE_LIGHT, false);
            generateBox(level, box, 0, 2, 3, 0, 2, 4, BASE_GRAY, BASE_GRAY, false);
            generateBox(level, box, 0, 1, 3, 1, 1, 4, BASE_LIGHT, BASE_LIGHT, false);
        }
        if (mRoomDefinition->mHasOpening[Direction::EAST->get3DDataValue()])
        {
            generateBox(level, box, 7, 3, 3, 7, 3, 4, BASE_LIGHT, BASE_LIGHT, false);
        }
        else
        {
            generateBox(level, box, 6, 3, 3, 7, 3, 4, BASE_LIGHT, BASE_LIGHT, false);
            generateBox(level, box, 7, 2, 3, 7, 2, 4, BASE_GRAY, BASE_GRAY, false);
            generateBox(level, box, 6, 1, 3, 7, 1, 4, BASE_LIGHT, BASE_LIGHT, false);
        }
    }
    else if (mMainDesign == 1)
    {
        generateBox(level, box, 2, 1, 2, 2, 3, 2, BASE_LIGHT, BASE_LIGHT, false);
        generateBox(level, box, 2, 1, 5, 2, 3, 5, BASE_LIGHT, BASE_LIGHT, false);
        generateBox(level, box, 5, 1, 5, 5, 3, 5, BASE_LIGHT, BASE_LIGHT, false);
        generateBox(level, box, 5, 1, 2, 5, 3, 2, BASE_LIGHT, BASE_LIGHT, false);
        placeBlock(level, LAMP_BLOCK, 2, 2, 2, box);
        placeBlock(level, LAMP_BLOCK, 2, 2, 5, box);
        placeBlock(level, LAMP_BLOCK, 5, 2, 5, box);
        placeBlock(level, LAMP_BLOCK, 5, 2, 2, box);
        generateBox(level, box, 0, 1, 0, 1, 3, 0, BASE_LIGHT, BASE_LIGHT, false);
        generateBox(level, box, 0, 1, 1, 0, 3, 1, BASE_LIGHT, BASE_LIGHT, false);
        generateBox(level, box, 0, 1, 7, 1, 3, 7, BASE_LIGHT, BASE_LIGHT, false);
        generateBox(level, box, 0, 1, 6, 0, 3, 6, BASE_LIGHT, BASE_LIGHT, false);
        generateBox(level, box, 6, 1, 7, 7, 3, 7, BASE_LIGHT, BASE_LIGHT, false);
        generateBox(level, box, 7, 1, 6, 7, 3, 6, BASE_LIGHT, BASE_LIGHT, false);
        generateBox(level, box, 6, 1, 0, 7, 3, 0, BASE_LIGHT, BASE_LIGHT, false);
        generateBox(level, box, 7, 1, 1, 7, 3, 1, BASE_LIGHT, BASE_LIGHT, false);
        placeBlock(level, BASE_GRAY, 1, 2, 0, box);
        placeBlock(level, BASE_GRAY, 0, 2, 1, box);
        placeBlock(level, BASE_GRAY, 1, 2, 7, box);
        placeBlock(level, BASE_GRAY, 0, 2, 6, box);
        placeBlock(level, BASE_GRAY, 6, 2, 7, box);
        placeBlock(level, BASE_GRAY, 7, 2, 6, box);
        placeBlock(level, BASE_GRAY, 6, 2, 0, box);
        placeBlock(level, BASE_GRAY, 7, 2, 1, box);
        if (!mRoomDefinition->mHasOpening[Direction::SOUTH->get3DDataValue()])
        {
            generateBox(level, box, 1, 3, 0, 6, 3, 0, BASE_LIGHT, BASE_LIGHT, false);
            generateBox(level, box, 1, 2, 0, 6, 2, 0, BASE_GRAY, BASE_GRAY, false);
            generateBox(level, box, 1, 1, 0, 6, 1, 0, BASE_LIGHT, BASE_LIGHT, false);
        }
        if (!mRoomDefinition->mHasOpening[Direction::NORTH->get3DDataValue()])
        {
            generateBox(level, box, 1, 3, 7, 6, 3, 7, BASE_LIGHT, BASE_LIGHT, false);
            generateBox(level, box, 1, 2, 7, 6, 2, 7, BASE_GRAY, BASE_GRAY, false);
            generateBox(level, box, 1, 1, 7, 6, 1, 7, BASE_LIGHT, BASE_LIGHT, false);
        }
        if (!mRoomDefinition->mHasOpening[Direction::WEST->get3DDataValue()])
        {
            generateBox(level, box, 0, 3, 1, 0, 3, 6, BASE_LIGHT, BASE_LIGHT, false);
            generateBox(level, box, 0, 2, 1, 0, 2, 6, BASE_GRAY, BASE_GRAY, false);
            generateBox(level, box, 0, 1, 1, 0, 1, 6, BASE_LIGHT, BASE_LIGHT, false);
        }
        if (!mRoomDefinition->mHasOpening[Direction::EAST->get3DDataValue()])
        {
            generateBox(level, box, 7, 3, 1, 7, 3, 6, BASE_LIGHT, BASE_LIGHT, false);
            generateBox(level, box, 7, 2, 1, 7, 2, 6, BASE_GRAY, BASE_GRAY, false);
            generateBox(level, box, 7, 1, 1, 7, 1, 6, BASE_LIGHT, BASE_LIGHT, false);
        }
    }
    else if (mMainDesign == 2)
    {
        generateBox(level, box, 0, 1, 0, 0, 1, 7, BASE_LIGHT, BASE_LIGHT, false);
        generateBox(level, box, 7, 1, 0, 7, 1, 7, BASE_LIGHT, BASE_LIGHT, false);
        generateBox(level, box, 1, 1, 0, 6, 1, 0, BASE_LIGHT, BASE_LIGHT, false);
        generateBox(level, box, 1, 1, 7, 6, 1, 7, BASE_LIGHT, BASE_LIGHT, false);
        generateBox(level, box, 0, 2, 0, 0, 2, 7, BASE_BLACK, BASE_BLACK, false);
        generateBox(level, box, 7, 2, 0, 7, 2, 7, BASE_BLACK, BASE_BLACK, false);
        generateBox(level, box, 1, 2, 0, 6, 2, 0, BASE_BLACK, BASE_BLACK, false);
        generateBox(level, box, 1, 2, 7, 6, 2, 7, BASE_BLACK, BASE_BLACK, false);
        generateBox(level, box, 0, 3, 0, 0, 3, 7, BASE_LIGHT, BASE_LIGHT, false);
        generateBox(level, box, 7, 3, 0, 7, 3, 7, BASE_LIGHT, BASE_LIGHT, false);
        generateBox(level, box, 1, 3, 0, 6, 3, 0, BASE_LIGHT, BASE_LIGHT, false);
        generateBox(level, box, 1, 3, 7, 6, 3, 7, BASE_LIGHT, BASE_LIGHT, false);
        generateBox(level, box, 0, 1, 3, 0, 2, 4, BASE_BLACK, BASE_BLACK, false);
        generateBox(level, box, 7, 1, 3, 7, 2, 4, BASE_BLACK, BASE_BLACK, false);
        generateBox(level, box, 3, 1, 0, 4, 2, 0, BASE_BLACK, BASE_BLACK, false);
        generateBox(level, box, 3, 1, 7, 4, 2, 7, BASE_BLACK, BASE_BLACK, false);
        if (mRoomDefinition->mHasOpening[Direction::SOUTH->get3DDataValue()])
        {
            generateWaterBox(level, box, 3, 1, 0, 4, 2, 0, false);
        }
        if (mRoomDefinition->mHasOpening[Direction::NORTH->get3DDataValue()])
        {
            generateWaterBox(level, box, 3, 1, 7, 4, 2, 7, false);
        }
        if (mRoomDefinition->mHasOpening[Direction::WEST->get3DDataValue()])
        {
            generateWaterBox(level, box, 0, 1, 3, 0, 2, 4, false);
        }
        if (mRoomDefinition->mHasOpening[Direction::EAST->get3DDataValue()])
        {
            generateWaterBox(level, box, 7, 1, 3, 7, 2, 4, false);
        }
    }

    if (centrePillar)
    {
        generateBox(level, box, 3, 1, 3, 4, 1, 4, BASE_LIGHT, BASE_LIGHT, false);
        generateBox(level, box, 3, 2, 3, 4, 2, 4, BASE_GRAY, BASE_GRAY, false);
        generateBox(level, box, 3, 3, 3, 4, 3, 4, BASE_LIGHT, BASE_LIGHT, false);
    }
    return true;
}

StructurePiece* OceanMonumentPieces::OceanMonumentSimpleRoom::Create()
{
    return new OceanMonumentSimpleRoom();
}

EStructurePiece OceanMonumentPieces::OceanMonumentSimpleRoom::GetType()
{
    return eStructurePiece_OceanMonumentSimpleRoom;
}

OceanMonumentPieces::OceanMonumentSimpleTopRoom::OceanMonumentSimpleTopRoom() : OceanMonumentPiece(0)
{
}

OceanMonumentPieces::OceanMonumentSimpleTopRoom::OceanMonumentSimpleTopRoom(
    const Direction* orientation, const boost::shared_ptr<RoomDefinition>& roomDefinition, Random& random)
    : OceanMonumentPiece(1, orientation, roomDefinition, 1, 1, 1)
{
    fjKeepReference(mRoomDefinition->fjConnection(Direction::UP));
}

bool OceanMonumentPieces::OceanMonumentSimpleTopRoom::postProcess(Level* level, Random& random, BoundingBox* box)
{
    if (mRoomDefinition->mIndex / 25 > 0)
    {
        generateDefaultFloor(level, box, 0, 0, mRoomDefinition->mHasOpening[Direction::DOWN->get3DDataValue()]);
    }
    if (!mRoomDefinition->fjConnection(Direction::UP))
    {
        generateBoxOnFillOnly(level, box, 1, 4, 1, 6, 4, 6, BASE_GRAY);
    }
    for (int x = 1; x <= 6; ++x)
    {
        for (int z = 1; z <= 6; ++z)
        {
            if (random.nextInt(3) != 0)
            {
                int y = 2 + (random.nextInt(4) == 0 ? 0 : 1);
                generateBox(level, box, x, y, z, x, 3, z, Blocks::sponge->getBlockState(1),
                            Blocks::sponge->getBlockState(1), false);
            }
        }
    }
    generateBox(level, box, 0, 1, 0, 0, 1, 7, BASE_LIGHT, BASE_LIGHT, false);
    generateBox(level, box, 7, 1, 0, 7, 1, 7, BASE_LIGHT, BASE_LIGHT, false);
    generateBox(level, box, 1, 1, 0, 6, 1, 0, BASE_LIGHT, BASE_LIGHT, false);
    generateBox(level, box, 1, 1, 7, 6, 1, 7, BASE_LIGHT, BASE_LIGHT, false);
    generateBox(level, box, 0, 2, 0, 0, 2, 7, BASE_BLACK, BASE_BLACK, false);
    generateBox(level, box, 7, 2, 0, 7, 2, 7, BASE_BLACK, BASE_BLACK, false);
    generateBox(level, box, 1, 2, 0, 6, 2, 0, BASE_BLACK, BASE_BLACK, false);
    generateBox(level, box, 1, 2, 7, 6, 2, 7, BASE_BLACK, BASE_BLACK, false);
    generateBox(level, box, 0, 3, 0, 0, 3, 7, BASE_LIGHT, BASE_LIGHT, false);
    generateBox(level, box, 7, 3, 0, 7, 3, 7, BASE_LIGHT, BASE_LIGHT, false);
    generateBox(level, box, 1, 3, 0, 6, 3, 0, BASE_LIGHT, BASE_LIGHT, false);
    generateBox(level, box, 1, 3, 7, 6, 3, 7, BASE_LIGHT, BASE_LIGHT, false);
    generateBox(level, box, 0, 1, 3, 0, 2, 4, BASE_BLACK, BASE_BLACK, false);
    generateBox(level, box, 7, 1, 3, 7, 2, 4, BASE_BLACK, BASE_BLACK, false);
    generateBox(level, box, 3, 1, 0, 4, 2, 0, BASE_BLACK, BASE_BLACK, false);
    generateBox(level, box, 3, 1, 7, 4, 2, 7, BASE_BLACK, BASE_BLACK, false);
    if (mRoomDefinition->mHasOpening[Direction::SOUTH->get3DDataValue()])
    {
        generateWaterBox(level, box, 3, 1, 0, 4, 2, 0, false);
    }
    return true;
}

StructurePiece* OceanMonumentPieces::OceanMonumentSimpleTopRoom::Create()
{
    return new OceanMonumentSimpleTopRoom();
}

EStructurePiece OceanMonumentPieces::OceanMonumentSimpleTopRoom::GetType()
{
    return eStructurePiece_OceanMonumentSimpleTopRoom;
}

OceanMonumentPieces::OceanMonumentDoubleYRoom::OceanMonumentDoubleYRoom() : OceanMonumentPiece(0)
{
}

OceanMonumentPieces::OceanMonumentDoubleYRoom::OceanMonumentDoubleYRoom(
    const Direction* orientation, const boost::shared_ptr<RoomDefinition>& roomDefinition, Random& random)
    : OceanMonumentPiece(1, orientation, roomDefinition, 1, 2, 1)
{
    fjKeepReference(mRoomDefinition->fjConnection(Direction::UP));
    fjKeepReference(mRoomDefinition->fjConnection(Direction::UP)->fjConnection(Direction::UP));
}

bool OceanMonumentPieces::OceanMonumentDoubleYRoom::postProcess(Level* level, Random& random, BoundingBox* box)
{
    if (mRoomDefinition->mIndex / 25 > 0)
    {
        generateDefaultFloor(level, box, 0, 0, mRoomDefinition->mHasOpening[Direction::DOWN->get3DDataValue()]);
    }
    boost::shared_ptr<RoomDefinition> upperRoom = mRoomDefinition->fjConnection(Direction::UP);
    if (!upperRoom->fjConnection(Direction::UP))
    {
        generateBoxOnFillOnly(level, box, 1, 8, 1, 6, 8, 6, BASE_GRAY);
    }
    generateBox(level, box, 0, 4, 0, 0, 4, 7, BASE_LIGHT, BASE_LIGHT, false);
    generateBox(level, box, 7, 4, 0, 7, 4, 7, BASE_LIGHT, BASE_LIGHT, false);
    generateBox(level, box, 1, 4, 0, 6, 4, 0, BASE_LIGHT, BASE_LIGHT, false);
    generateBox(level, box, 1, 4, 7, 6, 4, 7, BASE_LIGHT, BASE_LIGHT, false);
    generateBox(level, box, 2, 4, 1, 2, 4, 2, BASE_LIGHT, BASE_LIGHT, false);
    generateBox(level, box, 1, 4, 2, 1, 4, 2, BASE_LIGHT, BASE_LIGHT, false);
    generateBox(level, box, 5, 4, 1, 5, 4, 2, BASE_LIGHT, BASE_LIGHT, false);
    generateBox(level, box, 6, 4, 2, 6, 4, 2, BASE_LIGHT, BASE_LIGHT, false);
    generateBox(level, box, 2, 4, 5, 2, 4, 6, BASE_LIGHT, BASE_LIGHT, false);
    generateBox(level, box, 1, 4, 5, 1, 4, 5, BASE_LIGHT, BASE_LIGHT, false);
    generateBox(level, box, 5, 4, 5, 5, 4, 6, BASE_LIGHT, BASE_LIGHT, false);
    generateBox(level, box, 6, 4, 5, 6, 4, 5, BASE_LIGHT, BASE_LIGHT, false);

    boost::shared_ptr<RoomDefinition> room = mRoomDefinition;
    for (int y = 1; y <= 5; y += 4)
    {
        if (room->mHasOpening[Direction::SOUTH->get3DDataValue()])
        {
            generateBox(level, box, 2, y, 0, 2, y + 2, 0, BASE_LIGHT, BASE_LIGHT, false);
            generateBox(level, box, 5, y, 0, 5, y + 2, 0, BASE_LIGHT, BASE_LIGHT, false);
            generateBox(level, box, 3, y + 2, 0, 4, y + 2, 0, BASE_LIGHT, BASE_LIGHT, false);
        }
        else
        {
            generateBox(level, box, 0, y, 0, 7, y + 2, 0, BASE_LIGHT, BASE_LIGHT, false);
            generateBox(level, box, 0, y + 1, 0, 7, y + 1, 0, BASE_GRAY, BASE_GRAY, false);
        }
        if (room->mHasOpening[Direction::NORTH->get3DDataValue()])
        {
            generateBox(level, box, 2, y, 7, 2, y + 2, 7, BASE_LIGHT, BASE_LIGHT, false);
            generateBox(level, box, 5, y, 7, 5, y + 2, 7, BASE_LIGHT, BASE_LIGHT, false);
            generateBox(level, box, 3, y + 2, 7, 4, y + 2, 7, BASE_LIGHT, BASE_LIGHT, false);
        }
        else
        {
            generateBox(level, box, 0, y, 7, 7, y + 2, 7, BASE_LIGHT, BASE_LIGHT, false);
            generateBox(level, box, 0, y + 1, 7, 7, y + 1, 7, BASE_GRAY, BASE_GRAY, false);
        }
        if (room->mHasOpening[Direction::WEST->get3DDataValue()])
        {
            generateBox(level, box, 0, y, 2, 0, y + 2, 2, BASE_LIGHT, BASE_LIGHT, false);
            generateBox(level, box, 0, y, 5, 0, y + 2, 5, BASE_LIGHT, BASE_LIGHT, false);
            generateBox(level, box, 0, y + 2, 3, 0, y + 2, 4, BASE_LIGHT, BASE_LIGHT, false);
        }
        else
        {
            generateBox(level, box, 0, y, 0, 0, y + 2, 7, BASE_LIGHT, BASE_LIGHT, false);
            generateBox(level, box, 0, y + 1, 0, 0, y + 1, 7, BASE_GRAY, BASE_GRAY, false);
        }
        if (room->mHasOpening[Direction::EAST->get3DDataValue()])
        {
            generateBox(level, box, 7, y, 2, 7, y + 2, 2, BASE_LIGHT, BASE_LIGHT, false);
            generateBox(level, box, 7, y, 5, 7, y + 2, 5, BASE_LIGHT, BASE_LIGHT, false);
            generateBox(level, box, 7, y + 2, 3, 7, y + 2, 4, BASE_LIGHT, BASE_LIGHT, false);
        }
        else
        {
            generateBox(level, box, 7, y, 0, 7, y + 2, 7, BASE_LIGHT, BASE_LIGHT, false);
            generateBox(level, box, 7, y + 1, 0, 7, y + 1, 7, BASE_GRAY, BASE_GRAY, false);
        }
        room = upperRoom;
    }
    return true;
}

StructurePiece* OceanMonumentPieces::OceanMonumentDoubleYRoom::Create()
{
    return new OceanMonumentDoubleYRoom();
}

EStructurePiece OceanMonumentPieces::OceanMonumentDoubleYRoom::GetType()
{
    return eStructurePiece_OceanMonumentDoubleYRoom;
}

OceanMonumentPieces::OceanMonumentDoubleXRoom::OceanMonumentDoubleXRoom() : OceanMonumentPiece(0)
{
}

OceanMonumentPieces::OceanMonumentDoubleXRoom::OceanMonumentDoubleXRoom(
    const Direction* orientation, const boost::shared_ptr<RoomDefinition>& roomDefinition, Random& random)
    : OceanMonumentPiece(1, orientation, roomDefinition, 2, 1, 1)
{
    fjKeepReference(mRoomDefinition->fjConnection(Direction::UP));
    fjKeepReference(mRoomDefinition->fjConnection(Direction::EAST));
    fjKeepReference(mRoomDefinition->fjConnection(Direction::EAST)->fjConnection(Direction::UP));
}

bool OceanMonumentPieces::OceanMonumentDoubleXRoom::postProcess(Level* level, Random& random, BoundingBox* box)
{
    boost::shared_ptr<RoomDefinition> eastRoom = mRoomDefinition->fjConnection(Direction::EAST);
    boost::shared_ptr<RoomDefinition> westRoom = mRoomDefinition;
    if (mRoomDefinition->mIndex / 25 > 0)
    {
        generateDefaultFloor(level, box, 8, 0, eastRoom->mHasOpening[Direction::DOWN->get3DDataValue()]);
        generateDefaultFloor(level, box, 0, 0, westRoom->mHasOpening[Direction::DOWN->get3DDataValue()]);
    }
    if (!westRoom->fjConnection(Direction::UP))
    {
        generateBoxOnFillOnly(level, box, 1, 4, 1, 7, 4, 6, BASE_GRAY);
    }
    if (!eastRoom->fjConnection(Direction::UP))
    {
        generateBoxOnFillOnly(level, box, 8, 4, 1, 14, 4, 6, BASE_GRAY);
    }
    generateBox(level, box, 0, 3, 0, 0, 3, 7, BASE_LIGHT, BASE_LIGHT, false);
    generateBox(level, box, 15, 3, 0, 15, 3, 7, BASE_LIGHT, BASE_LIGHT, false);
    generateBox(level, box, 1, 3, 0, 15, 3, 0, BASE_LIGHT, BASE_LIGHT, false);
    generateBox(level, box, 1, 3, 7, 14, 3, 7, BASE_LIGHT, BASE_LIGHT, false);
    generateBox(level, box, 0, 2, 0, 0, 2, 7, BASE_GRAY, BASE_GRAY, false);
    generateBox(level, box, 15, 2, 0, 15, 2, 7, BASE_GRAY, BASE_GRAY, false);
    generateBox(level, box, 1, 2, 0, 15, 2, 0, BASE_GRAY, BASE_GRAY, false);
    generateBox(level, box, 1, 2, 7, 14, 2, 7, BASE_GRAY, BASE_GRAY, false);
    generateBox(level, box, 0, 1, 0, 0, 1, 7, BASE_LIGHT, BASE_LIGHT, false);
    generateBox(level, box, 15, 1, 0, 15, 1, 7, BASE_LIGHT, BASE_LIGHT, false);
    generateBox(level, box, 1, 1, 0, 15, 1, 0, BASE_LIGHT, BASE_LIGHT, false);
    generateBox(level, box, 1, 1, 7, 14, 1, 7, BASE_LIGHT, BASE_LIGHT, false);
    generateBox(level, box, 5, 1, 0, 10, 1, 4, BASE_LIGHT, BASE_LIGHT, false);
    generateBox(level, box, 6, 2, 0, 9, 2, 3, BASE_GRAY, BASE_GRAY, false);
    generateBox(level, box, 5, 3, 0, 10, 3, 4, BASE_LIGHT, BASE_LIGHT, false);
    placeBlock(level, LAMP_BLOCK, 6, 2, 3, box);
    placeBlock(level, LAMP_BLOCK, 9, 2, 3, box);
    if (westRoom->mHasOpening[Direction::SOUTH->get3DDataValue()])
    {
        generateWaterBox(level, box, 3, 1, 0, 4, 2, 0, false);
    }
    if (westRoom->mHasOpening[Direction::NORTH->get3DDataValue()])
    {
        generateWaterBox(level, box, 3, 1, 7, 4, 2, 7, false);
    }
    if (westRoom->mHasOpening[Direction::WEST->get3DDataValue()])
    {
        generateWaterBox(level, box, 0, 1, 3, 0, 2, 4, false);
    }
    if (eastRoom->mHasOpening[Direction::SOUTH->get3DDataValue()])
    {
        generateWaterBox(level, box, 11, 1, 0, 12, 2, 0, false);
    }
    if (eastRoom->mHasOpening[Direction::NORTH->get3DDataValue()])
    {
        generateWaterBox(level, box, 11, 1, 7, 12, 2, 7, false);
    }
    if (eastRoom->mHasOpening[Direction::EAST->get3DDataValue()])
    {
        generateWaterBox(level, box, 15, 1, 3, 15, 2, 4, false);
    }
    return true;
}

StructurePiece* OceanMonumentPieces::OceanMonumentDoubleXRoom::Create()
{
    return new OceanMonumentDoubleXRoom();
}

EStructurePiece OceanMonumentPieces::OceanMonumentDoubleXRoom::GetType()
{
    return eStructurePiece_OceanMonumentDoubleXRoom;
}

OceanMonumentPieces::OceanMonumentDoubleXYRoom::OceanMonumentDoubleXYRoom() : OceanMonumentPiece(0)
{
}

OceanMonumentPieces::OceanMonumentDoubleXYRoom::OceanMonumentDoubleXYRoom(
    const Direction* orientation, const boost::shared_ptr<RoomDefinition>& roomDefinition, Random& random)
    : OceanMonumentPiece(1, orientation, roomDefinition, 2, 2, 1)
{
    fjKeepReference(mRoomDefinition->fjConnection(Direction::UP));
    fjKeepReference(mRoomDefinition->fjConnection(Direction::EAST));
    fjKeepReference(mRoomDefinition->fjConnection(Direction::EAST)->fjConnection(Direction::UP));
}

bool OceanMonumentPieces::OceanMonumentDoubleXYRoom::postProcess(Level* level, Random& random, BoundingBox* box)
{
    boost::shared_ptr<RoomDefinition> eastRoom = mRoomDefinition->fjConnection(Direction::EAST);
    boost::shared_ptr<RoomDefinition> westUpperRoom = mRoomDefinition->fjConnection(Direction::UP);
    boost::shared_ptr<RoomDefinition> eastUpperRoom = eastRoom->fjConnection(Direction::UP);
    if (mRoomDefinition->mIndex / 25 > 0)
    {
        generateDefaultFloor(level, box, 8, 0, eastRoom->mHasOpening[Direction::DOWN->get3DDataValue()]);
        generateDefaultFloor(level, box, 0, 0, mRoomDefinition->mHasOpening[Direction::DOWN->get3DDataValue()]);
    }
    if (!westUpperRoom->fjConnection(Direction::UP))
    {
        generateBoxOnFillOnly(level, box, 1, 8, 1, 7, 8, 6, BASE_GRAY);
    }
    if (!eastUpperRoom->fjConnection(Direction::UP))
    {
        generateBoxOnFillOnly(level, box, 8, 8, 1, 14, 8, 6, BASE_GRAY);
    }
    for (int y = 1; y <= 7; ++y)
    {
        const BlockState* state = BASE_LIGHT;
        if (y == 2 || y == 6)
        {
            state = BASE_GRAY;
        }
        generateBox(level, box, 0, y, 0, 0, y, 7, state, state, false);
        generateBox(level, box, 15, y, 0, 15, y, 7, state, state, false);
        generateBox(level, box, 1, y, 0, 15, y, 0, state, state, false);
        generateBox(level, box, 1, y, 7, 14, y, 7, state, state, false);
    }
    generateBox(level, box, 2, 1, 3, 2, 7, 4, BASE_LIGHT, BASE_LIGHT, false);
    generateBox(level, box, 3, 1, 2, 4, 7, 2, BASE_LIGHT, BASE_LIGHT, false);
    generateBox(level, box, 3, 1, 5, 4, 7, 5, BASE_LIGHT, BASE_LIGHT, false);
    generateBox(level, box, 13, 1, 3, 13, 7, 4, BASE_LIGHT, BASE_LIGHT, false);
    generateBox(level, box, 11, 1, 2, 12, 7, 2, BASE_LIGHT, BASE_LIGHT, false);
    generateBox(level, box, 11, 1, 5, 12, 7, 5, BASE_LIGHT, BASE_LIGHT, false);
    generateBox(level, box, 5, 1, 3, 5, 3, 4, BASE_LIGHT, BASE_LIGHT, false);
    generateBox(level, box, 10, 1, 3, 10, 3, 4, BASE_LIGHT, BASE_LIGHT, false);
    generateBox(level, box, 5, 7, 2, 10, 7, 5, BASE_LIGHT, BASE_LIGHT, false);
    generateBox(level, box, 5, 5, 2, 5, 7, 2, BASE_LIGHT, BASE_LIGHT, false);
    generateBox(level, box, 10, 5, 2, 10, 7, 2, BASE_LIGHT, BASE_LIGHT, false);
    generateBox(level, box, 5, 5, 5, 5, 7, 5, BASE_LIGHT, BASE_LIGHT, false);
    generateBox(level, box, 10, 5, 5, 10, 7, 5, BASE_LIGHT, BASE_LIGHT, false);
    placeBlock(level, BASE_LIGHT, 6, 6, 2, box);
    placeBlock(level, BASE_LIGHT, 9, 6, 2, box);
    placeBlock(level, BASE_LIGHT, 6, 6, 5, box);
    placeBlock(level, BASE_LIGHT, 9, 6, 5, box);
    generateBox(level, box, 5, 4, 3, 6, 4, 4, BASE_LIGHT, BASE_LIGHT, false);
    generateBox(level, box, 9, 4, 3, 10, 4, 4, BASE_LIGHT, BASE_LIGHT, false);
    placeBlock(level, LAMP_BLOCK, 5, 4, 2, box);
    placeBlock(level, LAMP_BLOCK, 5, 4, 5, box);
    placeBlock(level, LAMP_BLOCK, 10, 4, 2, box);
    placeBlock(level, LAMP_BLOCK, 10, 4, 5, box);
    if (mRoomDefinition->mHasOpening[Direction::SOUTH->get3DDataValue()])
    {
        generateWaterBox(level, box, 3, 1, 0, 4, 2, 0, false);
    }
    if (mRoomDefinition->mHasOpening[Direction::NORTH->get3DDataValue()])
    {
        generateWaterBox(level, box, 3, 1, 7, 4, 2, 7, false);
    }
    if (mRoomDefinition->mHasOpening[Direction::WEST->get3DDataValue()])
    {
        generateWaterBox(level, box, 0, 1, 3, 0, 2, 4, false);
    }
    if (eastRoom->mHasOpening[Direction::SOUTH->get3DDataValue()])
    {
        generateWaterBox(level, box, 11, 1, 0, 12, 2, 0, false);
    }
    if (eastRoom->mHasOpening[Direction::NORTH->get3DDataValue()])
    {
        generateWaterBox(level, box, 11, 1, 7, 12, 2, 7, false);
    }
    if (eastRoom->mHasOpening[Direction::EAST->get3DDataValue()])
    {
        generateWaterBox(level, box, 15, 1, 3, 15, 2, 4, false);
    }
    if (westUpperRoom->mHasOpening[Direction::SOUTH->get3DDataValue()])
    {
        generateWaterBox(level, box, 3, 5, 0, 4, 6, 0, false);
    }
    if (westUpperRoom->mHasOpening[Direction::NORTH->get3DDataValue()])
    {
        generateWaterBox(level, box, 3, 5, 7, 4, 6, 7, false);
    }
    if (westUpperRoom->mHasOpening[Direction::WEST->get3DDataValue()])
    {
        generateWaterBox(level, box, 0, 5, 3, 0, 6, 4, false);
    }
    if (eastUpperRoom->mHasOpening[Direction::SOUTH->get3DDataValue()])
    {
        generateWaterBox(level, box, 11, 5, 0, 12, 6, 0, false);
    }
    if (eastUpperRoom->mHasOpening[Direction::NORTH->get3DDataValue()])
    {
        generateWaterBox(level, box, 11, 5, 7, 12, 6, 7, false);
    }
    if (eastUpperRoom->mHasOpening[Direction::EAST->get3DDataValue()])
    {
        generateWaterBox(level, box, 15, 5, 3, 15, 6, 4, false);
    }
    return true;
}

StructurePiece* OceanMonumentPieces::OceanMonumentDoubleXYRoom::Create()
{
    return new OceanMonumentDoubleXYRoom();
}

EStructurePiece OceanMonumentPieces::OceanMonumentDoubleXYRoom::GetType()
{
    return eStructurePiece_OceanMonumentDoubleXYRoom;
}

OceanMonumentPieces::OceanMonumentDoubleYZRoom::OceanMonumentDoubleYZRoom() : OceanMonumentPiece(0)
{
}

OceanMonumentPieces::OceanMonumentDoubleYZRoom::OceanMonumentDoubleYZRoom(
    const Direction* orientation, const boost::shared_ptr<RoomDefinition>& roomDefinition, Random& random)
    : OceanMonumentPiece(1, orientation, roomDefinition, 1, 2, 2)
{
    fjKeepReference(mRoomDefinition->fjConnection(Direction::UP));
    fjKeepReference(mRoomDefinition->fjConnection(Direction::UP)->fjConnection(Direction::UP));
    fjKeepReference(mRoomDefinition->fjConnection(Direction::NORTH));
    fjKeepReference(mRoomDefinition->fjConnection(Direction::NORTH)->fjConnection(Direction::UP));
    fjKeepReference(
        mRoomDefinition->fjConnection(Direction::NORTH)->fjConnection(Direction::UP)->fjConnection(Direction::UP));
}

bool OceanMonumentPieces::OceanMonumentDoubleYZRoom::postProcess(Level* level, Random& random, BoundingBox* box)
{
    boost::shared_ptr<RoomDefinition> northRoom = mRoomDefinition->fjConnection(Direction::NORTH);
    boost::shared_ptr<RoomDefinition> southRoom = mRoomDefinition;
    boost::shared_ptr<RoomDefinition> northUpperRoom = northRoom->fjConnection(Direction::UP);
    boost::shared_ptr<RoomDefinition> southUpperRoom = southRoom->fjConnection(Direction::UP);
    if (mRoomDefinition->mIndex / 25 > 0)
    {
        generateDefaultFloor(level, box, 0, 8, northRoom->mHasOpening[Direction::DOWN->get3DDataValue()]);
        generateDefaultFloor(level, box, 0, 0, southRoom->mHasOpening[Direction::DOWN->get3DDataValue()]);
    }
    if (!southUpperRoom->fjConnection(Direction::UP))
    {
        generateBoxOnFillOnly(level, box, 1, 8, 1, 6, 8, 7, BASE_GRAY);
    }
    if (!northUpperRoom->fjConnection(Direction::UP))
    {
        generateBoxOnFillOnly(level, box, 1, 8, 8, 6, 8, 14, BASE_GRAY);
    }
    for (int y = 1; y <= 7; ++y)
    {
        const BlockState* state = BASE_LIGHT;
        if (y == 2 || y == 6)
        {
            state = BASE_GRAY;
        }
        generateBox(level, box, 0, y, 0, 0, y, 15, state, state, false);
        generateBox(level, box, 7, y, 0, 7, y, 15, state, state, false);
        generateBox(level, box, 1, y, 0, 6, y, 0, state, state, false);
        generateBox(level, box, 1, y, 15, 6, y, 15, state, state, false);
    }
    for (int y = 1; y <= 7; ++y)
    {
        const BlockState* state = BASE_BLACK;
        if (y == 2 || y == 6)
        {
            state = LAMP_BLOCK;
        }
        generateBox(level, box, 3, y, 7, 4, y, 8, state, state, false);
    }
    if (southRoom->mHasOpening[Direction::SOUTH->get3DDataValue()])
    {
        generateWaterBox(level, box, 3, 1, 0, 4, 2, 0, false);
    }
    if (southRoom->mHasOpening[Direction::EAST->get3DDataValue()])
    {
        generateWaterBox(level, box, 7, 1, 3, 7, 2, 4, false);
    }
    if (southRoom->mHasOpening[Direction::WEST->get3DDataValue()])
    {
        generateWaterBox(level, box, 0, 1, 3, 0, 2, 4, false);
    }
    if (northRoom->mHasOpening[Direction::NORTH->get3DDataValue()])
    {
        generateWaterBox(level, box, 3, 1, 15, 4, 2, 15, false);
    }
    if (northRoom->mHasOpening[Direction::WEST->get3DDataValue()])
    {
        generateWaterBox(level, box, 0, 1, 11, 0, 2, 12, false);
    }
    if (northRoom->mHasOpening[Direction::EAST->get3DDataValue()])
    {
        generateWaterBox(level, box, 7, 1, 11, 7, 2, 12, false);
    }
    if (southUpperRoom->mHasOpening[Direction::SOUTH->get3DDataValue()])
    {
        generateWaterBox(level, box, 3, 5, 0, 4, 6, 0, false);
    }
    if (southUpperRoom->mHasOpening[Direction::EAST->get3DDataValue()])
    {
        generateWaterBox(level, box, 7, 5, 3, 7, 6, 4, false);
        generateBox(level, box, 5, 4, 2, 6, 4, 5, BASE_LIGHT, BASE_LIGHT, false);
        generateBox(level, box, 6, 1, 2, 6, 3, 2, BASE_LIGHT, BASE_LIGHT, false);
        generateBox(level, box, 6, 1, 5, 6, 3, 5, BASE_LIGHT, BASE_LIGHT, false);
    }
    if (southUpperRoom->mHasOpening[Direction::WEST->get3DDataValue()])
    {
        generateWaterBox(level, box, 0, 5, 3, 0, 6, 4, false);
        generateBox(level, box, 1, 4, 2, 2, 4, 5, BASE_LIGHT, BASE_LIGHT, false);
        generateBox(level, box, 1, 1, 2, 1, 3, 2, BASE_LIGHT, BASE_LIGHT, false);
        generateBox(level, box, 1, 1, 5, 1, 3, 5, BASE_LIGHT, BASE_LIGHT, false);
    }
    if (northUpperRoom->mHasOpening[Direction::NORTH->get3DDataValue()])
    {
        generateWaterBox(level, box, 3, 5, 15, 4, 6, 15, false);
    }
    if (northUpperRoom->mHasOpening[Direction::WEST->get3DDataValue()])
    {
        generateWaterBox(level, box, 0, 5, 11, 0, 6, 12, false);
        generateBox(level, box, 1, 4, 10, 2, 4, 13, BASE_LIGHT, BASE_LIGHT, false);
        generateBox(level, box, 1, 1, 10, 1, 3, 10, BASE_LIGHT, BASE_LIGHT, false);
        generateBox(level, box, 1, 1, 13, 1, 3, 13, BASE_LIGHT, BASE_LIGHT, false);
    }
    if (northUpperRoom->mHasOpening[Direction::EAST->get3DDataValue()])
    {
        generateWaterBox(level, box, 7, 5, 11, 7, 6, 12, false);
        generateBox(level, box, 5, 4, 10, 6, 4, 13, BASE_LIGHT, BASE_LIGHT, false);
        generateBox(level, box, 6, 1, 10, 6, 3, 10, BASE_LIGHT, BASE_LIGHT, false);
        generateBox(level, box, 6, 1, 13, 6, 3, 13, BASE_LIGHT, BASE_LIGHT, false);
    }
    return true;
}

StructurePiece* OceanMonumentPieces::OceanMonumentDoubleYZRoom::Create()
{
    return new OceanMonumentDoubleYZRoom();
}

EStructurePiece OceanMonumentPieces::OceanMonumentDoubleYZRoom::GetType()
{
    return eStructurePiece_OceanMonumentDoubleYZRoom;
}

OceanMonumentPieces::OceanMonumentDoubleZRoom::OceanMonumentDoubleZRoom() : OceanMonumentPiece(0)
{
}

OceanMonumentPieces::OceanMonumentDoubleZRoom::OceanMonumentDoubleZRoom(
    const Direction* orientation, const boost::shared_ptr<RoomDefinition>& roomDefinition, Random& random)
    : OceanMonumentPiece(1, orientation, roomDefinition, 1, 1, 2)
{
    fjKeepReference(mRoomDefinition->fjConnection(Direction::UP));
    fjKeepReference(mRoomDefinition->fjConnection(Direction::NORTH));
    fjKeepReference(mRoomDefinition->fjConnection(Direction::NORTH)->fjConnection(Direction::UP));
}

bool OceanMonumentPieces::OceanMonumentDoubleZRoom::postProcess(Level* level, Random& random, BoundingBox* box)
{
    boost::shared_ptr<RoomDefinition> northRoom = mRoomDefinition->fjConnection(Direction::NORTH);
    boost::shared_ptr<RoomDefinition> southRoom = mRoomDefinition;
    if (mRoomDefinition->mIndex / 25 > 0)
    {
        generateDefaultFloor(level, box, 0, 8, northRoom->mHasOpening[Direction::DOWN->get3DDataValue()]);
        generateDefaultFloor(level, box, 0, 0, southRoom->mHasOpening[Direction::DOWN->get3DDataValue()]);
    }
    if (!southRoom->fjConnection(Direction::UP))
    {
        generateBoxOnFillOnly(level, box, 1, 4, 1, 6, 4, 7, BASE_GRAY);
    }
    if (!northRoom->fjConnection(Direction::UP))
    {
        generateBoxOnFillOnly(level, box, 1, 4, 8, 6, 4, 14, BASE_GRAY);
    }
    generateBox(level, box, 0, 3, 0, 0, 3, 15, BASE_LIGHT, BASE_LIGHT, false);
    generateBox(level, box, 7, 3, 0, 7, 3, 15, BASE_LIGHT, BASE_LIGHT, false);
    generateBox(level, box, 1, 3, 0, 7, 3, 0, BASE_LIGHT, BASE_LIGHT, false);
    generateBox(level, box, 1, 3, 15, 6, 3, 15, BASE_LIGHT, BASE_LIGHT, false);
    generateBox(level, box, 0, 2, 0, 0, 2, 15, BASE_GRAY, BASE_GRAY, false);
    generateBox(level, box, 7, 2, 0, 7, 2, 15, BASE_GRAY, BASE_GRAY, false);
    generateBox(level, box, 1, 2, 0, 7, 2, 0, BASE_GRAY, BASE_GRAY, false);
    generateBox(level, box, 1, 2, 15, 6, 2, 15, BASE_GRAY, BASE_GRAY, false);
    generateBox(level, box, 0, 1, 0, 0, 1, 15, BASE_LIGHT, BASE_LIGHT, false);
    generateBox(level, box, 7, 1, 0, 7, 1, 15, BASE_LIGHT, BASE_LIGHT, false);
    generateBox(level, box, 1, 1, 0, 7, 1, 0, BASE_LIGHT, BASE_LIGHT, false);
    generateBox(level, box, 1, 1, 15, 6, 1, 15, BASE_LIGHT, BASE_LIGHT, false);
    generateBox(level, box, 1, 1, 1, 1, 1, 2, BASE_LIGHT, BASE_LIGHT, false);
    generateBox(level, box, 6, 1, 1, 6, 1, 2, BASE_LIGHT, BASE_LIGHT, false);
    generateBox(level, box, 1, 3, 1, 1, 3, 2, BASE_LIGHT, BASE_LIGHT, false);
    generateBox(level, box, 6, 3, 1, 6, 3, 2, BASE_LIGHT, BASE_LIGHT, false);
    generateBox(level, box, 1, 1, 13, 1, 1, 14, BASE_LIGHT, BASE_LIGHT, false);
    generateBox(level, box, 6, 1, 13, 6, 1, 14, BASE_LIGHT, BASE_LIGHT, false);
    generateBox(level, box, 1, 3, 13, 1, 3, 14, BASE_LIGHT, BASE_LIGHT, false);
    generateBox(level, box, 6, 3, 13, 6, 3, 14, BASE_LIGHT, BASE_LIGHT, false);
    generateBox(level, box, 2, 1, 6, 2, 3, 6, BASE_LIGHT, BASE_LIGHT, false);
    generateBox(level, box, 5, 1, 6, 5, 3, 6, BASE_LIGHT, BASE_LIGHT, false);
    generateBox(level, box, 2, 1, 9, 2, 3, 9, BASE_LIGHT, BASE_LIGHT, false);
    generateBox(level, box, 5, 1, 9, 5, 3, 9, BASE_LIGHT, BASE_LIGHT, false);
    generateBox(level, box, 3, 2, 6, 4, 2, 6, BASE_LIGHT, BASE_LIGHT, false);
    generateBox(level, box, 3, 2, 9, 4, 2, 9, BASE_LIGHT, BASE_LIGHT, false);
    generateBox(level, box, 2, 2, 7, 2, 2, 8, BASE_LIGHT, BASE_LIGHT, false);
    generateBox(level, box, 5, 2, 7, 5, 2, 8, BASE_LIGHT, BASE_LIGHT, false);
    placeBlock(level, LAMP_BLOCK, 2, 2, 5, box);
    placeBlock(level, LAMP_BLOCK, 5, 2, 5, box);
    placeBlock(level, LAMP_BLOCK, 2, 2, 10, box);
    placeBlock(level, LAMP_BLOCK, 5, 2, 10, box);
    placeBlock(level, BASE_LIGHT, 2, 3, 5, box);
    placeBlock(level, BASE_LIGHT, 5, 3, 5, box);
    placeBlock(level, BASE_LIGHT, 2, 3, 10, box);
    placeBlock(level, BASE_LIGHT, 5, 3, 10, box);
    if (southRoom->mHasOpening[Direction::SOUTH->get3DDataValue()])
    {
        generateWaterBox(level, box, 3, 1, 0, 4, 2, 0, false);
    }
    if (southRoom->mHasOpening[Direction::EAST->get3DDataValue()])
    {
        generateWaterBox(level, box, 7, 1, 3, 7, 2, 4, false);
    }
    if (southRoom->mHasOpening[Direction::WEST->get3DDataValue()])
    {
        generateWaterBox(level, box, 0, 1, 3, 0, 2, 4, false);
    }
    if (northRoom->mHasOpening[Direction::NORTH->get3DDataValue()])
    {
        generateWaterBox(level, box, 3, 1, 15, 4, 2, 15, false);
    }
    if (northRoom->mHasOpening[Direction::WEST->get3DDataValue()])
    {
        generateWaterBox(level, box, 0, 1, 11, 0, 2, 12, false);
    }
    if (northRoom->mHasOpening[Direction::EAST->get3DDataValue()])
    {
        generateWaterBox(level, box, 7, 1, 11, 7, 2, 12, false);
    }
    return true;
}

StructurePiece* OceanMonumentPieces::OceanMonumentDoubleZRoom::Create()
{
    return new OceanMonumentDoubleZRoom();
}

EStructurePiece OceanMonumentPieces::OceanMonumentDoubleZRoom::GetType()
{
    return eStructurePiece_OceanMonumentDoubleZRoom;
}

OceanMonumentPieces::OceanMonumentCoreRoom::OceanMonumentCoreRoom() : OceanMonumentPiece(0)
{
}

OceanMonumentPieces::OceanMonumentCoreRoom::OceanMonumentCoreRoom(
    const Direction* orientation, const boost::shared_ptr<RoomDefinition>& roomDefinition, Random& random)
    : OceanMonumentPiece(1, orientation, roomDefinition, 2, 2, 2)
{
}

bool OceanMonumentPieces::OceanMonumentCoreRoom::postProcess(Level* level, Random& random, BoundingBox* box)
{
    generateBoxOnFillOnly(level, box, 1, 8, 0, 14, 8, 14, BASE_GRAY);
    const BlockState* state = BASE_LIGHT;
    generateBox(level, box, 0, 7, 0, 0, 7, 15, state, state, false);
    generateBox(level, box, 15, 7, 0, 15, 7, 15, state, state, false);
    generateBox(level, box, 1, 7, 0, 15, 7, 0, state, state, false);
    generateBox(level, box, 1, 7, 15, 14, 7, 15, state, state, false);
    for (int y = 1; y <= 6; ++y)
    {
        state = BASE_LIGHT;
        if (y == 2 || y == 6)
        {
            state = BASE_GRAY;
        }
        for (int x = 0; x <= 15; x += 15)
        {
            generateBox(level, box, x, y, 0, x, y, 1, state, state, false);
            generateBox(level, box, x, y, 6, x, y, 9, state, state, false);
            generateBox(level, box, x, y, 14, x, y, 15, state, state, false);
        }
        generateBox(level, box, 1, y, 0, 1, y, 0, state, state, false);
        generateBox(level, box, 6, y, 0, 9, y, 0, state, state, false);
        generateBox(level, box, 14, y, 0, 14, y, 0, state, state, false);
        generateBox(level, box, 1, y, 15, 14, y, 15, state, state, false);
    }
    generateBox(level, box, 6, 3, 6, 9, 6, 9, BASE_BLACK, BASE_BLACK, false);
    generateBox(level, box, 7, 4, 7, 8, 5, 8, Blocks::gold_block->defaultBlockState(),
                Blocks::gold_block->defaultBlockState(), false);
    for (int y = 3; y <= 6; y += 3)
    {
        for (int x = 6; x <= 9; x += 3)
        {
            placeBlock(level, LAMP_BLOCK, x, y, 6, box);
            placeBlock(level, LAMP_BLOCK, x, y, 9, box);
        }
    }
    generateBox(level, box, 5, 1, 6, 5, 2, 6, BASE_LIGHT, BASE_LIGHT, false);
    generateBox(level, box, 5, 1, 9, 5, 2, 9, BASE_LIGHT, BASE_LIGHT, false);
    generateBox(level, box, 10, 1, 6, 10, 2, 6, BASE_LIGHT, BASE_LIGHT, false);
    generateBox(level, box, 10, 1, 9, 10, 2, 9, BASE_LIGHT, BASE_LIGHT, false);
    generateBox(level, box, 6, 1, 5, 6, 2, 5, BASE_LIGHT, BASE_LIGHT, false);
    generateBox(level, box, 9, 1, 5, 9, 2, 5, BASE_LIGHT, BASE_LIGHT, false);
    generateBox(level, box, 6, 1, 10, 6, 2, 10, BASE_LIGHT, BASE_LIGHT, false);
    generateBox(level, box, 9, 1, 10, 9, 2, 10, BASE_LIGHT, BASE_LIGHT, false);
    generateBox(level, box, 5, 2, 5, 5, 6, 5, BASE_LIGHT, BASE_LIGHT, false);
    generateBox(level, box, 5, 2, 10, 5, 6, 10, BASE_LIGHT, BASE_LIGHT, false);
    generateBox(level, box, 10, 2, 5, 10, 6, 5, BASE_LIGHT, BASE_LIGHT, false);
    generateBox(level, box, 10, 2, 10, 10, 6, 10, BASE_LIGHT, BASE_LIGHT, false);
    generateBox(level, box, 5, 7, 1, 5, 7, 6, BASE_LIGHT, BASE_LIGHT, false);
    generateBox(level, box, 10, 7, 1, 10, 7, 6, BASE_LIGHT, BASE_LIGHT, false);
    generateBox(level, box, 5, 7, 9, 5, 7, 14, BASE_LIGHT, BASE_LIGHT, false);
    generateBox(level, box, 10, 7, 9, 10, 7, 14, BASE_LIGHT, BASE_LIGHT, false);
    generateBox(level, box, 1, 7, 5, 6, 7, 5, BASE_LIGHT, BASE_LIGHT, false);
    generateBox(level, box, 1, 7, 10, 6, 7, 10, BASE_LIGHT, BASE_LIGHT, false);
    generateBox(level, box, 9, 7, 5, 14, 7, 5, BASE_LIGHT, BASE_LIGHT, false);
    generateBox(level, box, 9, 7, 10, 14, 7, 10, BASE_LIGHT, BASE_LIGHT, false);
    generateBox(level, box, 2, 1, 2, 2, 1, 3, BASE_LIGHT, BASE_LIGHT, false);
    generateBox(level, box, 3, 1, 2, 3, 1, 2, BASE_LIGHT, BASE_LIGHT, false);
    generateBox(level, box, 13, 1, 2, 13, 1, 3, BASE_LIGHT, BASE_LIGHT, false);
    generateBox(level, box, 12, 1, 2, 12, 1, 2, BASE_LIGHT, BASE_LIGHT, false);
    generateBox(level, box, 2, 1, 12, 2, 1, 13, BASE_LIGHT, BASE_LIGHT, false);
    generateBox(level, box, 3, 1, 13, 3, 1, 13, BASE_LIGHT, BASE_LIGHT, false);
    generateBox(level, box, 13, 1, 12, 13, 1, 13, BASE_LIGHT, BASE_LIGHT, false);
    generateBox(level, box, 12, 1, 13, 12, 1, 13, BASE_LIGHT, BASE_LIGHT, false);
    return true;
}

StructurePiece* OceanMonumentPieces::OceanMonumentCoreRoom::Create()
{
    return new OceanMonumentCoreRoom();
}

EStructurePiece OceanMonumentPieces::OceanMonumentCoreRoom::GetType()
{
    return eStructurePiece_OceanMonumentCoreRoom;
}

OceanMonumentPieces::OceanMonumentWingRoom::OceanMonumentWingRoom() : OceanMonumentPiece(0)
{
}

OceanMonumentPieces::OceanMonumentWingRoom::OceanMonumentWingRoom(const Direction* orientation,
                                                                  BoundingBox* boundingBox, int randomValue)
    : OceanMonumentPiece(orientation, boundingBox)
{
    mMainDesign = randomValue & 1;
}

bool OceanMonumentPieces::OceanMonumentWingRoom::postProcess(Level* level, Random& random, BoundingBox* box)
{
    if (mMainDesign == 0)
    {
        for (int i = 0; i < 4; ++i)
        {
            generateBox(level, box, 10 - i, 3 - i, 20 - i, 12 + i, 3 - i, 20, BASE_LIGHT, BASE_LIGHT, false);
        }
        generateBox(level, box, 7, 0, 6, 15, 0, 16, BASE_LIGHT, BASE_LIGHT, false);
        generateBox(level, box, 6, 0, 6, 6, 3, 20, BASE_LIGHT, BASE_LIGHT, false);
        generateBox(level, box, 16, 0, 6, 16, 3, 20, BASE_LIGHT, BASE_LIGHT, false);
        generateBox(level, box, 7, 1, 7, 7, 1, 20, BASE_LIGHT, BASE_LIGHT, false);
        generateBox(level, box, 15, 1, 7, 15, 1, 20, BASE_LIGHT, BASE_LIGHT, false);
        generateBox(level, box, 7, 1, 6, 9, 3, 6, BASE_LIGHT, BASE_LIGHT, false);
        generateBox(level, box, 13, 1, 6, 15, 3, 6, BASE_LIGHT, BASE_LIGHT, false);
        generateBox(level, box, 8, 1, 7, 9, 1, 7, BASE_LIGHT, BASE_LIGHT, false);
        generateBox(level, box, 13, 1, 7, 14, 1, 7, BASE_LIGHT, BASE_LIGHT, false);
        generateBox(level, box, 9, 0, 5, 13, 0, 5, BASE_LIGHT, BASE_LIGHT, false);
        generateBox(level, box, 10, 0, 7, 12, 0, 7, BASE_BLACK, BASE_BLACK, false);
        generateBox(level, box, 8, 0, 10, 8, 0, 12, BASE_BLACK, BASE_BLACK, false);
        generateBox(level, box, 14, 0, 10, 14, 0, 12, BASE_BLACK, BASE_BLACK, false);
        for (int z = 18; z >= 7; z -= 3)
        {
            placeBlock(level, LAMP_BLOCK, 6, 3, z, box);
            placeBlock(level, LAMP_BLOCK, 16, 3, z, box);
        }
        placeBlock(level, LAMP_BLOCK, 10, 0, 10, box);
        placeBlock(level, LAMP_BLOCK, 12, 0, 10, box);
        placeBlock(level, LAMP_BLOCK, 10, 0, 12, box);
        placeBlock(level, LAMP_BLOCK, 12, 0, 12, box);
        placeBlock(level, LAMP_BLOCK, 8, 3, 6, box);
        placeBlock(level, LAMP_BLOCK, 14, 3, 6, box);
        placeBlock(level, BASE_LIGHT, 4, 2, 4, box);
        placeBlock(level, LAMP_BLOCK, 4, 1, 4, box);
        placeBlock(level, BASE_LIGHT, 4, 0, 4, box);
        placeBlock(level, BASE_LIGHT, 18, 2, 4, box);
        placeBlock(level, LAMP_BLOCK, 18, 1, 4, box);
        placeBlock(level, BASE_LIGHT, 18, 0, 4, box);
        placeBlock(level, BASE_LIGHT, 4, 2, 18, box);
        placeBlock(level, LAMP_BLOCK, 4, 1, 18, box);
        placeBlock(level, BASE_LIGHT, 4, 0, 18, box);
        placeBlock(level, BASE_LIGHT, 18, 2, 18, box);
        placeBlock(level, LAMP_BLOCK, 18, 1, 18, box);
        placeBlock(level, BASE_LIGHT, 18, 0, 18, box);
        placeBlock(level, BASE_LIGHT, 9, 7, 20, box);
        placeBlock(level, BASE_LIGHT, 13, 7, 20, box);
        generateBox(level, box, 6, 0, 21, 7, 4, 21, BASE_LIGHT, BASE_LIGHT, false);
        generateBox(level, box, 15, 0, 21, 16, 4, 21, BASE_LIGHT, BASE_LIGHT, false);
        spawnElder(level, box, 11, 2, 16);
    }
    else if (mMainDesign == 1)
    {
        generateBox(level, box, 9, 3, 18, 13, 3, 20, BASE_LIGHT, BASE_LIGHT, false);
        generateBox(level, box, 9, 0, 18, 9, 2, 18, BASE_LIGHT, BASE_LIGHT, false);
        generateBox(level, box, 13, 0, 18, 13, 2, 18, BASE_LIGHT, BASE_LIGHT, false);
        int x = 9;
        for (int i = 0; i < 2; ++i)
        {
            placeBlock(level, BASE_LIGHT, x, 6, 20, box);
            placeBlock(level, LAMP_BLOCK, x, 5, 20, box);
            placeBlock(level, BASE_LIGHT, x, 4, 20, box);
            x = 13;
        }
        generateBox(level, box, 7, 3, 7, 15, 3, 14, BASE_LIGHT, BASE_LIGHT, false);
        x = 10;
        for (int i = 0; i < 2; ++i)
        {
            generateBox(level, box, x, 0, 10, x, 6, 10, BASE_LIGHT, BASE_LIGHT, false);
            generateBox(level, box, x, 0, 12, x, 6, 12, BASE_LIGHT, BASE_LIGHT, false);
            placeBlock(level, LAMP_BLOCK, x, 0, 10, box);
            placeBlock(level, LAMP_BLOCK, x, 0, 12, box);
            placeBlock(level, LAMP_BLOCK, x, 4, 10, box);
            placeBlock(level, LAMP_BLOCK, x, 4, 12, box);
            x = 12;
        }
        x = 8;
        for (int i = 0; i < 2; ++i)
        {
            generateBox(level, box, x, 0, 7, x, 2, 7, BASE_LIGHT, BASE_LIGHT, false);
            generateBox(level, box, x, 0, 14, x, 2, 14, BASE_LIGHT, BASE_LIGHT, false);
            x = 14;
        }
        generateBox(level, box, 8, 3, 8, 8, 3, 13, BASE_BLACK, BASE_BLACK, false);
        generateBox(level, box, 14, 3, 8, 14, 3, 13, BASE_BLACK, BASE_BLACK, false);
        spawnElder(level, box, 11, 5, 16);
    }
    return true;
}

EStructurePiece OceanMonumentPieces::OceanMonumentWingRoom::GetType()
{
    return eStructurePiece_OceanMonumentWingRoom;
}

OceanMonumentPieces::OceanMonumentPenthouse::OceanMonumentPenthouse() : OceanMonumentPiece(0)
{
}

OceanMonumentPieces::OceanMonumentPenthouse::OceanMonumentPenthouse(const Direction* orientation,
                                                                    BoundingBox* boundingBox)
    : OceanMonumentPiece(orientation, boundingBox)
{
}

bool OceanMonumentPieces::OceanMonumentPenthouse::postProcess(Level* level, Random& random, BoundingBox* box)
{
    generateBox(level, box, 2, -1, 2, 11, -1, 11, BASE_LIGHT, BASE_LIGHT, false);
    generateBox(level, box, 0, -1, 0, 1, -1, 11, BASE_GRAY, BASE_GRAY, false);
    generateBox(level, box, 12, -1, 0, 13, -1, 11, BASE_GRAY, BASE_GRAY, false);
    generateBox(level, box, 2, -1, 0, 11, -1, 1, BASE_GRAY, BASE_GRAY, false);
    generateBox(level, box, 2, -1, 12, 11, -1, 13, BASE_GRAY, BASE_GRAY, false);
    generateBox(level, box, 0, 0, 0, 0, 0, 13, BASE_LIGHT, BASE_LIGHT, false);
    generateBox(level, box, 13, 0, 0, 13, 0, 13, BASE_LIGHT, BASE_LIGHT, false);
    generateBox(level, box, 1, 0, 0, 12, 0, 0, BASE_LIGHT, BASE_LIGHT, false);
    generateBox(level, box, 1, 0, 13, 12, 0, 13, BASE_LIGHT, BASE_LIGHT, false);
    for (int i = 2; i <= 11; i += 3)
    {
        placeBlock(level, LAMP_BLOCK, 0, 0, i, box);
        placeBlock(level, LAMP_BLOCK, 13, 0, i, box);
        placeBlock(level, LAMP_BLOCK, i, 0, 0, box);
    }
    generateBox(level, box, 2, 0, 3, 4, 0, 9, BASE_LIGHT, BASE_LIGHT, false);
    generateBox(level, box, 9, 0, 3, 11, 0, 9, BASE_LIGHT, BASE_LIGHT, false);
    generateBox(level, box, 4, 0, 9, 9, 0, 11, BASE_LIGHT, BASE_LIGHT, false);
    placeBlock(level, BASE_LIGHT, 5, 0, 8, box);
    placeBlock(level, BASE_LIGHT, 8, 0, 8, box);
    placeBlock(level, BASE_LIGHT, 10, 0, 10, box);
    placeBlock(level, BASE_LIGHT, 3, 0, 10, box);
    generateBox(level, box, 3, 0, 3, 3, 0, 7, BASE_BLACK, BASE_BLACK, false);
    generateBox(level, box, 10, 0, 3, 10, 0, 7, BASE_BLACK, BASE_BLACK, false);
    generateBox(level, box, 6, 0, 10, 7, 0, 10, BASE_BLACK, BASE_BLACK, false);
    int x = 3;
    for (int i = 0; i < 2; ++i)
    {
        for (int z = 2; z <= 8; z += 3)
        {
            generateBox(level, box, x, 0, z, x, 2, z, BASE_LIGHT, BASE_LIGHT, false);
        }
        x = 10;
    }
    generateBox(level, box, 5, 0, 10, 5, 2, 10, BASE_LIGHT, BASE_LIGHT, false);
    generateBox(level, box, 8, 0, 10, 8, 2, 10, BASE_LIGHT, BASE_LIGHT, false);
    generateBox(level, box, 6, -1, 7, 7, -1, 8, BASE_BLACK, BASE_BLACK, false);
    generateWaterBox(level, box, 6, -1, 3, 7, -1, 4, false);
    spawnElder(level, box, 6, 1, 6);
    return true;
}

StructurePiece* OceanMonumentPieces::OceanMonumentPenthouse::Create()
{
    return new OceanMonumentPenthouse();
}

EStructurePiece OceanMonumentPieces::OceanMonumentPenthouse::GetType()
{
    return eStructurePiece_OceanMonumentPenthouse;
}
