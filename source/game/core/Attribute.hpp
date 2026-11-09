#ifndef GAME_CORE_ATTRIBUTE_HPP
#define GAME_CORE_ATTRIBUTE_HPP

enum eATTRIBUTE_ID
{
    eATTRIBUTE_ID_MaxHealth,
    eATTRIBUTE_ID_FollowRange,
    eATTRIBUTE_ID_KnockbackResistance,
    eATTRIBUTE_ID_MovementSpeed,
    eATTRIBUTE_ID_AttackDamage,
    eATTRIBUTE_ID_HorseJumpStrength,
    eATTRIBUTE_ID_ZombieSpawnReinforcements,
    eATTRIBUTE_ID_AttackSpeed,
    eATTRIBUTE_ID_Armor,
    eATTRIBUTE_ID_ArmorToughness,
    eATTRIBUTE_ID_Luck,
    eATTRIBUTE_ID_FlyingSpeed,
    eATTRIBUTE_ID_Count
};

class Attribute
{
public:
    virtual ~Attribute() {}

    virtual eATTRIBUTE_ID getId() = 0;
    virtual double sanitizeValue(double value) = 0;
    virtual double getDefaultValue() = 0;
    virtual bool isClientSyncable() = 0;
    virtual Attribute* getParentAttribute() = 0;

    static int getName(eATTRIBUTE_ID id);
};

#endif
