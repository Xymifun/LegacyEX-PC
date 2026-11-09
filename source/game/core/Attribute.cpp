#include "game/core/Attribute.hpp"

static const unsigned int kNameMaxHealth = 0xBADA09B9;
static const unsigned int kNameFollowRange = 0x46BDC66F;
static const unsigned int kNameKnockbackResistance = 0x102AF61B;
static const unsigned int kNameMovementSpeed = 0xAA6BDC35;
static const unsigned int kNameAttackDamage = 0xF25163D2;
static const unsigned int kNameHorseJumpStrength = 0x400FEB0C;
static const unsigned int kNameZombieSpawnReinforcements = 0xAD135001;
static const unsigned int kNameAttackSpeed = 0x1EA0E2FE;
static const unsigned int kNameArmor = 0x6CA19582;
static const unsigned int kNameArmorToughness = 0xAAE7CF20;
static const unsigned int kNameLuck = 0xC9BC92C8;
static const unsigned int kNameFlyingSpeed = 0;

static const unsigned int ATTRIBUTE_NAMES[eATTRIBUTE_ID_Count] = {kNameMaxHealth,
                                                                  kNameFollowRange,
                                                                  kNameKnockbackResistance,
                                                                  kNameMovementSpeed,
                                                                  kNameAttackDamage,
                                                                  kNameHorseJumpStrength,
                                                                  kNameZombieSpawnReinforcements,
                                                                  kNameAttackSpeed,
                                                                  kNameArmor,
                                                                  kNameArmorToughness,
                                                                  kNameLuck,
                                                                  kNameFlyingSpeed};

int Attribute::getName(eATTRIBUTE_ID id)
{
    return ATTRIBUTE_NAMES[id];
}
