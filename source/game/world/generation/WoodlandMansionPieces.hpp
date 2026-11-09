#ifndef GAME_WORLD_GENERATION_WOODLANDMANSIONPIECES_HPP
#define GAME_WORLD_GENERATION_WOODLANDMANSIONPIECES_HPP

#include <string>
#include <vector>

#include "game/world/block/BlockPos.hpp"
#include "game/core/Direction.hpp"
#include "game/core/Mirror.hpp"
#include "game/core/Rotation.hpp"
#include "game/util/Random.hpp"
#include "game/world/biome/arrayWithLength.hpp"
#include "game/world/chunk/array2DWithLength.hpp"
#include "game/world/generation/StructureManager.hpp"
#include "game/world/generation/TemplateStructurePiece.hpp"

class BoundingBox;
class DataInputStream;
class DataOutputStream;
class Level;

class WoodlandMansionPieces
{
public:
    class SimpleGrid
    {
    public:
        SimpleGrid(int width, int height, int value);
        ~SimpleGrid();
        void set(int x, int y, int value);
        void set(int x0, int y0, int x1, int y1, int value);
        int get(int x, int y);
        void setIf(int x, int y, int oldValue, int newValue);
        bool edgesTo(int x, int y, int value);
        array2DWithLength<int> mGrid;
        int mWidth;
        int mHeight;
        int mValueIfOutside;
    };

    class FloorRoomCollection
    {
    public:
        virtual ~FloorRoomCollection();
        virtual eMinecraftStructureTemplate get1x1(Random& random) = 0;
        virtual eMinecraftStructureTemplate get1x1Secret(Random& random) = 0;
        virtual eMinecraftStructureTemplate get1x2SideEntrance(Random& random, bool staircase) = 0;
        virtual eMinecraftStructureTemplate get1x2FrontEntrance(Random& random, bool staircase) = 0;
        virtual eMinecraftStructureTemplate get1x2Secret(Random& random) = 0;
        virtual eMinecraftStructureTemplate get2x2(Random& random) = 0;
        virtual eMinecraftStructureTemplate get2x2Secret(Random& random) = 0;
    };

    class FirstFloorRoomCollection : public FloorRoomCollection
    {
    public:
        virtual ~FirstFloorRoomCollection();
        virtual eMinecraftStructureTemplate get1x1(Random& random);
        virtual eMinecraftStructureTemplate get1x1Secret(Random& random);
        virtual eMinecraftStructureTemplate get1x2SideEntrance(Random& random, bool staircase);
        virtual eMinecraftStructureTemplate get1x2FrontEntrance(Random& random, bool staircase);
        virtual eMinecraftStructureTemplate get1x2Secret(Random& random);
        virtual eMinecraftStructureTemplate get2x2(Random& random);
        virtual eMinecraftStructureTemplate get2x2Secret(Random& random);
    };

    class SecondFloorRoomCollection : public FloorRoomCollection
    {
    public:
        virtual ~SecondFloorRoomCollection();
        virtual eMinecraftStructureTemplate get1x1(Random& random);
        virtual eMinecraftStructureTemplate get1x1Secret(Random& random);
        virtual eMinecraftStructureTemplate get1x2SideEntrance(Random& random, bool staircase);
        virtual eMinecraftStructureTemplate get1x2FrontEntrance(Random& random, bool staircase);
        virtual eMinecraftStructureTemplate get1x2Secret(Random& random);
        virtual eMinecraftStructureTemplate get2x2(Random& random);
        virtual eMinecraftStructureTemplate get2x2Secret(Random& random);
    };

    class ThirdFloorRoomCollection : public SecondFloorRoomCollection
    {
    public:
        virtual ~ThirdFloorRoomCollection();
    };

