#ifndef GAME_MINIGAME_GAMEMEDAL_HPP
#define GAME_MINIGAME_GAMEMEDAL_HPP

#include <vector>

#include <boost/shared_ptr.hpp>

#include "game/world/generation/PlayerStats.hpp"

class GameStats;
class ServerPlayer;
class StatsUID;

class GameMedal
{
public:
    enum EOperator
    {
        OPERATOR_AT_LEAST,
        OPERATOR_AT_MOST,
        OPERATOR_EQUAL
    };

    class Condition
    {
    public:
        Condition(float target, EOperator op, bool bestOnly);

        virtual ~Condition() {}

        virtual int findBest(const GameMedal* medal, const std::vector<boost::shared_ptr<ServerPlayer> >* players,
                             GameStats* gameStats) const;
        virtual bool canAward(const GameMedal* medal, const StatsUID& uid, GameStats* gameStats) const;
        virtual float getScore(const GameMedal* medal, PlayerStats* playerStats) const;

        float mTarget;
        bool mBestOnly;
        EOperator mOperator;
    };

    GameMedal(int name, unsigned int weight);
    GameMedal(int name, float target, unsigned int weight, EOperator op, bool bestOnly = false,
              bool canMultiAward = false);

    virtual ~GameMedal();

    virtual bool CanAward(const StatsUID& uid, GameStats* gameStats) const;
    virtual int GetName() const;
    virtual float GetScore(PlayerStats* playerStats) const;
    virtual int FindBest(const std::vector<boost::shared_ptr<ServerPlayer> >* players, GameStats* gameStats) const;

    static bool defaultCanAward(const StatsUID& uid, GameStats* gameStats, float score, float target, EOperator op);
    static float totalOfAllRounds(PlayerStats* playerStats, int (&getRoundValue)(const PlayerStats::RoundStats&));

    bool BestOnly() const;
    unsigned int GetWeight() const;
    void SetCanMultiAward(bool canMultiAward);
    bool CanMultiAward() const;
    void SetId(int id);
    int GetId() const;
    GameStats* getGameStats() const;

protected:
    int mName;
    unsigned int mWeight;
    int mId;
    bool mCanMultiAward;
    std::vector<Condition*> mConditions;
};

#endif
