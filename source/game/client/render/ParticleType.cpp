#include "game/client/render/ParticleType.hpp"

#include <boost/unordered_map.hpp>

const ParticleType* PARTICLE_EXPLODE = new ParticleType("explode", ePARTICLE_EXPLODE, true, 0);
const ParticleType* PARTICLE_LARGE_EXPLODE = new ParticleType("largeexplode", ePARTICLE_LARGE_EXPLODE, true, 0);
const ParticleType* PARTICLE_HUGE_EXPLOSION = new ParticleType("hugeexplosion", ePARTICLE_HUGE_EXPLOSION, true, 0);
const ParticleType* PARTICLE_FIREWORKS_SPARK = new ParticleType("fireworksSpark", ePARTICLE_FIREWORKS_SPARK, false, 0);
const ParticleType* PARTICLE_BUBBLE = new ParticleType("bubble", ePARTICLE_BUBBLE, false, 0);
const ParticleType* PARTICLE_SPLASH = new ParticleType("splash", ePARTICLE_SPLASH, false, 0);
const ParticleType* PARTICLE_WAKE = new ParticleType("wake", ePARTICLE_WAKE, false, 0);
const ParticleType* PARTICLE_SUSPENDED = new ParticleType("suspended", ePARTICLE_SUSPENDED, false, 0);
const ParticleType* PARTICLE_DEPTH_SUSPEND = new ParticleType("depthsuspend", ePARTICLE_DEPTH_SUSPEND, false, 0);
const ParticleType* PARTICLE_CRIT = new ParticleType("crit", ePARTICLE_CRIT, false, 0);
const ParticleType* PARTICLE_MAGIC_CRIT = new ParticleType("magicCrit", ePARTICLE_MAGIC_CRIT, false, 0);
const ParticleType* PARTICLE_SMOKE = new ParticleType("smoke", ePARTICLE_SMOKE, false, 0);
const ParticleType* PARTICLE_LARGE_SMOKE = new ParticleType("largesmoke", ePARTICLE_LARGE_SMOKE, false, 0);
const ParticleType* PARTICLE_SPELL = new ParticleType("spell", ePARTICLE_SPELL, false, 0);
const ParticleType* PARTICLE_INSTANT_SPELL = new ParticleType("instantSpell", ePARTICLE_INSTANT_SPELL, false, 0);
const ParticleType* PARTICLE_MOB_SPELL = new ParticleType("mobSpell", ePARTICLE_MOB_SPELL, false, 0);
const ParticleType* PARTICLE_MOB_SPELL_AMBIENT = new ParticleType("mobSpellAmbient", ePARTICLE_MOB_SPELL_AMBIENT, false,
                                                                  0);
const ParticleType* PARTICLE_WITCH_MAGIC = new ParticleType("witchMagic", ePARTICLE_WITCH_MAGIC, false, 0);
const ParticleType* PARTICLE_DRIP_WATER = new ParticleType("dripWater", ePARTICLE_DRIP_WATER, false, 0);
const ParticleType* PARTICLE_DRIP_LAVA = new ParticleType("dripLava", ePARTICLE_DRIP_LAVA, false, 0);
const ParticleType* PARTICLE_ANGRY_VILLAGER = new ParticleType("angryVillager", ePARTICLE_ANGRY_VILLAGER, false, 0);
const ParticleType* PARTICLE_HAPPY_VILLAGER = new ParticleType("happyVillager", ePARTICLE_HAPPY_VILLAGER, false, 0);
const ParticleType* PARTICLE_TOWN_AURA = new ParticleType("townaura", ePARTICLE_TOWN_AURA, false, 0);
const ParticleType* PARTICLE_NOTE = new ParticleType("note", ePARTICLE_NOTE, false, 0);
const ParticleType* PARTICLE_PORTAL = new ParticleType("portal", ePARTICLE_PORTAL, false, 0);
const ParticleType* PARTICLE_END_PORTAL = new ParticleType("endportal", ePARTICLE_END_PORTAL, false, 0);
const ParticleType* PARTICLE_ENCHANTMENT_TABLE = new ParticleType("enchantmenttable", ePARTICLE_ENCHANTMENT_TABLE,
                                                                  false, 0);
