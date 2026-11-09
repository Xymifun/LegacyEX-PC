#include "game/core/EndCityPieces.hpp"
#include "game/core/EntityType.hpp"

#include <boost/shared_ptr.hpp>
#include <boost/unordered_map.hpp>

#include "game/core/BoundingBox.hpp"
#include "game/core/BuiltInLootTables.hpp"
#include "game/core/DataInputStream.hpp"
#include "game/core/DataOutputStream.hpp"
#include "game/core/Direction.hpp"
#include "game/core/Rotation.hpp"
#include "game/core/Shulker.hpp"
#include "game/item/ItemFrame.hpp"
#include "game/item/ItemInstance.hpp"
#include "game/item/Items.hpp"
#include "game/util/Random.hpp"
#include "game/world/Level.hpp"
#include "game/world/block/BlockEntity.hpp"
#include "game/world/block/BlockPos.hpp"
#include "game/world/block/Blocks.hpp"
#include "game/world/block/ChestBlockEntity.hpp"
#include "game/world/generation/StructureFeatureIO.hpp"
#include "game/world/generation/StructureManager.hpp"
#include "game/world/generation/StructurePiece.hpp"
#include "game/world/generation/StructurePlaceSettings.hpp"
#include "game/world/generation/StructureTemplate.hpp"

static boost::unordered_map<Rotation*, BlockPos> gTowerBridges;
static boost::unordered_map<Rotation*, BlockPos> gFatTowerBridges;
static StructurePlaceSettings* gIgnoreEntitiesSettings = NULL;
static StructurePlaceSettings* gIgnoreEntitiesAndBlocksSettings = NULL;
static EndCityPieces::HouseTowerSectionGenerator* gHouseGenerator;
static EndCityPieces::TowerSectionGenerator* gTowerGenerator;
static EndCityPieces::TowerBridgeSectionGenerator* gTowerBridgeGenerator;
static EndCityPieces::FatTowerSectionGenerator* gFatTowerGenerator;

EndCityPieces::EndCityPiece* EndCityPieces::addHelper(std::vector<StructurePiece*>& pieces,
                                                      EndCityPieces::EndCityPiece* piece)
{
    pieces.push_back(piece);
    return piece;
}

EndCityPieces::EndCityPiece* EndCityPieces::addPiece(StructureManager* structureManager,
                                                     EndCityPieces::EndCityPiece* previous, const BlockPos& position,
                                                     eMinecraftStructureTemplate templateType, Rotation* rotation,
                                                     bool overwrite)
{
    BlockPos previousPosition = previous->getTemplatePosition();
    EndCityPiece* piece = new EndCityPiece(structureManager, templateType, previousPosition, rotation, overwrite);
    StructureTemplate* previousTemplate = previous->getStructureTemplate();
    StructurePlaceSettings* previousSettings = previous->getPlaceSettings();
    StructurePlaceSettings* pieceSettings = piece->getPlaceSettings();
    BlockPos connected = previousTemplate->calculateConnectedPosition(previousSettings, position, pieceSettings,
                                                                      BlockPos::ZERO);
    piece->move(connected.x, connected.y, connected.z);
    return piece;
}

bool EndCityPieces::recursiveChildren(StructureManager* structureManager, SectionGenerator* generator, int genDepth,
                                      EndCityPiece* previous, const BlockPos& position,
                                      std::vector<StructurePiece*>& pieces, Random& random)
{
    if (genDepth > 8)
        return false;
    std::vector<StructurePiece*> children;
    if (!generator->generate(structureManager, genDepth, previous, position, children, random))
        return false;
    int generationDepth = random.nextInt();
    std::vector<StructurePiece*>::iterator it;
    for (it = children.begin(); it != children.end(); ++it)
    {
        (*it)->setGenDepth(generationDepth);
        StructurePiece* collision = StructurePiece::findCollisionPiece(&pieces, (*it)->getBoundingBox());
        if (collision != NULL && collision->getGenDepth() != previous->getGenDepth())
            return false;
    }
    pieces.insert(pieces.end(), children.begin(), children.end());
    return true;
}

