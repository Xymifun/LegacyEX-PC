#ifndef GAME_CLIENT_RENDER_PARTICLETYPE_HPP
#define GAME_CLIENT_RENDER_PARTICLETYPE_HPP

#include <string>

enum ePARTICLE_TYPE
{
    ePARTICLE_BUBBLE = 0x00,
    ePARTICLE_SMOKE = 0x01,
    ePARTICLE_NOTE = 0x02,
    ePARTICLE_PORTAL = 0x03,
    ePARTICLE_END_PORTAL = 0x04,
    ePARTICLE_EXPLODE = 0x05,
    ePARTICLE_FLAME = 0x06,
    ePARTICLE_LAVA = 0x07,
    ePARTICLE_FOOTSTEP = 0x08,
    ePARTICLE_SPLASH = 0x09,
    ePARTICLE_LARGE_SMOKE = 0x0A,
    ePARTICLE_RED_DUST = 0x0B,
    ePARTICLE_SNOWBALL_POOF = 0x0C,
    ePARTICLE_SNOW_SHOVEL = 0x0D,
    ePARTICLE_SLIME = 0x0E,
    ePARTICLE_HEART = 0x0F,
    ePARTICLE_SUSPENDED = 0x10,
    ePARTICLE_DEPTH_SUSPEND = 0x11,
    ePARTICLE_CRIT = 0x12,
    ePARTICLE_HUGE_EXPLOSION = 0x13,
    ePARTICLE_LARGE_EXPLODE = 0x14,
    ePARTICLE_TOWN_AURA = 0x15,
    ePARTICLE_SPELL = 0x16,
    ePARTICLE_WITCH_MAGIC = 0x17,
    ePARTICLE_MOB_SPELL = 0x18,
    ePARTICLE_MOB_SPELL_AMBIENT = 0x19,
    ePARTICLE_DRIP_WATER = 0x1B,
    ePARTICLE_DRIP_LAVA = 0x1C,
    ePARTICLE_ENCHANTMENT_TABLE = 0x1D,
    ePARTICLE_ENDER = 0x1E,
    ePARTICLE_ANGRY_VILLAGER = 0x1F,
    ePARTICLE_HAPPY_VILLAGER = 0x20,
    ePARTICLE_FIREWORKS_SPARK = 0x21,
    ePARTICLE_WAKE = 0x22,
    ePARTICLE_MAGIC_CRIT = 0x23,
    ePARTICLE_INSTANT_SPELL = 0x24,
    ePARTICLE_CLOUD = 0x25,
    ePARTICLE_BARRIER = 0x26,
    ePARTICLE_ICON_CRACK = 0x27,
    ePARTICLE_BLOCK_DUST = 0x28,
    ePARTICLE_DROPLET = 0x29,
    ePARTICLE_TAKE = 0x2A,
    ePARTICLE_MOB_APPEARANCE = 0x2B,
    ePARTICLE_DRAGON_BREATH = 0x2C,
    ePARTICLE_END_ROD = 0x2D,
    ePARTICLE_DAMAGE_INDICATOR = 0x2E,
    ePARTICLE_SWEEP_ATTACK = 0x2F,
    ePARTICLE_FALLING_DUST = 0x30,
    ePARTICLE_TOTEM = 0x31,
    ePARTICLE_INK = 0x32,
    ePARTICLE_SPIT = 0x33,
    ePARTICLE_BUBBLE_POP = 0x34,
    ePARTICLE_CURRENT_DOWN = 0x35,
    ePARTICLE_CONDUIT = 0x36,
    ePARTICLE_BLOCK_CRACK = 0x200000,
    ePARTICLE_CHEST_REFILL = 0x300000,
    ePARTICLE_WINNING_INSTANT_SPELL = 0x300001
};

class ParticleType
{
public:
    ParticleType(const std::string& name, ePARTICLE_TYPE id, bool overrideLimiter, int paramCount);

    std::string getName() const;
    ePARTICLE_TYPE getId() const;
    bool isPartial() const;
    int getParamCount() const;
    bool getOverrideLimiter() const;

