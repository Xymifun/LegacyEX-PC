#ifndef GAME_UI_TOOLTIPS_HPP
#define GAME_UI_TOOLTIPS_HPP

#include <boost/shared_ptr.hpp>

#include "game/core/IUIController.hpp"
#include "game/core/InteractionResult.hpp"

class HitResult;
class MultiPlayerLevel;
class MultiplayerLocalPlayer;

class Tooltips
{
public:
    static void staticCtor();
    static void OverwriteTooltip(int playerId, EControllerActions action, int tooltip);
    static void ResetTooltip(int playerId, EControllerActions action);
    static void SetInteractTooltips(boost::shared_ptr<MultiplayerLocalPlayer> player, MultiPlayerLevel* level,
                                    const HitResult* hitResult,
                                    InteractionResult::EInteractionResult interactionResult);
    static void ResetAllTooltips(int playerId);
    static void ForceTooltipReload(int playerId);
    static void ForceAllPlayerTooltipReset();
    static void DisplayTooltips(int playerId);
};

#endif
