#ifndef GAME_CORE_MUSHROOMCOW_HPP
#define GAME_CORE_MUSHROOMCOW_HPP

#include <boost/shared_ptr.hpp>

#include "game/core/Cow.hpp"
#include "game/core/InteractionHand.hpp"

class DataFixerUpper;
class EntityTooltipDataHolder;
class Level;

class MushroomCow : public Cow
{
public:
    explicit MushroomCow(Level* level);

    static Entity* create(Level* level);
    static void addDataWalkers(DataFixerUpper* fixer);
    virtual bool mobInteract(const boost::shared_ptr<Player>& player, InteractionHand::EInteractionHand hand);
    virtual bool checkSpawnRules();
    virtual boost::shared_ptr<AgableMob> getBreedOffspring(boost::shared_ptr<AgableMob> parent);
    virtual unsigned int GetInteractTooltip(const EntityTooltipDataHolder& tooltipData);
    virtual unsigned long long GetType();
};

#endif
