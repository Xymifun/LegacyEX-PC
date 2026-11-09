#ifndef GAME_ITEM_POWERUPITEMS_HPP
#define GAME_ITEM_POWERUPITEMS_HPP

#include <boost/shared_ptr.hpp>

class Player;

class PowerupItems
{
public:
    enum eGlide_Timed_Powerup_ID
    {
        eGlide_Timed_Powerup_ID_SlowFall = 0,
        eGlide_Timed_Powerup_ID_InvertControls = 1,
        eGlide_Timed_Powerup_ID_Invulnerable = 2
    };

    static void DoLargeBoost(boost::shared_ptr<Player> player);
    static void DoSmallBoost(boost::shared_ptr<Player> player);
    static void DoDropTNT(boost::shared_ptr<Player> player);
    static void DoGainPoint(boost::shared_ptr<Player> player);
    static bool IsDebuff(eGlide_Timed_Powerup_ID powerupId);
    static void BroadcastTimedPowerup(boost::shared_ptr<Player> player, eGlide_Timed_Powerup_ID powerupId, int time);
    static void ActivateTimedPowerup(boost::shared_ptr<Player> player, eGlide_Timed_Powerup_ID powerupId, int time);
    static void DoTimedPowerup(boost::shared_ptr<Player> player, eGlide_Timed_Powerup_ID powerupId, int time);
    static void DoSetInvulnerable(boost::shared_ptr<Player> player, int time);
    static void UsePowerupItem(boost::shared_ptr<Player> player, int itemId);
};

#endif
