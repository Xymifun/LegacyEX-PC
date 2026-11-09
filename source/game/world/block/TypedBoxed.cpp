#include "game/world/block/TypedBoxed.hpp"

#include "game/world/block/ComparatorBlock.hpp"
#include "game/world/block/DoorBlock.hpp"
#include "game/world/block/RedSandStoneBlock.hpp"

#include <string>

template std::wstring _toString<RedSandStoneBlock::Variant>(RedSandStoneBlock::Variant*);
template std::wstring _toString<ComparatorBlock::ComparatorMode>(ComparatorBlock::ComparatorMode*);
template std::wstring _toString<DoorBlock::DoorHalf>(DoorBlock::DoorHalf*);
