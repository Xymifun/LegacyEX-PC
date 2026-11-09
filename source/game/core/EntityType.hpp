#ifndef GAME_CORE_ENTITYTYPE_HPP
#define GAME_CORE_ENTITYTYPE_HPP

namespace EntityType
{
const unsigned long long CONCRETE_TYPE_MASK = 0x3F;
const unsigned long long ENTITY = 0x08000000ULL;
const unsigned long long LIVING_ENTITY = ENTITY | 0x4000000;
const unsigned long long MOB = LIVING_ENTITY | 0x1000000;
const unsigned long long PATHFINDER_MOB = MOB | 0x400000;
const unsigned long long AGABLE_MOB = PATHFINDER_MOB | 0x200000;
const unsigned long long ANIMAL = AGABLE_MOB | 0x80000;
const unsigned long long ABSTRACT_HORSE = ANIMAL | 0x10000;
const unsigned long long ABSTRACT_CHESTED_HORSE = ABSTRACT_HORSE | 0x8000;
const unsigned long long ENEMY = 0x40000000ULL;
const unsigned long long MONSTER = PATHFINDER_MOB | ENEMY | 0x140000;
const unsigned long long ABSTRACT_ILLAGER = MONSTER | 0x1040;
const unsigned long long ABSTRACT_SKELETON = MONSTER | 0x440;
const unsigned long long AGABLE_WATER_ANIMAL = AGABLE_MOB | 0x2000;
const unsigned long long AMBIENT_CREATURE = MOB | 0x100;
const unsigned long long AREA_EFFECT_CLOUD = ENTITY | 0xF;
const unsigned long long ARMOR_STAND = LIVING_ENTITY | 0x1;
const unsigned long long PROJECTILE = 0x80000000ULL;
const unsigned long long ARROW = ENTITY | PROJECTILE | 0x2020000;
const unsigned long long BAT = AMBIENT_CREATURE | 0x10000001;
const unsigned long long BLAZE = MONSTER | 0x10000045;
const unsigned long long BOAT = ENTITY | 0x8;
const unsigned long long SPIDER = MONSTER | 0x10020040;
const unsigned long long CAVE_SPIDER = SPIDER | 0x1;
const unsigned long long CHICKEN = ANIMAL | 0x4;
const unsigned long long CREATURE_CATEGORY = 0x20000000ULL;
const unsigned long long COW = ANIMAL | CREATURE_CATEGORY | 0x20000;
const unsigned long long CREEPER = MONSTER | 0x10000041;
const unsigned long long DELAYED_RELEASE = ENTITY | 0x7;
const unsigned long long DOLPHIN = AGABLE_WATER_ANIMAL | 0x1;
const unsigned long long DONKEY = ABSTRACT_CHESTED_HORSE | CREATURE_CATEGORY | 0x1;
const unsigned long long FIREBALL = ENTITY | PROJECTILE | 0x2400000;
const unsigned long long DRAGON_FIREBALL = FIREBALL | 0x1;
const unsigned long long ZOMBIE = MONSTER | 0x10010040;
const unsigned long long DROWNED = ZOMBIE | 0x4;
const unsigned long long GUARDIAN = MONSTER | 0x10004000;
const unsigned long long ELDER_GUARDIAN = GUARDIAN | 0x1;
const unsigned long long ENDERMITE = MONSTER | 0x4B;
const unsigned long long ENDER_CRYSTAL = ENTITY | 0xE;
const unsigned long long ENDER_DRAGON = MOB | 0x5;
const unsigned long long ENDER_MAN = MONSTER | 0x10000043;
const unsigned long long EVOCATION_FANGS = ENTITY | 0x11;
const unsigned long long SPELLCASTER_ILLAGER = ABSTRACT_ILLAGER | 0x800;
const unsigned long long EVOCATION_ILLAGER = SPELLCASTER_ILLAGER | 0x10000001;
const unsigned long long EXPERIENCE_ORB = ENTITY | 0x3;
const unsigned long long EYE_OF_ENDER_SIGNAL = ENTITY | PROJECTILE | 0x4;
const unsigned long long FALLING_BLOCK_ENTITY = ENTITY | 0x9;
const unsigned long long FIREWORKS_ROCKET_ENTITY = ENTITY | PROJECTILE | 0x5;
const unsigned long long WATER_ANIMAL = PATHFINDER_MOB | 0x102000;
const unsigned long long FISH = WATER_ANIMAL | 0x2;
const unsigned long long FISHING_HOOK = ENTITY | PROJECTILE | 0x6;
const unsigned long long FLYING_MOB = MOB | 0x80;
const unsigned long long GHAST = FLYING_MOB | ENEMY | 0x10000041;
const unsigned long long PLAYER = LIVING_ENTITY | 0x800000;
const unsigned long long GHOST_PLAYER = PLAYER | 0x4;
const unsigned long long GIANT = MONSTER | 0x10000042;
const unsigned long long GOLEM = PATHFINDER_MOB | 0x180000;
const unsigned long long HANGING_ENTITY = ENTITY | 0x2080000;
const unsigned long long HORSE = ABSTRACT_HORSE | CREATURE_CATEGORY | 0x1;
const unsigned long long HUSK = ZOMBIE | 0x3;
const unsigned long long ILLUSION_ILLAGER = SPELLCASTER_ILLAGER | 0x10000002;
const unsigned long long ITEM_ENTITY = ENTITY | 0xA;
const unsigned long long ITEM_FRAME = HANGING_ENTITY | 0x2;
const unsigned long long LARGE_FIREBALL = FIREBALL | 0x3;
const unsigned long long LEASH_FENCE_KNOT_ENTITY = HANGING_ENTITY | 0x3;
const unsigned long long LIGHTNING_BOLT = ENTITY | 0x3000001;
const unsigned long long LLAMA = ABSTRACT_CHESTED_HORSE | CREATURE_CATEGORY | 0x3;
const unsigned long long LLAMA_SPIT = ENTITY | PROJECTILE | 0x12;
const unsigned long long LOCAL_PLAYER = PLAYER | 0x3;
const unsigned long long SLIME = MOB | ENEMY | 0x10108000;
const unsigned long long MAGMA_CUBE = SLIME | 0x1;
const unsigned long long MINECART = ENTITY | 0x2800000;
const unsigned long long MINECART_CHEST = MINECART | 0x40002;
const unsigned long long MINECART_COMMAND_BLOCK = MINECART | 0x5;
const unsigned long long MINECART_FURNACE = MINECART | 0x3;
const unsigned long long MINECART_HOPPER = MINECART | 0x40005;
const unsigned long long MINECART_RIDEABLE = MINECART | 0x1;
const unsigned long long MINECART_SPAWNER = MINECART | 0x6;
const unsigned long long MINECART_TNT = MINECART | 0x4;
const unsigned long long MULE = ABSTRACT_CHESTED_HORSE | CREATURE_CATEGORY | 0x2;
const unsigned long long MULTI_ENTITY_MOB_PART = ENTITY | 0xC;
const unsigned long long MUSHROOM_COW = COW | 0x1;
const unsigned long long TAMABLE_ANIMAL = ANIMAL | 0x40000;
const unsigned long long OCELOT = TAMABLE_ANIMAL | CREATURE_CATEGORY | 0x1;
const unsigned long long PAINTING = HANGING_ENTITY | 0x1;
const unsigned long long PARROT = TAMABLE_ANIMAL | CREATURE_CATEGORY | 0x3;
const unsigned long long PHANTOM = MONSTER | 0x10000049;
const unsigned long long PIG = ANIMAL | CREATURE_CATEGORY | 0x3;
const unsigned long long PIG_ZOMBIE = ZOMBIE | 0x1;
const unsigned long long POLAR_BEAR = ANIMAL | CREATURE_CATEGORY | 0x6;
const unsigned long long PRIMED_TNT = ENTITY | 0xB;
const unsigned long long PUFFERFISH = WATER_ANIMAL | 0x3;
const unsigned long long RABBIT = ANIMAL | CREATURE_CATEGORY | 0x5;
const unsigned long long REMOTE_PLAYER = PLAYER | 0x2;
const unsigned long long SALMON = WATER_ANIMAL | 0x4;
const unsigned long long SERVER_PLAYER = PLAYER | 0x1;
const unsigned long long SHEEP = ANIMAL | CREATURE_CATEGORY | 0x2;
const unsigned long long SHULKER = GOLEM | ENEMY | 0x43;
const unsigned long long SHULKER_BULLET = ENTITY | 0x10;
const unsigned long long SILVERFISH = MONSTER | 0x10000044;
const unsigned long long SKELETON = ABSTRACT_SKELETON | 0x10000001;
const unsigned long long SKELETON_HORSE = ABSTRACT_HORSE | CREATURE_CATEGORY | 0x2;
const unsigned long long SMALL_FIREBALL = FIREBALL | 0x4;
const unsigned long long THROWABLE_PROJECTILE = ENTITY | PROJECTILE | 0x2200000;
const unsigned long long SNOWBALL = THROWABLE_PROJECTILE | 0x1;
const unsigned long long SNOW_MAN = GOLEM | CREATURE_CATEGORY | 0x1;
const unsigned long long SPECTRAL_ARROW = ARROW | 0x1;
const unsigned long long SQUID = WATER_ANIMAL | 0x1;
const unsigned long long STRAY = ABSTRACT_SKELETON | 0x10000003;
const unsigned long long THROWN_EGG = THROWABLE_PROJECTILE | 0x2;
const unsigned long long THROWN_ENDERPEARL = THROWABLE_PROJECTILE | 0x3;
const unsigned long long THROWN_EXP_BOTTLE = THROWABLE_PROJECTILE | 0x5;
const unsigned long long THROWN_POTION = THROWABLE_PROJECTILE | 0x4;
const unsigned long long THROWN_TRIDENT = THROWABLE_PROJECTILE | 0x6;
const unsigned long long TIPPABLE_ARROW = ARROW | 0x2;
const unsigned long long TROPICAL_FISH = WATER_ANIMAL | 0x5;
const unsigned long long TURTLE = ANIMAL | CREATURE_CATEGORY | 0x7;
const unsigned long long VEX = MONSTER | 0x10000048;
const unsigned long long VILLAGER = AGABLE_MOB | 0x1;
const unsigned long long VILLAGER_GOLEM = GOLEM | 0x2;
const unsigned long long VINDICATION_ILLAGER = ABSTRACT_ILLAGER | 0x10000001;
const unsigned long long WITCH = MONSTER | 0x10000046;
const unsigned long long WITHER_BOSS = MONSTER | 0x10000047;
const unsigned long long WITHER_SKELETON = ABSTRACT_SKELETON | 0x10000002;
const unsigned long long WITHER_SKULL = FIREBALL | 0x2;
const unsigned long long WOLF = TAMABLE_ANIMAL | 0x2;
const unsigned long long ZOMBIE_HORSE = ABSTRACT_HORSE | CREATURE_CATEGORY | 0x3;
const unsigned long long ZOMBIE_VILLAGER = ZOMBIE | 0x2;
}

