#ifndef GAME_ENTITY_MOB_LOOKATENTITYHINT_HPP
#define GAME_ENTITY_MOB_LOOKATENTITYHINT_HPP

#include <boost/shared_ptr.hpp>

#include "game/item/not_null_ptr.hpp"

#include "game/minigame/TutorialHint.hpp"

class Entity;
class ItemInstance;

class LookAtEntityHint : public TutorialHint
{
public:
    LookAtEntityHint(eTutorial_Hint hint, Tutorial* tutorial, int descriptionId, int entityId,
                     unsigned long long entityType);
    LookAtEntityHint(eTutorial_Hint hint, Tutorial* tutorial, int descriptionId, int entityId,
                     unsigned long long entityType, bool (*entityPredicate)(boost::shared_ptr<Entity>));
    virtual ~LookAtEntityHint();

    not_null_ptr<ItemInstance> GetEntityAsItem(boost::shared_ptr<Entity> entity);
    virtual unsigned int onLookAtEntity(boost::shared_ptr<Entity> entity);

private:
    unsigned long long entityType;
    int entityId;
    bool (*entityPredicate)(boost::shared_ptr<Entity>);
};

#endif