void EndCityPieces::startHouseTower(StructureManager* structureManager, const BlockPos& position, Rotation* rotation,
                                    std::vector<StructurePiece*>& pieces, Random& random)
{
    gFatTowerGenerator->init();
    gHouseGenerator->init();
    gTowerBridgeGenerator->init();
    gTowerGenerator->init();
    EndCityPiece* piece = addHelper(
        pieces,
        new EndCityPiece(structureManager, eMinecraftStructureTemplate_EndCity_BaseFloor, position, rotation, true));
    piece = addHelper(pieces, addPiece(structureManager, piece, BlockPos(-1, 0, -1),
                                       eMinecraftStructureTemplate_EndCity_SecondFloor, rotation, false));
    piece = addHelper(pieces, addPiece(structureManager, piece, BlockPos(-1, 4, -1),
                                       eMinecraftStructureTemplate_EndCity_ThirdFloor, rotation, false));
    piece = addHelper(pieces, addPiece(structureManager, piece, BlockPos(-1, 8, -1),
                                       eMinecraftStructureTemplate_EndCity_ThirdRoof, rotation, true));
    recursiveChildren(structureManager, gTowerGenerator, 1, piece, BlockPos(), pieces, random);
}

boost::unordered_map<Rotation*, BlockPos> EndCityPieces::createTowerBridgesMap()
{
    boost::unordered_map<Rotation*, BlockPos> bridges;
    bridges[Rotation::NONE] = BlockPos(1, -1, 0);
    bridges[Rotation::CLOCKWISE_90] = BlockPos(6, -1, 1);
    bridges[Rotation::COUNTERCLOCKWISE_90] = BlockPos(0, -1, 5);
    bridges[Rotation::CLOCKWISE_180] = BlockPos(5, -1, 6);
    return bridges;
}

boost::unordered_map<Rotation*, BlockPos> EndCityPieces::createFatTowerBridgesMap()
{
    boost::unordered_map<Rotation*, BlockPos> bridges;
    bridges[Rotation::NONE] = BlockPos(4, -1, 0);
    bridges[Rotation::CLOCKWISE_90] = BlockPos(12, -1, 4);
    bridges[Rotation::COUNTERCLOCKWISE_90] = BlockPos(0, -1, 8);
    bridges[Rotation::CLOCKWISE_180] = BlockPos(8, -1, 12);
    return bridges;
}

void EndCityPieces::loadStatic()
{
    StructureFeatureIO::setPieceId(eStructurePiece_EndCityPiece, &EndCityPiece::Create, L"ECP");
    gTowerBridges = createTowerBridgesMap();
    gFatTowerBridges = createFatTowerBridgesMap();
    gIgnoreEntitiesSettings = (new StructurePlaceSettings())->setIgnoreEntities(true);
    gIgnoreEntitiesAndBlocksSettings =
        (new StructurePlaceSettings())->setIgnoreEntities(true)->setIgnoreBlock(Blocks::air);
    gHouseGenerator = new HouseTowerSectionGenerator();
    gTowerGenerator = new TowerSectionGenerator();
    gTowerBridgeGenerator = new TowerBridgeSectionGenerator();
    gFatTowerGenerator = new FatTowerSectionGenerator();
}

EndCityPieces::EndCityPiece::EndCityPiece(StructureManager* structureManager, eMinecraftStructureTemplate templateType,
                                          const BlockPos& position, Rotation* rotation, bool overwrite)
    : TemplateStructurePiece(0),
      mTemplateType(templateType)
{
    mTemplatePosition = position;
    mRotation = rotation;
    mOverwrite = overwrite;
    loadTemplate(structureManager);
}

EndCityPieces::EndCityPiece::EndCityPiece()
    : TemplateStructurePiece(),
      mTemplateType(eMinecraftStructureTemplate_Count),
      mRotation(Rotation::NONE),
      mOverwrite(false)
{
}

void EndCityPieces::EndCityPiece::loadTemplate(StructureManager* structureManager)
{
    const StructureTemplate* structure = structureManager->getOrCreate(mTemplateType);
    if (mOverwrite)
        setup(structure, mTemplatePosition, gIgnoreEntitiesSettings->copy()->setRotation(mRotation));
    else
        setup(structure, mTemplatePosition, gIgnoreEntitiesAndBlocksSettings->copy()->setRotation(mRotation));
}

