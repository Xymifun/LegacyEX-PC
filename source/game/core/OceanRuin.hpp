#ifndef GAME_CORE_OCEANRUIN_HPP
#define GAME_CORE_OCEANRUIN_HPP

#include "game/world/block/BlockPos.hpp"
#include "game/world/generation/ScatteredFeaturePieces.hpp"
#include "game/world/generation/StructureManager.hpp"

class BlockState;
class BoundingBox;
class DataInputStream;
class DataOutputStream;
class Level;
class Mirror;
class Random;
class Rotation;
class StructureManager;
class StructurePlaceSettings;
class StructureTemplate;

class OceanRuin : public ScatteredFeaturePieces::ScatteredFeaturePiece
{
public:
    enum ERuinType
    {
        eRuinType_Cold = 0,
        eRuinType_Warm = 1,
        eRuinType_ColdBrick = 2
    };

    OceanRuin();
    OceanRuin(Level* level, Random& random, int x, int z, int width, int height, int depth, bool isWarm, bool isLarge);

    eMinecraftStructureTemplate getBigWarmRuin(Random& random);
    eMinecraftStructureTemplate getSmallWarmRuin(Random& random);
    eMinecraftStructureTemplate getRuin(int index, bool large, ERuinType type);
    void updateChestReplacer(eMinecraftStructureTemplate structureTemplate);
    const StructureTemplate* placeRuin(Random& random, StructureManager* structureManager, Level* level,
                                       const BlockPos& pos, StructurePlaceSettings& settings);
    BlockPos adaptPosition(const BlockPos& pos, Mirror* mirror, Rotation* rotation, Level* level);
    void fillBelow(const BlockPos& pos, Mirror* mirror, Rotation* rotation, Level* level);
    virtual bool postProcess(Level* level, Random& random, BoundingBox* box);
    virtual void addAdditionalSaveData(DataOutputStream* output);
    virtual void readAdditionalSaveData(DataInputStream* input, StructureManager* structureManager);
    virtual EStructurePiece GetType();

    static StructurePiece* Create();

private:
    const BlockState* mBlockState;
    bool mIsLarge;
    bool mIsWarm;
};

#endif
