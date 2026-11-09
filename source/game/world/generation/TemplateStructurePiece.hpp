#ifndef GAME_WORLD_GENERATION_TEMPLATESTRUCTUREPIECE_HPP
#define GAME_WORLD_GENERATION_TEMPLATESTRUCTUREPIECE_HPP

#include <string>

#include "game/world/block/BlockPos.hpp"
#include "game/world/generation/StructurePiece.hpp"

class BoundingBox;
class DataInputStream;
class DataOutputStream;
class Level;
class Random;
class StructureManager;
class StructurePlaceSettings;
class StructureTemplate;

class TemplateStructurePiece : public StructurePiece
{
public:
    TemplateStructurePiece();
    TemplateStructurePiece(int genDepth);
    TemplateStructurePiece(int genDepth, const StructureTemplate* templateObject, const BlockPos& templatePosition,
                           StructurePlaceSettings* placeSettings);
    virtual ~TemplateStructurePiece();

    void setBoundingBoxFromTemplate();
    void setup(const StructureTemplate* templateObject, const BlockPos& templatePosition,
               StructurePlaceSettings* placeSettings);

    virtual void addAdditionalSaveData(DataOutputStream* output);
    virtual void readAdditionalSaveData(DataInputStream* input, StructureManager* structureManager);
    virtual bool postProcess(Level* level, Random& random, BoundingBox* box);
    virtual void move(int x, int y, int z);
    virtual Rotation* getRotation();

    virtual void handleDataMarker(const std::wstring& marker, const BlockPos& position, Level* level, Random& random,
                                  BoundingBox* box) = 0;

    BlockPos getTemplatePosition();
    StructureTemplate* getStructureTemplate();
    StructurePlaceSettings* getPlaceSettings();

protected:
    StructureTemplate* mTemplate;
    StructurePlaceSettings* mPlaceSettings;
    BlockPos mTemplatePosition;
};

#endif
