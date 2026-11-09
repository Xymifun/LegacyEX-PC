#ifndef GAME_WORLD_BLOCK_BLOCKSTATEMAPPER_HPP
#define GAME_WORLD_BLOCK_BLOCKSTATEMAPPER_HPP

#include <boost/unordered_map.hpp>
#include <boost/unordered_set.hpp>

class Block;
class BlockState;
class ModelResourceLocation;
class StateMapper;

class BlockStateMapper
{
public:
    void add(Block* block, StateMapper* stateMapper);
    void ignore(Block* block, ...);

    boost::unordered_map<const BlockState*, ModelResourceLocation*>* getMapping();

private:
    boost::unordered_map<Block*, StateMapper*> stateMappers;
    boost::unordered_set<Block*> ignoredBlocks;
};

#endif
