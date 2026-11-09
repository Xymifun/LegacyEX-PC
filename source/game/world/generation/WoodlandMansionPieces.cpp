#include "game/world/generation/WoodlandMansionPieces.hpp"

#include <utility>

#include "game/core/BoundingBox.hpp"
#include "game/core/BuiltInLootTables.hpp"
#include "game/core/DataInputStream.hpp"
#include "game/core/DataOutputStream.hpp"
#include "game/core/EvocationIllager.hpp"
#include "game/core/Mth.hpp"
#include "game/core/VindicationIllager.hpp"
#include "game/world/Level.hpp"
#include "game/world/block/Blocks.hpp"
#include "game/world/block/ChestBlock.hpp"
#include "game/world/block/DirectionProperty.hpp"
#include "game/world/block/TypedBoxed.hpp"
#include "game/world/generation/StructureFeatureIO.hpp"
#include "game/world/generation/StructurePlaceSettings.hpp"
#include "game/world/generation/StructureTemplate.hpp"

WoodlandMansionPieces::SimpleGrid::SimpleGrid(int width, int height, int valueIfOutside)
{
    mWidth = width;
    mHeight = height;
    mValueIfOutside = valueIfOutside;
    mGrid = array2DWithLength<int>(width, height);
}

WoodlandMansionPieces::SimpleGrid::~SimpleGrid()
{
    for (unsigned int x = 0; x < mGrid.length; ++x)
        delete[] mGrid.data[x]->data;
    delete[] mGrid.data;
}

void WoodlandMansionPieces::SimpleGrid::set(int x, int y, int value)
{
    if (x >= 0 && x < mWidth && y >= 0 && y < mHeight)
        mGrid.data[x]->data[y] = value;
}

void WoodlandMansionPieces::SimpleGrid::set(int x0, int y0, int x1, int y1, int value)
{
    for (int y = y0; y <= y1; ++y)
    {
        for (int x = x0; x <= x1; ++x)
            set(x, y, value);
    }
}

int WoodlandMansionPieces::SimpleGrid::get(int x, int y)
{
    if (x >= 0 && x < mWidth && y >= 0 && y < mHeight)
        return mGrid.data[x]->data[y];
    return mValueIfOutside;
}

void WoodlandMansionPieces::SimpleGrid::setIf(int x, int y, int ifValue, int value)
{
    if (get(x, y) == ifValue)
        set(x, y, value);
}

bool WoodlandMansionPieces::SimpleGrid::edgesTo(int x, int y, int value)
{
    return get(x - 1, y) == value || get(x + 1, y) == value || get(x, y + 1) == value || get(x, y - 1) == value;
}

WoodlandMansionPieces::FloorRoomCollection::~FloorRoomCollection()
{
}

WoodlandMansionPieces::FirstFloorRoomCollection::~FirstFloorRoomCollection()
{
}
eMinecraftStructureTemplate WoodlandMansionPieces::FirstFloorRoomCollection::get1x1(Random& random)
{
    return static_cast<eMinecraftStructureTemplate>(eMinecraftStructureTemplate_Mansion_1x1A1 + random.nextInt(5));
}

eMinecraftStructureTemplate WoodlandMansionPieces::FirstFloorRoomCollection::get1x1Secret(Random& random)
{
    return static_cast<eMinecraftStructureTemplate>(eMinecraftStructureTemplate_Mansion_1x1As1 + random.nextInt(4));
}

eMinecraftStructureTemplate WoodlandMansionPieces::FirstFloorRoomCollection::get1x2SideEntrance(Random& random, bool)
{
    return static_cast<eMinecraftStructureTemplate>(eMinecraftStructureTemplate_Mansion_1x2A1 + random.nextInt(9));
}

eMinecraftStructureTemplate WoodlandMansionPieces::FirstFloorRoomCollection::get1x2FrontEntrance(Random& random, bool)
{
    return static_cast<eMinecraftStructureTemplate>(eMinecraftStructureTemplate_Mansion_1x2B1 + random.nextInt(5));
}

eMinecraftStructureTemplate WoodlandMansionPieces::FirstFloorRoomCollection::get1x2Secret(Random& random)
{
    return static_cast<eMinecraftStructureTemplate>(eMinecraftStructureTemplate_Mansion_1x2S1 + random.nextInt(2));
}

eMinecraftStructureTemplate WoodlandMansionPieces::FirstFloorRoomCollection::get2x2(Random& random)
{
    return static_cast<eMinecraftStructureTemplate>(eMinecraftStructureTemplate_Mansion_2x2A1 + random.nextInt(4));
}

eMinecraftStructureTemplate WoodlandMansionPieces::FirstFloorRoomCollection::get2x2Secret(Random&)
{
    return eMinecraftStructureTemplate_Mansion_2x2S1;
}

WoodlandMansionPieces::SecondFloorRoomCollection::~SecondFloorRoomCollection()
{
}
eMinecraftStructureTemplate WoodlandMansionPieces::SecondFloorRoomCollection::get1x1(Random& random)
{
    return static_cast<eMinecraftStructureTemplate>(eMinecraftStructureTemplate_Mansion_1x1B1 + random.nextInt(5));
}

eMinecraftStructureTemplate WoodlandMansionPieces::SecondFloorRoomCollection::get1x1Secret(Random& random)
{
    return static_cast<eMinecraftStructureTemplate>(eMinecraftStructureTemplate_Mansion_1x1As1 + random.nextInt(4));
}

eMinecraftStructureTemplate WoodlandMansionPieces::SecondFloorRoomCollection::get1x2SideEntrance(Random& random,
                                                                                                 bool staircase)
{
    if (staircase)
        return eMinecraftStructureTemplate_Mansion_1x2CStairs;
    int index = random.nextInt(4);
    return static_cast<eMinecraftStructureTemplate>(eMinecraftStructureTemplate_Mansion_1x2C1 + index);
}

eMinecraftStructureTemplate WoodlandMansionPieces::SecondFloorRoomCollection::get1x2FrontEntrance(Random& random,
                                                                                                  bool staircase)
{
    if (staircase)
        return eMinecraftStructureTemplate_Mansion_1x2DStairs;
    int index = random.nextInt(5);
    return static_cast<eMinecraftStructureTemplate>(eMinecraftStructureTemplate_Mansion_1x2D1 + index);
}

eMinecraftStructureTemplate WoodlandMansionPieces::SecondFloorRoomCollection::get1x2Secret(Random&)
{
    return eMinecraftStructureTemplate_Mansion_1x2Se1;
}

eMinecraftStructureTemplate WoodlandMansionPieces::SecondFloorRoomCollection::get2x2(Random& random)
{
    return static_cast<eMinecraftStructureTemplate>(eMinecraftStructureTemplate_Mansion_2x2B1 + random.nextInt(5));
}

eMinecraftStructureTemplate WoodlandMansionPieces::SecondFloorRoomCollection::get2x2Secret(Random&)
{
    return eMinecraftStructureTemplate_Mansion_2x2S1;
}

WoodlandMansionPieces::ThirdFloorRoomCollection::~ThirdFloorRoomCollection()
{
}

