#ifndef GAME_CORE_SHIPWRECKPIECE_HPP
#define GAME_CORE_SHIPWRECKPIECE_HPP

#include "game/world/generation/StructurePiece.hpp"

class BoundingBox;
class DataInputStream;
class DataOutputStream;
class Direction;
class GenericOverworldLevelSource;
class Level;
class Random;
class Rotation;
class StructureManager;

class ShipwreckPiece : public StructurePiece
{
public:
    ShipwreckPiece(Level* level, GenericOverworldLevelSource* levelSource, int x, int z, Random& random,
                   const Direction* orientation);

    static int GetNonUpsideDownShipwreckType(Random& random);
    static bool IsUpsideDownShipwreckType(int type);

    virtual bool postProcess(Level* level, Random& random, BoundingBox* box);
    virtual void addAdditionalSaveData(DataOutputStream* output);
    virtual void readAdditionalSaveData(DataInputStream* input, StructureManager* structureManager);
    virtual EStructurePiece GetType();

private:
    Rotation* mRotation;
    int mType;
    int mNonUpsideDownType;
    int mWoodType;
    int mHeight;
};

#endif
