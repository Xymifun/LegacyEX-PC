#ifndef GAME_WORLD_GENERATION_LEVELGENERATORS_HPP
#define GAME_WORLD_GENERATION_LEVELGENERATORS_HPP

#include <vector>

class LevelGenerationOptions;

class LevelGenerators
{
public:
    LevelGenerators();
    void removeLevelGenerator(LevelGenerationOptions* levelGenerator);
    std::vector<LevelGenerationOptions*>* getLevelGenerators();
    static bool SortLevelGenerators(const LevelGenerationOptions* first, const LevelGenerationOptions* second);

private:
    std::vector<LevelGenerationOptions*> mLevelGenerators;
};

#endif
