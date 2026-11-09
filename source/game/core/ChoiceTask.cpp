#include "game/core/ChoiceTask.hpp"

#include "game/core/ConsoleUIController.hpp"
#include "game/core/InputConstraint.hpp"
#include "game/core/Minecraft.hpp"
#include "game/core/UnityClient6.hpp"
#include "game/world/block/Material.hpp"
#include "game/world/generation/MultiplayerLocalPlayer.hpp"
#include "platform/wiiu/CInput.hpp"

ChoiceTask::ChoiceTask(Tutorial* tutorial, int descriptionId, int promptId, bool preCompletionEnabled, int firstInput,
                       int secondInput, eTutorial_CompletionAction completionAction,
                       ETelemetryChallenges telemetryChallenge)
    : TutorialTask(tutorial, descriptionId, false, NULL, true, false, false)
{
    if (preCompletionEnabled)
    {
        constraints.push_back(new InputConstraint(firstInput));
        constraints.push_back(new InputConstraint(secondInput));
    }

    this->firstInput = firstInput;
    secondChoice = false;
    firstChoice = false;
    this->secondInput = secondInput;
    this->completionAction = completionAction;
    this->promptId = promptId;
    tutorial->addMessage(promptId, false, 3);
    this->telemetryChallenge = telemetryChallenge;
}

void ChoiceTask::sendTelemetry()
{
    Minecraft* minecraft = Minecraft::GetInstance();

    if (telemetryChallenge == eTelemetryChallenges_None)
    {
        return;
    }

    switch (telemetryChallenge)
    {
    case eTelemetryChallenges_TutorialStart:
        tutorial->getCompleted(386);
        tutorial->setCompleted(386);
        break;
    case eTelemetryChallenges_CraftingChoice:
        tutorial->getCompleted(387);
        tutorial->setCompleted(387);
        break;
    default:
        break;
    }

    telemetryManagerInstance->RecordEnemyKilledOrOvercome(minecraft->mPlayer->mUserIndex, 0, 0, 0, 0, 0, 0,
                                                          telemetryChallenge);
}

bool ChoiceTask::isCompleted()
{
    Minecraft* minecraft = Minecraft::GetInstance();

    bool choiceMade = firstChoice;

    if (choiceMade || secondChoice)
    {
        sendTelemetry();
        enableConstraints(false, true);
        return true;
    }

    Tutorial* owner = tutorial;

    if (!gConsoleUIController.GetMenuDisplayed(owner->playerId))
    {
        boost::shared_ptr<MultiplayerLocalPlayer>& localPlayer = minecraft->mLocalPlayers[owner->playerId];

        if (localPlayer->isUnderLiquid(Material::WATER))
        {
            return false;
        }

        if (!firstChoice
            && g_pInput->GetValue(minecraft->mPlayer->mUserIndex, static_cast<unsigned char>(firstInput), false) > 0)
        {
            firstChoice = true;
        }

        if (!secondChoice
            && g_pInput->GetValue(minecraft->mPlayer->mUserIndex, static_cast<unsigned char>(secondInput), false) > 0)
        {
            secondChoice = true;
        }

        choiceMade = firstChoice;

        if (choiceMade || secondChoice)
        {
            sendTelemetry();
            enableConstraints(false, true);
            choiceMade = firstChoice;
        }
    }

    return choiceMade || secondChoice;
}

eTutorial_CompletionAction ChoiceTask::getCompletionAction()
{
    return secondChoice ? completionAction : eTutorial_CompletionAction_None;
}

int ChoiceTask::getPromptId()
{
    if (!shownForMinimumTime)
    {
        return -1;
    }
    return promptId;
}

void ChoiceTask::setAsCurrentTask(bool currentTask)
{
    enableConstraints(currentTask, false);
    TutorialTask::setAsCurrentTask(currentTask);
}

void ChoiceTask::handleUIInput(int input)
{
    if (!activated || !shownForMinimumTime)
    {
        return;
    }

    if (input == firstInput)
    {
        firstChoice = true;
    }
    else if (input == secondInput)
    {
        secondChoice = true;
    }
}
