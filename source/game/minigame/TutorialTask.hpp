#ifndef GAME_MINIGAME_TUTORIALTASK_HPP
#define GAME_MINIGAME_TUTORIALTASK_HPP

#include <vector>
#include <boost/shared_ptr.hpp>
#include "game/item/not_null_ptr.hpp"
#include "game/minigame/Tutorial.hpp"

class BlockPos;
class Entity;
class ItemInstance;
class Level;
class MobEffect;
class TutorialConstraint;

class TutorialTask
{
public:
    TutorialTask(Tutorial* tutorial, int descriptionId, bool preCompletionEnabled,
                 std::vector<TutorialConstraint*>* constraints, bool showMinimumTime, bool allowFade,
                 bool repeatMessage);
    virtual ~TutorialTask();
    virtual int getDescriptionId();
    virtual int getPromptId();
    virtual bool isCompleted() = 0;
    virtual eTutorial_CompletionAction getCompletionAction();
    virtual bool isPreCompletionEnabled();
    virtual void taskCompleted();
    virtual void enableConstraints(bool enable, bool delayed);
    virtual void setAsCurrentTask(bool currentTask);
    virtual void setShownForMinimumTime();
    virtual bool hasBeenActivated();
    virtual bool AllowFade();
    virtual bool ShowMinimumTime();
    virtual void useItemOn(Level* level, not_null_ptr<ItemInstance> item, const BlockPos& pos, bool cancelled);
    virtual void useItem(not_null_ptr<ItemInstance> item, bool cancelled);
    virtual void completeUsingItem(not_null_ptr<ItemInstance> item);
    virtual void handleUIInput(int input);
    virtual void onCrafted(not_null_ptr<ItemInstance> item);
    virtual void onEquip(not_null_ptr<ItemInstance> item);
    virtual void onTake(not_null_ptr<ItemInstance> item, unsigned int itemCount, unsigned int exactCount);
    virtual void onTakeFromAnvil(not_null_ptr<ItemInstance> item);
    virtual void onStateChange(eTutorial_State state);
    virtual void onEffectChanged(MobEffect* effect, bool added);
    virtual void onLookAtEntity(boost::shared_ptr<Entity> entity);
    virtual void onRideEntity(boost::shared_ptr<Entity> entity);

protected:
    friend class Tutorial;

    int descriptionId;
    int promptId;
    Tutorial* tutorial;
    bool preCompletionEnabled;
    bool activated;
    bool allowFade;
    bool repeatMessage;
    bool showMinimumTime;
    bool completed;
    bool shownForMinimumTime;
    std::vector<TutorialConstraint*> constraints;
    bool constraintsEnabled;
};

#endif