WoodlandMansionPieces::MansionGrid::MansionGrid(Random& random) : Random(), mFloorRooms()
{
    Random::operator=(random);
    mEntranceX = 7;
    mEntranceY = 4;
    mBaseGrid = new SimpleGrid(11, 11, 5);
    mBaseGrid->set(mEntranceX, mEntranceY, mEntranceX + 1, mEntranceY + 1, 3);
    mBaseGrid->set(mEntranceX - 1, mEntranceY, mEntranceX - 1, mEntranceY + 1, 2);
    mBaseGrid->set(mEntranceX + 2, mEntranceY - 2, mEntranceX + 3, mEntranceY + 3, 5);
    mBaseGrid->set(mEntranceX + 1, mEntranceY - 2, mEntranceX + 1, mEntranceY - 1, 1);
    mBaseGrid->set(mEntranceX + 1, mEntranceY + 2, mEntranceX + 1, mEntranceY + 3, 1);
    mBaseGrid->set(mEntranceX - 1, mEntranceY - 1, 1);
    mBaseGrid->set(mEntranceX - 1, mEntranceY + 2, 1);
    mBaseGrid->set(0, 0, 11, 1, 5);
    mBaseGrid->set(0, 9, 11, 11, 5);
    recursiveCorridor(mBaseGrid, mEntranceX, mEntranceY - 2, Direction::WEST, 6);
    recursiveCorridor(mBaseGrid, mEntranceX, mEntranceY + 3, Direction::WEST, 6);
    recursiveCorridor(mBaseGrid, mEntranceX - 2, mEntranceY - 1, Direction::WEST, 3);
    recursiveCorridor(mBaseGrid, mEntranceX - 2, mEntranceY + 2, Direction::WEST, 3);
    while (cleanEdges(mBaseGrid))
    {
    }

    mFloorRooms = arrayWithLength<SimpleGrid*>(3, true);
    mFloorRooms.data[0] = new SimpleGrid(11, 11, 5);
    mFloorRooms.data[1] = new SimpleGrid(11, 11, 5);
    mFloorRooms.data[2] = new SimpleGrid(11, 11, 5);
    identifyRooms(mBaseGrid, mFloorRooms.data[0]);
    identifyRooms(mBaseGrid, mFloorRooms.data[1]);
    mFloorRooms.data[0]->set(mEntranceX + 1, mEntranceY, mEntranceX + 1, mEntranceY + 1, 0x800000);
    mFloorRooms.data[1]->set(mEntranceX + 1, mEntranceY, mEntranceX + 1, mEntranceY + 1, 0x800000);
    mThirdFloorGrid = new SimpleGrid(mBaseGrid->mWidth, mBaseGrid->mHeight, 5);
    setupThirdFloor();
    identifyRooms(mThirdFloorGrid, mFloorRooms.data[2]);
}

WoodlandMansionPieces::MansionGrid::~MansionGrid()
{
    delete mBaseGrid;
    mBaseGrid = NULL;
    delete mThirdFloorGrid;
    mThirdFloorGrid = NULL;
    for (unsigned int i = 0; i < mFloorRooms.length; ++i)
    {
        delete mFloorRooms.data[i];
        mFloorRooms.data[i] = NULL;
    }
    delete[] mFloorRooms.data;
    mFloorRooms = arrayWithLength<SimpleGrid*>();
}

void WoodlandMansionPieces::MansionGrid::recursiveCorridor(SimpleGrid* grid, int x, int y, const Direction* direction,
                                                           int length)
{
    if (length > 0)
    {
        grid->set(x, y, 1);
        grid->setIf(x + direction->getStepX(), y + direction->getStepZ(), 0, 1);

        for (int attempt = 0; attempt < 8; ++attempt)
        {
            const Direction* next = Direction::from2DDataValue(nextInt(4));
            if (next != direction->getOpposite() && (next != Direction::EAST || !nextBoolean()))
            {
                int nextX = x + direction->getStepX();
                int nextY = y + direction->getStepZ();
                if (grid->get(nextX + next->getStepX(), nextY + next->getStepZ()) == 0
                    && grid->get(nextX + next->getStepX() * 2, nextY + next->getStepZ() * 2) == 0)
                {
                    recursiveCorridor(grid, x + direction->getStepX() + next->getStepX(),
                                      y + direction->getStepZ() + next->getStepZ(), next, length - 1);
                    break;
                }
            }
        }

        const Direction* clockWise = direction->getClockWise();
        const Direction* counterClockWise = direction->getCounterClockWise();
        grid->setIf(x + clockWise->getStepX(), y + clockWise->getStepZ(), 0, 2);
        grid->setIf(x + counterClockWise->getStepX(), y + counterClockWise->getStepZ(), 0, 2);
        grid->setIf(x + direction->getStepX() + clockWise->getStepX(),
                    y + direction->getStepZ() + clockWise->getStepZ(), 0, 2);
        grid->setIf(x + direction->getStepX() + counterClockWise->getStepX(),
                    y + direction->getStepZ() + counterClockWise->getStepZ(), 0, 2);
        grid->setIf(x + direction->getStepX() * 2, y + direction->getStepZ() * 2, 0, 2);
        grid->setIf(x + clockWise->getStepX() * 2, y + clockWise->getStepZ() * 2, 0, 2);
        grid->setIf(x + counterClockWise->getStepX() * 2, y + counterClockWise->getStepZ() * 2, 0, 2);
    }
}

bool WoodlandMansionPieces::MansionGrid::isHouse(SimpleGrid* grid, int x, int y)
{
    int value = grid->get(x, y);
    return value == 1 || value == 2 || value == 3 || value == 4;
}

bool WoodlandMansionPieces::MansionGrid::cleanEdges(SimpleGrid* grid)
{
    bool changed = false;
    for (int y = 0; y < grid->mHeight; ++y)
    {
        for (int x = 0; x < grid->mWidth; ++x)
        {
            if (grid->get(x, y) != 0)
                continue;

            int neighbours = 0;
            neighbours += isHouse(grid, x + 1, y) ? 1 : 0;
            neighbours += isHouse(grid, x - 1, y) ? 1 : 0;
            neighbours += isHouse(grid, x, y + 1) ? 1 : 0;
            neighbours += isHouse(grid, x, y - 1) ? 1 : 0;
            if (neighbours >= 3)
            {
                grid->set(x, y, 2);
                changed = true;
            }
            else if (neighbours == 2)
            {
                int diagonals = 0;
                diagonals += isHouse(grid, x + 1, y + 1) ? 1 : 0;
                diagonals += isHouse(grid, x - 1, y + 1) ? 1 : 0;
                diagonals += isHouse(grid, x + 1, y - 1) ? 1 : 0;
                diagonals += isHouse(grid, x - 1, y - 1) ? 1 : 0;
                if (diagonals <= 1)
                {
                    grid->set(x, y, 2);
                    changed = true;
                }
            }
        }
    }
    return changed;
}

void WoodlandMansionPieces::MansionGrid::identifyRooms(SimpleGrid* grid, SimpleGrid* rooms)
{
    std::vector<std::pair<int, int> > candidates;
    for (int y = 0; y < grid->mHeight; ++y)
    {
        for (int x = 0; x < grid->mWidth; ++x)
        {
            if (grid->get(x, y) == 2)
                candidates.push_back(std::pair<int, int>(x, y));
        }
    }

    Mth::shuffle(&candidates, *this);

    int roomId = 10;
    for (std::vector<std::pair<int, int> >::iterator it = candidates.begin(); it != candidates.end(); ++it)
    {
        int x = it->first;
        int y = it->second;
        if (rooms->get(x, y) == 0)
        {
            int x0 = x;
            int x1 = x;
            int y0 = y;
            int y1 = y;
            int type = 0x10000;
            if (rooms->get(x + 1, y) == 0 && rooms->get(x, y + 1) == 0 && rooms->get(x + 1, y + 1) == 0
                && grid->get(x + 1, y) == 2 && grid->get(x, y + 1) == 2 && grid->get(x + 1, y + 1) == 2)
            {
                ++x1;
                ++y1;
                type = 0x40000;
            }
            else if (rooms->get(x - 1, y) == 0 && rooms->get(x, y + 1) == 0 && rooms->get(x - 1, y + 1) == 0
                     && grid->get(x - 1, y) == 2 && grid->get(x, y + 1) == 2 && grid->get(x - 1, y + 1) == 2)
            {
                --x0;
                ++y1;
                type = 0x40000;
            }
            else if (rooms->get(x - 1, y) == 0 && rooms->get(x, y - 1) == 0 && rooms->get(x - 1, y - 1) == 0
                     && grid->get(x - 1, y) == 2 && grid->get(x, y - 1) == 2 && grid->get(x - 1, y - 1) == 2)
            {
                --x0;
                --y0;
                type = 0x40000;
            }
            else if (rooms->get(x + 1, y) == 0 && grid->get(x + 1, y) == 2)
            {
                ++x1;
                type = 0x20000;
            }
            else if (rooms->get(x, y + 1) == 0 && grid->get(x, y + 1) == 2)
            {
                ++y1;
                type = 0x20000;
            }
            else if (rooms->get(x - 1, y) == 0 && grid->get(x - 1, y) == 2)
            {
                --x0;
                type = 0x20000;
            }
            else if (rooms->get(x, y - 1) == 0 && grid->get(x, y - 1) == 2)
            {
                --y0;
                type = 0x20000;
            }

            int doorX = nextBoolean() ? x0 : x1;
            int doorY = nextBoolean() ? y0 : y1;
            int doorFlag = 0x200000;
            if (!grid->edgesTo(doorX, doorY, 1))
            {
                doorX = doorX == x0 ? x1 : x0;
                doorY = doorY == y0 ? y1 : y0;
                if (!grid->edgesTo(doorX, doorY, 1))
                {
                    doorY = doorY == y0 ? y1 : y0;
                    if (!grid->edgesTo(doorX, doorY, 1))
                    {
                        doorX = doorX == x0 ? x1 : x0;
                        doorY = doorY == y0 ? y1 : y0;
                        if (!grid->edgesTo(doorX, doorY, 1))
                        {
                            doorFlag = 0;
                            doorX = x0;
                            doorY = y0;
                        }
                    }
                }
            }

            for (int roomY = y0; roomY <= y1; ++roomY)
            {
                for (int roomX = x0; roomX <= x1; ++roomX)
                {
                    if (roomX == doorX && roomY == doorY)
                        rooms->set(roomX, roomY, 0x100000 | doorFlag | type | roomId);
                    else
                        rooms->set(roomX, roomY, type | roomId);
                }
            }
            ++roomId;
        }
    }
}
bool WoodlandMansionPieces::MansionGrid::isRoomId(SimpleGrid*, int x, int y, int floor, int roomId)
{
    return (mFloorRooms.data[floor]->get(x, y) & 0xffff) == roomId;
}

