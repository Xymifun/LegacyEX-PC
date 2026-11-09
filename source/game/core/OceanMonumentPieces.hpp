#ifndef GAME_CORE_OCEANMONUMENTPIECES_HPP
#define GAME_CORE_OCEANMONUMENTPIECES_HPP

#include <vector>

#include <boost/enable_shared_from_this.hpp>
#include <boost/shared_ptr.hpp>
#include <boost/weak_ptr.hpp>

#include "game/world/generation/StructurePiece.hpp"

class BlockState;
class BoundingBox;
class DataInputStream;
class DataOutputStream;
class Direction;
class Level;
class Random;
class StructureManager;

class OceanMonumentPieces
{
public:
    class RoomDefinition;
    class OceanMonumentPiece;

    OceanMonumentPieces();
    static void loadStatic();

    class RoomDefinition : public boost::enable_shared_from_this<RoomDefinition>
    {
    public:
        explicit RoomDefinition(int index);

        void setConnection(const Direction* direction, const boost::shared_ptr<RoomDefinition>& connection);
        boost::shared_ptr<RoomDefinition> fjConnection(int index) const;
        boost::shared_ptr<RoomDefinition> fjConnection(const Direction* direction) const;
        void updateOpenings();
        bool findSource(int scanIndex);
        bool isSpecial();
        int countOpenings();

        int mIndex;
        bool mHasOpening[6];
        bool mClaimed;
        bool mIsSource;
        int mScanIndex;
        boost::weak_ptr<RoomDefinition> mConnections[6];
    };

    class OceanMonumentPiece : public StructurePiece
    {
    public:
        static const BlockState* BASE_GRAY;
        static const BlockState* BASE_LIGHT;
        static const BlockState* BASE_BLACK;
        static const BlockState* DOT_DECO_DATA;
        static const BlockState* LAMP_BLOCK;
        static const BlockState* FILL_BLOCK;
        static const int GRIDROOM_SOURCE_INDEX;
        static const int GRIDROOM_TOP_CONNECT_INDEX;
        static const int GRIDROOM_LEFTWING_CONNECT_INDEX;
        static const int GRIDROOM_RIGHTWING_CONNECT_INDEX;

        explicit OceanMonumentPiece(int genDepth);
        OceanMonumentPiece(const Direction* orientation, BoundingBox* boundingBox);
        OceanMonumentPiece(int genDepth, const Direction* orientation,
                           const boost::shared_ptr<RoomDefinition>& roomDefinition, int roomWidth, int roomHeight,
                           int roomDepth);
        virtual ~OceanMonumentPiece();

        static void staticCtor();
        static int getRoomIndex(int roomX, int roomY, int roomZ);
        static int getRoomX(int roomIndex);
        static int getRoomY(int roomIndex);
        static int getRoomZ(int roomIndex);

        void fjKeepReference(const boost::shared_ptr<RoomDefinition>& roomDefinition);

        virtual void addAdditionalSaveData(DataOutputStream* output);
        virtual void readAdditionalSaveData(DataInputStream* input, StructureManager* structureManager);

    protected:
        void generateWaterBox(Level* level, BoundingBox* box, int x0, int y0, int z0, int x1, int y1, int z1,
                              bool skipAir);
        void generateDefaultFloor(Level* level, BoundingBox* box, int x, int z, bool hasOpening);
        void generateBoxOnFillOnly(Level* level, BoundingBox* box, int x0, int y0, int z0, int x1, int y1, int z1,
                                   const BlockState* state);
        bool chunkIntersects(BoundingBox* box, int x0, int z0, int x1, int z1);
        bool spawnElder(Level* level, BoundingBox* box, int x, int y, int z);

        boost::shared_ptr<RoomDefinition> mRoomDefinition;
        std::vector<boost::shared_ptr<RoomDefinition> > mKeptRooms;
    };

    class MonumentRoomFitter
    {
    public:
        virtual ~MonumentRoomFitter();
        virtual bool fits(const boost::shared_ptr<RoomDefinition>& roomDefinition) = 0;
        virtual OceanMonumentPiece* create(const Direction* orientation,
                                           const boost::shared_ptr<RoomDefinition>& roomDefinition, Random& random) = 0;
    };

