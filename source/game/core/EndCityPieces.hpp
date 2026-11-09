#ifndef GAME_CORE_ENDCITYPIECES_HPP
#define GAME_CORE_ENDCITYPIECES_HPP

#include <vector>

#include <boost/unordered_map.hpp>

#include "game/world/generation/StructureManager.hpp"
#include "game/world/generation/TemplateStructurePiece.hpp"

class BlockPos;
class BoundingBox;
class DataInputStream;
class DataOutputStream;
class Level;
class Random;
class Rotation;
class StructureManager;

class EndCityPieces
{
public:
    class EndCityPiece;

    class SectionGenerator
    {
    public:
        virtual void init() = 0;
        virtual bool generate(StructureManager* structureManager, int genDepth, EndCityPiece* previous,
                              const BlockPos& position, std::vector<StructurePiece*>& pieces, Random& random) = 0;
    };

    class FatTowerSectionGenerator : public SectionGenerator
    {
    public:
        virtual void init();
        virtual bool generate(StructureManager* structureManager, int genDepth, EndCityPiece* previous,
                              const BlockPos& position, std::vector<StructurePiece*>& pieces, Random& random);
    };

    class HouseTowerSectionGenerator : public SectionGenerator
    {
    public:
        virtual void init();
        virtual bool generate(StructureManager* structureManager, int genDepth, EndCityPiece* previous,
                              const BlockPos& position, std::vector<StructurePiece*>& pieces, Random& random);

    private:
        bool mGenerated[8];
    };

    class TowerBridgeSectionGenerator : public SectionGenerator
    {
    public:
        TowerBridgeSectionGenerator();
        virtual void init();
        virtual bool generate(StructureManager* structureManager, int genDepth, EndCityPiece* previous,
                              const BlockPos& position, std::vector<StructurePiece*>& pieces, Random& random);

    private:
        bool mGenerated;
    };

    class TowerSectionGenerator : public SectionGenerator
    {
    public:
        virtual void init();
        virtual bool generate(StructureManager* structureManager, int genDepth, EndCityPiece* previous,
                              const BlockPos& position, std::vector<StructurePiece*>& pieces, Random& random);
    };

    class EndCityPiece : public TemplateStructurePiece
    {
    public:
        EndCityPiece(StructureManager* structureManager, eMinecraftStructureTemplate templateType,
                     const BlockPos& position, Rotation* rotation, bool overwrite);
        EndCityPiece();

        void loadTemplate(StructureManager* structureManager);
        virtual void addAdditionalSaveData(DataOutputStream* output);
        virtual void readAdditionalSaveData(DataInputStream* input, StructureManager* structureManager);
        virtual void handleDataMarker(const std::wstring& marker, const BlockPos& position, Level* level,
                                      Random& random, BoundingBox* box);
        static StructurePiece* Create();
        virtual EStructurePiece GetType();

    private:
        eMinecraftStructureTemplate mTemplateType;
        Rotation* mRotation;
        bool mOverwrite;
    };

    static EndCityPiece* addHelper(std::vector<StructurePiece*>& pieces, EndCityPiece* piece);
    static EndCityPiece* addPiece(StructureManager* structureManager, EndCityPiece* previous, const BlockPos& position,
                                  eMinecraftStructureTemplate templateType, Rotation* rotation, bool overwrite);
    static bool recursiveChildren(StructureManager* structureManager, SectionGenerator* generator, int genDepth,
                                  EndCityPiece* previous, const BlockPos& position,
                                  std::vector<StructurePiece*>& pieces, Random& random);
    static void startHouseTower(StructureManager* structureManager, const BlockPos& position, Rotation* rotation,
                                std::vector<StructurePiece*>& pieces, Random& random);
    static boost::unordered_map<Rotation*, BlockPos> createTowerBridgesMap();
    static boost::unordered_map<Rotation*, BlockPos> createFatTowerBridgesMap();
    static void loadStatic();
};

#endif