const Direction* WoodlandMansionPieces::MansionGrid::get1x2RoomDirection(SimpleGrid* grid, int x, int y, int floor,
                                                                         int roomId)
{
    arrayWithLength<const Direction*> faces = Direction::Plane::HORIZONTAL->getFaces();
    for (unsigned int i = 0; i < faces.length; ++i)
    {
        const Direction* direction = faces.data[i];
        if (isRoomId(grid, x + direction->getStepX(), y + direction->getStepZ(), floor, roomId))
            return direction;
    }
    return NULL;
}

void WoodlandMansionPieces::MansionGrid::setupThirdFloor()
{
    std::vector<std::pair<int, int> > candidates;
    SimpleGrid* secondFloorRooms = mFloorRooms.data[1];
    for (int y = 0; y < mThirdFloorGrid->mHeight; ++y)
    {
        for (int x = 0; x < mThirdFloorGrid->mWidth; ++x)
        {
            int value = secondFloorRooms->get(x, y);
            if ((value & 0xf0000) == 0x20000 && (value & 0x200000) == 0x200000)
                candidates.push_back(std::pair<int, int>(x, y));
        }
    }

    if (candidates.empty())
    {
        mThirdFloorGrid->set(0, 0, mThirdFloorGrid->mWidth, mThirdFloorGrid->mHeight, 5);
        return;
    }

    std::pair<int, int> staircase = candidates.at(nextInt((int)candidates.size()));
    int staircaseX = staircase.first;
    int staircaseY = staircase.second;
    int value = secondFloorRooms->get(staircaseX, staircaseY);
    secondFloorRooms->set(staircaseX, staircaseY, value | 0x400000);

    const Direction* direction = get1x2RoomDirection(mBaseGrid, staircaseX, staircaseY, 1, value & 0xffff);
    int landingX = staircaseX + direction->getStepX();
    int landingY = staircaseY + direction->getStepZ();
    for (int y = 0; y < mThirdFloorGrid->mHeight; ++y)
    {
        for (int x = 0; x < mThirdFloorGrid->mWidth; ++x)
        {
            if (!isHouse(mBaseGrid, x, y))
            {
                mThirdFloorGrid->set(x, y, 5);
            }
            else if (x == staircaseX && y == staircaseY)
            {
                mThirdFloorGrid->set(x, y, 3);
            }
            else if (x == landingX && y == landingY)
            {
                mThirdFloorGrid->set(x, y, 3);
                mFloorRooms.data[2]->set(x, y, 0x800000);
            }
        }
    }

    std::vector<const Direction*> corridorDirections;
    arrayWithLength<const Direction*> faces = Direction::Plane::HORIZONTAL->getFaces();
    for (unsigned int i = 0; i < faces.length; ++i)
    {
        const Direction* face = faces.data[i];
        if (mThirdFloorGrid->get(landingX + face->getStepX(), landingY + face->getStepZ()) == 0)
            corridorDirections.push_back(face);
    }

    if (corridorDirections.empty())
    {
        mThirdFloorGrid->set(0, 0, mThirdFloorGrid->mWidth, mThirdFloorGrid->mHeight, 5);
        secondFloorRooms->set(staircaseX, staircaseY, value);
        return;
    }

    const Direction* corridor = corridorDirections.at(nextInt((int)corridorDirections.size()));
    recursiveCorridor(mThirdFloorGrid, landingX + corridor->getStepX(), landingY + corridor->getStepZ(), corridor, 4);
    while (cleanEdges(mThirdFloorGrid))
    {
    }
}

WoodlandMansionPieces::WoodlandMansionPiece::WoodlandMansionPiece(StructureManager* structureManager,
                                                                  eMinecraftStructureTemplate templateType,
                                                                  const BlockPos& position, Rotation* rotation)
    : TemplateStructurePiece(0)
{
    _init(structureManager, templateType, position, rotation, Mirror::NONE);
}

WoodlandMansionPieces::WoodlandMansionPiece::WoodlandMansionPiece(StructureManager* structureManager,
                                                                  eMinecraftStructureTemplate templateType,
                                                                  const BlockPos& position, Rotation* rotation,
                                                                  Mirror* mirror)
    : TemplateStructurePiece(0)
{
    _init(structureManager, templateType, position, rotation, mirror);
}

WoodlandMansionPieces::WoodlandMansionPiece::WoodlandMansionPiece() : TemplateStructurePiece()
{
}

void WoodlandMansionPieces::WoodlandMansionPiece::loadTemplate(StructureManager* structureManager)
{
    const StructureTemplate* structureTemplate = structureManager->getOrCreate(mTemplateType);
    StructurePlaceSettings* settings =
        (new StructurePlaceSettings())->setIgnoreEntities(true)->setRotation(mRotation)->setMirror(mMirror);
    setup(structureTemplate, mTemplatePosition, settings);
}

void WoodlandMansionPieces::WoodlandMansionPiece::_init(StructureManager* structureManager,
                                                        eMinecraftStructureTemplate templateType,
                                                        const BlockPos& position, Rotation* rotation, Mirror* mirror)
{
    mTemplateType = templateType;
    mTemplatePosition = position;
    mRotation = rotation;
    mMirror = mirror;
    loadTemplate(structureManager);
}

void WoodlandMansionPieces::WoodlandMansionPiece::addAdditionalSaveData(DataOutputStream* output)
{
    TemplateStructurePiece::addAdditionalSaveData(output);
    output->writeVarInt(mTemplateType);
    output->writeByte((unsigned char)mPlaceSettings->getRotation()->getType());
    output->writeByte((unsigned char)mPlaceSettings->getMirror()->getType());
}

void WoodlandMansionPieces::WoodlandMansionPiece::readAdditionalSaveData(DataInputStream* input,
                                                                         StructureManager* structureManager)
{
    TemplateStructurePiece::readAdditionalSaveData(input, structureManager);
    mTemplateType = (eMinecraftStructureTemplate)input->readVarInt();
    mRotation = Rotation::byType((Rotation::eTYPE)input->readByte());
    mMirror = Mirror::VALUES[input->readByte()];
    loadTemplate(structureManager);
}