    class FitSimpleRoom : public MonumentRoomFitter
    {
    public:
        virtual ~FitSimpleRoom();
        virtual bool fits(const boost::shared_ptr<RoomDefinition>& roomDefinition);
        virtual OceanMonumentPiece* create(const Direction* orientation,
                                           const boost::shared_ptr<RoomDefinition>& roomDefinition, Random& random);
    };

    class FitSimpleTopRoom : public MonumentRoomFitter
    {
    public:
        virtual ~FitSimpleTopRoom();
        virtual bool fits(const boost::shared_ptr<RoomDefinition>& roomDefinition);
        virtual OceanMonumentPiece* create(const Direction* orientation,
                                           const boost::shared_ptr<RoomDefinition>& roomDefinition, Random& random);
    };

    class FitDoubleYRoom : public MonumentRoomFitter
    {
    public:
        virtual ~FitDoubleYRoom();
        virtual bool fits(const boost::shared_ptr<RoomDefinition>& roomDefinition);
        virtual OceanMonumentPiece* create(const Direction* orientation,
                                           const boost::shared_ptr<RoomDefinition>& roomDefinition, Random& random);
    };

    class FitDoubleXRoom : public MonumentRoomFitter
    {
    public:
        virtual ~FitDoubleXRoom();
        virtual bool fits(const boost::shared_ptr<RoomDefinition>& roomDefinition);
        virtual OceanMonumentPiece* create(const Direction* orientation,
                                           const boost::shared_ptr<RoomDefinition>& roomDefinition, Random& random);
    };

    class FitDoubleZRoom : public MonumentRoomFitter
    {
    public:
        virtual ~FitDoubleZRoom();
        virtual bool fits(const boost::shared_ptr<RoomDefinition>& roomDefinition);
        virtual OceanMonumentPiece* create(const Direction* orientation,
                                           const boost::shared_ptr<RoomDefinition>& roomDefinition, Random& random);
    };

    class FitDoubleXYRoom : public MonumentRoomFitter
    {
    public:
        virtual ~FitDoubleXYRoom();
        virtual bool fits(const boost::shared_ptr<RoomDefinition>& roomDefinition);
        virtual OceanMonumentPiece* create(const Direction* orientation,
                                           const boost::shared_ptr<RoomDefinition>& roomDefinition, Random& random);
    };

    class FitDoubleYZRoom : public MonumentRoomFitter
    {
    public:
        virtual ~FitDoubleYZRoom();
        virtual bool fits(const boost::shared_ptr<RoomDefinition>& roomDefinition);
        virtual OceanMonumentPiece* create(const Direction* orientation,
                                           const boost::shared_ptr<RoomDefinition>& roomDefinition, Random& random);
    };

    class MonumentBuilding : public OceanMonumentPiece
    {
    public:
        MonumentBuilding();
        MonumentBuilding(Random& random, int x, int z, const Direction* orientation);
        virtual ~MonumentBuilding();

        static StructurePiece* Create();
        virtual EStructurePiece GetType();
        virtual bool postProcess(Level* level, Random& random, BoundingBox* box);

    private:
        std::vector<boost::shared_ptr<RoomDefinition> >* generateRoomGraph(Random& random);
        void generateWing(bool flipped, int xOffset, Level* level, Random& random, BoundingBox* box);
        void generateEntranceArchs(Level* level, Random& random, BoundingBox* box);
        void generateEntranceWall(Level* level, Random& random, BoundingBox* box);
        void generateRoofPiece(Level* level, Random& random, BoundingBox* box);
        void generateLowerWall(Level* level, Random& random, BoundingBox* box);
        void generateMiddleWall(Level* level, Random& random, BoundingBox* box);
        void generateUpperWall(Level* level, Random& random, BoundingBox* box);

        boost::shared_ptr<RoomDefinition> mSourceRoom;
        boost::shared_ptr<RoomDefinition> mCoreRoom;
        std::vector<OceanMonumentPiece*> mChildPieces;
    };

    class OceanMonumentEntryRoom : public OceanMonumentPiece
    {
    public:
        OceanMonumentEntryRoom();
        OceanMonumentEntryRoom(const Direction* orientation, const boost::shared_ptr<RoomDefinition>& roomDefinition);

        static StructurePiece* Create();
        virtual EStructurePiece GetType();
        virtual bool postProcess(Level* level, Random& random, BoundingBox* box);
    };