namespace BlockEntityType
{
const unsigned long long BLOCK_ENTITY = 0x01000200ULL;
const unsigned long long BANNER = BLOCK_ENTITY | 0xC;
const unsigned long long CONTAINER = BLOCK_ENTITY | 0x800000;
const unsigned long long LOCKABLE_CONTAINER = CONTAINER | 0x400000;
const unsigned long long TICKABLE = BLOCK_ENTITY | 0x200000;
const unsigned long long BEACON = LOCKABLE_CONTAINER | TICKABLE | 0x4;
const unsigned long long BED = BLOCK_ENTITY | 0x10;
const unsigned long long BREWING_STAND = LOCKABLE_CONTAINER | TICKABLE | 0x3;
const unsigned long long CAULDRON = BLOCK_ENTITY | 0x11;
const unsigned long long CHEST = LOCKABLE_CONTAINER | TICKABLE | 0x1;
const unsigned long long COMMAND = BLOCK_ENTITY | 0x9;
const unsigned long long COMPARATOR = BLOCK_ENTITY | 0xA;
const unsigned long long CONDUIT = TICKABLE | 0x12;
const unsigned long long DAYLIGHT_DETECTOR = TICKABLE | 0xB;
const unsigned long long DISPENSER = LOCKABLE_CONTAINER | 0x2000000;
const unsigned long long DROPPER = DISPENSER | 0x1;
const unsigned long long ENCHANTMENT_TABLE_ENTITY = TICKABLE | 0x6;
const unsigned long long ENDER_CHEST = TICKABLE | 0x8;
const unsigned long long FLOWER_POT = BLOCK_ENTITY | 0xD;
const unsigned long long FURNACE = LOCKABLE_CONTAINER | TICKABLE | 0x2;
const unsigned long long HOPPER = LOCKABLE_CONTAINER | TICKABLE | 0x5;
const unsigned long long JUKEBOX = BLOCK_ENTITY | 0x4;
const unsigned long long MOB_SPAWNER = TICKABLE | 0x1;
const unsigned long long MUSIC = BLOCK_ENTITY | 0x3;
const unsigned long long PISTON_MOVING = TICKABLE | 0x5;
const unsigned long long SHULKER_BOX = TICKABLE | 0xF;
const unsigned long long SIGN = BLOCK_ENTITY | 0x2;
const unsigned long long SKULL = TICKABLE | 0x7;
const unsigned long long THE_END_PORTAL = TICKABLE | 0x100000;
const unsigned long long THE_END_GATEWAY_ENTITY = THE_END_PORTAL | 0x1;
}