void WoodlandMansionPieces::WoodlandMansionPiece::handleDataMarker(const std::wstring& marker, const BlockPos& position,
                                                                   Level* level, Random& random, BoundingBox* box)
{
    if (marker.find(L"Chest") == 0)
    {
        Rotation* rotation = mPlaceSettings->getRotation();
        const BlockState* state = Blocks::chest->defaultBlockState();
        if (marker == L"ChestWest")
        {
            state = state->setValue(ChestBlock::FACING, rotation->rotate(Direction::WEST));
        }
        else if (marker == L"ChestEast")
        {
            state = state->setValue(ChestBlock::FACING, rotation->rotate(Direction::EAST));
        }
        else if (marker == L"ChestSouth")
        {
            state = state->setValue(ChestBlock::FACING, rotation->rotate(Direction::SOUTH));
        }
        else if (marker == L"ChestNorth")
        {
            state = state->setValue(ChestBlock::FACING, rotation->rotate(Direction::NORTH));
        }
        createChest(level, box, random, position, &BuiltInLootTables::WOODLAND_MANSION, state);
    }
    else if (marker == L"Mage")
    {
        boost::shared_ptr<EvocationIllager> evoker(new EvocationIllager(level));
        evoker->setPersistenceRequired(true);
        evoker->moveTo(position, 0.0f, 0.0f);
        level->addEntity(evoker);
        level->setBlock(position, Blocks::air->defaultBlockState(), Block::UPDATE_CLIENTS, false);
    }
    else if (marker == L"Warrior")
    {
        boost::shared_ptr<VindicationIllager> vindicator(new VindicationIllager(level));
        vindicator->setPersistenceRequired(true);
        vindicator->moveTo(position, 0.0f, 0.0f);
        vindicator->finalizeMobSpawn(level->getCurrentDifficultyAt(BlockPos(vindicator)), NULL, L"");
        level->addEntity(vindicator);
        level->setBlock(position, Blocks::air->defaultBlockState(), Block::UPDATE_CLIENTS, false);
    }
}

StructurePiece* WoodlandMansionPieces::WoodlandMansionPiece::create()
{
    return new WoodlandMansionPiece;
}

EStructurePiece WoodlandMansionPieces::WoodlandMansionPiece::GetType()
{
    return eStructurePiece_WoodlandMansionPiece;
}

WoodlandMansionPieces::MansionPiecePlacer::MansionPiecePlacer(StructureManager* structureManager, Random& random)
    : mRandom()
{
    mStructureManager = structureManager;
    mRandom = random;
}

void WoodlandMansionPieces::MansionPiecePlacer::entrance(std::vector<WoodlandMansionPiece*>& pieces,
                                                         PlacementData& data)
{
    BlockPos position = data.position.relative(data.rotation->rotate(Direction::WEST), 9);
    pieces.push_back(new WoodlandMansionPiece(mStructureManager, eMinecraftStructureTemplate_Mansion_Entrance, position,
                                              data.rotation));
    data.position = data.position.relative(data.rotation->rotate(Direction::SOUTH), 16);
}

void WoodlandMansionPieces::MansionPiecePlacer::traverseTurn(std::vector<WoodlandMansionPiece*>& pieces,
                                                             PlacementData& data)
{
    data.position = data.position.relative(data.rotation->rotate(Direction::SOUTH), -1);
    pieces.push_back(new WoodlandMansionPiece(mStructureManager, eMinecraftStructureTemplate_Mansion_WallCorner,
                                              data.position, data.rotation));
    data.position = data.position.relative(data.rotation->rotate(Direction::SOUTH), -7);
    data.position = data.position.relative(data.rotation->rotate(Direction::WEST), -6);
    data.rotation = data.rotation->getRotated(Rotation::CLOCKWISE_90);
}

void WoodlandMansionPieces::MansionPiecePlacer::traverseWallPiece(std::vector<WoodlandMansionPiece*>& pieces,
                                                                  PlacementData& data)
{
    BlockPos position = data.position.relative(data.rotation->rotate(Direction::EAST), 7);
    pieces.push_back(new WoodlandMansionPiece(mStructureManager, data.wallType, position, data.rotation));
    data.position = data.position.relative(data.rotation->rotate(Direction::SOUTH), 8);
}

void WoodlandMansionPieces::MansionPiecePlacer::traverseInnerTurn(std::vector<WoodlandMansionPiece*>&,
                                                                  PlacementData& data)
{
    data.position = data.position.relative(data.rotation->rotate(Direction::SOUTH), 6);
    data.position = data.position.relative(data.rotation->rotate(Direction::EAST), 8);
    data.rotation = data.rotation->getRotated(Rotation::COUNTERCLOCKWISE_90);
}

void WoodlandMansionPieces::MansionPiecePlacer::traverseOuterWalls(std::vector<WoodlandMansionPiece*>& pieces,
                                                                   PlacementData& data, SimpleGrid* grid,
                                                                   const Direction* direction, int startX, int startY,
                                                                   int endX, int endY)
{
    int x = startX;
    int y = startY;
    const Direction* current = direction;
    do
    {
        if (!MansionGrid::isHouse(grid, x + current->getStepX(), y + current->getStepZ()))
        {
            traverseTurn(pieces, data);
            current = current->getClockWise();
            if (x != endX || y != endY || direction != current)
                traverseWallPiece(pieces, data);
        }
        else if (MansionGrid::isHouse(grid, x + current->getStepX() + current->getCounterClockWise()->getStepX(),
                                      y + current->getStepZ() + current->getCounterClockWise()->getStepZ()))
        {
            traverseInnerTurn(pieces, data);
            x += current->getStepX();
            y += current->getStepZ();
            current = current->getCounterClockWise();
        }
        else
        {
            x += current->getStepX();
            y += current->getStepZ();
            if (x != endX || y != endY || direction != current)
                traverseWallPiece(pieces, data);
        }
    } while (x != endX || y != endY || direction != current);
}

