#include "game/core/InfoTask.hpp"

#include "game/core/ConsoleUIController.hpp"
#include "game/core/InputConstraint.hpp"
#include "game/core/Minecraft.hpp"
#include "game/core/UnityClient6.hpp"
#include "game/world/block/Material.hpp"
#include "game/world/generation/MultiplayerLocalPlayer.hpp"
#include "platform/wiiu/CInput.hpp"

extern ConsoleUIController gConsoleUIController;

InfoTask::InfoTask(Tutorial* tutorial, int descriptionId, int promptId, bool addConstraint, int completionId,
                   ETelemetryChallenges telemetryChallenge)
    : TutorialTask(tutorial, descriptionId, false, NULL, true, false, false),
      completedInputs()
{
    if (addConstraint)
    {
        constraints.push_back(new InputConstraint(completionId));
    }

    completedInputs[completionId] = false;
    this->promptId = promptId;
    tutorial->addMessage(promptId, false, 3);
    this->telemetryChallenge = telemetryChallenge;
}

void InfoTask::sendTelemetry()
{
    Minecraft* minecraft = Minecraft::GetInstance();
    if (telemetryChallenge != eTelemetryChallenges_None)
    {
        if (telemetryChallenge == eTelemetryChallenges_TutorialWelcome)
        {
            tutorial->getCompleted(388);
            tutorial->setCompleted(388);
        }

        telemetryManagerInstance->RecordEnemyKilledOrOvercome(minecraft->mPlayer->mUserIndex, 0, 0, 0, 0, 0, 0,
                                                              telemetryChallenge);
    }
}

bool InfoTask::isCompleted()
{
    if (completed)
    {
        return true;
    }

    if (tutorial->popupPending || !activated || !shownForMinimumTime)
    {
        return false;
    }

    bool result = true;
    Minecraft* minecraft = Minecraft::GetInstance();

    if (minecraft->mLocalPlayers[tutorial->playerId]->isUnderLiquid(Material::WATER))
    {
        return false;
    }

    if (gConsoleUIController.GetMenuDisplayed(tutorial->playerId))
    {
        for (boost::unordered_map<int, bool>::iterator it = completedInputs.begin(); it != completedInputs.end(); ++it)
        {
            if (!it->second)
            {
                result = false;
                break;
            }
        }
    }
    else
    {
        for (boost::unordered_map<int, bool>::iterator it = completedInputs.begin(); it != completedInputs.end(); ++it)
        {
            if (!it->second)
            {
                if (g_pInput->GetValue(minecraft->mPlayer->mUserIndex, static_cast<unsigned char>(it->first), false)
                    > 0)
                {
                    it->second = true;
                    result = true;
                }
                else
                {
                    result = false;
                }
            }
        }
    }

    if (result)
    {
        sendTelemetry();
        enableConstraints(false, true);
    }

    completed = result;
    return result;
}

int InfoTask::getPromptId()
{
    if (!shownForMinimumTime)
    {
        return -1;
    }
    return promptId;
}

void InfoTask::setAsCurrentTask(bool currentTask)
{
    enableConstraints(currentTask, false);
    TutorialTask::setAsCurrentTask(currentTask);
}

void InfoTask::handleUIInput(int input)
{
    if (activated)
    {
        for (boost::unordered_map<int, bool>::iterator it = completedInputs.begin(); it != completedInputs.end(); ++it)
        {
            if (input == it->first)
            {
                it->second = true;
            }
        }
    }
}
