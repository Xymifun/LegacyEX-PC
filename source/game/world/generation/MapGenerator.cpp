#include "game/world/generation/MapGenerator.hpp"

#include "game/core/System.hpp"

MapGenerator::MapGenerator(Random random) : mRandom(random)
{
}

MapGenerator::~MapGenerator()
{
    std::vector<GenerationWriteTask*>::iterator task = mGenerationWriteTasks.begin();
    while (task != mGenerationWriteTasks.end())
    {
        delete *task;
        ++task;
    }
}

bool MapGenerator::FinishedGeneration() const
{
    return mGenerationWriteTasks.empty();
}

bool MapGenerator::ContinueGeneration()
{
    const long long endTime = System::processTimeInMilliSecs() + 15;
    std::vector<GenerationWriteTask*>::iterator it = mGenerationWriteTasks.begin();

    while (it != mGenerationWriteTasks.end())
    {
        GenerationWriteTask* task = *it;
        task->doWork(endTime);
        if (task->hasFinished())
        {
            delete task;
            it = mGenerationWriteTasks.erase(it);
        }
        else
        {
            ++it;
        }

        if (System::processTimeInMilliSecs() >= endTime)
        {
            break;
        }
    }

    return FinishedGeneration();
}

unsigned int MapGenerator::GetBlockCountOnLayer(unsigned int layer)
{
    return 0;
}