void WoodlandMansionPieces::MansionPiecePlacer::createRoof(std::vector<WoodlandMansionPiece*>& pieces,
                                                           const BlockPos& position, Rotation* rotation,
                                                           SimpleGrid* grid, SimpleGrid* upperGrid)
{
    for (int y = 0; y < grid->mHeight; ++y)
    {
        for (int x = 0; x < grid->mWidth; ++x)
        {
            BlockPos cell = position.relative(rotation->rotate(Direction::SOUTH), 8 + (y - mStartY) * 8);
            cell = cell.relative(rotation->rotate(Direction::EAST), (x - mStartX) * 8);
            bool covered = upperGrid != NULL && MansionGrid::isHouse(upperGrid, x, y);
            if (!MansionGrid::isHouse(grid, x, y) || covered)
                continue;

            pieces.push_back(new WoodlandMansionPiece(mStructureManager, eMinecraftStructureTemplate_Mansion_Roof,
                                                      cell.above(3), rotation));
            if (!MansionGrid::isHouse(grid, x + 1, y))
            {
                BlockPos front = cell.relative(rotation->rotate(Direction::EAST), 6);
                pieces.push_back(new WoodlandMansionPiece(
                    mStructureManager, eMinecraftStructureTemplate_Mansion_RoofFront, front, rotation));
            }
            if (!MansionGrid::isHouse(grid, x - 1, y))
            {
                BlockPos front = cell.relative(rotation->rotate(Direction::EAST), 0);
                front = front.relative(rotation->rotate(Direction::SOUTH), 7);
                pieces.push_back(new WoodlandMansionPiece(mStructureManager,
                                                          eMinecraftStructureTemplate_Mansion_RoofFront, front,
                                                          rotation->getRotated(Rotation::CLOCKWISE_180)));
            }
            if (!MansionGrid::isHouse(grid, x, y - 1))
            {
                BlockPos front = cell.relative(rotation->rotate(Direction::WEST), 1);
                pieces.push_back(new WoodlandMansionPiece(mStructureManager,
                                                          eMinecraftStructureTemplate_Mansion_RoofFront, front,
                                                          rotation->getRotated(Rotation::COUNTERCLOCKWISE_90)));
            }
            if (!MansionGrid::isHouse(grid, x, y + 1))
            {
                BlockPos front = cell.relative(rotation->rotate(Direction::EAST), 6);
                front = front.relative(rotation->rotate(Direction::SOUTH), 6);
                pieces.push_back(new WoodlandMansionPiece(mStructureManager,
                                                          eMinecraftStructureTemplate_Mansion_RoofFront, front,
                                                          rotation->getRotated(Rotation::CLOCKWISE_90)));
            }
        }
    }

    if (upperGrid != NULL)
    {
        for (int y = 0; y < grid->mHeight; ++y)
        {
            for (int x = 0; x < grid->mWidth; ++x)
            {
                BlockPos cell = position.relative(rotation->rotate(Direction::SOUTH), 8 + (y - mStartY) * 8);
                cell = cell.relative(rotation->rotate(Direction::EAST), (x - mStartX) * 8);
                bool covered = MansionGrid::isHouse(upperGrid, x, y);
                if (!MansionGrid::isHouse(grid, x, y) || !covered)
                    continue;

                if (!MansionGrid::isHouse(grid, x + 1, y))
                {
                    BlockPos wall = cell.relative(rotation->rotate(Direction::EAST), 7);
                    pieces.push_back(new WoodlandMansionPiece(
                        mStructureManager, eMinecraftStructureTemplate_Mansion_SmallWall, wall, rotation));
                }
                if (!MansionGrid::isHouse(grid, x - 1, y))
                {
                    BlockPos wall = cell.relative(rotation->rotate(Direction::WEST), 1);
                    wall = wall.relative(rotation->rotate(Direction::SOUTH), 6);
                    pieces.push_back(new WoodlandMansionPiece(mStructureManager,
                                                              eMinecraftStructureTemplate_Mansion_SmallWall, wall,
                                                              rotation->getRotated(Rotation::CLOCKWISE_180)));
                }
                if (!MansionGrid::isHouse(grid, x, y - 1))
                {
                    BlockPos wall = cell.relative(rotation->rotate(Direction::WEST), 0);
                    wall = wall.relative(rotation->rotate(Direction::NORTH), 1);
                    pieces.push_back(new WoodlandMansionPiece(mStructureManager,
                                                              eMinecraftStructureTemplate_Mansion_SmallWall, wall,
                                                              rotation->getRotated(Rotation::COUNTERCLOCKWISE_90)));
                }
                if (!MansionGrid::isHouse(grid, x, y + 1))
                {
                    BlockPos wall = cell.relative(rotation->rotate(Direction::EAST), 6);
                    wall = wall.relative(rotation->rotate(Direction::SOUTH), 7);
                    pieces.push_back(new WoodlandMansionPiece(mStructureManager,
                                                              eMinecraftStructureTemplate_Mansion_SmallWall, wall,
                                                              rotation->getRotated(Rotation::CLOCKWISE_90)));
                }
                if (!MansionGrid::isHouse(grid, x + 1, y))
                {
                    if (!MansionGrid::isHouse(grid, x, y - 1))
                    {
                        BlockPos corner = cell.relative(rotation->rotate(Direction::EAST), 7);
                        corner = corner.relative(rotation->rotate(Direction::NORTH), 2);
                        pieces.push_back(new WoodlandMansionPiece(
                            mStructureManager, eMinecraftStructureTemplate_Mansion_SmallWallCorner, corner, rotation));
                    }
                    if (!MansionGrid::isHouse(grid, x, y + 1))
                    {
                        BlockPos corner = cell.relative(rotation->rotate(Direction::EAST), 8);
                        corner = corner.relative(rotation->rotate(Direction::SOUTH), 7);
                        pieces.push_back(new WoodlandMansionPiece(
                            mStructureManager, eMinecraftStructureTemplate_Mansion_SmallWallCorner, corner,
                            rotation->getRotated(Rotation::CLOCKWISE_90)));
                    }
                }
                if (!MansionGrid::isHouse(grid, x - 1, y))
                {
                    if (!MansionGrid::isHouse(grid, x, y - 1))
                    {
                        BlockPos corner = cell.relative(rotation->rotate(Direction::WEST), 2);
                        corner = corner.relative(rotation->rotate(Direction::NORTH), 1);
                        pieces.push_back(new WoodlandMansionPiece(
                            mStructureManager, eMinecraftStructureTemplate_Mansion_SmallWallCorner, corner,
                            rotation->getRotated(Rotation::COUNTERCLOCKWISE_90)));
                    }
                    if (!MansionGrid::isHouse(grid, x, y + 1))
                    {
                        BlockPos corner = cell.relative(rotation->rotate(Direction::WEST), 1);
                        corner = corner.relative(rotation->rotate(Direction::SOUTH), 8);
                        pieces.push_back(new WoodlandMansionPiece(
                            mStructureManager, eMinecraftStructureTemplate_Mansion_SmallWallCorner, corner,
                            rotation->getRotated(Rotation::CLOCKWISE_180)));
                    }
                }
            }
        }
    }

    for (int y = 0; y < grid->mHeight; ++y)
    {
        for (int x = 0; x < grid->mWidth; ++x)
        {
            BlockPos cell = position.relative(rotation->rotate(Direction::SOUTH), 8 + (y - mStartY) * 8);
            cell = cell.relative(rotation->rotate(Direction::EAST), (x - mStartX) * 8);
            bool covered = upperGrid != NULL && MansionGrid::isHouse(upperGrid, x, y);
            if (!MansionGrid::isHouse(grid, x, y) || covered)
                continue;

            if (!MansionGrid::isHouse(grid, x + 1, y))
            {
                BlockPos edge = cell.relative(rotation->rotate(Direction::EAST), 6);
                if (!MansionGrid::isHouse(grid, x, y + 1))
                {
                    BlockPos corner = edge.relative(rotation->rotate(Direction::SOUTH), 6);
                    pieces.push_back(new WoodlandMansionPiece(
                        mStructureManager, eMinecraftStructureTemplate_Mansion_RoofCorner, corner, rotation));
                }
                else if (MansionGrid::isHouse(grid, x + 1, y + 1))
                {
                    BlockPos corner = edge.relative(rotation->rotate(Direction::SOUTH), 5);
                    pieces.push_back(new WoodlandMansionPiece(
                        mStructureManager, eMinecraftStructureTemplate_Mansion_RoofInnerCorner, corner, rotation));
                }
                if (!MansionGrid::isHouse(grid, x, y - 1))
                {
                    pieces.push_back(new WoodlandMansionPiece(mStructureManager,
                                                              eMinecraftStructureTemplate_Mansion_RoofCorner, edge,
                                                              rotation->getRotated(Rotation::COUNTERCLOCKWISE_90)));
                }
                else if (MansionGrid::isHouse(grid, x + 1, y - 1))
                {
                    BlockPos corner = cell.relative(rotation->rotate(Direction::EAST), 9);
                    corner = corner.relative(rotation->rotate(Direction::NORTH), 2);
                    pieces.push_back(new WoodlandMansionPiece(mStructureManager,
                                                              eMinecraftStructureTemplate_Mansion_RoofInnerCorner,
                                                              corner, rotation->getRotated(Rotation::CLOCKWISE_90)));
                }
            }
            if (!MansionGrid::isHouse(grid, x - 1, y))
            {
                BlockPos edge = cell.relative(rotation->rotate(Direction::EAST), 0);
                edge = edge.relative(rotation->rotate(Direction::SOUTH), 0);
                if (!MansionGrid::isHouse(grid, x, y + 1))
                {
                    BlockPos corner = edge.relative(rotation->rotate(Direction::SOUTH), 6);
                    pieces.push_back(new WoodlandMansionPiece(mStructureManager,
                                                              eMinecraftStructureTemplate_Mansion_RoofCorner, corner,
                                                              rotation->getRotated(Rotation::CLOCKWISE_90)));
                }
                else if (MansionGrid::isHouse(grid, x - 1, y + 1))
                {
                    BlockPos corner = edge.relative(rotation->rotate(Direction::SOUTH), 8);
                    corner = corner.relative(rotation->rotate(Direction::WEST), 3);
                    pieces.push_back(
                        new WoodlandMansionPiece(mStructureManager, eMinecraftStructureTemplate_Mansion_RoofInnerCorner,
                                                 corner, rotation->getRotated(Rotation::COUNTERCLOCKWISE_90)));
                }
                if (!MansionGrid::isHouse(grid, x, y - 1))
                {
                    pieces.push_back(new WoodlandMansionPiece(mStructureManager,
                                                              eMinecraftStructureTemplate_Mansion_RoofCorner, edge,
                                                              rotation->getRotated(Rotation::CLOCKWISE_180)));
                }
                else if (MansionGrid::isHouse(grid, x - 1, y - 1))
                {
                    BlockPos corner = edge.relative(rotation->rotate(Direction::SOUTH), 1);
                    pieces.push_back(new WoodlandMansionPiece(mStructureManager,
                                                              eMinecraftStructureTemplate_Mansion_RoofInnerCorner,
                                                              corner, rotation->getRotated(Rotation::CLOCKWISE_180)));
                }
            }
        }
    }
}

