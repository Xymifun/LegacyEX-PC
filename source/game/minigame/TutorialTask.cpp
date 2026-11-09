#include "game/minigame/TutorialTask.hpp"

#include "game/core/GameModeConstraints.hpp"

TutorialTask::TutorialTask(Tutorial* tutorial, int descriptionId, bool preCompletionEnabled,
                           std::vector<TutorialConstraint*>* constraints, bool showMinimumTime, bool allowFade,
                           bool repeatMessage)
    : descriptionId(descriptionId),
      promptId(-1),
      tutorial(tutorial),
      preCompletionEnabled(preCompletionEnabled),
      activated(false),
      allowFade(allowFade),
      repeatMessage(repeatMessage),
      showMinimumTime(showMinimumTime),
      completed(false),
      shownForMinimumTime(false),
      constraintsEnabled(false)
{
    if (constraints != NULL)
    {
        std::vector<TutorialConstraint*>::iterator constraint = constraints->begin();
        while (constraint != constraints->end())
        {
            this->constraints.push_back(*constraint);
            ++constraint;
        }
        delete constraints;
    }

    tutorial->addMessage(descriptionId, false, 3);
}

TutorialTask::~TutorialTask()
{
    enableConstraints(false, false);

    for (std::vector<TutorialConstraint*>::iterator constraint = constraints.begin(); constraint < constraints.end();
         ++constraint)
    {
        TutorialConstraint* currentConstraint = *constraint;
        if (currentConstraint->removalPending)
        {
            currentConstraint->deleteWhenRemoved = true;
        }
        else
        {
            delete currentConstraint;
        }
    }
}

void TutorialTask::enableConstraints(bool enable, bool delayed)
{
    if (!enable)
    {
        if (constraintsEnabled || !delayed)
        {
            std::vector<TutorialConstraint*>::iterator constraint = constraints.begin();
            while (constraint != constraints.end())
            {
                tutorial->gameModeConstraints->RemoveConstraint(*constraint, delayed);
                ++constraint;
            }
            constraintsEnabled = false;
        }
    }
    else if (!constraintsEnabled)
    {
        std::vector<TutorialConstraint*>::iterator constraint = constraints.begin();
        while (constraint != constraints.end())
        {
            tutorial->gameModeConstraints->AddConstraint(*constraint, false);
            ++constraint;
        }
        constraintsEnabled = true;
    }
}

void TutorialTask::taskCompleted()
{
    if (constraintsEnabled)
    {
        enableConstraints(false, false);
    }
}

void TutorialTask::setAsCurrentTask(bool currentTask)
{
    activated = currentTask;
}

int TutorialTask::getDescriptionId()
{
    return descriptionId;
}

int TutorialTask::getPromptId()
{
    return promptId;
}

eTutorial_CompletionAction TutorialTask::getCompletionAction()
{
    return eTutorial_CompletionAction_None;
}

bool TutorialTask::isPreCompletionEnabled()
{
    return preCompletionEnabled;
}

void TutorialTask::setShownForMinimumTime()
{
    shownForMinimumTime = true;
}

bool TutorialTask::hasBeenActivated()
{
    return activated;
}

bool TutorialTask::AllowFade()
{
    return allowFade;
}

bool TutorialTask::ShowMinimumTime()
{
    return showMinimumTime;
}

void TutorialTask::useItemOn(Level*, not_null_ptr<ItemInstance>, const BlockPos&, bool)
{
}

void TutorialTask::useItem(not_null_ptr<ItemInstance>, bool)
{
}

void TutorialTask::completeUsingItem(not_null_ptr<ItemInstance>)
{
}

void TutorialTask::handleUIInput(int)
{
}

void TutorialTask::onCrafted(not_null_ptr<ItemInstance>)
{
}

void TutorialTask::onEquip(not_null_ptr<ItemInstance>)
{
}

void TutorialTask::onTake(not_null_ptr<ItemInstance>, unsigned int, unsigned int)
{
}

void TutorialTask::onTakeFromAnvil(not_null_ptr<ItemInstance>)
{
}

void TutorialTask::onStateChange(eTutorial_State)
{
}

void TutorialTask::onEffectChanged(MobEffect*, bool)
{
}

void TutorialTask::onLookAtEntity(boost::shared_ptr<Entity>)
{
}

void TutorialTask::onRideEntity(boost::shared_ptr<Entity>)
{
}