    class OceanMonumentSimpleRoom : public OceanMonumentPiece
    {
    public:
        OceanMonumentSimpleRoom();
        OceanMonumentSimpleRoom(const Direction* orientation, const boost::shared_ptr<RoomDefinition>& roomDefinition,
                                Random& random);

        static StructurePiece* Create();
        virtual EStructurePiece GetType();
        virtual bool postProcess(Level* level, Random& random, BoundingBox* box);

    private:
        int mMainDesign;
    };

    class OceanMonumentSimpleTopRoom : public OceanMonumentPiece
    {
    public:
        OceanMonumentSimpleTopRoom();
        OceanMonumentSimpleTopRoom(const Direction* orientation,
                                   const boost::shared_ptr<RoomDefinition>& roomDefinition, Random& random);

        static StructurePiece* Create();
        virtual EStructurePiece GetType();
        virtual bool postProcess(Level* level, Random& random, BoundingBox* box);
    };

    class OceanMonumentDoubleYRoom : public OceanMonumentPiece
    {
    public:
        OceanMonumentDoubleYRoom();
        OceanMonumentDoubleYRoom(const Direction* orientation, const boost::shared_ptr<RoomDefinition>& roomDefinition,
                                 Random& random);

        static StructurePiece* Create();
        virtual EStructurePiece GetType();
        virtual bool postProcess(Level* level, Random& random, BoundingBox* box);
    };

    class OceanMonumentDoubleXRoom : public OceanMonumentPiece
    {
    public:
        OceanMonumentDoubleXRoom();
        OceanMonumentDoubleXRoom(const Direction* orientation, const boost::shared_ptr<RoomDefinition>& roomDefinition,
                                 Random& random);

        static StructurePiece* Create();
        virtual EStructurePiece GetType();
        virtual bool postProcess(Level* level, Random& random, BoundingBox* box);
    };

    class OceanMonumentDoubleXYRoom : public OceanMonumentPiece
    {
    public:
        OceanMonumentDoubleXYRoom();
        OceanMonumentDoubleXYRoom(const Direction* orientation, const boost::shared_ptr<RoomDefinition>& roomDefinition,
                                  Random& random);

        static StructurePiece* Create();
        virtual EStructurePiece GetType();
        virtual bool postProcess(Level* level, Random& random, BoundingBox* box);
    };

    class OceanMonumentDoubleYZRoom : public OceanMonumentPiece
    {
    public:
        OceanMonumentDoubleYZRoom();
        OceanMonumentDoubleYZRoom(const Direction* orientation, const boost::shared_ptr<RoomDefinition>& roomDefinition,
                                  Random& random);

        static StructurePiece* Create();
        virtual EStructurePiece GetType();
        virtual bool postProcess(Level* level, Random& random, BoundingBox* box);
    };

    class OceanMonumentDoubleZRoom : public OceanMonumentPiece
    {
    public:
        OceanMonumentDoubleZRoom();
        OceanMonumentDoubleZRoom(const Direction* orientation, const boost::shared_ptr<RoomDefinition>& roomDefinition,
                                 Random& random);

        static StructurePiece* Create();
        virtual EStructurePiece GetType();
        virtual bool postProcess(Level* level, Random& random, BoundingBox* box);
    };

    class OceanMonumentCoreRoom : public OceanMonumentPiece
    {
    public:
        OceanMonumentCoreRoom();
        OceanMonumentCoreRoom(const Direction* orientation, const boost::shared_ptr<RoomDefinition>& roomDefinition,
                              Random& random);

        static StructurePiece* Create();
        virtual EStructurePiece GetType();
        virtual bool postProcess(Level* level, Random& random, BoundingBox* box);
    };

    class OceanMonumentWingRoom : public OceanMonumentPiece
    {
    public:
        OceanMonumentWingRoom();
        OceanMonumentWingRoom(const Direction* orientation, BoundingBox* boundingBox, int randomValue);

        virtual EStructurePiece GetType();
        virtual bool postProcess(Level* level, Random& random, BoundingBox* box);

    private:
        int mMainDesign;
    };

    class OceanMonumentPenthouse : public OceanMonumentPiece
    {
    public:
        OceanMonumentPenthouse();
        OceanMonumentPenthouse(const Direction* orientation, BoundingBox* boundingBox);

        static StructurePiece* Create();
        virtual EStructurePiece GetType();
        virtual bool postProcess(Level* level, Random& random, BoundingBox* box);
    };
};

#endif