void WoodlandMansionPieces::MansionPiecePlacer::addRoom1x1(std::vector<WoodlandMansionPiece*>& pieces,
                                                           const BlockPos& position, Rotation* rotation,
                                                           const Direction* direction, FloorRoomCollection* rooms)
{
    Rotation* roomRotation = Rotation::NONE;
    eMinecraftStructureTemplate room = rooms->get1x1(mRandom);
    if (direction != Direction::EAST)
    {
        if (direction == Direction::NORTH)
            roomRotation = roomRotation->getRotated(Rotation::COUNTERCLOCKWISE_90);
        else if (direction == Direction::WEST)
            roomRotation = roomRotation->getRotated(Rotation::CLOCKWISE_180);
        else if (direction == Direction::SOUTH)
            roomRotation = roomRotation->getRotated(Rotation::CLOCKWISE_90);
        else
            room = rooms->get1x1Secret(mRandom);
    }

    BlockPos offset = StructureTemplate::getZeroPositionWithTransform(BlockPos(1, 0, 0), Mirror::NONE, roomRotation, 7,
                                                                      7);
    roomRotation = roomRotation->getRotated(rotation);
    offset = offset.rotate(rotation);
    BlockPos roomPosition = position.offset(offset.x, 0, offset.z);
    pieces.push_back(new WoodlandMansionPiece(mStructureManager, room, roomPosition, roomRotation));
}

void WoodlandMansionPieces::MansionPiecePlacer::addRoom1x2(std::vector<WoodlandMansionPiece*>& pieces,
                                                           const BlockPos& position, Rotation* rotation,
                                                           const Direction* frontDirection,
                                                           const Direction* sideDirection, FloorRoomCollection* rooms,
                                                           bool staircase)
{
    if (sideDirection == Direction::EAST && frontDirection == Direction::SOUTH)
    {
        BlockPos roomPosition = position.relative(rotation->rotate(Direction::EAST), 1);
        pieces.push_back(new WoodlandMansionPiece(mStructureManager, rooms->get1x2SideEntrance(mRandom, staircase),
                                                  roomPosition, rotation));
    }
    else if (sideDirection == Direction::EAST && frontDirection == Direction::NORTH)
    {
        BlockPos roomPosition = position.relative(rotation->rotate(Direction::EAST), 1);
        roomPosition = roomPosition.relative(rotation->rotate(Direction::SOUTH), 6);
        pieces.push_back(new WoodlandMansionPiece(mStructureManager, rooms->get1x2SideEntrance(mRandom, staircase),
                                                  roomPosition, rotation, Mirror::LEFT_RIGHT));
    }
    else if (sideDirection == Direction::WEST && frontDirection == Direction::NORTH)
    {
        BlockPos roomPosition = position.relative(rotation->rotate(Direction::EAST), 7);
        roomPosition = roomPosition.relative(rotation->rotate(Direction::SOUTH), 6);
        pieces.push_back(new WoodlandMansionPiece(mStructureManager, rooms->get1x2SideEntrance(mRandom, staircase),
                                                  roomPosition, rotation->getRotated(Rotation::CLOCKWISE_180)));
    }
    else if (sideDirection == Direction::WEST && frontDirection == Direction::SOUTH)
    {
        BlockPos roomPosition = position.relative(rotation->rotate(Direction::EAST), 7);
        pieces.push_back(new WoodlandMansionPiece(mStructureManager, rooms->get1x2SideEntrance(mRandom, staircase),
                                                  roomPosition, rotation, Mirror::FRONT_BACK));
    }
    else if (sideDirection == Direction::SOUTH && frontDirection == Direction::EAST)
    {
        BlockPos roomPosition = position.relative(rotation->rotate(Direction::EAST), 1);
        pieces.push_back(new WoodlandMansionPiece(mStructureManager, rooms->get1x2SideEntrance(mRandom, staircase),
                                                  roomPosition, rotation->getRotated(Rotation::CLOCKWISE_90),
                                                  Mirror::LEFT_RIGHT));
    }
    else if (sideDirection == Direction::SOUTH && frontDirection == Direction::WEST)
    {
        BlockPos roomPosition = position.relative(rotation->rotate(Direction::EAST), 7);
        pieces.push_back(new WoodlandMansionPiece(mStructureManager, rooms->get1x2SideEntrance(mRandom, staircase),
                                                  roomPosition, rotation->getRotated(Rotation::CLOCKWISE_90)));
    }
    else if (sideDirection == Direction::NORTH && frontDirection == Direction::WEST)
    {
        BlockPos roomPosition = position.relative(rotation->rotate(Direction::EAST), 7);
        roomPosition = roomPosition.relative(rotation->rotate(Direction::SOUTH), 6);
        pieces.push_back(new WoodlandMansionPiece(mStructureManager, rooms->get1x2SideEntrance(mRandom, staircase),
                                                  roomPosition, rotation->getRotated(Rotation::CLOCKWISE_90),
                                                  Mirror::FRONT_BACK));
    }
    else if (sideDirection == Direction::NORTH && frontDirection == Direction::EAST)
    {
        BlockPos roomPosition = position.relative(rotation->rotate(Direction::EAST), 1);
        roomPosition = roomPosition.relative(rotation->rotate(Direction::SOUTH), 6);
        pieces.push_back(new WoodlandMansionPiece(mStructureManager, rooms->get1x2SideEntrance(mRandom, staircase),
                                                  roomPosition, rotation->getRotated(Rotation::COUNTERCLOCKWISE_90)));
    }
    else if (sideDirection == Direction::SOUTH && frontDirection == Direction::NORTH)
    {
        BlockPos roomPosition = position.relative(rotation->rotate(Direction::EAST), 1);
        roomPosition = roomPosition.relative(rotation->rotate(Direction::NORTH), 8);
        pieces.push_back(new WoodlandMansionPiece(mStructureManager, rooms->get1x2FrontEntrance(mRandom, staircase),
                                                  roomPosition, rotation));
    }
    else if (sideDirection == Direction::NORTH && frontDirection == Direction::SOUTH)
    {
        BlockPos roomPosition = position.relative(rotation->rotate(Direction::EAST), 7);
        roomPosition = roomPosition.relative(rotation->rotate(Direction::SOUTH), 14);
        pieces.push_back(new WoodlandMansionPiece(mStructureManager, rooms->get1x2FrontEntrance(mRandom, staircase),
                                                  roomPosition, rotation->getRotated(Rotation::CLOCKWISE_180)));
    }
    else if (sideDirection == Direction::WEST && frontDirection == Direction::EAST)
    {
        BlockPos roomPosition = position.relative(rotation->rotate(Direction::EAST), 15);
        pieces.push_back(new WoodlandMansionPiece(mStructureManager, rooms->get1x2FrontEntrance(mRandom, staircase),
                                                  roomPosition, rotation->getRotated(Rotation::CLOCKWISE_90)));
    }
    else if (sideDirection == Direction::EAST && frontDirection == Direction::WEST)
    {
        BlockPos roomPosition = position.relative(rotation->rotate(Direction::WEST), 7);
        roomPosition = roomPosition.relative(rotation->rotate(Direction::SOUTH), 6);
        pieces.push_back(new WoodlandMansionPiece(mStructureManager, rooms->get1x2FrontEntrance(mRandom, staircase),
                                                  roomPosition, rotation->getRotated(Rotation::COUNTERCLOCKWISE_90)));
    }
    else if (sideDirection == Direction::UP && frontDirection == Direction::EAST)
    {
        BlockPos roomPosition = position.relative(rotation->rotate(Direction::EAST), 15);
        pieces.push_back(new WoodlandMansionPiece(mStructureManager, rooms->get1x2Secret(mRandom), roomPosition,
                                                  rotation->getRotated(Rotation::CLOCKWISE_90)));
    }
    else if (sideDirection == Direction::UP && frontDirection == Direction::SOUTH)
    {
        BlockPos roomPosition = position.relative(rotation->rotate(Direction::EAST), 1);
        roomPosition = roomPosition.relative(rotation->rotate(Direction::NORTH), 0);
        pieces.push_back(
            new WoodlandMansionPiece(mStructureManager, rooms->get1x2Secret(mRandom), roomPosition, rotation));
    }
}

