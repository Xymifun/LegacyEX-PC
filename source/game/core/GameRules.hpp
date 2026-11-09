#ifndef GAME_CORE_GAMERULES_HPP
#define GAME_CORE_GAMERULES_HPP

class GameRules
{
public:
    enum ERule
    {
        RULE_DOFIRETICK = 0,
        RULE_MOBGRIEFING = 1,
        RULE_KEEPINVENTORY = 2,
        RULE_DOMOBSPAWNING = 3,
        RULE_DOMOBLOOT = 4,
        RULE_DOBLOCKDROPS = 5,
        RULE_DOENTITYDROPS = 6,
        RULE_NATURAL_REGENERATION = 7,
        RULE_DAYLIGHT = 8,
        RULE_SENDCOMMANDFEEDBACK = 9,
        RULE_SHOWDEATHMESSAGES = 11,
        RULE_SPAWN_RADIUS = 15,
        RULE_DISABLE_ELYTRA_MOVEMENT_CHECK = 16,
        RULE_MAX_ENTITY_CRAMMING = 17,
        RULE_WEATHER_CYCLE = 18
    };

    GameRules();
    ~GameRules();
    bool getBoolean(int rule);
    unsigned int getInt(int rule);
    void set(int rule, bool value);
};

#endif