    static const ParticleType* byId(ePARTICLE_TYPE id);
    static void staticCtor();

private:
    std::string name;
    ePARTICLE_TYPE id;
    bool overrideLimiter;
    int paramCount;
};

extern const ParticleType* PARTICLE_EXPLODE;
extern const ParticleType* PARTICLE_LARGE_EXPLODE;
extern const ParticleType* PARTICLE_HUGE_EXPLOSION;
extern const ParticleType* PARTICLE_FIREWORKS_SPARK;
extern const ParticleType* PARTICLE_BUBBLE;
extern const ParticleType* PARTICLE_SPLASH;
extern const ParticleType* PARTICLE_WAKE;
extern const ParticleType* PARTICLE_SUSPENDED;
extern const ParticleType* PARTICLE_DEPTH_SUSPEND;
extern const ParticleType* PARTICLE_CRIT;
extern const ParticleType* PARTICLE_MAGIC_CRIT;
extern const ParticleType* PARTICLE_SMOKE;
extern const ParticleType* PARTICLE_LARGE_SMOKE;
extern const ParticleType* PARTICLE_SPELL;
extern const ParticleType* PARTICLE_INSTANT_SPELL;
extern const ParticleType* PARTICLE_MOB_SPELL;
extern const ParticleType* PARTICLE_MOB_SPELL_AMBIENT;
extern const ParticleType* PARTICLE_WITCH_MAGIC;
extern const ParticleType* PARTICLE_DRIP_WATER;
extern const ParticleType* PARTICLE_DRIP_LAVA;
extern const ParticleType* PARTICLE_ANGRY_VILLAGER;
extern const ParticleType* PARTICLE_HAPPY_VILLAGER;
extern const ParticleType* PARTICLE_TOWN_AURA;
extern const ParticleType* PARTICLE_NOTE;
extern const ParticleType* PARTICLE_PORTAL;
extern const ParticleType* PARTICLE_END_PORTAL;
extern const ParticleType* PARTICLE_ENCHANTMENT_TABLE;
extern const ParticleType* PARTICLE_ENDER;
extern const ParticleType* PARTICLE_FLAME;
extern const ParticleType* PARTICLE_LAVA;
extern const ParticleType* PARTICLE_FOOTSTEP;
extern const ParticleType* PARTICLE_CLOUD;
extern const ParticleType* PARTICLE_RED_DUST;
extern const ParticleType* PARTICLE_SNOWBALL_POOF;
extern const ParticleType* PARTICLE_SNOW_SHOVEL;
extern const ParticleType* PARTICLE_SLIME;
extern const ParticleType* PARTICLE_HEART;
extern const ParticleType* PARTICLE_BARRIER;
extern const ParticleType* PARTICLE_ICON_CRACK;
extern const ParticleType* PARTICLE_BLOCK_CRACK;
extern const ParticleType* PARTICLE_BLOCK_DUST;
extern const ParticleType* PARTICLE_DROPLET;
extern const ParticleType* PARTICLE_TAKE;
extern const ParticleType* PARTICLE_MOB_APPEARANCE;
extern const ParticleType* PARTICLE_DRAGON_BREATH;
extern const ParticleType* PARTICLE_END_ROD;
extern const ParticleType* PARTICLE_DAMAGE_INDICATOR;
extern const ParticleType* PARTICLE_SWEEP_ATTACK;
extern const ParticleType* PARTICLE_FALLING_DUST;
extern const ParticleType* PARTICLE_TOTEM;
extern const ParticleType* PARTICLE_INK;
extern const ParticleType* PARTICLE_SPIT;
extern const ParticleType* PARTICLE_BUBBLE_POP;
extern const ParticleType* PARTICLE_CURRENT_DOWN;
extern const ParticleType* PARTICLE_CONDUIT;
extern const ParticleType* PARTICLE_CHEST_REFILL;
extern const ParticleType* PARTICLE_WINNING_INSTANT_SPELL;

#endif