void WoodlandMansionPieces::MansionPiecePlacer::addRoom2x2(std::vector<WoodlandMansionPiece*>& pieces,
                                                           BlockPos position, Rotation* rotation,
                                                           const Direction* frontDirection,
                                                           const Direction* sideDirection, FloorRoomCollection* rooms)
{
    int east = 0;
    int south = 0;
    Rotation* roomRotation = rotation;
    Mirror* mirror = Mirror::NONE;
    if (sideDirection == Direction::EAST && frontDirection == Direction::SOUTH)
    {
        east = -7;
    }
    else if (sideDirection == Direction::EAST && frontDirection == Direction::NORTH)
    {
        east = -7;
        south = 6;
        mirror = Mirror::LEFT_RIGHT;
    }
    else if (sideDirection == Direction::NORTH && frontDirection == Direction::EAST)
    {
        east = 1;
        south = 14;
        roomRotation = rotation->getRotated(Rotation::COUNTERCLOCKWISE_90);
    }
    else if (sideDirection == Direction::NORTH && frontDirection == Direction::WEST)
    {
        east = 7;
        south = 14;
        roomRotation = rotation->getRotated(Rotation::COUNTERCLOCKWISE_90);
        mirror = Mirror::LEFT_RIGHT;
    }
    else if (sideDirection == Direction::SOUTH && frontDirection == Direction::WEST)
    {
        east = 7;
        south = -8;
        roomRotation = rotation->getRotated(Rotation::CLOCKWISE_90);
    }
    else if (sideDirection == Direction::SOUTH && frontDirection == Direction::EAST)
    {
        east = 1;
        south = -8;
        roomRotation = rotation->getRotated(Rotation::CLOCKWISE_90);
        mirror = Mirror::LEFT_RIGHT;
    }
    else if (sideDirection == Direction::WEST && frontDirection == Direction::NORTH)
    {
        east = 15;
        south = 6;
        roomRotation = rotation->getRotated(Rotation::CLOCKWISE_180);
    }
    else if (sideDirection == Direction::WEST && frontDirection == Direction::SOUTH)
    {
        east = 15;
        mirror = Mirror::FRONT_BACK;
    }

    BlockPos roomPosition = position.relative(rotation->rotate(Direction::EAST), east);
    roomPosition = roomPosition.relative(rotation->rotate(Direction::SOUTH), south);
    pieces.push_back(
        new WoodlandMansionPiece(mStructureManager, rooms->get2x2(mRandom), roomPosition, roomRotation, mirror));
}

void WoodlandMansionPieces::MansionPiecePlacer::addRoom2x2Secret(std::vector<WoodlandMansionPiece*>& pieces,
                                                                 const BlockPos& position, Rotation* rotation,
                                                                 FloorRoomCollection* rooms)
{
    BlockPos roomPosition = position.relative(rotation->rotate(Direction::EAST), 1);
    pieces.push_back(new WoodlandMansionPiece(mStructureManager, rooms->get2x2Secret(mRandom), roomPosition, rotation,
                                              Mirror::NONE));
}