void EndCityPieces::EndCityPiece::addAdditionalSaveData(DataOutputStream* output)
{
    TemplateStructurePiece::addAdditionalSaveData(output);
}

void EndCityPieces::EndCityPiece::readAdditionalSaveData(DataInputStream* input, StructureManager* structureManager)
{
    TemplateStructurePiece::readAdditionalSaveData(input, structureManager);
}

void EndCityPieces::EndCityPiece::handleDataMarker(const std::wstring& marker, const BlockPos& position, Level* level,
                                                   Random& random, BoundingBox* box)
{
    std::wstring chestMarker(L"Chest");
    std::wstring sentryMarker(L"Sentry");
    std::wstring elytraMarker(L"Elytra");
    if (marker.compare(0, chestMarker.length(), chestMarker) == 0)
    {
        BlockPos below = position.below();
        if (box->isInside(below))
        {
            boost::shared_ptr<BlockEntity> blockEntity = level->getBlockEntity(below);
            if (blockEntity)
            {
                bool isChest = eTYPE_DERIVED_FROM(BlockEntityType::CHEST, blockEntity->GetType());
                if (isChest)
                {
                    boost::shared_ptr<ChestBlockEntity> chest = boost::static_pointer_cast<ChestBlockEntity>(
                        blockEntity);
                    chest->setLootTable(&BuiltInLootTables::END_CITY_TREASURE, random.nextLong());
                }
            }
        }
    }
    else if (marker.compare(0, sentryMarker.length(), sentryMarker) == 0)
    {
        boost::shared_ptr<Shulker> shulker(new Shulker(level));
        shulker->setPos(position.x + 0.5, position.y + 0.5, position.z + 0.5);
        shulker->setAttachPosition(new BlockPos(position));
        level->addEntity(shulker);
    }
    else if (marker.compare(0, elytraMarker.length(), elytraMarker) == 0)
    {
        boost::shared_ptr<ItemFrame> frame(new ItemFrame(level, position, mRotation->rotate(Direction::SOUTH)));
        frame->setItem(not_null_ptr<ItemInstance>(new ItemInstance(Items::elytra)));
        level->addEntity(frame);
    }
}

StructurePiece* EndCityPieces::EndCityPiece::Create()
{
    return new EndCityPiece();
}

EStructurePiece EndCityPieces::EndCityPiece::GetType()
{
    return eStructurePiece_EndCityPiece;
}

void EndCityPieces::HouseTowerSectionGenerator::init()
{
    int i;
    for (i = 0; i < 8; ++i)
        mGenerated[i] = false;
}

bool EndCityPieces::HouseTowerSectionGenerator::generate(StructureManager* structureManager, int genDepth,
                                                         EndCityPiece* previous, const BlockPos& position,
                                                         std::vector<StructurePiece*>& pieces, Random& random)
{
    if (genDepth > 8)
        return false;
    Rotation* rotation = previous->getPlaceSettings()->getRotation();
    EndCityPiece* piece = addHelper(pieces, addPiece(structureManager, previous, position,
                                                     eMinecraftStructureTemplate_EndCity_BaseFloor, rotation, true));
    int roof = random.nextInt(3);
    if (!mGenerated[genDepth])
    {
        mGenerated[genDepth] = true;
        roof = 2;
    }
    if (roof == 0)
    {
        addHelper(pieces, addPiece(structureManager, piece, BlockPos(-1, 4, -1),
                                   eMinecraftStructureTemplate_EndCity_BaseRoof, rotation, true));
    }
    else if (roof == 1)
    {
        piece = addHelper(pieces, addPiece(structureManager, piece, BlockPos(-1, 0, -1),
                                           eMinecraftStructureTemplate_EndCity_SecondFloor2, rotation, false));
        piece = addHelper(pieces, addPiece(structureManager, piece, BlockPos(-1, 8, -1),
                                           eMinecraftStructureTemplate_EndCity_SecondRoof, rotation, false));
        recursiveChildren(structureManager, gTowerGenerator, genDepth + 1, piece, BlockPos(), pieces, random);
    }
    else if (roof == 2)
    {
        piece = addHelper(pieces, addPiece(structureManager, piece, BlockPos(-1, 0, -1),
                                           eMinecraftStructureTemplate_EndCity_SecondFloor2, rotation, false));
        piece = addHelper(pieces, addPiece(structureManager, piece, BlockPos(-1, 4, -1),
                                           eMinecraftStructureTemplate_EndCity_ThirdFloorC, rotation, false));
        piece = addHelper(pieces, addPiece(structureManager, piece, BlockPos(-1, 8, -1),
                                           eMinecraftStructureTemplate_EndCity_ThirdRoof, rotation, true));
        recursiveChildren(structureManager, gTowerGenerator, genDepth + 1, piece, BlockPos(), pieces, random);
    }
    return true;
}

