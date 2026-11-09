#ifndef GAME_CORE_DISPENSERBOOTSTRAP_HPP
#define GAME_CORE_DISPENSERBOOTSTRAP_HPP

#include "game/core/Boat.hpp"
#include "game/item/DefaultDispenseItemBehavior.hpp"
#include "game/item/not_null_ptr.hpp"

class BlockSource;
class ItemInstance;

class DispenserBootstrap
{
public:
    class OptionalDispenseItemBehavior : public DefaultDispenseItemBehavior
    {
    public:
        virtual ~OptionalDispenseItemBehavior() {}
        virtual void playSound(BlockSource*, DefaultDispenseItemBehavior::eOUTCOME);
    };

    class BoatDispenseBehavior : public DefaultDispenseItemBehavior
    {
    public:
        explicit BoatDispenseBehavior(Boat::Type type);
        virtual ~BoatDispenseBehavior();
        virtual not_null_ptr<ItemInstance> execute(BlockSource*, not_null_ptr<ItemInstance>,
                                                   DefaultDispenseItemBehavior::eOUTCOME&);
        virtual void playSound(BlockSource*, DefaultDispenseItemBehavior::eOUTCOME);

    private:
        Boat::Type mType;
        DefaultDispenseItemBehavior mDefaultDispenseItemBehavior;
    };

    class ShulkerBoxDispenseBehavior : public OptionalDispenseItemBehavior
    {
    public:
        virtual ~ShulkerBoxDispenseBehavior();
        virtual not_null_ptr<ItemInstance> execute(BlockSource*, not_null_ptr<ItemInstance>,
                                                   DefaultDispenseItemBehavior::eOUTCOME&);
    };

    static void StaticCtor();
};

#endif
