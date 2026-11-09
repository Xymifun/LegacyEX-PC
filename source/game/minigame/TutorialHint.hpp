#ifndef GAME_MINIGAME_TUTORIALHINT_HPP
#define GAME_MINIGAME_TUTORIALHINT_HPP

#include <boost/shared_ptr.hpp>

#include "game/item/not_null_ptr.hpp"
#include "game/minigame/Tutorial.hpp"

class Block;
class BlockPos;
class BlockState;
class Entity;
class ItemInstance;
class Level;

class TutorialHint
{
public:
    enum eHintType
    {
        HINT_NONE = 0,
        HINT_START_DESTROY_BLOCK = 1,
        HINT_CREATE_ITEM_SELECTED = 2,
        HINT_ITEM_DAMAGED = 3,
        HINT_AREA = 5,
        HINT_LOOK_AT_BLOCK = 6,
        HINT_LOOK_AT_ENTITY = 7,
        HINT_WATER_MOVEMENT = 8,
        HINT_GAME_MODE = 9
    };

    TutorialHint(eTutorial_Hint hint, Tutorial* tutorial, int descriptionId, eHintType type, bool allowFade,
                 bool repeatMessage, int minimumDisplayTime);
    virtual ~TutorialHint() {}

    virtual int startDestroyBlock(not_null_ptr<ItemInstance> item, const BlockState* state);
    virtual unsigned int destroyBlock(Block* block);
    virtual unsigned int attack(not_null_ptr<ItemInstance> item, boost::shared_ptr<Entity> entity);
    virtual int createItemSelected(not_null_ptr<ItemInstance> item, bool selected);
    virtual int itemDamaged(not_null_ptr<ItemInstance> item);
    virtual unsigned int onTake(not_null_ptr<ItemInstance> item);
    virtual unsigned int onLookAt(const BlockState* state);
    virtual unsigned int onLookAtSkull(Level* level, const BlockPos& pos, const BlockState* state);
    virtual unsigned int onLookAtEntity(boost::shared_ptr<Entity> entity);
    virtual unsigned int onGameModeChange(eTutorial_GameMode gameMode);
    virtual int tick();
    unsigned int getMinimumDisplayTime();
    virtual unsigned char allowFade();

protected:
    friend class Tutorial;

    eHintType type;
    int descriptionId;
    Tutorial* tutorial;
    eTutorial_Hint hint;
    int consecutiveTicks;
    Block* lastBlock;
    bool active;
    bool fadeAllowed;
    int minimumTime;
};

#endif
