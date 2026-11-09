#ifndef GAME_WORLD_GENERATION_CORALCRUSTFEATURE_HPP
#define GAME_WORLD_GENERATION_CORALCRUSTFEATURE_HPP

#include <utility>
#include <vector>

#include "game/world/generation/Feature.hpp"

class BlockPos;
class BlockState;
class Direction;
class Level;
class Random;
class Rotation;
class StructurePlaceSettings;
class StructureTemplate;

class CoralCrustFeature : public Feature
{
public:
    virtual ~CoralCrustFeature();
    BlockPos getOffsetFromRot(Rotation* rotation, int type) const;
    BlockPos getCropOffsetFromRot(Rotation* rotation, int type) const;
    const BlockState* setColorData(Level* level, const BlockState* state, int color) const;
    bool canOverwrite(Level* level, const BlockPos& pos);
    void placeCoral(Level* level, const BlockPos& pos, Random& random, std::vector<BlockPos>& positions,
                    std::vector<std::pair<BlockPos, const Direction*> >& sides, int type);
    void placeTopDecorations(Level* level, const BlockPos& pos, Random& random);
    bool canPlaceHangOn(Level* level, const BlockPos& pos);
    const Direction* randomDirectionExcept(Random& random, const Direction* except) const;
    int turnFacingIntoint(const Direction* direction) const;
    const BlockState* setCoralHangData(int facing, int variant, int type) const;
    void placeSideDecorations(Level* level, const BlockPos& pos, Random& random, const Direction* direction);
    void placeCoralBase(Level* level, const BlockPos& pos, Random& random, const StructureTemplate* structure,
                        StructurePlaceSettings& settings);
    virtual bool place(Level* level, Random& random, const BlockPos& origin);
};

#endif
