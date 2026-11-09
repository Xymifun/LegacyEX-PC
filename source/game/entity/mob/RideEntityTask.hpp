#ifndef GAME_ENTITY_MOB_RIDEENTITYTASK_HPP
#define GAME_ENTITY_MOB_RIDEENTITYTASK_HPP

#include <vector>

#include <boost/shared_ptr.hpp>

#include "game/minigame/TutorialTask.hpp"

class Entity;
class Tutorial;
class TutorialConstraint;

class RideEntityTask : public TutorialTask
{
public:
    RideEntityTask(int entityType, Tutorial* tutorial, int descriptionId, bool preCompletionEnabled,
                   std::vector<TutorialConstraint*>* constraints, bool showMinimumTime, bool allowFade,
                   bool repeatMessage);


    virtual bool isCompleted();
    virtual void onRideEntity(boost::shared_ptr<Entity> entity);

    int mEntityType;
};
#endif