void WoodlandMansionPieces::MansionPiecePlacer::createMansion(const BlockPos& position, Rotation* rotation,
                                                              std::vector<WoodlandMansionPiece*>& pieces,
                                                              MansionGrid* grid)
{
    PlacementData groundFloor;
    groundFloor.position = position;
    groundFloor.rotation = rotation;
    groundFloor.wallType = eMinecraftStructureTemplate_Mansion_WallFlat;
    PlacementData secondFloor;
    entrance(pieces, groundFloor);
    secondFloor.position = groundFloor.position.above(8);
    secondFloor.rotation = groundFloor.rotation;
    secondFloor.wallType = eMinecraftStructureTemplate_Mansion_WallWindow;

    SimpleGrid* baseGrid = grid->mBaseGrid;
    SimpleGrid* thirdFloorGrid = grid->mThirdFloorGrid;
    mStartX = grid->mEntranceX + 1;
    mStartY = grid->mEntranceY + 1;
    int endX = grid->mEntranceX + 1;
    int endY = grid->mEntranceY;
    traverseOuterWalls(pieces, groundFloor, baseGrid, Direction::SOUTH, mStartX, mStartY, endX, endY);
    traverseOuterWalls(pieces, secondFloor, baseGrid, Direction::SOUTH, mStartX, mStartY, endX, endY);

    PlacementData thirdFloor;
    thirdFloor.position = groundFloor.position.above(19);
    thirdFloor.rotation = groundFloor.rotation;
    thirdFloor.wallType = eMinecraftStructureTemplate_Mansion_WallWindow;
    bool placedThirdFloor = false;
    for (int y = 0; y < thirdFloorGrid->mHeight && !placedThirdFloor; ++y)
    {
        for (int x = thirdFloorGrid->mWidth - 1; x >= 0 && !placedThirdFloor; --x)
        {
            if (MansionGrid::isHouse(thirdFloorGrid, x, y))
            {
                thirdFloor.position = thirdFloor.position.relative(rotation->rotate(Direction::SOUTH),
                                                                   8 + (y - mStartY) * 8);
                thirdFloor.position = thirdFloor.position.relative(rotation->rotate(Direction::EAST),
                                                                   (x - mStartX) * 8);
                traverseWallPiece(pieces, thirdFloor);
                traverseOuterWalls(pieces, thirdFloor, thirdFloorGrid, Direction::SOUTH, x, y, x, y);
                placedThirdFloor = true;
            }
        }
    }

    createRoof(pieces, position.above(16), rotation, baseGrid, thirdFloorGrid);
    createRoof(pieces, position.above(27), rotation, thirdFloorGrid, NULL);

    FloorRoomCollection* floorRooms[3] = {new FirstFloorRoomCollection(), new SecondFloorRoomCollection(),
                                          new ThirdFloorRoomCollection()};

    for (int floor = 0; floor < 3; ++floor)
    {
        BlockPos floorPosition = position.above(8 * floor + (floor == 2 ? 3 : 0));
        SimpleGrid* rooms = grid->mFloorRooms.data[floor];
        SimpleGrid* floorGrid = floor == 2 ? thirdFloorGrid : baseGrid;
        eMinecraftStructureTemplate corridorFloor = floor == 0 ? eMinecraftStructureTemplate_Mansion_CorridorFloor
                                                               : eMinecraftStructureTemplate_Mansion_CorridorFloor2;
        eMinecraftStructureTemplate carpetSouth = floor == 0 ? eMinecraftStructureTemplate_Mansion_CarpetSouth
                                                             : eMinecraftStructureTemplate_Mansion_CarpetSouth2;
        eMinecraftStructureTemplate carpetWest = floor == 0 ? eMinecraftStructureTemplate_Mansion_CarpetWest
                                                            : eMinecraftStructureTemplate_Mansion_CarpetWest2;

        for (int y = 0; y < floorGrid->mHeight; ++y)
        {
            for (int x = 0; x < floorGrid->mWidth; ++x)
            {
                if (floorGrid->get(x, y) != 1)
                    continue;

                BlockPos corridor = floorPosition.relative(rotation->rotate(Direction::SOUTH), 8 + (y - mStartY) * 8);
                corridor = corridor.relative(rotation->rotate(Direction::EAST), (x - mStartX) * 8);
                pieces.push_back(new WoodlandMansionPiece(mStructureManager, corridorFloor, corridor, rotation));

                if (floorGrid->get(x, y - 1) == 1 || (rooms->get(x, y - 1) & 0x800000) != 0)
                {
                    pieces.push_back(new WoodlandMansionPiece(
                        mStructureManager, eMinecraftStructureTemplate_Mansion_CarpetNorth,
                        corridor.relative(rotation->rotate(Direction::EAST), 1).above(), rotation));
                }
                if (floorGrid->get(x + 1, y) == 1 || (rooms->get(x + 1, y) & 0x800000) != 0)
                {
                    BlockPos carpet = corridor.relative(rotation->rotate(Direction::SOUTH), 1);
                    carpet = carpet.relative(rotation->rotate(Direction::EAST), 5);
                    pieces.push_back(new WoodlandMansionPiece(
                        mStructureManager, eMinecraftStructureTemplate_Mansion_CarpetEast, carpet.above(), rotation));
                }
                if (floorGrid->get(x, y + 1) == 1 || (rooms->get(x, y + 1) & 0x800000) != 0)
                {
                    BlockPos carpet = corridor.relative(rotation->rotate(Direction::SOUTH), 5);
                    carpet = carpet.relative(rotation->rotate(Direction::WEST), 1);
                    pieces.push_back(new WoodlandMansionPiece(mStructureManager, carpetSouth, carpet, rotation));
                }
                if (floorGrid->get(x - 1, y) == 1 || (rooms->get(x - 1, y) & 0x800000) != 0)
                {
                    BlockPos carpet = corridor.relative(rotation->rotate(Direction::WEST), 1);
                    carpet = carpet.relative(rotation->rotate(Direction::NORTH), 1);
                    pieces.push_back(new WoodlandMansionPiece(mStructureManager, carpetWest, carpet, rotation));
                }
            }
        }

        eMinecraftStructureTemplate indoorsWall = floor == 0 ? eMinecraftStructureTemplate_Mansion_IndoorsWall
                                                             : eMinecraftStructureTemplate_Mansion_IndoorsWall2;
        eMinecraftStructureTemplate indoorsDoor = floor == 0 ? eMinecraftStructureTemplate_Mansion_IndoorsDoor
                                                             : eMinecraftStructureTemplate_Mansion_IndoorsDoor2;
        std::vector<const Direction*> doorDirections;
        for (int y = 0; y < floorGrid->mHeight; ++y)
        {
            for (int x = 0; x < floorGrid->mWidth; ++x)
            {
                bool staircaseLanding = floor == 2 && floorGrid->get(x, y) == 3;
                if (floorGrid->get(x, y) != 2 && !staircaseLanding)
                    continue;

                int value = rooms->get(x, y);
                int type = value & 0xf0000;
                int roomId = value & 0xffff;
                staircaseLanding = staircaseLanding && (value & 0x800000) != 0;
                doorDirections.clear();
                if ((value & 0x200000) != 0)
                {
                    arrayWithLength<const Direction*> faces = Direction::Plane::HORIZONTAL->iterator();
                    for (unsigned int i = 0; i < faces.length; ++i)
                    {
                        const Direction* face = faces.data[i];
                        if (floorGrid->get(x + face->getStepX(), y + face->getStepZ()) == 1)
                            doorDirections.push_back(face);
                    }
                }

                const Direction* door = NULL;
                if (!doorDirections.empty())
                    door = doorDirections.at(mRandom.nextInt((int)doorDirections.size()));
                else if ((value & 0x100000) != 0)
                    door = Direction::UP;

                BlockPos room = floorPosition.relative(rotation->rotate(Direction::SOUTH), 8 + (y - mStartY) * 8);
                room = room.relative(rotation->rotate(Direction::EAST), -1 + (x - mStartX) * 8);

                if (MansionGrid::isHouse(floorGrid, x - 1, y) && !grid->isRoomId(floorGrid, x - 1, y, floor, roomId))
                {
                    pieces.push_back(new WoodlandMansionPiece(
                        mStructureManager, door == Direction::WEST ? indoorsDoor : indoorsWall, room, rotation));
                }
                if (floorGrid->get(x + 1, y) == 1 && !staircaseLanding)
                {
                    BlockPos wall = room.relative(rotation->rotate(Direction::EAST), 8);
                    pieces.push_back(new WoodlandMansionPiece(
                        mStructureManager, door == Direction::EAST ? indoorsDoor : indoorsWall, wall, rotation));
                }
                if (MansionGrid::isHouse(floorGrid, x, y + 1) && !grid->isRoomId(floorGrid, x, y + 1, floor, roomId))
                {
                    BlockPos wall = room.relative(rotation->rotate(Direction::SOUTH), 7);
                    wall = wall.relative(rotation->rotate(Direction::EAST), 7);
                    pieces.push_back(new WoodlandMansionPiece(mStructureManager,
                                                              door == Direction::SOUTH ? indoorsDoor : indoorsWall,
                                                              wall, rotation->getRotated(Rotation::CLOCKWISE_90)));
                }
                if (floorGrid->get(x, y - 1) == 1 && !staircaseLanding)
                {
                    BlockPos wall = room.relative(rotation->rotate(Direction::NORTH), 1);
                    wall = wall.relative(rotation->rotate(Direction::EAST), 7);
                    pieces.push_back(new WoodlandMansionPiece(mStructureManager,
                                                              door == Direction::NORTH ? indoorsDoor : indoorsWall,
                                                              wall, rotation->getRotated(Rotation::CLOCKWISE_90)));
                }

                if (type == 0x10000)
                {
                    addRoom1x1(pieces, room, rotation, door, floorRooms[floor]);
                }
                else if (type == 0x20000 && door != NULL)
                {
                    const Direction* roomDirection = grid->get1x2RoomDirection(floorGrid, x, y, floor, roomId);
                    bool staircase = (value & 0x400000) != 0;
                    addRoom1x2(pieces, room, rotation, roomDirection, door, floorRooms[floor], staircase);
                }
                else if (type == 0x40000 && door != NULL && door != Direction::UP)
                {
                    const Direction* roomDirection = door->getClockWise();
                    if (!grid->isRoomId(floorGrid, x + roomDirection->getStepX(), y + roomDirection->getStepZ(), floor,
                                        roomId))
                        roomDirection = roomDirection->getOpposite();
                    addRoom2x2(pieces, room, rotation, roomDirection, door, floorRooms[floor]);
                }
                else if (type == 0x40000 && door == Direction::UP)
                {
                    addRoom2x2Secret(pieces, room, rotation, floorRooms[floor]);
                }
            }
        }
    }

    for (int floor = 0; floor < 3; ++floor)
        delete floorRooms[floor];
}

void WoodlandMansionPieces::generateMansion(StructureManager* structureManager, const BlockPos& position,
                                            Rotation* rotation, std::vector<WoodlandMansionPiece*>& pieces,
                                            Random& random)
{
    MansionGrid grid(random);
    MansionPiecePlacer placer(structureManager, random);
    placer.createMansion(position, rotation, pieces, &grid);
}

void WoodlandMansionPieces::loadStatic()
{
    StructureFeatureIO::setPieceId(eStructurePiece_WoodlandMansionPiece, &WoodlandMansionPiece::create, L"WMP");
}
