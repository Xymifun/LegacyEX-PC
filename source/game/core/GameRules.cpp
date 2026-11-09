#include "game/core/CConsoleMinecraftApp.hpp"
#include "game/core/GameRules.hpp"
#include "game/core/CMinecraftApp.hpp"

GameRules::GameRules()
{
}

GameRules::~GameRules()
{
}

bool GameRules::getBoolean(int rule)
{
    switch (rule)
    {
    case RULE_DOFIRETICK:
        return gApp.GetGameHostOption(eGameHostOption_FireSpreads) != 0;
    case RULE_MOBGRIEFING:
        return gApp.GetGameHostOption(eGameHostOption_MobGriefing) != 0;
    case RULE_KEEPINVENTORY:
        return gApp.GetGameHostOption(eGameHostOption_KeepInventory) != 0;
    case RULE_DOMOBSPAWNING:
        return gApp.GetGameHostOption(eGameHostOption_DoMobSpawning) != 0;
    case RULE_DOMOBLOOT:
        return gApp.GetGameHostOption(eGameHostOption_DoMobLoot) != 0;
    case RULE_DOBLOCKDROPS:
    case RULE_DOENTITYDROPS:
        return gApp.GetGameHostOption(eGameHostOption_DoTileDrops) != 0;
    case RULE_NATURAL_REGENERATION:
        return gApp.GetGameHostOption(eGameHostOption_NaturalRegeneration) != 0;
    case RULE_DAYLIGHT:
        return gApp.GetGameHostOption(eGameHostOption_DoDaylightCycle) != 0
               && gApp.GetGameHostOption(eGameHostOption_MiniGame) == 0;
    case RULE_SENDCOMMANDFEEDBACK:
        return true;
    case RULE_SHOWDEATHMESSAGES:
        return true;
    case RULE_WEATHER_CYCLE:
        return gApp.GetGameHostOption(eGameHostOption_DoWeatherCycle) != 0;
    default:
        return false;
    }
}

unsigned int GameRules::getInt(int rule)
{
    if (rule == RULE_SPAWN_RADIUS)
        return 10;
    if (rule != RULE_MAX_ENTITY_CRAMMING)
        return 0;
    return 0x18;
}

void GameRules::set(int rule, bool value)
{
}
