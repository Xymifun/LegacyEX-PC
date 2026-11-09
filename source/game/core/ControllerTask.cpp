#include "game/core/ControllerTask.hpp"

#include "game/core/GameSettings.hpp"
#include "game/core/InputConstraint.hpp"
#include "game/core/Minecraft.hpp"
#include "game/minigame/Tutorial.hpp"
#include "game/world/generation/MultiplayerLocalPlayer.hpp"
#include "platform/wiiu/CInput.hpp"
#ifdef MINECRAFT_PC
#include "platform/pc/PcInput.hpp"

static bool PcMouseLooked(int action)
{
    if (action == EControllerActions_CycleHeldItemLeft || action == EControllerActions_CycleHeldItemRight)
        return PcInputRecentHotbarWheel(action == EControllerActions_CycleHeldItemLeft ? 0 : 1);
    if (action < EControllerActions_LookUp || action > EControllerActions_LookRight)
        return false;
    return PcInputRecentMouseLook(action - EControllerActions_LookUp);
}
#endif

ControllerTask::ControllerTask(Tutorial* tutorial, int descriptionId, bool preCompletionEnabled, bool showMinimumTime,
                               int* mappings, unsigned int mappingCount, int* completionIdValues,
                               int completionIdValueCount, int* alternateMappings, unsigned int alternateMappingCount)
    : TutorialTask(tutorial, descriptionId, preCompletionEnabled, NULL, showMinimumTime, true, true)
{
    unsigned int i;

    for (i = 0; i < mappingCount; ++i)
    {
        constraints.push_back(new InputConstraint(mappings[i]));
        constrainedMappings[mappings[i]] = false;
    }

    if (alternateMappingCount != 0)
    {
        useAlternateMappings = true;
    }

    for (i = 0; i < alternateMappingCount; ++i)
    {
        inputMappings[alternateMappings[i]] = false;
    }

    completionIds = new int[completionIdValueCount];

    for (int completionId = 0; completionId < completionIdValueCount; ++completionId)
    {
        completionIds[completionId] = completionIdValues[completionId];
    }

    completionIdCount = completionIdValueCount;
    completionMask = 0;
}

ControllerTask::~ControllerTask()
{
    delete[] completionIds;
}

unsigned int ControllerTask::CompletionMaskIsValid()
{
    for (int i = 0; i < completionIdCount; ++i)
    {
        if (completionMask == static_cast<unsigned int>(completionIds[i]))
        {
            return 1;
        }
    }

    return 0;
}

bool ControllerTask::isCompleted()
{
    if (completed)
    {
        return true;
    }

    bool result = true;
    Minecraft* minecraft = Minecraft::GetInstance();
    unsigned int mappingIndex = 0;

    if (useAlternateMappings
        && GameSettings::GetGameSettings(minecraft->mPlayer->mUserIndex, eGameSetting_SouthPaw) != 0)
    {
        for (boost::unordered_map<int, bool>::iterator mapping = inputMappings.begin(); mapping != inputMappings.end();
             ++mapping, ++mappingIndex)
        {
            if (!mapping->second)
            {
                if (g_pInput->GetValue(minecraft->mPlayer->mUserIndex, static_cast<unsigned char>(mapping->first),
                                       false)
                        > 0
#ifdef MINECRAFT_PC
                    || PcMouseLooked(mapping->first)
#endif
                )
                {
                    mapping->second = true;
                    completionMask |= 1 << (mappingIndex & 0x3f);
                }
                else
                {
                    result = false;
                }
            }
        }
    }
    else
    {
        for (boost::unordered_map<int, bool>::iterator mapping = constrainedMappings.begin();
             mapping != constrainedMappings.end(); ++mapping, ++mappingIndex)
        {
            if (!mapping->second)
            {
                if (g_pInput->GetValue(minecraft->mPlayer->mUserIndex, static_cast<unsigned char>(mapping->first),
                                       false)
                        > 0
#ifdef MINECRAFT_PC
                    || PcMouseLooked(mapping->first)
#endif
                )
                {
                    mapping->second = true;
                    completionMask |= 1 << (mappingIndex & 0x3f);
                }
                else
                {
                    result = false;
                }
            }
        }
    }

    if (completionIds != NULL && CompletionMaskIsValid())
    {
        completed = true;
        return true;
    }

    completed = result;
    return result;
}

void ControllerTask::setAsCurrentTask(bool currentTask)
{
    TutorialTask::setAsCurrentTask(currentTask);
    enableConstraints(!currentTask, false);
}
