#ifndef GAME_CORE_GHOSTGAMEMODE_HPP
#define GAME_CORE_GHOSTGAMEMODE_HPP

#include <boost/shared_ptr.hpp>

#include "game/core/GameMode.hpp"

class BlockPos;
class Direction;
class GhostPlayer;
class GameType;
class Level;
class MiniGameDef;
class Player;

class GhostGameMode : public GameMode
{
public:
    GhostGameMode(GhostPlayer* player);
    virtual ~GhostGameMode();

    virtual bool startDestroyBlock(const BlockPos& pos, const Direction* direction);
    virtual void tick();
    virtual InteractionResult::EInteractionResult useItem(boost::shared_ptr<Player> player, Level* level,
                                                          InteractionHand::EInteractionHand hand, bool simulate);
    virtual void SetGameType(const GameType* gameType);
    virtual const GameType* GetGameType();
    virtual const MiniGameDef* GetMiniGame();
    virtual boost::shared_ptr<Player> GetPlayer();
    virtual Level* GetLevel();
    virtual bool RecordLastCheckpoint(int checkpoint, int lap, bool first, bool second, bool third);
    virtual bool RecordLapCompleted();
    virtual bool RecordProgress(double first, double second, double third, double fourth);

private:
    GhostPlayer* mGhostPlayer;
};

#endif
