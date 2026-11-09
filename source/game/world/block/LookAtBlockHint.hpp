#ifndef GAME_WORLD_BLOCK_LOOKATBLOCKHINT_HPP
#define GAME_WORLD_BLOCK_LOOKATBLOCKHINT_HPP

#include "game/minigame/TutorialHint.hpp"

class BlockState;
class Block;
class BlockPos;
class Level;
class Property;

class LookAtBlockHint : public TutorialHint
{
public:
    LookAtBlockHint(eTutorial_Hint hint, Tutorial* tutorial, const BlockState** states, unsigned int stateCount,
                    const Property* const* properties, unsigned int propertyCount, int descriptionId, int auxValue,
                    bool useState);
    virtual ~LookAtBlockHint();

    virtual unsigned int onLookAt(const BlockState* state);
    virtual unsigned int onLookAtSkull(Level* level, const BlockPos& pos, const BlockState* state);

private:
    unsigned int mStateCount;
    const BlockState** mStates;
    Block** mStateIds;
    unsigned int mPropertyCount;
    const Property** mPropertyValues;
    int mDescriptionId;
    int mAuxValue;
    bool mUseState;
};

#endif
