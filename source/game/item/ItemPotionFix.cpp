#include "game/item/ItemPotionFix.hpp"
#include <string>
#include "game/data/CompoundTag.hpp"
#include "game/item/Item.hpp"
#include "game/item/Potion.hpp"
#include "game/item/PotionUtils.hpp"
#include "game/item/Items.hpp"
namespace
{
std::wstring blankPotionName;
std::wstring missingPotionName;
std::wstring potionNames[128];
}
ItemPotionFix::~ItemPotionFix()
{
}
int ItemPotionFix::getVersion()
{
    return 102;
}
CompoundTag* ItemPotionFix::fix(CompoundTag* tag)
{
    if (CheckID(tag, L"minecraft:potion"))
    {
        CompoundTag* itemTag = tag->getCompound(L"tag");
        short damage = tag->getShort(L"Damage");
        if (!itemTag->contains(L"Potion", Tag::TAG_STRING))
        {
            std::wstring name = potionNames[damage & 0x7f];
            itemTag->putString(L"Potion", name.empty() ? missingPotionName : name);
            tag->put(L"tag", itemTag);
            if ((damage & 0x4000) != 0)
                tag->putString(L"id", L"minecraft:splash_potion");
        }
        if (damage != 0)
            tag->putShort(L"Damage", 0);
    }
    return tag;
}
void ItemPotionFix::fix(not_null_ptr<ItemInstance> item)
{
    if (item->getItem() == Items::potion || item->getItem() == Items::splashPotion
        || item->getItem() == Items::lingeringPotion || item->getItem() == Items::tippedArrow)
    {
        short auxValue = item->getAuxValue();
        std::wstring name = potionNames[auxValue & 0x7f];
        if (name.empty())
            name = missingPotionName;
        PotionUtils::setPotion(item, Potion::byName(name));
        if (auxValue != 0)
            item->setAuxValue(0);
    }
}
void ItemPotionFix::staticCtor()
{
    potionNames[0] = L"minecraft:water";
    potionNames[1] = L"minecraft:regeneration";
    potionNames[2] = L"minecraft:swiftness";
    potionNames[3] = L"minecraft:fire_resistance";
    potionNames[4] = L"minecraft:poison";
    potionNames[5] = L"minecraft:healing";
    potionNames[6] = L"minecraft:night_vision";
    potionNames[7] = blankPotionName;
    potionNames[8] = L"minecraft:weakness";
    potionNames[9] = L"minecraft:strength";
    potionNames[10] = L"minecraft:slowness";
    potionNames[11] = L"minecraft:leaping";
    potionNames[12] = L"minecraft:harming";
    potionNames[13] = L"minecraft:water_breathing";
    potionNames[14] = L"minecraft:invisibility";
    potionNames[15] = blankPotionName;
    potionNames[16] = L"minecraft:awkward";
    potionNames[17] = L"minecraft:regeneration";
    potionNames[18] = L"minecraft:swiftness";
    potionNames[19] = L"minecraft:fire_resistance";
    potionNames[20] = L"minecraft:poison";
    potionNames[21] = L"minecraft:healing";
    potionNames[22] = L"minecraft:night_vision";
    potionNames[23] = blankPotionName;
    potionNames[24] = L"minecraft:weakness";
    potionNames[25] = L"minecraft:strength";
    potionNames[26] = L"minecraft:slowness";
    potionNames[27] = L"minecraft:leaping";
    potionNames[28] = L"minecraft:harming";
    potionNames[29] = L"minecraft:water_breathing";
    potionNames[30] = L"minecraft:invisibility";
    potionNames[31] = blankPotionName;
    potionNames[32] = L"minecraft:thick";
    potionNames[33] = L"minecraft:strong_regeneration";
    potionNames[34] = L"minecraft:strong_swiftness";
    potionNames[35] = L"minecraft:fire_resistance";
    potionNames[36] = L"minecraft:strong_poison";
    potionNames[37] = L"minecraft:strong_healing";
    potionNames[38] = L"minecraft:night_vision";
    potionNames[39] = blankPotionName;
    potionNames[40] = L"minecraft:weakness";
    potionNames[41] = L"minecraft:strong_strength";
    potionNames[42] = L"minecraft:slowness";
    potionNames[43] = L"minecraft:strong_leaping";
    potionNames[44] = L"minecraft:strong_harming";
    potionNames[45] = L"minecraft:water_breathing";
    potionNames[46] = L"minecraft:invisibility";
    potionNames[47] = blankPotionName;
    potionNames[48] = blankPotionName;
    potionNames[49] = L"minecraft:strong_regeneration";
    potionNames[50] = L"minecraft:strong_swiftness";
    potionNames[51] = L"minecraft:fire_resistance";
    potionNames[52] = L"minecraft:strong_poison";
    potionNames[53] = L"minecraft:strong_healing";
    potionNames[54] = L"minecraft:night_vision";
    potionNames[55] = blankPotionName;
    potionNames[56] = L"minecraft:weakness";
    potionNames[57] = L"minecraft:strong_strength";
    potionNames[58] = L"minecraft:slowness";
    potionNames[59] = L"minecraft:strong_leaping";
    potionNames[60] = L"minecraft:strong_harming";
    potionNames[61] = L"minecraft:water_breathing";
    potionNames[62] = L"minecraft:invisibility";
    potionNames[63] = blankPotionName;
    potionNames[64] = L"minecraft:mundane";
    potionNames[65] = L"minecraft:long_regeneration";
    potionNames[66] = L"minecraft:long_swiftness";
    potionNames[67] = L"minecraft:long_fire_resistance";
    potionNames[68] = L"minecraft:long_poison";
    potionNames[69] = L"minecraft:healing";
    potionNames[70] = L"minecraft:long_night_vision";
    potionNames[71] = blankPotionName;
    potionNames[72] = L"minecraft:long_weakness";
    potionNames[73] = L"minecraft:long_strength";
    potionNames[74] = L"minecraft:long_slowness";
    potionNames[75] = L"minecraft:long_leaping";
    potionNames[76] = L"minecraft:harming";
    potionNames[77] = L"minecraft:long_water_breathing";
    potionNames[78] = L"minecraft:long_invisibility";
    potionNames[79] = blankPotionName;
    potionNames[80] = L"minecraft:awkward";
    potionNames[81] = L"minecraft:long_regeneration";
    potionNames[82] = L"minecraft:long_swiftness";
    potionNames[83] = L"minecraft:long_fire_resistance";
    potionNames[84] = L"minecraft:long_poison";
    potionNames[85] = L"minecraft:healing";
    potionNames[86] = L"minecraft:long_night_vision";
    potionNames[87] = blankPotionName;
    potionNames[88] = L"minecraft:long_weakness";
    potionNames[89] = L"minecraft:long_strength";
    potionNames[90] = L"minecraft:long_slowness";
    potionNames[91] = L"minecraft:long_leaping";
    potionNames[92] = L"minecraft:harming";
    potionNames[93] = L"minecraft:long_water_breathing";
    potionNames[94] = L"minecraft:long_invisibility";
    potionNames[95] = blankPotionName;
    potionNames[96] = L"minecraft:thick";
    potionNames[97] = L"minecraft:regeneration";
    potionNames[98] = L"minecraft:swiftness";
    potionNames[99] = L"minecraft:long_fire_resistance";
    potionNames[100] = L"minecraft:poison";
    potionNames[101] = L"minecraft:strong_healing";
    potionNames[102] = L"minecraft:long_night_vision";
    potionNames[103] = blankPotionName;
    potionNames[104] = L"minecraft:long_weakness";
    potionNames[105] = L"minecraft:strength";
    potionNames[106] = L"minecraft:long_slowness";
    potionNames[107] = L"minecraft:leaping";
    potionNames[108] = L"minecraft:strong_harming";
    potionNames[109] = L"minecraft:long_water_breathing";
    potionNames[110] = L"minecraft:long_invisibility";
    potionNames[111] = blankPotionName;
    potionNames[112] = blankPotionName;
    potionNames[113] = L"minecraft:regeneration";
    potionNames[114] = L"minecraft:swiftness";
    potionNames[115] = L"minecraft:long_fire_resistance";
    potionNames[116] = L"minecraft:poison";
    potionNames[117] = L"minecraft:strong_healing";
    potionNames[118] = L"minecraft:long_night_vision";
    potionNames[119] = blankPotionName;
    potionNames[120] = L"minecraft:long_weakness";
    potionNames[121] = L"minecraft:strength";
    potionNames[122] = L"minecraft:long_slowness";
    potionNames[123] = L"minecraft:leaping";
    potionNames[124] = L"minecraft:strong_harming";
    potionNames[125] = L"minecraft:long_water_breathing";
    potionNames[126] = L"minecraft:long_invisibility";
    potionNames[127] = blankPotionName;
}
