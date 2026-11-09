#include "game/world/generation/LevelGenerators.hpp"

#include <algorithm>

#include "game/core/CConsoleMinecraftApp.hpp"
#include "game/core/DLCPack.hpp"
#include "game/core/cDLCInfo.hpp"
#include "game/world/LevelGenerationOptions.hpp"
#include "platform/wiiu/cNintendoDLCInfo.hpp"

LevelGenerators::LevelGenerators()
{
}

void LevelGenerators::removeLevelGenerator(LevelGenerationOptions* levelGenerator)
{
    std::vector<LevelGenerationOptions*>::iterator it;
    while ((it = std::find(mLevelGenerators.begin(), mLevelGenerators.end(), levelGenerator)) != mLevelGenerators.end())
    {
        mLevelGenerators.erase(it);
    }
}

std::vector<LevelGenerationOptions*>* LevelGenerators::getLevelGenerators()
{
    return &mLevelGenerators;
}

bool LevelGenerators::SortLevelGenerators(const LevelGenerationOptions* first, const LevelGenerationOptions* second)
{
    unsigned int firstPackId = first->getPack()->GetPackId();
    unsigned int secondPackId = second->getPack()->GetPackId();
#ifdef MINECRAFT_PC
    const bool firstTutorial = const_cast<LevelGenerationOptions*>(first)->isTutorial();
    const bool secondTutorial = const_cast<LevelGenerationOptions*>(second)->isTutorial();
    if (firstTutorial != secondTutorial)
        return firstTutorial;
#endif

    if (firstPackId == 4)
    {
        unsigned int themeId = first->getThemeId();
        if (themeId == 0 || themeId == static_cast<unsigned int>(-1))
            return true;
    }

    if (secondPackId == 4)
    {
        unsigned int themeId = second->getThemeId();
        if (themeId == 0 || themeId == static_cast<unsigned int>(-1))
            return false;
    }

    DLC_INFO* firstInfo = gApp.GetDLCInfoInstance()->GetDLCInfoForPackID(firstPackId);
    DLC_INFO* secondInfo = gApp.GetDLCInfoInstance()->GetDLCInfoForPackID(secondPackId);
    if (firstInfo == NULL || secondInfo == NULL)
        return false;

    bool result = firstInfo->SortIndex > secondInfo->SortIndex;
    return result;
}
