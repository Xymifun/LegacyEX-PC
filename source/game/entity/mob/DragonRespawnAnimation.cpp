#include "game/entity/mob/DragonRespawnAnimation.hpp"

#include "game/entity/mob/DragonRespawnAnimation_PREPARING_TO_SUMMON_PILLARS.hpp"
#include "game/entity/mob/DragonRespawnAnimation_START.hpp"
#include "game/entity/mob/DragonRespawnAnimation_SUMMONING_DRAGON.hpp"
#include "game/entity/mob/DragonRespawnAnimation_SUMMONING_PILLARS.hpp"

DragonRespawnAnimation* DragonRespawnAnimation::START = new DragonRespawnAnimation_START();
DragonRespawnAnimation* DragonRespawnAnimation::PREPARING_TO_SUMMON_PILLARS =
    new DragonRespawnAnimation_PREPARING_TO_SUMMON_PILLARS();
DragonRespawnAnimation* DragonRespawnAnimation::SUMMONING_PILLARS = new DragonRespawnAnimation_SUMMONING_PILLARS();
DragonRespawnAnimation* DragonRespawnAnimation::SUMMONING_DRAGON = new DragonRespawnAnimation_SUMMONING_DRAGON();
DragonRespawnAnimation* DragonRespawnAnimation::END = new DragonRespawnAnimation_SUMMONING_PILLARS();