void EndCityPieces::FatTowerSectionGenerator::init()
{
}

bool EndCityPieces::FatTowerSectionGenerator::generate(StructureManager* structureManager, int genDepth,
                                                       EndCityPiece* previous, const BlockPos&,
                                                       std::vector<StructurePiece*>& pieces, Random& random)
{
    Rotation* rotation = previous->getPlaceSettings()->getRotation();
    EndCityPiece* piece = addHelper(pieces, addPiece(structureManager, previous, BlockPos(-3, 4, -3),
                                                     eMinecraftStructureTemplate_EndCity_FatTowerBase, rotation, true));
    piece = addHelper(pieces, addPiece(structureManager, piece, BlockPos(0, 4, 0),
                                       eMinecraftStructureTemplate_EndCity_FatTowerMiddle, rotation, true));
    for (int floor = 0; floor < 2; ++floor)
    {
        piece = addHelper(pieces, addPiece(structureManager, piece, BlockPos(0, 8, 0),
                                           eMinecraftStructureTemplate_EndCity_FatTowerMiddle, rotation, true));
        boost::unordered_map<Rotation*, BlockPos>::iterator it;
        for (it = gFatTowerBridges.begin(); it != gFatTowerBridges.end(); ++it)
        {
            if (random.nextInt(10) > 2)
            {
                EndCityPiece* bridge = addHelper(pieces, addPiece(structureManager, piece, it->second,
                                                                  eMinecraftStructureTemplate_EndCity_BridgeEnd,
                                                                  rotation->getRotated(it->first), true));
                recursiveChildren(structureManager, gTowerBridgeGenerator, genDepth + 1, bridge, BlockPos(), pieces,
                                  random);
            }
        }
    }
    addHelper(pieces, addPiece(structureManager, piece, BlockPos(-2, 8, -2),
                               eMinecraftStructureTemplate_EndCity_FatTowerTop, rotation, true));
    return true;
}

EndCityPieces::TowerBridgeSectionGenerator::TowerBridgeSectionGenerator() : mGenerated(false)
{
}

void EndCityPieces::TowerBridgeSectionGenerator::init()
{
    mGenerated = false;
}