const ParticleType* PARTICLE_ENDER = new ParticleType("ender", ePARTICLE_ENDER, false, 0);
const ParticleType* PARTICLE_FLAME = new ParticleType("flame", ePARTICLE_FLAME, false, 0);
const ParticleType* PARTICLE_LAVA = new ParticleType("lava", ePARTICLE_LAVA, false, 0);
const ParticleType* PARTICLE_FOOTSTEP = new ParticleType("footstep", ePARTICLE_FOOTSTEP, false, 0);
const ParticleType* PARTICLE_CLOUD = new ParticleType("cloud", ePARTICLE_CLOUD, false, 0);
const ParticleType* PARTICLE_RED_DUST = new ParticleType("reddust", ePARTICLE_RED_DUST, false, 0);
const ParticleType* PARTICLE_SNOWBALL_POOF = new ParticleType("snowballpoof", ePARTICLE_SNOWBALL_POOF, false, 0);
const ParticleType* PARTICLE_SNOW_SHOVEL = new ParticleType("snowshovel", ePARTICLE_SNOW_SHOVEL, false, 0);
const ParticleType* PARTICLE_SLIME = new ParticleType("slime", ePARTICLE_SLIME, false, 0);
const ParticleType* PARTICLE_HEART = new ParticleType("heart", ePARTICLE_HEART, false, 0);
const ParticleType* PARTICLE_BARRIER = new ParticleType("barrier", ePARTICLE_BARRIER, false, 0);
const ParticleType* PARTICLE_ICON_CRACK = new ParticleType("iconcrack", ePARTICLE_ICON_CRACK, false, 2);
const ParticleType* PARTICLE_BLOCK_CRACK = new ParticleType("blockcrack", ePARTICLE_BLOCK_CRACK, false, 1);
const ParticleType* PARTICLE_BLOCK_DUST = new ParticleType("blockdust", ePARTICLE_BLOCK_DUST, false, 1);
const ParticleType* PARTICLE_DROPLET = new ParticleType("droplet", ePARTICLE_DROPLET, false, 0);
const ParticleType* PARTICLE_TAKE = new ParticleType("take", ePARTICLE_TAKE, false, 0);
const ParticleType* PARTICLE_MOB_APPEARANCE = new ParticleType("mobappearance", ePARTICLE_MOB_APPEARANCE, true, 0);
const ParticleType* PARTICLE_DRAGON_BREATH = new ParticleType("dragonbreath", ePARTICLE_DRAGON_BREATH, false, 0);
const ParticleType* PARTICLE_END_ROD = new ParticleType("endRod", ePARTICLE_END_ROD, false, 0);
const ParticleType* PARTICLE_DAMAGE_INDICATOR = new ParticleType("damageIndicator", ePARTICLE_DAMAGE_INDICATOR, true,
                                                                 0);
const ParticleType* PARTICLE_SWEEP_ATTACK = new ParticleType("sweepAttack", ePARTICLE_SWEEP_ATTACK, true, 0);
const ParticleType* PARTICLE_FALLING_DUST = new ParticleType("fallingdust", ePARTICLE_FALLING_DUST, false, 1);
const ParticleType* PARTICLE_TOTEM = new ParticleType("totem", ePARTICLE_TOTEM, false, 0);
const ParticleType* PARTICLE_INK = new ParticleType("ink", ePARTICLE_INK, false, 0);
const ParticleType* PARTICLE_SPIT = new ParticleType("spit", ePARTICLE_SPIT, true, 0);
const ParticleType* PARTICLE_BUBBLE_POP = new ParticleType("bubble_pop", ePARTICLE_BUBBLE_POP, false, 0);
const ParticleType* PARTICLE_CURRENT_DOWN = new ParticleType("current_down", ePARTICLE_CURRENT_DOWN, false, 0);
const ParticleType* PARTICLE_CONDUIT = new ParticleType("conduit", ePARTICLE_CONDUIT, false, 0);
const ParticleType* PARTICLE_CHEST_REFILL = new ParticleType("chestrefill", ePARTICLE_CHEST_REFILL, false, 0);
const ParticleType* PARTICLE_WINNING_INSTANT_SPELL = new ParticleType("instantSpell", ePARTICLE_WINNING_INSTANT_SPELL,
                                                                      false, 0);

