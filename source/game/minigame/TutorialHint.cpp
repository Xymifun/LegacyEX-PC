#include "game/minigame/TutorialHint.hpp"

#include "game/core/Minecraft.hpp"
#include "game/world/block/BlockState.hpp"
#include "game/world/block/Material.hpp"
#include "game/world/generation/MultiplayerLocalPlayer.hpp"

TutorialHint::TutorialHint(eTutorial_Hint hintValue, Tutorial* tutorialValue, int description, eHintType hintType,
                           bool allowFadeValue, bool repeatMessage, int minimumDisplayTime)
    : type(hintType),
      descriptionId(description),
      tutorial(tutorialValue),
      hint(hintValue),
      consecutiveTicks(0),
      lastBlock(NULL),
      active(true),
      fadeAllowed(allowFadeValue),
      minimumTime(minimumDisplayTime)
{
    tutorialValue->addMessage(description, repeatMessage && hintType != HINT_CREATE_ITEM_SELECTED, 3);
}

int TutorialHint::startDestroyBlock(not_null_ptr<ItemInstance> item, const BlockState* state)
{
    int result = -1;
    if (type == HINT_START_DESTROY_BLOCK)
    {
        if (state->getBlock() == lastBlock && active)
        {
            if (++consecutiveTicks > 20)
            {
                result = descriptionId;
                lastBlock = state->getBlock();
                return result;
            }
        }
        else
        {
            consecutiveTicks = 0;
        }
        lastBlock = state->getBlock();
    }
    return result;
}

unsigned int TutorialHint::destroyBlock(Block* block)
{
    if (type == HINT_START_DESTROY_BLOCK && block == lastBlock && consecutiveTicks > 0)
    {
        active = false;
    }
    return 0xffffffffU;
}

unsigned int TutorialHint::attack(not_null_ptr<ItemInstance> item, boost::shared_ptr<Entity> entity)
{
    return 0xffffffffU;
}

int TutorialHint::createItemSelected(not_null_ptr<ItemInstance> item, bool selected)
{
    int result = -1;
    if (type == HINT_CREATE_ITEM_SELECTED && !selected)
    {
        result = descriptionId;
    }
    return result;
}

int TutorialHint::itemDamaged(not_null_ptr<ItemInstance> item)
{
    int result = -1;
    if (type == HINT_ITEM_DAMAGED)
    {
        result = descriptionId;
    }
    return result;
}

unsigned int TutorialHint::onTake(not_null_ptr<ItemInstance> item)
{
    return 0;
}

unsigned int TutorialHint::onLookAt(const BlockState* state)
{
    return 0;
}

unsigned int TutorialHint::onLookAtSkull(Level* level, const BlockPos& pos, const BlockState* state)
{
    return 0;
}

unsigned int TutorialHint::onLookAtEntity(boost::shared_ptr<Entity> entity)
{
    return 0;
}

unsigned int TutorialHint::onGameModeChange(eTutorial_GameMode gameMode)
{
    return 0xffffffffU;
}

int TutorialHint::tick()
{
    int result = -1;
    if (type == HINT_WATER_MOVEMENT)
    {
        Tutorial* owner = tutorial;
        Minecraft* minecraft = Minecraft::GetInstance();
        if (minecraft->mLocalPlayers[owner->playerId]->isUnderLiquid(Material::WATER))
        {
            owner = tutorial;
            Minecraft* current = Minecraft::GetInstance();
            if (!current->mLocalPlayers[owner->playerId]->isPassenger())
            {
                result = descriptionId;
            }
        }
    }
    return result;
}

unsigned int TutorialHint::getMinimumDisplayTime()
{
    return (unsigned int)minimumTime;
}

unsigned char TutorialHint::allowFade()
{
    return (unsigned char)fadeAllowed;
}
