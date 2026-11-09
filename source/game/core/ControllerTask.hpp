#ifndef GAME_CORE_CONTROLLERTASK_HPP
#define GAME_CORE_CONTROLLERTASK_HPP

#include <boost/unordered_map.hpp>

#include "game/minigame/TutorialTask.hpp"

class ControllerTask : public TutorialTask
{
public:
    ControllerTask(Tutorial* tutorial, int descriptionId, bool preCompletionEnabled, bool showMinimumTime,
                   int* mappings, unsigned int mappingCount, int* completionIdValues, int completionIdValueCount,
                   int* alternateMappings, unsigned int alternateMappingCount);
    virtual ~ControllerTask();

    unsigned int CompletionMaskIsValid();

    virtual bool isCompleted();
    virtual void setAsCurrentTask(bool currentTask);

private:
    boost::unordered_map<int, bool> constrainedMappings;
    boost::unordered_map<int, bool> inputMappings;
    bool useAlternateMappings;
    unsigned int completionMask;
    int* completionIds;
    int completionIdCount;
};

#endif
