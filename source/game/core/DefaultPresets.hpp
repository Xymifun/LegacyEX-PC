#ifndef GAME_CORE_DEFAULTPRESETS_HPP
#define GAME_CORE_DEFAULTPRESETS_HPP

class SuperflatConfig;

class DefaultPresets
{
public:
    enum Preset
    {
        CLASSIC,
        TUNNELERS_DREAM,
        WATER_WORLD,
        OVERWORLD,
        SNOWY_KINGDOM,
        BOTTOMLESS_PIT,
        DESERT,
        REDSTONE_READY,
        THE_VOID,
        DEBUG_JUNGLE,
        DEBUG_WOODLAND_MANSION,
        DEBUG_OCEAN_MONUMENT,
        DEBUG_WARM_OCEAN
    };

    static void makePreset(SuperflatConfig* config, Preset preset);
};

#endif
