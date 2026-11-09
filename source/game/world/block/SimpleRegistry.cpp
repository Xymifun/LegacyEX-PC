#include "game/world/block/SimpleRegistry.hpp"

#include "game/util/ResourceLocation.hpp"

class Block;
class Enchantment;
class Item;
class MobEffect;
class Potion;
class ItemPropertyFunction;
class SoundEvent;

template class SimpleRegistry<ResourceLocation, Block*, boost::hash<ResourceLocation>,
                              std::equal_to<ResourceLocation> >;
template class SimpleRegistry<ResourceLocation, Enchantment*, boost::hash<ResourceLocation>,
                              std::equal_to<ResourceLocation> >;
template class SimpleRegistry<ResourceLocation, Item*, boost::hash<ResourceLocation>, std::equal_to<ResourceLocation> >;
template class SimpleRegistry<ResourceLocation, MobEffect*, boost::hash<ResourceLocation>,
                              std::equal_to<ResourceLocation> >;
template class SimpleRegistry<ResourceLocation, Potion*, boost::hash<ResourceLocation>,
                              std::equal_to<ResourceLocation> >;
template class SimpleRegistry<ResourceLocation, const ItemPropertyFunction*, boost::hash<ResourceLocation>,
                              std::equal_to<ResourceLocation> >;
template class SimpleRegistry<ResourceLocation, const SoundEvent*, boost::hash<ResourceLocation>,
                              std::equal_to<ResourceLocation> >;
template class SimpleRegistry<ResourceLocation, unsigned long long, boost::hash<ResourceLocation>,
                              std::equal_to<ResourceLocation> >;