bool EndCityPieces::TowerBridgeSectionGenerator::generate(StructureManager* structureManager, int genDepth,
                                                          EndCityPiece* previous, const BlockPos&,
                                                          std::vector<StructurePiece*>& pieces, Random& random)
{
    Rotation* rotation = previous->getPlaceSettings()->getRotation();
    int count = random.nextInt(4) + 1;
    EndCityPiece* piece = addHelper(pieces, addPiece(structureManager, previous, BlockPos(0, 0, -4),
                                                     eMinecraftStructureTemplate_EndCity_BridgePiece, rotation, true));
    piece->setGenDepth(-1);
    int y = 0;
    for (int i = 0; i < count; ++i)
    {
        if (random.nextBoolean())
        {
            piece = addHelper(pieces, addPiece(structureManager, piece, BlockPos(0, y, -4),
                                               eMinecraftStructureTemplate_EndCity_BridgePiece, rotation, true));
            y = 0;
        }
        else
        {
            if (random.nextBoolean())
                piece = addHelper(pieces,
                                  addPiece(structureManager, piece, BlockPos(0, y, -4),
                                           eMinecraftStructureTemplate_EndCity_BridgeSteepStairs, rotation, true));
            else
                piece = addHelper(pieces,
                                  addPiece(structureManager, piece, BlockPos(0, y, -8),
                                           eMinecraftStructureTemplate_EndCity_BridgeGentleStairs, rotation, true));
            y = 4;
        }
    }
    if (!mGenerated)
    {
        addHelper(pieces, addPiece(structureManager, piece, BlockPos(random.nextInt(8) - 8, y, -48),
                                   eMinecraftStructureTemplate_EndCity_Ship, rotation, true));
        mGenerated = true;
    }
    else if (!recursiveChildren(structureManager, gHouseGenerator, genDepth + 1, piece, BlockPos(-3, y + 1, -11),
                                pieces, random))
    {
        return false;
    }
    piece = addHelper(pieces, addPiece(structureManager, piece, BlockPos(4, y, 0),
                                       eMinecraftStructureTemplate_EndCity_BridgeEnd,
                                       rotation->getRotated(Rotation::CLOCKWISE_180), true));
    piece->setGenDepth(-1);
    return true;
}

void EndCityPieces::TowerSectionGenerator::init()
{
}

bool EndCityPieces::TowerSectionGenerator::generate(StructureManager* structureManager, int genDepth,
                                                    EndCityPiece* previous, const BlockPos&,
                                                    std::vector<StructurePiece*>& pieces, Random& random)
{
    Rotation* rotation = previous->getPlaceSettings()->getRotation();
    EndCityPiece* piece = addHelper(pieces, addPiece(structureManager, previous, BlockPos(4, -3, 4),
                                                     eMinecraftStructureTemplate_EndCity_TowerBase, rotation, true));
    piece = addHelper(pieces, addPiece(structureManager, piece, BlockPos(0, 7, 0),
                                       eMinecraftStructureTemplate_EndCity_TowerPiece, rotation, true));
    EndCityPiece* bridgePiece = random.nextInt(3) == 0 ? piece : NULL;
    int extraFloors = random.nextInt(3);
    for (int i = 0; i < extraFloors + 1; ++i)
    {
        piece = addHelper(pieces, addPiece(structureManager, piece, BlockPos(0, 4, 0),
                                           eMinecraftStructureTemplate_EndCity_TowerPiece, rotation, true));
        if (i < extraFloors && random.nextInt(10) > 2)
            bridgePiece = piece;
    }
    if (genDepth == 1 && bridgePiece == NULL)
        bridgePiece = piece;

    if (bridgePiece == NULL)
    {
        if (genDepth == 7)
        {
            addHelper(pieces, addPiece(structureManager, piece, BlockPos(-1, 4, -1),
                                       eMinecraftStructureTemplate_EndCity_TowerTop, rotation, true));
            return true;
        }
        return recursiveChildren(structureManager, gFatTowerGenerator, genDepth + 1, piece, BlockPos(), pieces, random);
    }

    double bridgeWeight = 3 - genDepth > 0 ? static_cast<double>(3 - genDepth) : 0.0;
    double remaining = static_cast<double>(gTowerBridges.size());
    boost::unordered_map<Rotation*, BlockPos>::iterator it;
    for (it = gTowerBridges.begin(); it != gTowerBridges.end(); ++it)
    {
        if (random.nextFloat() < bridgeWeight / remaining || random.nextInt(10) > 2)
        {
            EndCityPiece* bridge = addHelper(pieces, addPiece(structureManager, bridgePiece, it->second,
                                                              eMinecraftStructureTemplate_EndCity_BridgeEnd,
                                                              rotation->getRotated(it->first), true));
            bridgeWeight -= recursiveChildren(structureManager, gTowerBridgeGenerator, genDepth + 1, bridge, BlockPos(),
                                              pieces, random)
                                ? 1.0
                                : 0.0;
        }
        remaining -= 1.0;
    }
    addHelper(pieces, addPiece(structureManager, piece, BlockPos(-1, 4, -1),
                               eMinecraftStructureTemplate_EndCity_TowerTop, rotation, true));
    return true;
}