static boost::unordered_map<ePARTICLE_TYPE, const ParticleType*> sParticleTypes;

ePARTICLE_TYPE ParticleType::getId() const
{
    return this->id;
}

void ParticleType::staticCtor()
{
    sParticleTypes[PARTICLE_EXPLODE->getId()] = PARTICLE_EXPLODE;
    sParticleTypes[PARTICLE_LARGE_EXPLODE->getId()] = PARTICLE_LARGE_EXPLODE;
    sParticleTypes[PARTICLE_HUGE_EXPLOSION->getId()] = PARTICLE_HUGE_EXPLOSION;
    sParticleTypes[PARTICLE_FIREWORKS_SPARK->getId()] = PARTICLE_FIREWORKS_SPARK;
    sParticleTypes[PARTICLE_BUBBLE->getId()] = PARTICLE_BUBBLE;
    sParticleTypes[PARTICLE_SPLASH->getId()] = PARTICLE_SPLASH;
    sParticleTypes[PARTICLE_WAKE->getId()] = PARTICLE_WAKE;
    sParticleTypes[PARTICLE_SUSPENDED->getId()] = PARTICLE_SUSPENDED;
    sParticleTypes[PARTICLE_DEPTH_SUSPEND->getId()] = PARTICLE_DEPTH_SUSPEND;
    sParticleTypes[PARTICLE_CRIT->getId()] = PARTICLE_CRIT;
    sParticleTypes[PARTICLE_MAGIC_CRIT->getId()] = PARTICLE_MAGIC_CRIT;
    sParticleTypes[PARTICLE_SMOKE->getId()] = PARTICLE_SMOKE;
    sParticleTypes[PARTICLE_LARGE_SMOKE->getId()] = PARTICLE_LARGE_SMOKE;
    sParticleTypes[PARTICLE_SPELL->getId()] = PARTICLE_SPELL;
    sParticleTypes[PARTICLE_INSTANT_SPELL->getId()] = PARTICLE_INSTANT_SPELL;
    sParticleTypes[PARTICLE_MOB_SPELL->getId()] = PARTICLE_MOB_SPELL;
    sParticleTypes[PARTICLE_MOB_SPELL_AMBIENT->getId()] = PARTICLE_MOB_SPELL_AMBIENT;
    sParticleTypes[PARTICLE_WITCH_MAGIC->getId()] = PARTICLE_WITCH_MAGIC;
    sParticleTypes[PARTICLE_DRIP_WATER->getId()] = PARTICLE_DRIP_WATER;
    sParticleTypes[PARTICLE_DRIP_LAVA->getId()] = PARTICLE_DRIP_LAVA;
    sParticleTypes[PARTICLE_ANGRY_VILLAGER->getId()] = PARTICLE_ANGRY_VILLAGER;
    sParticleTypes[PARTICLE_HAPPY_VILLAGER->getId()] = PARTICLE_HAPPY_VILLAGER;
    sParticleTypes[PARTICLE_TOWN_AURA->getId()] = PARTICLE_TOWN_AURA;
    sParticleTypes[PARTICLE_NOTE->getId()] = PARTICLE_NOTE;
    sParticleTypes[PARTICLE_PORTAL->getId()] = PARTICLE_PORTAL;
    sParticleTypes[PARTICLE_ENCHANTMENT_TABLE->getId()] = PARTICLE_ENCHANTMENT_TABLE;
    sParticleTypes[PARTICLE_FLAME->getId()] = PARTICLE_FLAME;
    sParticleTypes[PARTICLE_LAVA->getId()] = PARTICLE_LAVA;
    sParticleTypes[PARTICLE_FOOTSTEP->getId()] = PARTICLE_FOOTSTEP;
    sParticleTypes[PARTICLE_CLOUD->getId()] = PARTICLE_CLOUD;
    sParticleTypes[PARTICLE_RED_DUST->getId()] = PARTICLE_RED_DUST;
    sParticleTypes[PARTICLE_SNOWBALL_POOF->getId()] = PARTICLE_SNOWBALL_POOF;
    sParticleTypes[PARTICLE_SNOW_SHOVEL->getId()] = PARTICLE_SNOW_SHOVEL;
    sParticleTypes[PARTICLE_SLIME->getId()] = PARTICLE_SLIME;
    sParticleTypes[PARTICLE_HEART->getId()] = PARTICLE_HEART;
    sParticleTypes[PARTICLE_BARRIER->getId()] = PARTICLE_BARRIER;
    sParticleTypes[PARTICLE_ICON_CRACK->getId()] = PARTICLE_ICON_CRACK;
    sParticleTypes[PARTICLE_BLOCK_CRACK->getId()] = PARTICLE_BLOCK_CRACK;
    sParticleTypes[PARTICLE_BLOCK_DUST->getId()] = PARTICLE_BLOCK_DUST;
    sParticleTypes[PARTICLE_DROPLET->getId()] = PARTICLE_DROPLET;
    sParticleTypes[PARTICLE_TAKE->getId()] = PARTICLE_TAKE;
    sParticleTypes[PARTICLE_MOB_APPEARANCE->getId()] = PARTICLE_MOB_APPEARANCE;
    sParticleTypes[PARTICLE_DRAGON_BREATH->getId()] = PARTICLE_DRAGON_BREATH;
    sParticleTypes[PARTICLE_END_ROD->getId()] = PARTICLE_END_ROD;
    sParticleTypes[PARTICLE_DAMAGE_INDICATOR->getId()] = PARTICLE_DAMAGE_INDICATOR;
    sParticleTypes[PARTICLE_SWEEP_ATTACK->getId()] = PARTICLE_SWEEP_ATTACK;
    sParticleTypes[PARTICLE_FALLING_DUST->getId()] = PARTICLE_FALLING_DUST;
    sParticleTypes[PARTICLE_TOTEM->getId()] = PARTICLE_TOTEM;
    sParticleTypes[PARTICLE_INK->getId()] = PARTICLE_INK;
    sParticleTypes[PARTICLE_SPIT->getId()] = PARTICLE_SPIT;
    sParticleTypes[PARTICLE_BUBBLE_POP->getId()] = PARTICLE_BUBBLE_POP;
    sParticleTypes[PARTICLE_CURRENT_DOWN->getId()] = PARTICLE_CURRENT_DOWN;
    sParticleTypes[PARTICLE_CONDUIT->getId()] = PARTICLE_CONDUIT;
    sParticleTypes[PARTICLE_CHEST_REFILL->getId()] = PARTICLE_CHEST_REFILL;
    sParticleTypes[PARTICLE_WINNING_INSTANT_SPELL->getId()] = PARTICLE_WINNING_INSTANT_SPELL;
}

ParticleType::ParticleType(const std::string& name, ePARTICLE_TYPE id, bool overrideLimiter, int paramCount)
{
    this->name = name;
    this->paramCount = paramCount;
    this->overrideLimiter = overrideLimiter;
    this->id = id;
}

int ParticleType::getParamCount() const
{
    return this->paramCount;
}

bool ParticleType::getOverrideLimiter() const
{
    return this->overrideLimiter;
}

const ParticleType* ParticleType::byId(ePARTICLE_TYPE id)
{
    return sParticleTypes[id];
}