namespace ParticleKind
{
const unsigned long long PARTICLE = 0x00080200ULL;
const unsigned long long ATTACK_SWEEP = PARTICLE | 0x28;
const unsigned long long BARRIER = PARTICLE | 0x23;
const unsigned long long BREAKING_ITEM = PARTICLE | 0x2;
const unsigned long long BUBBLE = PARTICLE | 0x3;
const unsigned long long BUBBLE_POP = PARTICLE | 0x2D;
const unsigned long long CHEST_REFILL = PARTICLE | 0x1;
const unsigned long long CONDUIT = PARTICLE | 0x2F;
const unsigned long long CRIT = PARTICLE | 0x14;
const unsigned long long DRAGON_BREATH = PARTICLE | 0x1E;
const unsigned long long DRIP = PARTICLE | 0x1B;
const unsigned long long ENCHANTMENT_TABLE = PARTICLE | 0x1C;
const unsigned long long ENDER = PARTICLE | 0x1F;
const unsigned long long END_ROD = PARTICLE | 0x29;
const unsigned long long EXPLODE = PARTICLE | 0x4;
const unsigned long long FALLING_DUST = PARTICLE | 0x2A;
const unsigned long long FIREWORKS_OVERLAY = PARTICLE | 0x22;
const unsigned long long FIREWORKS_SPARK = PARTICLE | 0x21;
const unsigned long long FIREWORKS_STARTER = PARTICLE | 0x20;
const unsigned long long FLAME = PARTICLE | 0x5;
const unsigned long long FOOTSTEP = PARTICLE | 0x6;
const unsigned long long HEART = PARTICLE | 0x7;
const unsigned long long HUGE_EXPLOSION = PARTICLE | 0x16;
const unsigned long long HUGE_EXPLOSION_SEED = PARTICLE | 0x17;
const unsigned long long INK = PARTICLE | 0x3;
const unsigned long long ITEM_PICKUP = PARTICLE | 0x25;
const unsigned long long LARGE_SMOKE = PARTICLE | 0x26;
const unsigned long long LAVA = PARTICLE | 0x8;
const unsigned long long MOB_APPEARANCE = PARTICLE | 0x27;
const unsigned long long NETHER_PORTAL = PARTICLE | 0xA;
const unsigned long long NOTE = PARTICLE | 0x9;
const unsigned long long PLAYER_CLOUD = PARTICLE | 0x18;
const unsigned long long RED_DUST = PARTICLE | 0xB;
const unsigned long long SMOKE = PARTICLE | 0xC;
const unsigned long long SNOW_SHOVEL = PARTICLE | 0xD;
const unsigned long long SPELL = PARTICLE | 0x1D;
const unsigned long long SPIT = PARTICLE | 0x2C;
const unsigned long long SPLASH = PARTICLE | 0xE;
const unsigned long long SUSPENDED = PARTICLE | 0x19;
const unsigned long long SUSPENDED_TOWN = PARTICLE | 0x1A;
const unsigned long long TERRAIN = PARTICLE | 0x10;
const unsigned long long TERRAIN_DUST = PARTICLE | 0x11;
const unsigned long long TOTEM = PARTICLE | 0x2B;
const unsigned long long TRACKING_EMITTER = PARTICLE | 0x24;
const unsigned long long WAKE = PARTICLE | 0x13;
const unsigned long long WATER_CURRENT_DOWN = PARTICLE | 0x2E;
const unsigned long long WATER_DROP = PARTICLE | 0x12;
}

static inline bool eTYPE_DERIVED_FROM(unsigned long long base, unsigned long long type)
{
    if ((base & EntityType::CONCRETE_TYPE_MASK) != 0)
        return base == type;
    else
        return (base & type) == base;
}

#endif