    class MansionGrid : public Random
    {
    public:
        explicit MansionGrid(Random& random);
        ~MansionGrid();
        void recursiveCorridor(SimpleGrid* grid, int x, int y, const Direction* direction, int length);
        static bool isHouse(SimpleGrid* grid, int x, int y);
        bool cleanEdges(SimpleGrid* grid);
        void identifyRooms(SimpleGrid* grid, SimpleGrid* rooms);
        bool isRoomId(SimpleGrid* grid, int x, int y, int floor, int roomId);
        const Direction* get1x2RoomDirection(SimpleGrid* grid, int x, int y, int floor, int roomId);
        void setupThirdFloor();
        void print();
        SimpleGrid* mBaseGrid;
        SimpleGrid* mThirdFloorGrid;
        arrayWithLength<SimpleGrid*> mFloorRooms;
        int mEntranceX;
        int mEntranceY;
    };

    struct PlacementData
    {
        Rotation* rotation;
        BlockPos position;
        eMinecraftStructureTemplate wallType;
    };

    class WoodlandMansionPiece : public TemplateStructurePiece
    {
    public:
        WoodlandMansionPiece(StructureManager* manager, eMinecraftStructureTemplate type, const BlockPos& position,
                             Rotation* rotation);
        WoodlandMansionPiece(StructureManager* manager, eMinecraftStructureTemplate type, const BlockPos& position,
                             Rotation* rotation, Mirror* mirror);
        WoodlandMansionPiece();
        void loadTemplate(StructureManager* manager);
        void _init(StructureManager* manager, eMinecraftStructureTemplate type, const BlockPos& position,
                   Rotation* rotation, Mirror* mirror);
        virtual void addAdditionalSaveData(DataOutputStream* output);
        virtual void readAdditionalSaveData(DataInputStream* input, StructureManager* manager);
        virtual void handleDataMarker(const std::wstring& marker, const BlockPos& position, Level* level,
                                      Random& random, BoundingBox* box);
        static StructurePiece* create();
        virtual EStructurePiece GetType();
        eMinecraftStructureTemplate mTemplateType;
        Rotation* mRotation;
        Mirror* mMirror;
    };

    class MansionPiecePlacer
    {
    public:
        MansionPiecePlacer(StructureManager* manager, Random& random);
        void entrance(std::vector<WoodlandMansionPiece*>& pieces, PlacementData& data);
        void traverseTurn(std::vector<WoodlandMansionPiece*>& pieces, PlacementData& data);
        void traverseWallPiece(std::vector<WoodlandMansionPiece*>& pieces, PlacementData& data);
        void traverseInnerTurn(std::vector<WoodlandMansionPiece*>& pieces, PlacementData& data);
        void traverseOuterWalls(std::vector<WoodlandMansionPiece*>& pieces, PlacementData& data, SimpleGrid* grid,
                                const Direction* direction, int startX, int startY, int endX, int endY);
        void createRoof(std::vector<WoodlandMansionPiece*>& pieces, const BlockPos& position, Rotation* rotation,
                        SimpleGrid* grid, SimpleGrid* upperGrid);
        void addRoom1x1(std::vector<WoodlandMansionPiece*>& pieces, const BlockPos& position, Rotation* rotation,
                        const Direction* direction, FloorRoomCollection* rooms);
        void addRoom1x2(std::vector<WoodlandMansionPiece*>& pieces, const BlockPos& position, Rotation* rotation,
                        const Direction* frontDirection, const Direction* sideDirection, FloorRoomCollection* rooms,
                        bool staircase);
        void addRoom2x2(std::vector<WoodlandMansionPiece*>& pieces, BlockPos position, Rotation* rotation,
                        const Direction* frontDirection, const Direction* sideDirection, FloorRoomCollection* rooms);
        void addRoom2x2Secret(std::vector<WoodlandMansionPiece*>& pieces, const BlockPos& position, Rotation* rotation,
                              FloorRoomCollection* rooms);
        void createMansion(const BlockPos& position, Rotation* rotation, std::vector<WoodlandMansionPiece*>& pieces,
                           MansionGrid* grid);
        StructureManager* mStructureManager;
        Random mRandom;
        int mStartX;
        int mStartY;
    };

    static void generateMansion(StructureManager* manager, const BlockPos& position, Rotation* rotation,
                                std::vector<WoodlandMansionPiece*>& pieces, Random& random);
    static void loadStatic();
    static void main(std::basic_string<wchar_t> args);
};

#endif
