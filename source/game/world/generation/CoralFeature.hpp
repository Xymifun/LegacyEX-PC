#ifndef GAME_WORLD_GENERATION_CORALFEATURE_HPP
#define GAME_WORLD_GENERATION_CORALFEATURE_HPP
#include <utility>
#include <vector>
#include "game/world/generation/Feature.hpp"
class BlockPos;
class BlockState;
class Direction;
class Level;
class Material;
class Random;
class CoralFeature : public Feature
{
public:
    virtual ~CoralFeature();
    const Direction* _randomDirectionExcept(Random&, const Direction*);
    const Direction* _randomDirectionExcept(Random&, const Direction*, const Direction*);
    bool _canOverwrite(Level*, const BlockPos&, int);
    bool _setBlock(Level*, const BlockPos&, const BlockState*, int);
    bool _setBlockOnSolid(Level*, const BlockPos&, const BlockState*, int);
    void _starFormation(Level*, Random&, const BlockPos&, const BlockState*, float, int, bool);
    void _starCorners(Level*, Random&, const BlockPos&, const BlockState*, float, int, bool);
    void _buildPlantArm(Level*, Random&, const BlockPos&, const BlockState*, std::vector<BlockPos>&,
                        std::vector<std::pair<BlockPos, const Direction*> >&, const Direction*, bool);
    void _buildPlantLike(Level*, const BlockPos&, Random&, std::vector<BlockPos>&,
                         std::vector<std::pair<BlockPos, const Direction*> >&, const BlockState*);
    void _buildSpire(Level*, const BlockPos&, Random&, std::vector<BlockPos>&,
                     std::vector<std::pair<BlockPos, const Direction*> >&, const BlockState*);
    void _buildHand(Level*, const BlockPos&, Random&, std::vector<BlockPos>&,
                    std::vector<std::pair<BlockPos, const Direction*> >&, const BlockState*);
    void _buildSmallClump(Level*, const BlockPos&, Random&, std::vector<BlockPos>&,
                          std::vector<std::pair<BlockPos, const Direction*> >&, const BlockState*);
    void _placeCoral(Level*, const BlockPos&, Random&, std::vector<BlockPos>&,
                     std::vector<std::pair<BlockPos, const Direction*> >&, int);
    void _placeTopDecorations(Level*, const BlockPos&, Random&);
    bool _canPlaceHangOn(Level*, const BlockPos&);
    int _turnFacingIntoint(const Direction*);
    const BlockState* _setCoralHangData(int, int, int);
    void _placeSideDecorations(Level*, const BlockPos&, Random&, const Direction*);
    virtual bool place(Level*, Random&, const BlockPos&);
    bool _isFree(const Material*);
};
#endif
