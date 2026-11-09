#ifndef GAME_ENTITY_MOB_ENDDRAGONFIGHT_HPP
#define GAME_ENTITY_MOB_ENDDRAGONFIGHT_HPP

#include <string>
#include <vector>

#include <boost/shared_ptr.hpp>

#include "game/core/DamageSource.hpp"
#include "game/core/EnderCrystal.hpp"
#include "game/data/CompoundTag.hpp"
#include "game/entity/mob/EnderDragon.hpp"
#include "game/server/ServerBossEvent.hpp"
#include "game/server/ServerLevel.hpp"
#include "game/world/block/BlockPattern.hpp"
#include "game/world/block/BlockPos.hpp"
#include "game/world/block/Predicate.hpp"

class DragonRespawnAnimation;
class Player;

class EndDragonFight
{
public:
    EndDragonFight(ServerLevel* level, CompoundTag* data);
    ~EndDragonFight();

    int getCrystalsAlive();
    void resetSpikeCrystals();
    void spawnExitPortal(bool active);
    void updateCrystalCount();
    void onCrystalDestroyed(boost::shared_ptr<EnderCrystal> crystal, const DamageSource* source);
    BlockPattern::BlockPatternMatch* findExitPortal();
    void fjFindCrystals(std::vector<boost::shared_ptr<EnderCrystal> >& crystals, const BlockPos& pos);
    void respawnDragon(const std::vector<boost::shared_ptr<EnderCrystal> >& crystals);
    void tryRespawn();
    CompoundTag* saveData();
    static BlockPos getGatewayPosition(int index);
    bool hasExitPortal();
    void updatePlayers();
    void ensureArenaLoaded();
    void createNewDragon();
    void tick();
    void setRespawnStage(DragonRespawnAnimation* stage);
    void spawnNewGateway(const BlockPos& pos, int index);
    void spawnNewGateway();
    void setDragonKilled(boost::shared_ptr<EnderDragon> dragon);
    void updateDragon(boost::shared_ptr<EnderDragon> dragon);
    bool hasPreviouslyKilledDragon();

    static Predicate<boost::shared_ptr<Player> >* VALID_PLAYER;

private:
    ServerBossEvent* dragonEvent;
    ServerLevel* level;
    std::vector<int> gateways;
    BlockPattern* exitPortalPattern;
    int ticksSinceDragonSeen;
    int crystalsAlive;
    int ticksSinceCrystalsScanned;
    int ticksSinceLastPlayerScan;
    bool dragonKilled;
    bool previouslyKilledDragon;
    std::wstring dragonUUID;
    bool needsStateScanning;
    BlockPos* exitPortalLocation;
    DragonRespawnAnimation* respawnStage;
    int respawnTime;
    std::vector<boost::shared_ptr<EnderCrystal> > respawnCrystals;
};

#endif
